/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2015, 2016, 2017 Peter Caspers
 Copyright (C) 2017 Paul Giltinan
 Copyright (C) 2017 Werner Kuerzinger
 Copyright (C) 2020 Marcin Rybacki

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

/*! \file blackswaptionengine.hpp
    \brief Black-formula swaption engine
*/

#ifndef quantlib_pricers_black_swaption_hpp
#define quantlib_pricers_black_swaption_hpp

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/exercise.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <utility>

namespace QuantLib {

    class Quote;

    namespace detail {

    /*! Generic Black-style-formula swaption engine
        This is the base class for the Black and Bachelier swaption engines */
    template<class Spec>
    class BlackStyleSwaptionEngine : public Swaption::engine {
      public:
        enum CashAnnuityModel { SwapRate, DiscountCurve };
        BlackStyleSwaptionEngine(Handle<YieldTermStructure> discountCurve,
                                 Volatility vol,
                                 const DayCounter& dc = Actual365Fixed(),
                                 Real displacement = 0.0,
                                 CashAnnuityModel model = DiscountCurve);
        BlackStyleSwaptionEngine(Handle<YieldTermStructure> discountCurve,
                                 const Handle<Quote>& vol,
                                 const DayCounter& dc = Actual365Fixed(),
                                 Real displacement = 0.0,
                                 CashAnnuityModel model = DiscountCurve);
        BlackStyleSwaptionEngine(Handle<YieldTermStructure> discountCurve,
                                 Handle<SwaptionVolatilityStructure> vol,
                                 CashAnnuityModel model = DiscountCurve);
        void calculate() const override;
        Handle<YieldTermStructure> termStructure() { return discountCurve_; }
        Handle<SwaptionVolatilityStructure> volatility() { return vol_; }

      private:
        Handle<YieldTermStructure> discountCurve_;
        Handle<SwaptionVolatilityStructure> vol_;
        CashAnnuityModel model_;
    };

    // shifted lognormal type engine
    struct Black76Spec {
        static constexpr VolatilityType type = ShiftedLognormal;
        Real value(const Option::Type type, const Real strike,
                   const Real atmForward, const Real stdDev, const Real annuity,
                   const Real displacement) {
            return blackFormula(type, strike, atmForward, stdDev, annuity,
                                displacement);
        }
        Real vega(const Real strike, const Real atmForward, const Real stdDev,
                  const Real exerciseTime, const Real annuity,
                  const Real displacement) {
            return std::sqrt(exerciseTime) *
                   blackFormulaStdDevDerivative(strike, atmForward, stdDev,
                                                annuity, displacement);
        }
        Real delta(const Option::Type type, const Real strike,
                   const Real atmForward, const Real stdDev, const Real annuity,
                   const Real displacement) {
            return blackFormulaForwardDerivative(type, strike, atmForward, stdDev,
                                                 annuity, displacement);
        }
    };

    // normal type engine
    struct BachelierSpec {
        static constexpr VolatilityType type = Normal;
        Real value(const Option::Type type, const Real strike,
                   const Real atmForward, const Real stdDev, const Real annuity,
                   const Real) {
            return bachelierBlackFormula(type, strike, atmForward, stdDev,
                                         annuity);
        }
        Real vega(const Real strike, const Real atmForward, const Real stdDev,
                  const Real exerciseTime, const Real annuity, const Real) {
            return std::sqrt(exerciseTime) *
                   bachelierBlackFormulaStdDevDerivative(
                       strike, atmForward, stdDev, annuity);
        }
        Real delta(const Option::Type type, const Real strike,
                   const Real atmForward, const Real stdDev, const Real annuity,
                   const Real) {
            return bachelierBlackFormulaForwardDerivative(
                type, strike, atmForward, stdDev, annuity);
        }
    };

    } // namespace detail

    //! Shifted Lognormal Black-formula swaption engine
    /*! \ingroup swaptionengines

        \warning The engine assumes that the exercise date lies before the
                 start date of the passed swap.
    */

