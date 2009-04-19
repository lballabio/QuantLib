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

#include <ql/experimental/inflation/yoyoptionletstripper.hpp>
#include <ql/experimental/inflation/piecewiseyoyoptionletvolatility.hpp>
#include <ql/experimental/inflation/yoyoptionlethelpers.hpp>
#include <ql/experimental/inflation/genericindexes.hpp>

namespace QuantLib {

    /*! The interpolated version interpolates along each K (as opposed
        to fitting a model, say).
    */
    template <class Interpolator1D>
    class InterpolatedYoYOptionletStripper : public YoYOptionletStripper {
      public:

        //! YoYOptionletStripper interface
        //@{
        virtual void initialize(
                       const boost::shared_ptr<YoYCapFloorTermPriceSurface> &,
                       const boost::shared_ptr<YoYInflationCapFloorEngine> &,
                       const Real slope) const;
        virtual Rate minStrike() const {
            return YoYCapFloorTermPriceSurface_->strikes().front();
        }
        virtual Rate maxStrike() const {
            return YoYCapFloorTermPriceSurface_->strikes().back();
        }
        virtual std::vector<Rate> strikes() const {
            return YoYCapFloorTermPriceSurface_->strikes();
        }
        virtual std::pair<std::vector<Rate>, std::vector<Volatility> >
        slice(const Date &d) const;
        //@}

      protected:
        mutable std::vector<boost::shared_ptr<YoYOptionletVolatilitySurface> >
        volCurves_;

        // used to set up the first point on each vol curve
        // using assumptions on unobserved vols at start
        class ObjectiveFunction {
          public:
            ObjectiveFunction(
                       YoYInflationCapFloor::Type type,
                       Real slope, Rate K,
                       Period &lag,
                       Natural fixingDays,
                       boost::shared_ptr<YoYInflationIndex> anIndex,
                       const boost::shared_ptr<YoYCapFloorTermPriceSurface> &,
                       const boost::shared_ptr<YoYInflationCapFloorEngine> &p,
                       Real priceToMatch);
            Real operator()(Volatility guess) const;
          protected:
            Rate K_;
            Real slope_;
            std::vector<Time> tvec_;
            std::vector<Date> dvec_;
            mutable std::vector<Volatility> vvec_;
            YoYInflationCapFloor capfloor_;
            Real priceToMatch_;
            boost::shared_ptr<YoYCapFloorTermPriceSurface> surf_;
            Period lag_;
            boost::shared_ptr<YoYInflationCapFloorEngine> p_;
        };
    };


    // template definitions

    template <class Interpolator1D>
    InterpolatedYoYOptionletStripper<Interpolator1D>::
    ObjectiveFunction::ObjectiveFunction(
                   YoYInflationCapFloor::Type type,
                   Real slope, Rate K,
                   Period &lag,
                   Natural fixingDays,
                   boost::shared_ptr<YoYInflationIndex> anIndex,
                   const boost::shared_ptr<YoYCapFloorTermPriceSurface> &surf,
                   const boost::shared_ptr<YoYInflationCapFloorEngine> &p,
                   Real priceToMatch)
    : K_(K), slope_(slope),
      capfloor_(MakeYoYInflationCapFloor(type, lag, fixingDays, anIndex, K_,
                                         (Rate)floor(0.5+surf->minMaturity()))
                .withNominal(10000.0) ),
      priceToMatch_(priceToMatch), surf_(surf), p_(p) {
        tvec_ = std::vector<Time>(2);
        vvec_ = std::vector<Volatility>(2);
        dvec_ = std::vector<Date>(2);
        lag_ = surf_->lag();
        capfloor_ =
            MakeYoYInflationCapFloor(type, lag_, fixingDays, anIndex, K_,
                                     (Rate)floor(0.5+surf->minMaturity()))
            .withNominal(10000.0) ;

        // shortest time available from price surface
        dvec_[0] = surf_->baseDate();
        dvec_[1] = surf_->baseDate() +
                   Period(surf_->minMaturity(), Years) +
                   Period(7, Days);
        tvec_[0] = surf_->dayCounter().yearFraction(surf_->referenceDate(),
                                                    dvec_[0] );
        tvec_[1] = surf_->dayCounter().yearFraction(surf_->referenceDate(),
                                                    dvec_[1]);

        Real eps = 1.0e-12;
        Size n = floor(0.5 + surf_->minMaturity());
        QL_REQUIRE( abs(surf_->minMaturity() - n) < eps,
                    "first maturity in price surface not an integer: "
                    << surf_->minMaturity());

        capfloor_.setPricingEngine(p_);
        // pricer already setup just need to do the volatility surface each time
    }


