/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2014, 2016 Peter Caspers

  This file is part of QuantLib, a free-software/open-source library
  for financial quantitative analysts and developers - http://quantlib.org/

  QuantLib is free software: you can redistribute it and/or modify it
  under the terms of the QuantLib license.  You should have received a
  copy of the license along with this program; if not, please email
  <quantlib-dev@lists.sf.net>. The license is also available online at
  <http://quantlib.org/license.shtml>.


  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file lineartsrpricer.hpp
    \brief linear terminal swap rate model for cms coupon pricing
*/

#ifndef quantlib_lineartsr_pricer_hpp
#define quantlib_lineartsr_pricer_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/math/integrals/integral.hpp>

namespace QuantLib {

    class CmsCoupon;
    class YieldTermStructure;

    //! CMS-coupon pricer
    /*! Prices a cms coupon using a linear terminal swap rate model
        The slope parameter is linked to a gaussian short rate model.
        Reference: Andersen, Piterbarg, Interest Rate Modeling, 16.3.2

        The cut off point for integration can be set
        - by explicitly specifying the lower and upper bound
        - by defining the lower and upper bound to be the strike where
          a vanilla swaption has 1% or less vega of the atm swaption
        - by defining the lower and upper bound to be the strike where
          undeflated (!) payer resp. receiver prices are below a given
          threshold
        - by specificying a number of standard deviations to cover
          using a Black Scholes process with an atm volatility as
          a benchmark
        In every case the lower and upper bound are applied though.
        In case the smile section is shifted lognormal, the specified
        lower and upper bound are applied to strike + shift so that
        e.g. a zero lower bound always refers to the lower bound of
        the rates in the shifted lognormal model.
        Note that for normal volatility input the lower rate bound
        is adjusted to min(-upperBound, lowerBound), except the bounds
        are set explicitly.
    */

    class LinearTsrPricer : public CmsCouponPricer, public MeanRevertingPricer {

      private:
        static const Real defaultLowerBound,
                          defaultUpperBound;

      public:

        struct Settings {

            Settings() : lowerRateBound_(defaultLowerBound), upperRateBound_(defaultUpperBound) {}

            Settings &withRateBound(const Real lowerRateBound = defaultLowerBound,
                                    const Real upperRateBound = defaultUpperBound) {
                strategy_ = RateBound;
                lowerRateBound_ = lowerRateBound;
                upperRateBound_ = upperRateBound;
                defaultBounds_ = false;
                return *this;
            }

            Settings &withVegaRatio(const Real vegaRatio = 0.01) {
                strategy_ = VegaRatio;
                vegaRatio_ = vegaRatio;
                lowerRateBound_ = defaultLowerBound;
                upperRateBound_ = defaultUpperBound;
                defaultBounds_ = true;
                return *this;
            }

            Settings &withVegaRatio(const Real vegaRatio,
                                    const Real lowerRateBound,
                                    const Real upperRateBound) {
                strategy_ = VegaRatio;
                vegaRatio_ = vegaRatio;
                lowerRateBound_ = lowerRateBound;
                upperRateBound_ = upperRateBound;
                defaultBounds_ = false;
                return *this;
            }

            Settings &withPriceThreshold(const Real priceThreshold = 1.0E-8) {
                strategy_ = PriceThreshold;
                priceThreshold_ = priceThreshold;
                lowerRateBound_ = defaultLowerBound;
                upperRateBound_ = defaultUpperBound;
                defaultBounds_ = true;
                return *this;
            }

            Settings &withPriceThreshold(const Real priceThreshold,
                                         const Real lowerRateBound,
                                         const Real upperRateBound) {
                strategy_ = PriceThreshold;
                priceThreshold_ = priceThreshold;
                lowerRateBound_ = lowerRateBound;
                upperRateBound_ = upperRateBound;
                defaultBounds_ = false;
                return *this;
            }

            Settings &withBSStdDevs(const Real stdDevs = 3.0) {
                strategy_ = BSStdDevs;
                stdDevs_ = stdDevs;
                lowerRateBound_ = defaultLowerBound;
                upperRateBound_ = defaultUpperBound;
                defaultBounds_ = true;
                return *this;
            }

            Settings &withBSStdDevs(const Real stdDevs,
                                    const Real lowerRateBound,
                                    const Real upperRateBound) {
                strategy_ = BSStdDevs;
                stdDevs_ = stdDevs;
                lowerRateBound_ = lowerRateBound;
                upperRateBound_ = upperRateBound;
                defaultBounds_ = false;
                return *this;
            }

            enum Strategy {
                RateBound,
                VegaRatio,
                PriceThreshold,
                BSStdDevs
            };

            Strategy strategy_ = RateBound;
            Real vegaRatio_ = 0.01;
            Real priceThreshold_ = 1.0E-8;
            Real stdDevs_ = 3.0;
            Real lowerRateBound_, upperRateBound_;
            bool defaultBounds_ = true;
        };


        LinearTsrPricer(
            const Handle<SwaptionVolatilityStructure>& swaptionVol,
            Handle<Quote> meanReversion,
            Handle<YieldTermStructure> couponDiscountCurve = Handle<YieldTermStructure>(),
            const Settings& settings = Settings(),
            std::shared_ptr<Integrator> integrator = std::shared_ptr<Integrator>());

        /* */
        Real swapletPrice() const override;
        Rate swapletRate() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;
        /* */
        Real meanReversion() const override;
        void setMeanReversion(const Handle<Quote>& meanReversion) override {
            unregisterWith(meanReversion_);
            meanReversion_ = meanReversion;
            registerWith(meanReversion_);
            update();
        }


      private:

        Real GsrG(const Date &d) const;
        Real singularTerms(Option::Type type, Real strike) const;
        Real integrand(Real strike) const;
        Real a_, b_;

        class integrand_f;

        class VegaRatioHelper {
          public:
            VegaRatioHelper(const SmileSection *section, const Real targetVega)
                : section_(section), targetVega_(targetVega) {}
            Real operator()(Real strike) const {
                return section_->vega(strike) - targetVega_;
            };
            const SmileSection *section_;
            const Real targetVega_;
        };

        class PriceHelper {
          public:
            PriceHelper(const SmileSection *section, const Option::Type type,
                        const Real targetPrice)
                : section_(section), targetPrice_(targetPrice), type_(type) {}
            Real operator()(Real strike) const {
                return section_->optionPrice(strike, type_) - targetPrice_;
            };
            const SmileSection *section_;
            const Real targetPrice_;
            const Option::Type type_;
        };

        void initialize(const FloatingRateCoupon& coupon) override;
        Real optionletPrice(Option::Type optionType, Real strike) const;
        Real strikeFromVegaRatio(Real ratio, Option::Type optionType,
                                 Real referenceStrike) const;
        Real strikeFromPrice(Real price, Option::Type optionType,
                             Real referenceStrike) const;

        Handle<Quote> meanReversion_;

        Handle<YieldTermStructure> forwardCurve_, discountCurve_;
        Handle<YieldTermStructure> couponDiscountCurve_;

        const CmsCoupon *coupon_;

        Date today_, paymentDate_, fixingDate_;

        Real gearing_, spread_;

        Period swapTenor_;
        Real spreadLegValue_, swapRateValue_, couponDiscountRatio_, annuity_;

        std::shared_ptr<SwapIndex> swapIndex_;
        std::shared_ptr<VanillaSwap> swap_;
        std::shared_ptr<SmileSection> smileSection_;

        Settings settings_;
        DayCounter volDayCounter_;
        std::shared_ptr<Integrator> integrator_;

        Real adjustedLowerBound_, adjustedUpperBound_;
    };
}

#endif
