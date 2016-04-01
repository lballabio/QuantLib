/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2015, 2016 Peter Caspers

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

#include <ql/instruments/swaption.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    class Quote;

    namespace detail {

    /*! Generic Black-style-formula swaption engine
        This is the base class for the Black and Bachelier swaption engines */
    template<class Spec>
    class BlackStyleSwaptionEngine : public Swaption::engine {
      public:
        BlackStyleSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            Volatility vol,
                            const DayCounter& dc = Actual365Fixed(),
                            Real displacement = 0.0);
        BlackStyleSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            const Handle<Quote>& vol,
                            const DayCounter& dc = Actual365Fixed(),
                            Real displacement = 0.0);
        /*! if displacement is Null<Real>(), it is read from the volatility
          structure, the parameter can be removed once the deprecated methods
          overriding the displacement are deleted */
        BlackStyleSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            const Handle<SwaptionVolatilityStructure>& vol,
                            Real displacement = Null<Real>());
        void calculate() const;
        Handle<YieldTermStructure> termStructure() { return discountCurve_; }
        Handle<SwaptionVolatilityStructure> volatility() { return vol_; }

      private:
        Handle<YieldTermStructure> discountCurve_;
        Handle<SwaptionVolatilityStructure> vol_;

      protected:
        // can be removed once the deprecated methods overriding the
        // displacement are deleted
        Real displacement_;
    };

    // shifted lognormal type engine
    struct Black76Spec {
        static const VolatilityType type = ShiftedLognormal;
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
    };

    // normal type engine
    struct BachelierSpec {
        static const VolatilityType type = Normal;
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
    };

    } // namespace detail

    //! Shifted Lognormal Black-formula swaption engine
    /*! \ingroup swaptionengines

        \warning The engine assumes that the exercise date equals the
                 start date of the passed swap.
    */

    class BlackSwaptionEngine
        : public detail::BlackStyleSwaptionEngine<detail::Black76Spec> {
      public:
        BlackSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            Volatility vol,
                            const DayCounter& dc = Actual365Fixed(),
                            Real displacement = 0.0);
        BlackSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            const Handle<Quote>& vol,
                            const DayCounter& dc = Actual365Fixed(),
                            Real displacement = 0.0);
        BlackSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            const Handle<SwaptionVolatilityStructure>& vol);

        /*! \deprecated
          overrides displacement from given volatility structure,
          this is not recommended to do */
        QL_DEPRECATED
        BlackSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                            const Handle<SwaptionVolatilityStructure>& vol,
                            Real displacement);

        /*! \deprecated
          might return Null<Real>(), if given by a volatility structure,
          use volatility()->shift() to get the displacement instead */
        QL_DEPRECATED
        Real displacement() { return displacement_; }
    };

    //! Normal Bachelier-formula swaption engine
    /*! \ingroup swaptionengines

        \warning The engine assumes that the exercise date equals the
                 start date of the passed swap.
    */

    class BachelierSwaptionEngine
        : public detail::BlackStyleSwaptionEngine<detail::BachelierSpec> {
      public:
        BachelierSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                                Volatility vol,
                                const DayCounter& dc = Actual365Fixed());
        BachelierSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                                const Handle<Quote>& vol,
                                const DayCounter& dc = Actual365Fixed());
        BachelierSwaptionEngine(const Handle<YieldTermStructure>& discountCurve,
                                const Handle<SwaptionVolatilityStructure>& vol);
    };

    // implementation

    namespace detail {

    template<class Spec>
    BlackStyleSwaptionEngine<Spec>::BlackStyleSwaptionEngine(
        const Handle<YieldTermStructure> &discountCurve, Volatility vol,
        const DayCounter &dc, Real displacement)
        : discountCurve_(discountCurve),
          vol_(boost::shared_ptr<SwaptionVolatilityStructure>(
              new ConstantSwaptionVolatility(0, NullCalendar(), Following, vol,
                                             dc, Spec().type, displacement))),
          displacement_(displacement) {
        registerWith(discountCurve_);
    }

    template<class Spec>
    BlackStyleSwaptionEngine<Spec>::BlackStyleSwaptionEngine(
        const Handle<YieldTermStructure> &discountCurve,
        const Handle<Quote> &vol, const DayCounter &dc, Real displacement)
        : discountCurve_(discountCurve),
          vol_(boost::shared_ptr<SwaptionVolatilityStructure>(
              new ConstantSwaptionVolatility(0, NullCalendar(), Following, vol,
                                             dc, Spec().type, displacement))),
          displacement_(displacement) {
        registerWith(discountCurve_);
        registerWith(vol_);
    }

    template<class Spec>
    BlackStyleSwaptionEngine<Spec>::BlackStyleSwaptionEngine(
        const Handle<YieldTermStructure> &discountCurve,
        const Handle<SwaptionVolatilityStructure> &volatility,
        Real displacement)
        : discountCurve_(discountCurve), vol_(volatility),
          displacement_(displacement) {
        registerWith(discountCurve_);
        registerWith(vol_);
    }

    template<class Spec>
    void BlackStyleSwaptionEngine<Spec>::calculate() const {
        static const Spread basisPoint = 1.0e-4;

        Date exerciseDate = arguments_.exercise->date(0);

        // the part of the swap preceding exerciseDate should be truncated
        // to avoid taking into account unwanted cashflows
        VanillaSwap swap = *arguments_.swap;

        Rate strike = swap.fixedRate();

        // using the discounting curve
        // swap.iborIndex() might be using a different forwarding curve
        swap.setPricingEngine(boost::shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(discountCurve_, false)));
        Rate atmForward = swap.fairRate();

        // Volatilities are quoted for zero-spreaded swaps.
        // Therefore, any spread on the floating leg must be removed
        // with a corresponding correction on the fixed leg.
        if (swap.spread()!=0.0) {
            Spread correction = swap.spread() *
                std::fabs(swap.floatingLegBPS()/swap.fixedLegBPS());
            strike -= correction;
            atmForward -= correction;
            results_.additionalResults["spreadCorrection"] = correction;
        } else {
            results_.additionalResults["spreadCorrection"] = 0.0;
        }
        results_.additionalResults["strike"] = strike;
        results_.additionalResults["atmForward"] = atmForward;

        // using the discounting curve
        swap.setPricingEngine(boost::shared_ptr<PricingEngine>(
                           new DiscountingSwapEngine(discountCurve_, false)));
        Real annuity;
        switch(arguments_.settlementType) {
          case Settlement::Physical: {
              annuity = std::fabs(swap.fixedLegBPS())/basisPoint;
              break;
          }
          case Settlement::Cash: {
              const Leg& fixedLeg = swap.fixedLeg();
              boost::shared_ptr<FixedRateCoupon> firstCoupon =
                  boost::dynamic_pointer_cast<FixedRateCoupon>(fixedLeg[0]);
              DayCounter dayCount = firstCoupon->dayCounter();
              Real fixedLegCashBPS =
                  CashFlows::bps(fixedLeg,
                                 InterestRate(atmForward, dayCount, Compounded, Annual),
                                 false, discountCurve_->referenceDate()) ;
              annuity = std::fabs(fixedLegCashBPS/basisPoint);
              break;
          }
          default:
            QL_FAIL("unknown settlement type");
        }
        results_.additionalResults["annuity"] = annuity;

        // the swap length calculation might be improved using the value date
        // of the exercise date
        Time swapLength =  vol_->swapLength(exerciseDate,
                                                   arguments_.floatingPayDates.back());
        results_.additionalResults["swapLength"] = swapLength;

        Real variance = vol_->blackVariance(exerciseDate,
                                                   swapLength,
                                                   strike);

        // once the deprecated methods allowing to override the displacement
        // are gone, we can avoid this and directly read the displacement
        // from the volatility structure
        Real displacement = displacement_ == Null<Real>()
                                ? vol_->shift(exerciseDate, swapLength)
                                : displacement_;

        Real stdDev = std::sqrt(variance);
        results_.additionalResults["stdDev"] = stdDev;
        Option::Type w = (arguments_.type==VanillaSwap::Payer) ?
                                                Option::Call : Option::Put;
        results_.value = Spec().value(w, strike, atmForward, stdDev, annuity,
                                                                displacement);

        Time exerciseTime = vol_->timeFromReference(exerciseDate);
        results_.additionalResults["vega"] = Spec().vega(
            strike, atmForward, stdDev, exerciseTime, annuity, displacement);
    }

    }  // namespace detail

}

#endif
