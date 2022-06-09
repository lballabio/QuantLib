/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file interpolatedyoyoptionletstripper.hpp
    \brief interpolated yoy inflation-cap stripping
*/

#ifndef quantlib_interpolated_yoy_optionlet_stripper_hpp
#define quantlib_interpolated_yoy_optionlet_stripper_hpp

#include <ql/experimental/inflation/genericindexes.hpp>
#include <ql/experimental/inflation/piecewiseyoyoptionletvolatility.hpp>
#include <ql/experimental/inflation/yoyoptionlethelpers.hpp>
#include <ql/experimental/inflation/yoyoptionletstripper.hpp>
#include <ql/instruments/makeyoyinflationcapfloor.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <utility>


namespace QuantLib {

    /*! The interpolated version interpolates along each K (as opposed
        to fitting a model, say).

        \bug Tests currently fail.
    */
    template <class Interpolator1D>
    class InterpolatedYoYOptionletStripper : public YoYOptionletStripper {
      public:

        //! YoYOptionletStripper interface
        //@{
        void initialize(const ext::shared_ptr<YoYCapFloorTermPriceSurface>&,
                        const ext::shared_ptr<YoYInflationCapFloorEngine>&,
                        Real slope) const override;
        Rate minStrike() const override { return YoYCapFloorTermPriceSurface_->strikes().front(); }
        Rate maxStrike() const override { return YoYCapFloorTermPriceSurface_->strikes().back(); }
        std::vector<Rate> strikes() const override {
            return YoYCapFloorTermPriceSurface_->strikes();
        }
        std::pair<std::vector<Rate>, std::vector<Volatility> > slice(const Date& d) const override;
        //@}

      protected:
        mutable std::vector<ext::shared_ptr<YoYOptionletVolatilitySurface> >
        volCurves_;

        // used to set up the first point on each vol curve
        // using assumptions on unobserved vols at start
        class ObjectiveFunction {
          public:
            ObjectiveFunction(YoYInflationCapFloor::Type type,
                              Real slope,
                              Rate K,
                              Period& lag,
                              Natural fixingDays,
                              const ext::shared_ptr<YoYInflationIndex>& anIndex,
                              const ext::shared_ptr<YoYCapFloorTermPriceSurface>&,
                              ext::shared_ptr<YoYInflationCapFloorEngine> p,
                              Real priceToMatch);
            Real operator()(Volatility guess) const;
          protected:
            Real slope_;
            Rate K_;
            Frequency frequency_;
            bool indexIsInterpolated_;
            std::vector<Time> tvec_;
            std::vector<Date> dvec_;
            mutable std::vector<Volatility> vvec_;
            ext::shared_ptr<YoYInflationCapFloor> capfloor_;
            Real priceToMatch_;
            ext::shared_ptr<YoYCapFloorTermPriceSurface> surf_;
            Period lag_;
            ext::shared_ptr<YoYInflationCapFloorEngine> p_;
        };
    };


    // template definitions

    template <class Interpolator1D>
    InterpolatedYoYOptionletStripper<Interpolator1D>::ObjectiveFunction::ObjectiveFunction(
        YoYInflationCapFloor::Type type,
        Real slope,
        Rate K,
        Period& lag,
        Natural fixingDays,
        const ext::shared_ptr<YoYInflationIndex>& anIndex,
        const ext::shared_ptr<YoYCapFloorTermPriceSurface>& surf,
        ext::shared_ptr<YoYInflationCapFloorEngine> p,
        Real priceToMatch)
    : slope_(slope), K_(K), frequency_(anIndex->frequency()),
      indexIsInterpolated_(anIndex->interpolated()), tvec_(std::vector<Time>(2)),
      dvec_(std::vector<Date>(2)), vvec_(std::vector<Volatility>(2)), priceToMatch_(priceToMatch),
      surf_(surf), p_(std::move(p)) {

        lag_ = surf_->observationLag();
        capfloor_ =
            MakeYoYInflationCapFloor(type, anIndex,
                                     ql_cast<Size>(std::floor(0.5+surf->timeFromReference(surf->minMaturity()))),
                                     surf->calendar(), lag)
            .withNominal(10000.0)
            .withStrike(K);

        // shortest time available from price surface
        dvec_[0] = surf_->baseDate();
        dvec_[1] = surf_->minMaturity() +
                   Period(7, Days);
        tvec_[0] = surf_->dayCounter().yearFraction(surf_->referenceDate(),
                                                    dvec_[0] );
        tvec_[1] = surf_->dayCounter().yearFraction(surf_->referenceDate(),
                                                    dvec_[1]);

        Size n = ql_cast<Size>(std::floor(0.5 + surf->timeFromReference(surf_->minMaturity())));
        QL_REQUIRE( n > 0,
                    "first maturity in price surface not > 0: "
                    << n);

        capfloor_->setPricingEngine(p_);
        // pricer already setup just need to do the volatility surface each time
    }


