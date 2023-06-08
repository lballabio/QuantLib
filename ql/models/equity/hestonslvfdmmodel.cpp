/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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


#include <ql/models/equity/hestonslvfdmmodel.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmhestonfwdop.hpp>
#include <ql/methods/finitedifferences/schemes/craigsneydscheme.hpp>
#include <ql/methods/finitedifferences/schemes/douglasscheme.hpp>
#include <ql/methods/finitedifferences/schemes/expliciteulerscheme.hpp>
#include <ql/methods/finitedifferences/schemes/hundsdorferscheme.hpp>
#include <ql/methods/finitedifferences/schemes/impliciteulerscheme.hpp>
#include <ql/methods/finitedifferences/schemes/modifiedcraigsneydscheme.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmmesherintegral.hpp>
#include <ql/methods/finitedifferences/utilities/localvolrndcalculator.hpp>
#include <ql/methods/finitedifferences/utilities/squarerootprocessrndcalculator.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/timegrid.hpp>
#include <functional>
#include <memory>
#include <utility>

namespace QuantLib {

    namespace {
        ext::shared_ptr<Fdm1dMesher> varianceMesher(
            const SquareRootProcessRNDCalculator& rnd,
            Time t0, Time t1, Size vGrid,
            Real v0, const HestonSLVFokkerPlanckFdmParams& params) {

            std::vector<ext::tuple<Real, Real, bool> > cPoints;

            const Real v0Density = params.v0Density;
            const Real upperBoundDensity = params.vUpperBoundDensity;
            const Real lowerBoundDensity = params.vLowerBoundDensity;

            Real lowerBound = Null<Real>(), upperBound = -Null<Real>();

            for (Size i=0; i <= 10; ++i) {
                const Time t = t0 + i/10.0*(t1-t0);
                lowerBound = std::min(
                    lowerBound, rnd.invcdf(params.vLowerEps, t));
                upperBound = std::max(
                    upperBound, rnd.invcdf(1.0-params.vUpperEps, t));
            }

            lowerBound = std::max(lowerBound, params.vMin);
            switch (params.trafoType) {
                case FdmSquareRootFwdOp::Log:
                  {
                    lowerBound = std::log(lowerBound);
                    upperBound = std::log(upperBound);

                    const Real v0Center = std::log(v0);

                    cPoints = {
                        {lowerBound, lowerBoundDensity, false},
                        {v0Center, v0Density, true},
                        {upperBound, upperBoundDensity, false}
                    };

                    return ext::make_shared<Concentrating1dMesher>(
                        lowerBound, upperBound, vGrid, cPoints, 1e-8);
                  }
                break;
                case FdmSquareRootFwdOp::Plain:
                  {
                      const Real v0Center = v0;

                      cPoints = {
                          {lowerBound, lowerBoundDensity, false},
                          {v0Center, v0Density, true},
                          {upperBound, upperBoundDensity, false}
                      };

                      return ext::make_shared<Concentrating1dMesher>(
                          lowerBound, upperBound, vGrid, cPoints, 1e-8);
                  }
                break;
                case FdmSquareRootFwdOp::Power:
                {
                    const Real v0Center = v0;

                    cPoints = {
                        {lowerBound, lowerBoundDensity, false},
                        {v0Center, v0Density, true},
                        {upperBound, upperBoundDensity, false}
                    };

                    return ext::make_shared<Concentrating1dMesher>(
                        lowerBound, upperBound, vGrid, cPoints, 1e-8);
                }
                break;
                default:
                    QL_FAIL("transformation type is not implemented");
            }
        }

        Real integratePDF(const Array& p,
                          const ext::shared_ptr<FdmMesherComposite>& mesher,
                          FdmSquareRootFwdOp::TransformationType trafoType,
                          Real alpha) {

            if (trafoType != FdmSquareRootFwdOp::Power) {
                return FdmMesherIntegral(
                        mesher, DiscreteSimpsonIntegral()).integrate(p);
            }
            else {
                Array tp(p.size());
                for (const auto& iter : *mesher->layout()) {
                    const Size idx = iter.index();
                    const Real nu = mesher->location(iter, 1);

                    tp[idx] = p[idx]*std::pow(nu, alpha-1);
                }

                return FdmMesherIntegral(
                        mesher, DiscreteSimpsonIntegral()).integrate(tp);
            }
        }


