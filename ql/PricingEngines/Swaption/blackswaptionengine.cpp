/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
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

#include <ql/PricingEngines/Swaption/blackswaptionengine.hpp>
#include <ql/Volatilities/swaptionconstantvol.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/Calendars/nullcalendar.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED

    BlackSwaptionEngine::BlackSwaptionEngine(
                                   const boost::shared_ptr<BlackModel>& model)
    : blackModel_(model) {
        Volatility vol = blackModel_->volatility();
        Handle<Quote> q(boost::shared_ptr<Quote>(new SimpleQuote(vol)));
        volatility_.linkTo(boost::shared_ptr<SwaptionVolatilityStructure>(
                        new SwaptionConstantVolatility(0, NullCalendar(),
                                                       q, Actual365Fixed())));
        registerWith(blackModel_);
    }

    #endif

    BlackSwaptionEngine::BlackSwaptionEngine(const Handle<Quote>& volatility) {
        volatility_.linkTo(boost::shared_ptr<SwaptionVolatilityStructure>(
               new SwaptionConstantVolatility(0, NullCalendar(),
                                              volatility, Actual365Fixed())));
        registerWith(volatility_);
    }

    BlackSwaptionEngine::BlackSwaptionEngine(
                        const Handle<SwaptionVolatilityStructure>& volatility)
    : volatility_(volatility) {
        registerWith(volatility_);
    }

    void BlackSwaptionEngine::update() {
        #ifndef QL_DISABLE_DEPRECATED
        if (blackModel_) {
            Volatility vol = blackModel_->volatility();
            Handle<Quote> q(boost::shared_ptr<Quote>(new SimpleQuote(vol)));
            volatility_.linkTo(
                    boost::shared_ptr<SwaptionVolatilityStructure>(
                        new SwaptionConstantVolatility(0, NullCalendar(),
                                                       q, Actual365Fixed())));
        }
        #endif
        notifyObservers();
    }

    void BlackSwaptionEngine::calculate() const
	{
        static const Spread basisPoint = 1.0e-4;
        Time exercise = arguments_.stoppingTimes[0];
        Time maturity = arguments_.floatingPayTimes.back();
        Real w = arguments_.payFixed ? 1.0 : -1.0;
        Real annuity;
        switch(arguments_.settlementType) {
          case Swaption::Physical :
            annuity = arguments_.fixedBPS/basisPoint;
            break;
          case Swaption::Cash :
            annuity = arguments_.fixedCashBPS/basisPoint;
            break;
          default:
            QL_FAIL("unknown settlement type");
        }
        Volatility vol = volatility_->volatility(exercise,
                                                 maturity-exercise,
                                                 arguments_.fixedRate);
        results_.value = annuity * detail::blackFormula(
                                 arguments_.fairRate, arguments_.fixedRate,
                                 vol*std::sqrt(exercise), w);
    }

}
