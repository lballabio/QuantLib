/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017, 2018 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file andreasenhugelocalvolatility.cpp
*/

#include <ql/timegrid.hpp>
#include <ql/exercise.hpp>
#include <ql/utilities/null.hpp>
#include <ql/math/array.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/andreasenhugevolatilityinterpl.hpp>

#include <boost/math/special_functions/fpclassify.hpp>

#include <limits>

namespace QuantLib {

    namespace {

        struct close_enough_to {
            Real y;
            Size n;
            explicit close_enough_to(Real y, Size n=42) : y(y), n(n) {}
            bool operator()(Real x) const { return close_enough(x, y, n); }
        };

    }

    class AndreasenHugeCostFunction : public CostFunction {
      public:
        AndreasenHugeCostFunction(
            const Array& marketNPVs,
            const Array& marketVegas,
            const Array& lnMarketStrikes,
            const Array& previousNPVs,
            const ext::shared_ptr<FdmMesherComposite>& mesher,
            Time dT,
            AndreasenHugeVolatilityInterpl::InterpolationType interpolationType)
        : marketNPVs_(marketNPVs), marketVegas_(marketVegas), lnMarketStrikes_(lnMarketStrikes),
          previousNPVs_(previousNPVs), mesher_(mesher), nGridPoints_(mesher->layout()->size()),
          dT_(dT), interpolationType_((lnMarketStrikes_.size() > 1) ?
                                          interpolationType :
                                          AndreasenHugeVolatilityInterpl::PiecewiseConstant),
          dxMap_(FirstDerivativeOp(0, mesher_)), dxxMap_(SecondDerivativeOp(0, mesher_)),
          d2CdK2_(dxMap_.mult(Array(mesher->layout()->size(), -1.0)).add(dxxMap_)),
          mapT_(0, mesher_) {}

        Disposable<Array> d2CdK2(const Array& c) const {
            return d2CdK2_.apply(c);
        }

        Disposable<Array> solveFor(
            Time dT, const Array& sig, const Array& b) const {

            Array x(lnMarketStrikes_.size());
            Interpolation sigInterpl;

            switch (interpolationType_) {
              case AndreasenHugeVolatilityInterpl::CubicSpline:
                sigInterpl = CubicNaturalSpline(
                    lnMarketStrikes_.begin(), lnMarketStrikes_.end(),
                    sig.begin());
                break;
              case AndreasenHugeVolatilityInterpl::Linear:
                sigInterpl = LinearInterpolation(
                    lnMarketStrikes_.begin(), lnMarketStrikes_.end(),
                    sig.begin());
                break;
              case AndreasenHugeVolatilityInterpl::PiecewiseConstant:
                for (Size i=0; i < x.size()-1; ++i)
                    x[i] = 0.5*(lnMarketStrikes_[i] + lnMarketStrikes_[i+1]);
                x.back() = lnMarketStrikes_.back();

                sigInterpl = BackwardFlatInterpolation(
                    x.begin(), x.end(), sig.begin());
                break;
              default:
                QL_FAIL("unknown interpolation type");
            }

            const ext::shared_ptr<FdmLinearOpLayout> layout =
                mesher_->layout();
            const FdmLinearOpIterator endIter = layout->end();

            Array z(layout->size());
            for (FdmLinearOpIterator iter = layout->begin();
                 iter!=endIter; ++iter) {
                const Size i = iter.index();
                const Real lnStrike = mesher_->location(iter, 0);

                const Real vol = sigInterpl(
                    std::min(std::max(lnStrike, lnMarketStrikes_.front()),
                            lnMarketStrikes_.back()), true);

                z[i] = 0.5*vol*vol;
            }

            mapT_.axpyb(z, dxMap_, dxxMap_.mult(-z), Array());
            return mapT_.mult(Array(z.size(), dT)).solve_splitting(b, 1.0);

        }

        Disposable<Array> apply(const Array& c) const {
            return -mapT_.apply(c);
        }

        Disposable<Array> values(const Array& sig) const {
            Array newNPVs = solveFor(dT_, sig, previousNPVs_);

            const std::vector<Real>& gridPoints =
                mesher_->getFdm1dMeshers().front()->locations();

            const MonotonicCubicNaturalSpline interpl(
                gridPoints.begin(), gridPoints.end(), newNPVs.begin());

            Array retVal(lnMarketStrikes_.size());
            for (Size i=0; i < retVal.size(); ++i) {
                const Real strike = lnMarketStrikes_[i];
                retVal[i] = interpl(strike) - marketNPVs_[i];
            }
            return retVal;
        }