        Array rescalePDF(
            const Array& p,
            const ext::shared_ptr<FdmMesherComposite>& mesher,
            FdmSquareRootFwdOp::TransformationType trafoType, Real alpha) {

            return p/integratePDF(p, mesher, trafoType, alpha);
        }


        template <class Interpolator>
        Array reshapePDF(
            const Array& p,
            const ext::shared_ptr<FdmMesherComposite>& oldMesher,
            const ext::shared_ptr<FdmMesherComposite>& newMesher,
            const Interpolator& interp = Interpolator()) {

            QL_REQUIRE(   oldMesher->layout()->size() == newMesher->layout()->size()
                       && oldMesher->layout()->size() == p.size(),
                       "inconsistent mesher or vector size given");

            Matrix m(oldMesher->layout()->dim()[1], oldMesher->layout()->dim()[0]);
            for (Size i=0; i < m.rows(); ++i) {
                std::copy(p.begin() + i*m.columns(),
                          p.begin() + (i+1)*m.columns(), m.row_begin(i));
            }
            const Interpolation2D interpol = interp.interpolate(
                oldMesher->getFdm1dMeshers()[0]->locations().begin(),
                oldMesher->getFdm1dMeshers()[0]->locations().end(),
                oldMesher->getFdm1dMeshers()[1]->locations().begin(),
                oldMesher->getFdm1dMeshers()[1]->locations().end(), m);

            Array pNew(p.size());
            for (const auto& iter : *newMesher->layout()) {
                const Real x = newMesher->location(iter, 0);
                const Real v = newMesher->location(iter, 1);

                if (   x > interpol.xMax() || x < interpol.xMin()
                    || v > interpol.yMax() || v < interpol.yMin() ) {
                    pNew[iter.index()] = 0;
                }
                else {
                    pNew[iter.index()] = interpol(x, v);
                }
            }

            return pNew;
        }

        class FdmScheme {
          public:
            virtual ~FdmScheme() = default;
            virtual void step(Array& a, Time t) = 0;
            virtual void setStep(Time dt) = 0;
        };

        template <class T>
        class FdmSchemeWrapper : public FdmScheme {
          public:
            explicit FdmSchemeWrapper(T* scheme)
            : scheme_(scheme) { }

            void step(Array& a, Time t) override { scheme_->step(a, t); }
            void setStep(Time dt) override { scheme_->setStep(dt); }

          private:
            const std::unique_ptr<T> scheme_;
        };

        ext::shared_ptr<FdmScheme> fdmSchemeFactory(
            const FdmSchemeDesc desc,
            const ext::shared_ptr<FdmLinearOpComposite>& op) {

            switch (desc.type) {
              case FdmSchemeDesc::HundsdorferType:
                  return ext::shared_ptr<FdmScheme>(
                      new FdmSchemeWrapper<HundsdorferScheme>(
                          new HundsdorferScheme(desc.theta, desc.mu, op)));
              case FdmSchemeDesc::DouglasType:
                  return ext::shared_ptr<FdmScheme>(
                      new FdmSchemeWrapper<DouglasScheme>(
                          new DouglasScheme(desc.theta, op)));
              case FdmSchemeDesc::CraigSneydType:
                  return ext::shared_ptr<FdmScheme>(
                      new FdmSchemeWrapper<CraigSneydScheme>(
                          new CraigSneydScheme(desc.theta, desc.mu, op)));
              case FdmSchemeDesc::ModifiedCraigSneydType:
                  return ext::shared_ptr<FdmScheme>(
                     new FdmSchemeWrapper<ModifiedCraigSneydScheme>(
                          new ModifiedCraigSneydScheme(
                              desc.theta, desc.mu, op)));
              case FdmSchemeDesc::ImplicitEulerType:
                  return ext::shared_ptr<FdmScheme>(
                      new FdmSchemeWrapper<ImplicitEulerScheme>(
                          new ImplicitEulerScheme(op)));
              case FdmSchemeDesc::ExplicitEulerType:
                  return ext::shared_ptr<FdmScheme>(
                      new FdmSchemeWrapper<ExplicitEulerScheme>(
                          new ExplicitEulerScheme(op)));
              default:
                  QL_FAIL("Unknown scheme type");
            }
        }
    }

