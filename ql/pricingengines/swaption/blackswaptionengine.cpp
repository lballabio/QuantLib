/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006, 2007 StatPro Italia srl

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

#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    BlackSwaptionEngine::BlackSwaptionEngine(
                              const Handle<YieldTermStructure>& termStructure,
                              Volatility volatility,
                              const DayCounter& dc)
    : termStructure_(termStructure),
      volatility_(boost::shared_ptr<SwaptionVolatilityStructure>(new
          ConstantSwaptionVolatility(0, volatility, dc,
                                     NullCalendar(), Following))) {
        registerWith(termStructure_);
    }

    BlackSwaptionEngine::BlackSwaptionEngine(
                            const Handle<YieldTermStructure>& termStructure,
                            const Handle<Quote>& volatility,
                            const DayCounter& dc)
    : termStructure_(termStructure),
      volatility_(boost::shared_ptr<SwaptionVolatilityStructure>(new
          ConstantSwaptionVolatility(0, volatility, dc,
                                     NullCalendar(), Following))) {
        registerWith(termStructure_);
        registerWith(volatility_);
    }

    BlackSwaptionEngine::BlackSwaptionEngine(
                        const Handle<YieldTermStructure>& termStructure,
                        const Handle<SwaptionVolatilityStructure>& volatility)
    : termStructure_(termStructure),
      volatility_(volatility) {
        registerWith(termStructure_);
        registerWith(volatility_);
    }

    void BlackSwaptionEngine::calculate() const {
        static const Spread basisPoint = 1.0e-4;

        // the part of the swap preceding the exercise date should be truncated
        VanillaSwap swap = *arguments_.swap;
        swap.setPricingEngine(boost::shared_ptr<PricingEngine>(new
                                    DiscountingSwapEngine(termStructure_)));

        // Volatilities are quoted for zero-spreaded swaps.
        // Therefore, any spread on the floating leg must be removed
        // with a corresponding correction on the fixed leg.
        Spread correction = swap.spread() *
            std::fabs(swap.floatingLegBPS()/swap.fixedLegBPS());
        Rate strike = swap.fixedRate() - correction;
        Rate forward = swap.fairRate() - correction;

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
              Real fixedCashBPS =
                  CashFlows::bps(fixedLeg,
                                 InterestRate(forward, dayCount, Compounded),
                                 termStructure_->referenceDate()) ;
              annuity = fixedCashBPS/basisPoint;
              break;
          }
          default:
            QL_FAIL("unknown settlement type");
        }

        Date exerciseDate = arguments_.exercise->date(0);

        // the swap length calculation might be improved using the value date
        // of the exercise date
        Time swapLength =  volatility_->swapLength(exerciseDate,
                                                   arguments_.floatingPayDates.back());

        Real variance = volatility_->blackVariance(exerciseDate,
                                                   swapLength,
                                                   strike);
        Real stdDev = std::sqrt(variance);
        Option::Type w = (arguments_.type==VanillaSwap::Payer) ?
                                                Option::Call : Option::Put;
        results_.value = blackFormula(w, strike, forward, stdDev, annuity);

        Time exerciseTime = volatility_->timeFromReference(exerciseDate);
        results_.additionalResults["vega"] = std::sqrt(exerciseTime) *
            blackFormulaStdDevDerivative(strike, forward, stdDev, annuity);
    }

    Handle<YieldTermStructure> BlackSwaptionEngine::termStructure() {
        return termStructure_;
    }

    Handle<SwaptionVolatilityStructure> BlackSwaptionEngine::volatility() {
        return volatility_;
    }

}