    class BlackSwaptionEngine
        : public detail::BlackStyleSwaptionEngine<detail::Black76Spec> {
      public:
        BlackSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            Volatility vol,
                            const DayCounter& dc = Actual365Fixed(),
                            Real displacement = 0.0,
                            CashAnnuityModel model = DiscountCurve);
        BlackSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            const Handle<Quote>& vol,
                            const DayCounter& dc = Actual365Fixed(),
                            Real displacement = 0.0,
                            CashAnnuityModel model = DiscountCurve);
        BlackSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            const Handle<SwaptionVolatilityStructure>& vol,
                            CashAnnuityModel model = DiscountCurve);
    };

    //! Normal Bachelier-formula swaption engine
    /*! \ingroup swaptionengines

        \warning The engine assumes that the exercise date lies before the
                 start date of the passed swap.
    */

    class BachelierSwaptionEngine
        : public detail::BlackStyleSwaptionEngine<detail::BachelierSpec> {
      public:
        BachelierSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                                Volatility vol,
                                const DayCounter& dc = Actual365Fixed(),
                                CashAnnuityModel model = DiscountCurve);
        BachelierSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                                const Handle<Quote>& vol,
                                const DayCounter& dc = Actual365Fixed(),
                                CashAnnuityModel model = DiscountCurve);
        BachelierSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                                const Handle<SwaptionVolatilityStructure>& vol,
                                CashAnnuityModel model = DiscountCurve);
    };

    // implementation

    namespace detail {

        template <class Spec>
        BlackStyleSwaptionEngine<Spec>::BlackStyleSwaptionEngine(
            Handle<YieldTermStructure> discountCurve,
            Volatility vol,
            const DayCounter& dc,
            Real displacement,
            CashAnnuityModel model)
        : discountCurve_(std::move(discountCurve)),
          vol_(ext::shared_ptr<SwaptionVolatilityStructure>(new ConstantSwaptionVolatility(
              0, NullCalendar(), Following, vol, dc, Spec().type, displacement))),
          model_(model) {
            registerWith(discountCurve_);
        }

        template <class Spec>
        BlackStyleSwaptionEngine<Spec>::BlackStyleSwaptionEngine(
            Handle<YieldTermStructure> discountCurve,
            const Handle<Quote>& vol,
            const DayCounter& dc,
            Real displacement,
            CashAnnuityModel model)
        : discountCurve_(std::move(discountCurve)),
          vol_(ext::shared_ptr<SwaptionVolatilityStructure>(new ConstantSwaptionVolatility(
              0, NullCalendar(), Following, vol, dc, Spec().type, displacement))),
          model_(model) {
            registerWith(discountCurve_);
            registerWith(vol_);
        }

        template <class Spec>
        BlackStyleSwaptionEngine<Spec>::BlackStyleSwaptionEngine(
            Handle<YieldTermStructure> discountCurve,
            Handle<SwaptionVolatilityStructure> volatility,
            CashAnnuityModel model)
        : discountCurve_(std::move(discountCurve)), vol_(std::move(volatility)), model_(model) {
            registerWith(discountCurve_);
            registerWith(vol_);
        }

    template<class Spec>
    void BlackStyleSwaptionEngine<Spec>::calculate() const {
        static const Spread basisPoint = 1.0e-4;

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not a European option");

        Date exerciseDate = arguments_.exercise->date(0);

        // The part of the swap preceding exerciseDate should be truncated to avoid taking into
        // account unwanted cashflows. For the moment we add a check avoiding this situation.
        // Furthermore, we take a copy of the underlying swap. This avoids notifying the swaption
        // when we set a pricing engine on the swap below.
        auto swap = arguments_.swap;

        const Leg& fixedLeg = swap->fixedLeg();
        ext::shared_ptr<FixedRateCoupon> firstCoupon =
            ext::dynamic_pointer_cast<FixedRateCoupon>(fixedLeg[0]);
        QL_REQUIRE(firstCoupon->accrualStartDate() >= exerciseDate,
                   "swap start (" << firstCoupon->accrualStartDate() << ") before exercise date ("
                                  << exerciseDate << ") not supported in Black swaption engine");

        Rate strike = swap->fixedRate();

        // using the discounting curve
        // swap.iborIndex() might be using a different forwarding curve
        auto engine = ext::make_shared<DiscountingSwapEngine>(discountCurve_, false);
        ObservableSettings::instance().disableUpdates();
        swap->setPricingEngine(engine);
        ObservableSettings::instance().enableUpdates();
        Date valuation_date = results_.valuationDate  = swap->valuationDate();
        Rate atmForward = swap->fairRate();

        // Volatilities are quoted for zero-spreaded swaps.
        // Therefore, any spread on the floating leg must be removed
        // with a corresponding correction on the fixed leg.
        Real spread = swap->spread();
        if (spread!=0.0) {
            Spread correction =
                spread * std::fabs(swap->floatingLegBPS() / swap->fixedLegBPS());
            strike -= correction;
            atmForward -= correction;
            results_.additionalResults["spreadCorrection"] = correction;
        } else {
            results_.additionalResults["spreadCorrection"] = Real(0.0);
        }
        results_.additionalResults["strike"] = strike;
        results_.additionalResults["atmForward"] = atmForward;

        Real annuity;
        if (arguments_.settlementType == Settlement::Physical ||
            (arguments_.settlementType == Settlement::Cash &&
             arguments_.settlementMethod ==
                 Settlement::CollateralizedCashPrice)) {
            annuity = std::fabs(swap->fixedLegBPS()) / basisPoint;
        } else if (arguments_.settlementType == Settlement::Cash &&
                   arguments_.settlementMethod == Settlement::ParYieldCurve) {
            DayCounter dayCount = firstCoupon->dayCounter();
            // we assume that the cash settlement date is equal
            // to the swap start date
            Date discountDate = model_ == DiscountCurve
                                    ? firstCoupon->accrualStartDate()
                                    : valuation_date;
            Frequency freq = Annual;
            const Schedule& fixedSchedule = swap->fixedSchedule();
            if (fixedSchedule.hasTenor()) {
                freq = fixedSchedule.tenor().frequency();
            }
            Real fixedLegCashBPS =
                CashFlows::bps(fixedLeg,
                        InterestRate(atmForward, dayCount, Compounded, freq),
                        false, discountDate);
            annuity = std::fabs(fixedLegCashBPS / basisPoint) *
                      discountCurve_->discount(discountDate);
        } else {
            QL_FAIL("invalid (settlementType, settlementMethod) pair");
        }
        results_.additionalResults["annuity"] = annuity;

        const Schedule& floatingSchedule = swap->floatingSchedule();
        Time swapLength =  vol_->swapLength(floatingSchedule.dates().front(),
                                            floatingSchedule.dates().back());

        // swapLength is rounded to whole months. To ensure we can read a variance
        // and a shift from vol_ we floor swapLength at 1/12 here therefore.
        swapLength = std::max(swapLength, 1.0 / 12.0);
        results_.additionalResults["swapLength"] = swapLength;

        Real variance = vol_->blackVariance(exerciseDate, swapLength, strike);

        Real displacement =
            vol_->volatilityType() == ShiftedLognormal ?
            vol_->shift(exerciseDate, swapLength) : 0.0;

        Real stdDev = std::sqrt(variance);
        results_.additionalResults["stdDev"] = stdDev;
        Option::Type w = (swap->type() == Swap::Payer) ? Option::Call : Option::Put;
        results_.value = Spec().value(w, strike, atmForward, stdDev, annuity, displacement);

        Time exerciseTime = vol_->timeFromReference(exerciseDate);
        results_.additionalResults["vega"] = Spec().vega(
            strike, atmForward, stdDev, exerciseTime, annuity, displacement);
        results_.additionalResults["delta"] = Spec().delta(
            w, strike, atmForward, stdDev, annuity, displacement);
        results_.additionalResults["timeToExpiry"] = exerciseTime;
        results_.additionalResults["impliedVolatility"] = Real(stdDev / std::sqrt(exerciseTime));
        results_.additionalResults["forwardPrice"] = results_.value / discountCurve_->discount(exerciseDate);
    }

    }  // namespace detail

}

#endif
