/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
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
#include <ql/termstructures/volatilities/interestrate/swaption/swaptionconstantvol.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    BlackSwaptionEngine::BlackSwaptionEngine(const Handle<Quote>& volatility,
                            const Handle<YieldTermStructure>& discountCurve)
    : volatility_(boost::shared_ptr<SwaptionVolatilityStructure>(new
                  SwaptionConstantVolatility(0, NullCalendar(),
                                             volatility, Actual365Fixed()))),
      discountCurve_(discountCurve) {
        registerWith(volatility_);
        registerWith(discountCurve_);
    }

    BlackSwaptionEngine::BlackSwaptionEngine(
                        const Handle<SwaptionVolatilityStructure>& volatility,
                        const Handle<YieldTermStructure>& discountCurve)
    : volatility_(volatility),
      discountCurve_(discountCurve) {
        registerWith(volatility_);
        registerWith(discountCurve_);
    }

    void BlackSwaptionEngine::update()
    {
        notifyObservers();
    }


    void BlackSwaptionEngine::calculate() const
    {
        static const Spread basisPoint = 1.0e-4;
        Time exercise = arguments_.stoppingTimes[0];
        Time maturity = arguments_.floatingPayTimes.back();
        Real annuity;
        switch(arguments_.settlementType) {
          case Settlement::Physical :
            annuity = arguments_.fixedBPS/basisPoint;
            break;
          case Settlement::Cash :
            annuity = arguments_.fixedCashBPS/basisPoint;
            break;
          default:
            QL_FAIL("unknown settlement type");
        }

        // FIXME: not coherent with the way volatility_ would calculate it
        Time swapLength = maturity-exercise;

        Volatility vol = volatility_->volatility(exercise,
                                                 swapLength,
                                                 arguments_.fixedRate);
        Option::Type w = (arguments_.type==VanillaSwap::Payer) ?
                                                Option::Call : Option::Put;
        Real forecastingDiscount = arguments_.forecastingDiscount;
        Real discount = discountCurve_->discount(maturity);
        results_.value = annuity * blackFormula(w, arguments_.fixedRate,
                                                arguments_.fairRate,
                                                vol*std::sqrt(exercise))*
                         discount / forecastingDiscount;
        Real variance = volatility_->blackVariance(exercise,
                                                   swapLength,
                                                   arguments_.fixedRate);
        Real stdDev = std::sqrt(variance);
        Rate forward = arguments_.fairRate;
        Rate strike = arguments_.fixedRate;
        results_.additionalResults["vega"] = std::sqrt(exercise) *
            blackFormulaStdDevDerivative(strike, forward, stdDev, annuity)*
                         discount / forecastingDiscount;
    }

    Handle<YieldTermStructure> BlackSwaptionEngine::termStructure() {
        return discountCurve_;
    }

    Handle<SwaptionVolatilityStructure> BlackSwaptionEngine::volatility() {
        return volatility_;
    }

}