    HestonSLVFDMModel::HestonSLVFDMModel(Handle<LocalVolTermStructure> localVol,
                                         Handle<HestonModel> hestonModel,
                                         const Date& endDate,
                                         HestonSLVFokkerPlanckFdmParams params,
                                         const bool logging,
                                         std::vector<Date> mandatoryDates,
                                         const Real mixingFactor)
    : localVol_(std::move(localVol)), hestonModel_(std::move(hestonModel)), endDate_(endDate),
      params_(std::move(params)), mandatoryDates_(std::move(mandatoryDates)),
      mixingFactor_(mixingFactor), logging_(logging) {

        registerWith(localVol_);
        registerWith(hestonModel_);
    }

    ext::shared_ptr<HestonProcess> HestonSLVFDMModel::hestonProcess() const {
        return hestonModel_->process();
    }

    ext::shared_ptr<LocalVolTermStructure> HestonSLVFDMModel::localVol() const {
        return localVol_.currentLink();
    }

    ext::shared_ptr<LocalVolTermStructure>
    HestonSLVFDMModel::leverageFunction() const {
        calculate();

        return leverageFunction_;
    }

    void HestonSLVFDMModel::performCalculations() const {
        logEntries_.clear();

        const ext::shared_ptr<HestonProcess> hestonProcess
            = hestonModel_->process();
        const ext::shared_ptr<Quote> spot
            = hestonProcess->s0().currentLink();
        const ext::shared_ptr<YieldTermStructure> rTS
            = hestonProcess->riskFreeRate().currentLink();
        const ext::shared_ptr<YieldTermStructure> qTS
            = hestonProcess->dividendYield().currentLink();

        const Real v0    = hestonProcess->v0();
        const Real kappa = hestonProcess->kappa();
        const Real theta = hestonProcess->theta();
        const Real sigma = hestonProcess->sigma();
        const Real mixedSigma = mixingFactor_ * sigma;
        const Real alpha = 2*kappa*theta/(mixedSigma*mixedSigma);

        const Size xGrid = params_.xGrid;
        const Size vGrid = params_.vGrid;

        const DayCounter dc = rTS->dayCounter();
        const Date referenceDate = rTS->referenceDate();

        const Time T = dc.yearFraction(referenceDate, endDate_);

        QL_REQUIRE(referenceDate < endDate_,
            "reference date must be smaller than final calibration date");

        QL_REQUIRE(localVol_->maxTime() >= T,
            "final calibration maturity exceeds local volatility surface");

        // set-up exponential time step scheme
        const Time maxDt = 1.0/params_.tMaxStepsPerYear;
        const Time minDt = 1.0/params_.tMinStepsPerYear;

        Time tIdx=0.0;
        std::vector<Time> times(1, tIdx);
        times.reserve(Size(T*params_.tMinStepsPerYear));
        while (tIdx < T) {
            const Real decayFactor = std::exp(-params_.tStepNumberDecay*tIdx);
            const Time dt = maxDt*decayFactor + minDt*(1.0-decayFactor);

            times.push_back(std::min(T, tIdx+=dt));
        }

        for (auto mandatoryDate : mandatoryDates_) {
            times.push_back(dc.yearFraction(referenceDate, mandatoryDate));
        }

        const ext::shared_ptr<TimeGrid> timeGrid(
            new TimeGrid(times.begin(), times.end()));

        // build 1d meshers
        const LocalVolRNDCalculator localVolRND(
            spot, rTS, qTS, localVol_.currentLink(),
            timeGrid, xGrid,
            params_.x0Density,
            params_.localVolEpsProb,
            params_.maxIntegrationIterations);

        const std::vector<Size> rescaleSteps
            = localVolRND.rescaleTimeSteps();

        const SquareRootProcessRNDCalculator squareRootRnd(
            v0, kappa, theta, mixedSigma);

        const FdmSquareRootFwdOp::TransformationType trafoType
          = params_.trafoType;

        std::vector<ext::shared_ptr<Fdm1dMesher> > xMesher, vMesher;
        xMesher.reserve(timeGrid->size());
        vMesher.reserve(timeGrid->size());

        xMesher.push_back(localVolRND.mesher(0.0));
        vMesher.push_back(ext::make_shared<Predefined1dMesher>(
            std::vector<Real>(vGrid, v0)));

        Size rescaleIdx = 0;
        for (Size i=1; i < timeGrid->size(); ++i) {
            xMesher.push_back(localVolRND.mesher(timeGrid->at(i)));

            if ((rescaleIdx < rescaleSteps.size())
                && (i == rescaleSteps[rescaleIdx])) {
                ++rescaleIdx;
                vMesher.push_back(varianceMesher(squareRootRnd,
                    timeGrid->at(rescaleSteps[rescaleIdx-1]),
                    (rescaleIdx < rescaleSteps.size())
                        ? timeGrid->at(rescaleSteps[rescaleIdx])
                        : timeGrid->back(),
                    vGrid, v0, params_));
            }
            else
                vMesher.push_back(vMesher.back());
        }

        // start probability distribution
        ext::shared_ptr<FdmMesherComposite> mesher
            = ext::make_shared<FdmMesherComposite>(
                xMesher.at(1), vMesher.at(1));

        const Volatility lv0
            = localVol_->localVol(0.0, spot->value())/std::sqrt(v0);

        ext::shared_ptr<Matrix> L(new Matrix(xGrid, timeGrid->size()));

        const Real l0 = lv0;
        std::fill(L->column_begin(0),L->column_end(0), l0);
        std::fill(L->column_begin(1),L->column_end(1), l0);

        // create strikes from meshers
        std::vector<ext::shared_ptr<std::vector<Real> > > vStrikes(
            timeGrid->size());

        for (Size i=0; i < timeGrid->size(); ++i) {
            vStrikes[i] = ext::make_shared<std::vector<Real> >(xGrid);
            if (xMesher[i]->locations().front()
                  == xMesher[i]->locations().back()) {
                std::fill(vStrikes[i]->begin(), vStrikes[i]->end(),
                    std::exp(xMesher[i]->locations().front()));
            }
            else {
                std::transform(xMesher[i]->locations().begin(),
                               xMesher[i]->locations().end(),
                               vStrikes[i]->begin(),
                               [](Real x) -> Real { return std::exp(x); });
            }
        }

        const ext::shared_ptr<FixedLocalVolSurface> leverageFct(
            new FixedLocalVolSurface(referenceDate, times, vStrikes, L, dc));

        ext::shared_ptr<FdmLinearOpComposite> hestonFwdOp(
            new FdmHestonFwdOp(mesher, hestonProcess, trafoType, leverageFct, mixingFactor_));

        Array p = FdmHestonGreensFct(mesher, hestonProcess, trafoType, lv0)
            .get(timeGrid->at(1), params_.greensAlgorithm);

        if (logging_) {
            const LogEntry entry = { timeGrid->at(1),
                ext::make_shared<Array>(p), mesher };
            logEntries_.push_back(entry);
        }

        for (Size i=2; i < times.size(); ++i) {
            const Time t = timeGrid->at(i);
            const Time dt = t - timeGrid->at(i-1);

            if (   mesher->getFdm1dMeshers()[0] != xMesher[i]
                || mesher->getFdm1dMeshers()[1] != vMesher[i]) {
                const ext::shared_ptr<FdmMesherComposite> newMesher(
                    new FdmMesherComposite(xMesher[i], vMesher[i]));

                p = reshapePDF<Bilinear>(p, mesher, newMesher);
                mesher = newMesher;

                p = rescalePDF(p, mesher, trafoType, alpha);

                hestonFwdOp = ext::shared_ptr<FdmLinearOpComposite>(
                                new FdmHestonFwdOp(mesher, hestonProcess,
                                               trafoType, leverageFct, mixingFactor_));
            }

            Array pn = p;
            const Array x(Exp(
                Array(mesher->getFdm1dMeshers()[0]->locations().begin(),
                      mesher->getFdm1dMeshers()[0]->locations().end())));
            const Array v(
                    mesher->getFdm1dMeshers()[1]->locations().begin(),
                    mesher->getFdm1dMeshers()[1]->locations().end());

            // predictor corrector steps
            for (Size r=0; r < params_.predictionCorretionSteps; ++r) {
                const FdmSchemeDesc fdmSchemeDesc
                    = (i < params_.nRannacherTimeSteps + 2)
                        ? FdmSchemeDesc::ImplicitEuler()
                        : params_.schemeDesc;

                const ext::shared_ptr<FdmScheme> fdmScheme(
                    fdmSchemeFactory(fdmSchemeDesc, hestonFwdOp));

                for (Size j=0; j < x.size(); ++j) {
                    Array pSlice(vGrid);
                    for (Size k=0; k < vGrid; ++k)
                        pSlice[k] = pn[j + k*xGrid];

                    const Real pInt = (trafoType == FdmSquareRootFwdOp::Power)
                       ? DiscreteSimpsonIntegral()(v, Pow(v, alpha-1)*pSlice)
                       : DiscreteSimpsonIntegral()(v, pSlice);

                    const Real vpInt = (trafoType == FdmSquareRootFwdOp::Log)
                      ? DiscreteSimpsonIntegral()(v, Exp(v)*pSlice)
                      : (trafoType == FdmSquareRootFwdOp::Power)
                      ? DiscreteSimpsonIntegral()(v, Pow(v, alpha)*pSlice)
                      : DiscreteSimpsonIntegral()(v, v*pSlice);

                    const Real scale = pInt/vpInt;
                    const Volatility localVol = localVol_->localVol(t, x[j]);

                    const Real l = (scale >= 0.0)
                      ? localVol*std::sqrt(scale) : Real(1.0);

                    (*L)[j][i] = std::min(50.0, std::max(0.001, l));

                    leverageFct->setInterpolation(Linear());
                }

                const Real sLowerBound = std::max(x.front(),
                    std::exp(localVolRND.invcdf(
                        params_.leverageFctPropEps, t)));
                const Real sUpperBound = std::min(x.back(),
                    std::exp(localVolRND.invcdf(
                        1.0-params_.leverageFctPropEps, t)));

                const Real lowerL = leverageFct->localVol(t, sLowerBound);
                const Real upperL = leverageFct->localVol(t, sUpperBound);

                for (Size j=0; j < x.size(); ++j) {
                    if (x[j] < sLowerBound)
                        std::fill(L->row_begin(j)+i,
                          std::min(L->row_begin(j)+i+1, L->row_end(j)),
                          lowerL);
                    else if (x[j] > sUpperBound)
                        std::fill(L->row_begin(j)+i,
                          std::min(L->row_begin(j)+i+1, L->row_end(j)),
                          upperL);
                    else if ((*L)[j][i] == Null<Real>())
                        QL_FAIL("internal error");
                }
                leverageFct->setInterpolation(Linear());

                pn = p;

                fdmScheme->setStep(dt);
                fdmScheme->step(pn, t);
            }
            p = pn;
            p = rescalePDF(p, mesher, trafoType, alpha);

            if (logging_) {
                const LogEntry entry
                    = { t, ext::make_shared<Array>(p), mesher };
                logEntries_.push_back(entry);
            }
        }

        leverageFunction_ = leverageFct;
    }

    const std::list<HestonSLVFDMModel::LogEntry>& HestonSLVFDMModel::logEntries()
    const {
        performCalculations();
        return logEntries_;
    }
}

