/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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

#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

namespace QuantLib {

    BlackCapFloorEngine::BlackCapFloorEngine(
                              const Handle<YieldTermStructure>& termStructure,
                              Volatility volatility,
                              const DayCounter& dc)
    : termStructure_(termStructure),
      volatility_(boost::shared_ptr<OptionletVolatilityStructure>(new
          ConstantOptionletVolatility(0, volatility, dc,
                                      NullCalendar(), Following))) {
        registerWith(termStructure_);
    }

    BlackCapFloorEngine::BlackCapFloorEngine(
                              const Handle<YieldTermStructure>& termStructure,
                              const Handle<Quote>& volatility,
                              const DayCounter& dc)
    : termStructure_(termStructure),
      volatility_(boost::shared_ptr<OptionletVolatilityStructure>(new
          ConstantOptionletVolatility(0, volatility, dc,
                                      NullCalendar(), Following))) {
        registerWith(termStructure_);
        registerWith(volatility_);
    }

    BlackCapFloorEngine::BlackCapFloorEngine(
                       const Handle<YieldTermStructure>& termStructure,
                       const Handle<OptionletVolatilityStructure>& volatility)
    : termStructure_(termStructure), volatility_(volatility) {
        registerWith(termStructure_);
        registerWith(volatility_);
    }

    void BlackCapFloorEngine::calculate() const {
        Real value = 0.0;
        Real vega = 0.0;
        std::vector<Real> optionletsPrice;
        CapFloor::Type type = arguments_.type;
        Date today = volatility_->referenceDate();
        Date settlement = termStructure_->referenceDate();

        for (Size i=0; i<arguments_.startDates.size(); ++i) {
            Date paymentDate = arguments_.endDates[i];
            if (paymentDate > settlement) { // discard expired caplets
                DiscountFactor d = arguments_.nominals[i] *
                                   arguments_.gearings[i] *
                                   termStructure_->discount(paymentDate) *
                                   arguments_.accrualTimes[i];

                Rate forward = arguments_.forwards[i];

                Date fixingDate = arguments_.fixingDates[i];
                Time sqrtTime = 0.0;
                if (fixingDate > today)
                    sqrtTime = std::sqrt(volatility_->timeFromReference(fixingDate));

                Real stdDev = 0.0;

                // include caplets with past fixing date
                if (type == CapFloor::Cap || type == CapFloor::Collar) {
                    Rate strike = arguments_.capRates[i];
                    // std dev is set to 0 if fixing is at a past date
                    if (sqrtTime>0.0)
                        stdDev = std::sqrt(
                            volatility_->blackVariance(fixingDate, strike));
                    Real caplet =
                        blackFormula(Option::Call, strike, forward, stdDev, d);
                    optionletsPrice.push_back(caplet);
                    value += caplet;
                    if (sqrtTime>0.0)
                        vega += blackFormulaStdDevDerivative(
                            strike, forward, stdDev, d) * sqrtTime;
                }
                if (type == CapFloor::Floor || type == CapFloor::Collar) {
                    Rate strike = arguments_.floorRates[i];
                    // std dev is set to 0 if fixing is at a past date
                    if (sqrtTime>0.0)
                        stdDev = std::sqrt(
                            volatility_->blackVariance(fixingDate, strike));
                    Real floorlet =
                        blackFormula(Option::Put, strike, forward, stdDev, d);
                    if (type == CapFloor::Floor) {
                        value += floorlet;
                        optionletsPrice.push_back(floorlet);
                        if (sqrtTime>0.0)
                            vega += blackFormulaStdDevDerivative(
                                strike, forward, stdDev, d) * sqrtTime;
                    } else {
                        // a collar is long a cap and short a floor
                        value -= floorlet;
                        optionletsPrice[optionletsPrice.size()-1] -= floorlet;
                        if (sqrtTime>0.0)
                            vega += blackFormulaStdDevDerivative(
                                strike, forward, stdDev, d) * sqrtTime;
                    }
                }
            }
        }
        results_.value = value;
        results_.additionalResults["optionletsPrice"] = optionletsPrice;
        results_.additionalResults["vega"] = vega;
    }

    Handle<YieldTermStructure> BlackCapFloorEngine::termStructure() {
        return termStructure_;
    }

    Handle<OptionletVolatilityStructure> BlackCapFloorEngine::volatility() {
        return volatility_;
    }

}