        Disposable<Array> vegaCalibrationError(const Array& sig) const {
            return values(sig)/marketVegas_;
        }

        Disposable<Array> initialValues() const {
            Array retVal(lnMarketStrikes_.size(), 0.25);
            return retVal;
        }


      private:
        const Array marketNPVs_, marketVegas_;
        const Array lnMarketStrikes_, previousNPVs_;
        const ext::shared_ptr<FdmMesherComposite> mesher_;
        const Size nGridPoints_;
        const Time dT_;
        const AndreasenHugeVolatilityInterpl::InterpolationType
            interpolationType_;

        const FirstDerivativeOp  dxMap_;
        const TripleBandLinearOp dxxMap_;
        const TripleBandLinearOp d2CdK2_;
        mutable TripleBandLinearOp mapT_;
    };

    class CombinedCostFunction : public CostFunction {
      public:
        CombinedCostFunction(
            const ext::shared_ptr<AndreasenHugeCostFunction>& putCostFct,
            const ext::shared_ptr<AndreasenHugeCostFunction>& callCostFct)
      : putCostFct_(putCostFct),
        callCostFct_(callCostFct) { }

        Disposable<Array> values(const Array& sig) const {
            if ((putCostFct_ != 0) && (callCostFct_ != 0)) {
                const Array pv = putCostFct_->values(sig);
                const Array cv = callCostFct_->values(sig);

                Array retVal(pv.size() + cv.size());
                std::copy(pv.begin(), pv.end(), retVal.begin());
                std::copy(cv.begin(), cv.end(), retVal.begin() + cv.size());

                return retVal;
            } else if (putCostFct_ != 0)
                return putCostFct_->values(sig);
            else if (callCostFct_ != 0)
                return callCostFct_->values(sig);
            else
                QL_FAIL("internal error: cost function not set");
        }

        Disposable<Array> initialValues() const {
            if ((putCostFct_ != 0) && (callCostFct_ != 0))
                return 0.5*(  putCostFct_->initialValues()
                            + callCostFct_->initialValues());
            else if (putCostFct_ != 0)
                return putCostFct_->initialValues();
            else if (callCostFct_ != 0)
                return callCostFct_->initialValues();
            else
                QL_FAIL("internal error: cost function not set");
        }

      private:
        const ext::shared_ptr<AndreasenHugeCostFunction> putCostFct_;
        const ext::shared_ptr<AndreasenHugeCostFunction> callCostFct_;
    };



    AndreasenHugeVolatilityInterpl::AndreasenHugeVolatilityInterpl(
        const CalibrationSet& calibrationSet,
        const Handle<Quote>& spot,
        const Handle<YieldTermStructure>& rTS,
        const Handle<YieldTermStructure>& qTS,
        InterpolationType interplationType,
        CalibrationType calibrationType,
        Size nGridPoints,
        Real _minStrike,
        Real _maxStrike,
        const ext::shared_ptr<OptimizationMethod>& optimizationMethod,
        const EndCriteria& endCriteria)
    : spot_(spot),
      rTS_(rTS),
      qTS_(qTS),
      interpolationType_(interplationType),
      calibrationType_(calibrationType),
      nGridPoints_(nGridPoints),
      minStrike_(_minStrike),
      maxStrike_(_maxStrike),
      optimizationMethod_(optimizationMethod),
      endCriteria_(endCriteria) {
        QL_REQUIRE(nGridPoints > 2 && !calibrationSet.empty(), "undefined grid or calibration set");

        std::set<Real> strikes;
        std::set<Date> expiries;

        calibrationSet_.reserve(calibrationSet.size());
        for (Size i=0; i < calibrationSet.size(); ++i) {

            const ext::shared_ptr<Exercise> exercise =
                calibrationSet[i].first->exercise();

            QL_REQUIRE(exercise->type() == Exercise::European,
                    "European option required");

            const Date expiry = exercise->lastDate();
            expiries.insert(expiry);

            const ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                    calibrationSet[i].first->payoff());

            QL_REQUIRE(payoff, "plain vanilla payoff required");

            const Real strike = payoff->strike();
            strikes.insert(strike);

            calibrationSet_.push_back(
                std::make_pair(
                    ext::make_shared<VanillaOption>(payoff, exercise),
                    calibrationSet[i].second)
            );

            registerWith(calibrationSet[i].second);
        }

        strikes_.assign(strikes.begin(), strikes.end());
        expiries_.assign(expiries.begin(), expiries.end());