    template <class Interpolator1D>
    Real InterpolatedYoYOptionletStripper<Interpolator1D>::
    ObjectiveFunction::operator()(Volatility guess) const {

        vvec_[1] = guess;
        vvec_[0] = guess - slope_ * (tvec_[1] - tvec_[0]) * guess;
        // could have Interpolator1D instead of Linear
        boost::shared_ptr<InterpolatedYoYOptionletVolatilityCurve<Linear> >
        vCurve(
            new InterpolatedYoYOptionletVolatilityCurve<Linear>(
                                               0, TARGET(), ModifiedFollowing,
                                               Actual365Fixed(), lag_,
                                               dvec_, vvec_,
                                               -1.0, 3.0) ); // strike limits
        Handle<YoYOptionletVolatilitySurface> hCurve(vCurve);
        p_->setVolatility(hCurve);
        // hopefully this gets to the pricer ... then

        return priceToMatch_ - capfloor_.NPV();
    }


    template <class Interpolator1D>
    void InterpolatedYoYOptionletStripper<Interpolator1D>::
    initialize(const boost::shared_ptr<YoYCapFloorTermPriceSurface> &s,
               const boost::shared_ptr<YoYInflationCapFloorEngine> &p,
               const Real slope) const {
        YoYCapFloorTermPriceSurface_ = s;
        p_ = p;
        lag_ = YoYCapFloorTermPriceSurface_->lag();
        frequency_ = YoYCapFloorTermPriceSurface_->frequency();
        Natural fixingDays_ = YoYCapFloorTermPriceSurface_->fixingDays();
        Natural settlementDays = 0; // always
        Calendar cal = YoYCapFloorTermPriceSurface_->calendar();
        BusinessDayConvention bdc =
            YoYCapFloorTermPriceSurface_->businessDayConvention();
        DayCounter dc = YoYCapFloorTermPriceSurface_->dayCounter();

        // switch from caps to floors when out of floors
        Rate maxFloor = YoYCapFloorTermPriceSurface_->floorStrikes().back();
        YoYInflationCapFloor::Type useType = YoYInflationCapFloor::Floor;
        Time Tmin = YoYCapFloorTermPriceSurface_->maturities().front();
        // create a "fake index" based on Generic, this should work
        // provided that the lag and frequency are correct
        Handle<YoYInflationTermStructure> hYoY(
                                       YoYCapFloorTermPriceSurface_->YoYTS());
        boost::shared_ptr<YoYInflationIndex> anIndex(
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
                 YoYCapFloorTermPriceSurface_->capPrice(Tmin, K) :
                 YoYCapFloorTermPriceSurface_->floorPrice(Tmin, K));

            try{
                found = solver.solve(
                      ObjectiveFunction(useType, slope, K, lag_, fixingDays_,
                                        anIndex, YoYCapFloorTermPriceSurface_,
                                        p_, priceToMatch),
                      solverTolerance_, guess, lo, hi );
            } catch( std::exception &e) {
                QL_FAIL("failed to find solution because: " << e.what());
            }

            // ***create helpers***
            Real notional = 10000; // work in bps
            std::vector<boost::shared_ptr<BootstrapHelper<YoYOptionletVolatilitySurface> > > helperInstruments;
            std::vector<boost::shared_ptr<YoYOptionletHelper> > helpers;
            for (Size j = 0; j < YoYCapFloorTermPriceSurface_->maturities().size(); j++){
                Time T = YoYCapFloorTermPriceSurface_->maturities()[j];

                Real nextPrice =
                    (useType == YoYInflationCapFloor::Cap ?
                     YoYCapFloorTermPriceSurface_->capPrice(T, K) :
                     YoYCapFloorTermPriceSurface_->floorPrice(T, K));

                Handle<Quote> quote1(boost::shared_ptr<Quote>(
                                               new SimpleQuote( nextPrice )));
                // helper should be an integer number of periods away,
                // this is enforced by rounding
                Size nT = (Size)floor(T+0.5);
                helpers.push_back(boost::shared_ptr<YoYOptionletHelper>(
                          new YoYOptionletHelper(quote1, notional, useType,
                                                 lag_, fixingDays_,
                                                 anIndex, K, nT, p_)));

                boost::shared_ptr<ConstantYoYOptionletVolatility> yoyVolBLACK(
                          new ConstantYoYOptionletVolatility(found,lag_,
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
            Volatility baseYoYVolatility = found - slope * Tmin * found;
            Rate eps = std::max(K, 0.02) / 1000.0;
            Rate minStrike = K-eps;
            Rate maxStrike = K+eps;
            boost::shared_ptr<
                PiecewiseYoYOptionletVolatilityCurve<Interpolator1D> > testPW(
                new PiecewiseYoYOptionletVolatilityCurve<Interpolator1D>(
                                           settlementDays, cal, bdc, dc, lag_,
                                           minStrike, maxStrike,
                                           baseYoYVolatility,
                                           helperInstruments) );
            testPW->recalculate();
            volCurves_.push_back(testPW);
            for(Size j = 0; j < YoYCapFloorTermPriceSurface_->maturities().size(); j++) {
                Date di =  helpers[j]->latestDate();
            }
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

