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
                        ConstantOptionletVol(volatility, Calendar(), dc))) {
        registerWith(termStructure_);
        registerWith(volatility_);
    }

    BlackCapFloorEngine::BlackCapFloorEngine(
                              const Handle<YieldTermStructure>& termStructure,
                              const Handle<Quote>& volatility,
                              const DayCounter& dc)
    : termStructure_(termStructure),
      volatility_(boost::shared_ptr<OptionletVolatilityStructure>(new
                        ConstantOptionletVol(volatility, Calendar(), dc))) {
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

        for (Size i=0; i<arguments_.startDates.size(); i++) {
            Date paymentDate = arguments_.endDates[i];
            Time accrualTime = arguments_.accrualTimes[i];
            // This is using the yield-curve day counter for
            // consistency with previous releases. Should it use the
            // volatility day counter?
            Time timeToMaturity =
                termStructure_->dayCounter().yearFraction(
                                            today, arguments_.fixingDates[i]);
            if (paymentDate > settlement) { // discard expired caplets
                Real nominal = arguments_.nominals[i];
                Real gearing = arguments_.gearings[i];
                DiscountFactor q = termStructure_->discount(paymentDate);
                Rate forward = arguments_.forwards[i];
                Real stdDev;

                // include caplets with past fixing date
                if (type == CapFloor::Cap || type == CapFloor::Collar) {
                    Rate strike = arguments_.capRates[i];
                    // std dev is set to 0 if fixing is at a past date
                    if (arguments_.fixingDates[i] > today) {
                        stdDev =
                             std::sqrt(volatility_->blackVariance(
                                          arguments_.fixingDates[i], strike));
                    } else {
                        stdDev = 0.0;
                    }
                    Real caplet = q * accrualTime * nominal * gearing *
                        blackFormula(Option::Call, strike, forward, stdDev);
                    optionletsPrice.push_back(caplet);
                    value += caplet;
                    // vega is set to 0 if fixing is at a past date
                    if (arguments_.fixingDates[i] > today) {
                        vega += nominal * gearing * accrualTime * q
                              * blackFormulaStdDevDerivative(strike, forward,
                                                             stdDev)
                              * std::sqrt(timeToMaturity);
                    }
                }
                if (type == CapFloor::Floor || type == CapFloor::Collar) {
                    Rate strike = arguments_.floorRates[i];
                    // std dev is set to 0 if fixing is at a past date
                    if (arguments_.fixingDates[i] > today) {
                        stdDev =
                            std::sqrt(volatility_->blackVariance(
                                          arguments_.fixingDates[i], strike));
                    } else {
                        stdDev = 0.0;
                    }
                    Real temp = q * accrualTime * nominal * gearing *
                        blackFormula(Option::Put, strike, forward, stdDev);
                    if (type == CapFloor::Floor) {
                        value += temp;
                        optionletsPrice.push_back(temp);
                        //vega is set to 0 if fixing is at a past date
                        if (arguments_.fixingDates[i] > today) {
                            vega += nominal * gearing * accrualTime * q
                                  * blackFormulaStdDevDerivative(strike,
                                                                 forward,
                                                                 stdDev)
                                  * std::sqrt(timeToMaturity);
                        }
                    } else {
                        // a collar is long a cap and short a floor
                        value -= temp;
                        // vega is set to 0 if fixing is at a past date
                        if (arguments_.fixingDates[i] > today) {
                            vega -= nominal * gearing * accrualTime * q
                                  * blackFormulaStdDevDerivative(strike,
                                                                 forward,
                                                                 stdDev)
                                  * std::sqrt(timeToMaturity);
                        }
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