        dT_.resize(expiries_.size());
        expiryTimes_.resize(expiries_.size());

        calibrationMatrix_ = std::vector< std::vector<Size> >(
            expiries.size(), std::vector<Size>(strikes.size(), Null<Size>()));

        for (Size i=0; i < calibrationSet.size(); ++i) {
            const Date expiry =
                calibrationSet[i].first->exercise()->lastDate();

            const Size l = std::distance(expiries.begin(), expiries.lower_bound(expiry));

            const Real strike =
                ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                    calibrationSet[i].first->payoff())->strike();

            const Size k = std::distance(strikes_.begin(),
                std::find_if(strikes_.begin(), strikes_.end(),
                             close_enough_to(strike)));

            calibrationMatrix_[l][k] = i;
        }

        registerWith(spot_);
        registerWith(rTS_);
        registerWith(qTS_);
    }

    ext::shared_ptr<AndreasenHugeCostFunction>
        AndreasenHugeVolatilityInterpl::buildCostFunction(
        Size iExpiry, Option::Type optionType,
        const Array& previousNPVs) const {

        if (calibrationType_ != CallPut
            && (   (calibrationType_ == Call && optionType ==Option::Put)
                || (calibrationType_ == Put  && optionType ==Option::Call)))
            return ext::shared_ptr<AndreasenHugeCostFunction>();

        const Time expiryTime = expiryTimes_[iExpiry];

        const DiscountFactor discount = rTS_->discount(expiryTime);
        const Real fwd = spot_->value()*qTS_->discount(expiryTime)/discount;

        const Size nOptions = std::count_if(
            calibrationMatrix_[iExpiry].begin(),
            calibrationMatrix_[iExpiry].end(),
            not_null<Size>());

        Array lnMarketStrikes(nOptions),
            marketNPVs(nOptions), marketVegas(nOptions);

        // calculate undiscounted market prices
        for (Size j=0, k=0; j < strikes_.size(); ++j) {
            const Size idx = calibrationMatrix_[iExpiry][j];

            if (idx != Null<Size>()) {

                const Volatility vol = calibrationSet_[idx].second->value();
                const Real stdDev = vol*std::sqrt(expiryTime);

                const BlackCalculator calculator(
                    optionType, strikes_[j], fwd, stdDev, discount);

                const Real npv = calculator.value();
                const Real vega = calculator.vega(expiryTime);

                marketNPVs[k] = npv/(discount*fwd);
                marketVegas[k] = vega/(discount*fwd);
                lnMarketStrikes[k++] = std::log(strikes_[j]/fwd);
            }
        }

        return ext::make_shared<AndreasenHugeCostFunction>(
            marketNPVs,
            marketVegas,
            lnMarketStrikes,
            previousNPVs,
            mesher_,
            dT_[iExpiry],
            interpolationType_);
    }


    void AndreasenHugeVolatilityInterpl::performCalculations() const {
        QL_REQUIRE(maxStrike() > minStrike(),
            "max strike must be greater than min strike");

        const DayCounter dc = rTS_->dayCounter();
        for (Size i=0; i < expiryTimes_.size(); ++i) {
            expiryTimes_[i] =
                dc.yearFraction(rTS_->referenceDate(), expiries_[i]);
            dT_[i] = expiryTimes_[i] - ( (i==0)? 0.0 : expiryTimes_[i-1]);
        }

        mesher_ =
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Concentrating1dMesher>(
                    std::log(minStrike()/spot_->value()),
                    std::log(maxStrike()/spot_->value()),
                    nGridPoints_,
                    std::pair<Real, Real>(0.0, 0.025)));

        gridPoints_ = mesher_->locations(0);
        gridInFwd_ = Exp(gridPoints_)*spot_->value();

        localVolCache_.clear();
        calibrationResults_.clear();

        avgError_ = 0.0;
        minError_ = std::numeric_limits<Real>::max();
        maxError_ = 0.0;

        calibrationResults_.reserve(expiries_.size());

        Array npvPuts(nGridPoints_);
        Array npvCalls(nGridPoints_);

        for (Size i=0; i < nGridPoints_; ++i) {
            const Real strike = std::exp(gridPoints_[i]);
            npvPuts[i] = PlainVanillaPayoff(Option::Put, strike)(1.0);
            npvCalls[i]= PlainVanillaPayoff(Option::Call, strike)(1.0);
        }

        for (Size i=0; i < expiries_.size(); ++i) {
            const ext::shared_ptr<AndreasenHugeCostFunction> putCostFct =
                buildCostFunction(i, Option::Put, npvPuts);
            const ext::shared_ptr<AndreasenHugeCostFunction> callCostFct =
                buildCostFunction(i, Option::Call, npvCalls);

            CombinedCostFunction costFunction(putCostFct, callCostFct);

            PositiveConstraint positiveConstraint;
            Problem problem(costFunction,
                positiveConstraint, costFunction.initialValues());

            optimizationMethod_->minimize(problem, endCriteria_);

            const Array& sig = problem.currentValue();

            const SingleStepCalibrationResult calibrationResult = {
                npvPuts, npvCalls, sig,
                (calibrationType_ == Call)? callCostFct : putCostFct
            };

            calibrationResults_.push_back(calibrationResult);

            Array vegaDiffs(sig.size());
            switch (calibrationType_) {
              case CallPut: {
                const Array vegaPutDiffs =
                    putCostFct->vegaCalibrationError(sig);
                const Array vegaCallDiffs =
                    callCostFct->vegaCalibrationError(sig);

                const Real fwd = spot_->value()*
                    qTS_->discount(expiryTimes_[i])/rTS_->discount(expiryTimes_[i]);

                for (Size j=0; j < vegaDiffs.size(); ++j)
                    vegaDiffs[j] = std::fabs(
                        (fwd > gridInFwd_[j])? vegaPutDiffs[j] : vegaCallDiffs[j]);
              }
              break;
              case Put:
                vegaDiffs = Abs(putCostFct->vegaCalibrationError(sig));
              break;
              case Call:
                vegaDiffs = Abs(callCostFct->vegaCalibrationError(sig));
              break;
              default:
                QL_FAIL("unknown calibration type");
            }

            avgError_ +=
                std::accumulate(vegaDiffs.begin(), vegaDiffs.end(), 0.0);
            minError_ = std::min(minError_,
                *std::min_element(vegaDiffs.begin(), vegaDiffs.end()));
            maxError_ = std::max(maxError_,
                *std::max_element(vegaDiffs.begin(), vegaDiffs.end()));

            if (putCostFct != 0)
                npvPuts = putCostFct->solveFor(dT_[i], sig, npvPuts);
            if (callCostFct != 0)
                npvCalls= callCostFct->solveFor(dT_[i], sig, npvCalls);
        }

        avgError_ /= calibrationSet_.size();
    }

    Date AndreasenHugeVolatilityInterpl::maxDate() const {
        return expiries_.back();
    }

    Real AndreasenHugeVolatilityInterpl::minStrike() const {
        return (minStrike_ == Null<Real>())
            ? 1/8.0*strikes_.front() : minStrike_;
    }

    Real AndreasenHugeVolatilityInterpl::maxStrike() const {
        return (maxStrike_ == Null<Real>())
            ? 8.0*strikes_.back() : maxStrike_;
    }

    Real AndreasenHugeVolatilityInterpl::fwd(Time t) const {
        return spot_->value()*qTS_->discount(t)/rTS_->discount(t);
    }

    const Handle<YieldTermStructure>&
    AndreasenHugeVolatilityInterpl::riskFreeRate() const {
        return rTS_;
    }

    boost::tuple<Real, Real, Real>
    AndreasenHugeVolatilityInterpl::calibrationError() const {
        calculate();

        return boost::make_tuple<Real, Real, Real>(
            minError_, maxError_, avgError_);
    }

    Size AndreasenHugeVolatilityInterpl::getExerciseTimeIdx(Time t) const {
        return std::min<Size>(expiryTimes_.size()-1,
            std::distance(expiryTimes_.begin(),
                std::upper_bound(
                    expiryTimes_.begin(), expiryTimes_.end(), t)));
    }

    Real AndreasenHugeVolatilityInterpl::getCacheValue(
        Real strike, const TimeValueCacheType::const_iterator& f) const {

        const Real fwd = f->second.get<0>();
        const Real k = std::log(strike / fwd);

        const Real s = std::max(gridPoints_[1],
            std::min(*(gridPoints_.end()-2), k));

        return (*(f->second.get<2>()))(s);
    }

    Disposable<Array> AndreasenHugeVolatilityInterpl::getPriceSlice(
        Time t, Option::Type optionType) const {

        const Size iu = getExerciseTimeIdx(t);

        return calibrationResults_[iu].costFunction->solveFor(
            (iu == 0) ? t : t-expiryTimes_[iu-1],
            calibrationResults_[iu].sigmas,
            (optionType == Option::Call)? calibrationResults_[iu].callNPVs
                                        : calibrationResults_[iu].putNPVs);
    }

    Real AndreasenHugeVolatilityInterpl::optionPrice(
        Time t, Real strike, Option::Type optionType) const {

        TimeValueCacheType::const_iterator f = priceCache_.find(t);

        const DiscountFactor df = rTS_->discount(t);

        if (f != priceCache_.end()) {
            const Real fwd = f->second.get<0>();

            Real price = getCacheValue(strike, f);

            if (optionType == Option::Put
                && (calibrationType_ == Call || calibrationType_ == CallPut))
                price = price + strike/fwd - 1.0;
            else if (optionType == Option::Call && calibrationType_ == Put)
                price = 1.0 - strike/fwd + price;

            return price*df*fwd;
        }

        calculate();


        const ext::shared_ptr<Array> prices(
            ext::make_shared<Array>(gridPoints_));

        switch (calibrationType_) {
          case Put:
            (*prices) = getPriceSlice(t, Option::Put);
          break;
          case Call:
          case CallPut:
            (*prices) = getPriceSlice(t, Option::Call);
          break;
          default:
            QL_FAIL("unknown calibration type");
        }

        const Real fwd = spot_->value()*qTS_->discount(t)/df;

        priceCache_[t] = boost::make_tuple<
            Real,
            ext::shared_ptr<Array>,
            ext::shared_ptr<Interpolation> >(
                fwd, prices,
                ext::make_shared<CubicNaturalSpline>(
                    gridPoints_.begin()+1, gridPoints_.end()-1,
                    prices->begin()+1));

        return this->optionPrice(t, strike, optionType);
    }

    Disposable<Array> AndreasenHugeVolatilityInterpl::getLocalVolSlice(
        Time t, Option::Type optionType) const {

        const Size iu = getExerciseTimeIdx(t);

        const Array& previousNPVs =
            (optionType == Option::Call)? calibrationResults_[iu].callNPVs
                                        : calibrationResults_[iu].putNPVs;

        const ext::shared_ptr<AndreasenHugeCostFunction> costFunction
            = calibrationResults_[iu].costFunction;

        const Time dt = (iu == 0) ? t : t-expiryTimes_[iu-1];
        const Array& sig = calibrationResults_[iu].sigmas;

        const Array cAtJ = costFunction->solveFor(dt, sig, previousNPVs);

        const Array dCdT =
            costFunction->solveFor(dt, sig,
                    costFunction->apply(
                        costFunction->solveFor(dt, sig, previousNPVs)));

        const Array d2CdK2 = costFunction->d2CdK2(cAtJ);

        Array localVol = Sqrt(2*dCdT/d2CdK2);

        for (Size i=1; i < localVol.size()-1; ++i)
            if (!boost::math::isfinite(localVol[i]) || localVol[i] < 0.0)
                localVol[i] = 0.25;

        return localVol;
    }

    Volatility AndreasenHugeVolatilityInterpl::localVol(Time t, Real strike)
    const {
        TimeValueCacheType::const_iterator f = localVolCache_.find(t);

        if (f != localVolCache_.end())
            return getCacheValue(strike, f);

        calculate();

        const ext::shared_ptr<Array> localVol(
            ext::make_shared<Array>(gridPoints_.size()));

        switch (calibrationType_) {
          case CallPut: {
            const Array putLocalVol = getLocalVolSlice(t, Option::Put);
            const Array callLocalVol = getLocalVolSlice(t, Option::Call);

            for (Size i=0, n=localVol->size(); i < n; ++i)
                (*localVol)[i] =
                    (gridPoints_[i] > 0.0)? callLocalVol[i] : putLocalVol[i];
          }
          break;
          case Put:
            (*localVol) = getLocalVolSlice(t, Option::Put);
          break;
          case Call:
            (*localVol) = getLocalVolSlice(t, Option::Call);
          break;
          default:
            QL_FAIL("unknown calibration type");
        }

        const Real fwd = spot_->value()*qTS_->discount(t)/rTS_->discount(t);

        localVolCache_[t] = boost::make_tuple<
            Real,
            ext::shared_ptr<Array>,
            ext::shared_ptr<Interpolation> >(
                fwd, localVol,
                ext::make_shared<LinearInterpolation>(
                    gridPoints_.begin()+1, gridPoints_.end()-1,
                    localVol->begin()+1));

        return this->localVol(t, strike);
    }
}
