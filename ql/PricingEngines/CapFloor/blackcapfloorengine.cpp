/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Volatilities/capletconstantvol.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/Calendars/nullcalendar.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED

    BlackCapFloorEngine::BlackCapFloorEngine(
                                   const boost::shared_ptr<BlackModel>& model)
    : blackModel_(model) {
        Volatility vol = blackModel_->volatility();
        Handle<Quote> q(boost::shared_ptr<Quote>(new SimpleQuote(vol)));
        volatility_.linkTo(boost::shared_ptr<CapletVolatilityStructure>(
                          new CapletConstantVolatility(0, NullCalendar(),
                                                       q, Actual365Fixed())));
        registerWith(blackModel_);
    }

    #endif

    BlackCapFloorEngine::BlackCapFloorEngine(const Handle<Quote>& volatility) {
        volatility_.linkTo(boost::shared_ptr<CapletVolatilityStructure>(
                 new CapletConstantVolatility(0, NullCalendar(),
                                              volatility, Actual365Fixed())));
        registerWith(volatility_);
    }

    BlackCapFloorEngine::BlackCapFloorEngine(
                          const Handle<CapletVolatilityStructure>& volatility)
    : volatility_(volatility) {
        registerWith(volatility_);
    }

    void BlackCapFloorEngine::update() {
        #ifndef QL_DISABLE_DEPRECATED
        if (blackModel_) {
            Volatility vol = blackModel_->volatility();
            Handle<Quote> q(boost::shared_ptr<Quote>(new SimpleQuote(vol)));
            volatility_.linkTo(
                      boost::shared_ptr<CapletVolatilityStructure>(
                          new CapletConstantVolatility(0, NullCalendar(),
                                                       q, Actual365Fixed())));
        }
        #endif
        notifyObservers();
    }

    void BlackCapFloorEngine::calculate() const {
        Real value = 0.0;
        CapFloor::Type type = arguments_.type;

        for (Size i=0; i<arguments_.startTimes.size(); i++) {
            Time fixing = arguments_.fixingTimes[i],
                 end = arguments_.endTimes[i],
                 accrualTime = arguments_.accrualTimes[i];
            if (end > 0.0) {    // discard expired caplets
                Real nominal = arguments_.nominals[i];
                Real gearing = arguments_.gearings[i];
                DiscountFactor q = arguments_.discounts[i];
                Rate forward = arguments_.forwards[i];

                if ((type == CapFloor::Cap) ||
                    (type == CapFloor::Collar)) {
                    Rate strike = arguments_.capRates[i];
                    Volatility vol = volatility_->volatility(fixing, strike);
                    value += q * accrualTime * nominal * gearing *
                        capletValue(fixing,forward,strike,vol);
                }
                if ((type == CapFloor::Floor) ||
                    (type == CapFloor::Collar)) {
                    Rate strike = arguments_.floorRates[i];
                    Volatility vol = volatility_->volatility(fixing, strike);
                    Real temp = q * accrualTime * nominal * gearing *
                        floorletValue(fixing,forward,strike,vol);
                    if (type == CapFloor::Floor)
                        value += temp;
                    else
                        // a collar is long a cap and short a floor
                        value -= temp;
                }
            }
        }
        results_.value = value;
    }

    Real BlackCapFloorEngine::capletValue(Time start, Rate forward,
                                          Rate strike, Volatility vol) const {
        if (start <= 0.0) {
            // the rate was fixed
            return std::max<Rate>(forward-strike,0.0);
        } else {
            // forecast
            return detail::blackFormula(forward, strike,
                                        vol*std::sqrt(start), 1);
        }
    }

    Real BlackCapFloorEngine::floorletValue(Time start, Rate forward,
                                            Rate strike, Volatility vol)
                                                                       const {
        if (start <= 0.0) {
            // the rate was fixed
            return std::max<Rate>(strike-forward,0.0);
        } else {
            // forecast
            return detail::blackFormula(forward, strike,
                                        vol*std::sqrt(start), -1);
        }
    }

}