    template <class Interpolator1D>
    Real InterpolatedYoYOptionletStripper<Interpolator1D>::
    ObjectiveFunction::operator()(Volatility guess) const {

        vvec_[1] = guess;
        vvec_[0] = guess - slope_ * (tvec_[1] - tvec_[0]) * guess;
        // could have Interpolator1D instead of Linear
        ext::shared_ptr<InterpolatedYoYOptionletVolatilityCurve<Linear> >
        vCurve(
            new InterpolatedYoYOptionletVolatilityCurve<Linear>(
                                               0, TARGET(), ModifiedFollowing,
                                               Actual365Fixed(), lag_,
                                               frequency_, indexIsInterpolated_,
                                               dvec_, vvec_,
                                               -1.0, 3.0) ); // strike limits
        Handle<YoYOptionletVolatilitySurface> hCurve(vCurve);
        p_->setVolatility(hCurve);
        // hopefully this gets to the pricer ... then
        return priceToMatch_ - capfloor_->NPV();
    }


    template <class Interpolator1D>
    void InterpolatedYoYOptionletStripper<Interpolator1D>::
    initialize(const ext::shared_ptr<YoYCapFloorTermPriceSurface> &s,
               const ext::shared_ptr<YoYInflationCapFloorEngine> &p,
               const Real slope) const {
        YoYCapFloorTermPriceSurface_ = s;
        p_ = p;
        lag_ = YoYCapFloorTermPriceSurface_->observationLag();
        frequency_ = YoYCapFloorTermPriceSurface_->frequency();
        indexIsInterpolated_ = YoYCapFloorTermPriceSurface_->indexIsInterpolated();
        Natural fixingDays_ = YoYCapFloorTermPriceSurface_->fixingDays();
        Natural settlementDays = 0; // always
        Calendar cal = YoYCapFloorTermPriceSurface_->calendar();
        BusinessDayConvention bdc =
            YoYCapFloorTermPriceSurface_->businessDayConvention();
        DayCounter dc = YoYCapFloorTermPriceSurface_->dayCounter();

        // switch from caps to floors when out of floors
        Rate maxFloor = YoYCapFloorTermPriceSurface_->floorStrikes().back();
        YoYInflationCapFloor::Type useType = YoYInflationCapFloor::Floor;
        Period TPmin = YoYCapFloorTermPriceSurface_->maturities().front();
        // create a "fake index" based on Generic, this should work
        // provided that the lag and frequency are correct
        RelinkableHandle<YoYInflationTermStructure> hYoY(
                                       YoYCapFloorTermPriceSurface_->YoYTS());
        ext::shared_ptr<YoYInflationIndex> anIndex(
                                           new YYGenericCPI(frequency_, false,
                                                            false, lag_,
                                                            Currency(), hYoY));

        // strip each K separatly
        for (Size i=0; i<YoYCapFloorTermPriceSurface_->strikes().size(); i++) {
            Rate K = YoYCapFloorTermPriceSurface_->strikes()[i];
            if (K > maxFloor) useType = YoYInflationCapFloor::Cap;

            // solve for the initial point on the vol curve
            Brent solver;
            Real solverTolerance_ = 1e-7;
             // these are VOLATILITY guesses (always +)
            Real lo = 0.00001, hi = 0.08;
            Real guess = (hi+lo)/2.0;
            Real found;
            Real priceToMatch =
                (useType == YoYInflationCapFloor::Cap ?
                 YoYCapFloorTermPriceSurface_->capPrice(TPmin, K) :
                 YoYCapFloorTermPriceSurface_->floorPrice(TPmin, K));

            try{
                found = solver.solve(
                      ObjectiveFunction(useType, slope, K, lag_, fixingDays_,
                                        anIndex, YoYCapFloorTermPriceSurface_,
                                        p_, priceToMatch),
                      solverTolerance_, guess, lo, hi );
            } catch( std::exception &e) {
                QL_FAIL("failed to find solution here because: " << e.what());
            }

            // ***create helpers***
            Real notional = 10000; // work in bps
            std::vector<ext::shared_ptr<BootstrapHelper<YoYOptionletVolatilitySurface> > > helperInstruments;
            std::vector<ext::shared_ptr<YoYOptionletHelper> > helpers;
            for (Size j = 0; j < YoYCapFloorTermPriceSurface_->maturities().size(); j++){
                Period Tp = YoYCapFloorTermPriceSurface_->maturities()[j];

                Real nextPrice =
                    (useType == YoYInflationCapFloor::Cap ?
                     YoYCapFloorTermPriceSurface_->capPrice(Tp, K) :
                     YoYCapFloorTermPriceSurface_->floorPrice(Tp, K));

                Handle<Quote> quote1(ext::shared_ptr<Quote>(
                                               new SimpleQuote( nextPrice )));
                // helper should be an integer number of periods away,
                // this is enforced by rounding
                Size nT = ql_cast<Size>(floor(s->timeFromReference(s->yoyOptionDateFromTenor(Tp))+0.5));
                helpers.push_back(ext::shared_ptr<YoYOptionletHelper>(
                          new YoYOptionletHelper(quote1, notional, useType,
                                                 lag_,
                                                 dc, cal,
                                                 fixingDays_,
                                                 anIndex, K, nT, p_)));

                ext::shared_ptr<ConstantYoYOptionletVolatility> yoyVolBLACK(
                          new ConstantYoYOptionletVolatility(found, settlementDays,
                                                             cal, bdc, dc,
                                                             lag_, frequency_,
                                                             false,
                                                             // -100% to +300%
                                                             -1.0,3.0));

                helpers[j]->setTermStructure(
                       // gets underlying pointer & removes const
                       const_cast<ConstantYoYOptionletVolatility*>(
                                                          yoyVolBLACK.get()));
                helperInstruments.push_back(helpers[j]);
            }
            // ***bootstrap***
            // this is the artificial vol at zero so that first section works
            Real Tmin = s->timeFromReference(s->yoyOptionDateFromTenor(TPmin));
            Volatility baseYoYVolatility = found - slope * Tmin * found;
            Rate eps = std::max(K, 0.02) / 1000.0;
            Rate minStrike = K-eps;
            Rate maxStrike = K+eps;
            ext::shared_ptr<
                PiecewiseYoYOptionletVolatilityCurve<Interpolator1D> > testPW(
                new PiecewiseYoYOptionletVolatilityCurve<Interpolator1D>(
                                            settlementDays, cal, bdc, dc, lag_,
                                            frequency_, indexIsInterpolated_,
                                            minStrike, maxStrike,
                                            baseYoYVolatility,
                                            helperInstruments) );
            testPW->recalculate();
            volCurves_.push_back(testPW);
        }
    }


    template <class Interpolator1D>
    std::pair<std::vector<Rate>, std::vector<Volatility> >
    InterpolatedYoYOptionletStripper<Interpolator1D>::slice(
                                                        const Date &d) const {

        const std::vector<Real>& Ks = strikes();

        const Size nK = Ks.size();

        std::pair<std::vector<Rate>, std::vector<Volatility> > result =
            std::make_pair(std::vector<Rate>(nK), std::vector<Volatility>(nK));

        for (Size i = 0; i < nK; i++) {
            Rate K = Ks[i];
            Volatility v = volCurves_[i]->volatility(d, K);
            result.first[i] = K;
            result.second[i] = v;
        }

        return result;
    }

}

#endif

