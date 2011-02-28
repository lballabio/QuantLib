/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

#include <ql/experimental/callablebonds/callablebondconstantvol.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    CallableBondConstantVolatility::CallableBondConstantVolatility(
                                                 const Date& referenceDate,
                                                 Volatility volatility,
                                                 const DayCounter& dayCounter)
    : CallableBondVolatilityStructure(referenceDate),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(volatility))),
      dayCounter_(dayCounter), maxBondTenor_(100*Years) {}

    CallableBondConstantVolatility::CallableBondConstantVolatility(
                                              const Date& referenceDate,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : CallableBondVolatilityStructure(referenceDate), volatility_(volatility),
      dayCounter_(dayCounter), maxBondTenor_(100*Years) {
        registerWith(volatility_);
    }

    CallableBondConstantVolatility::CallableBondConstantVolatility(
                                                 Natural settlementDays,
                                                 const Calendar& calendar,
                                                 Volatility volatility,
                                                 const DayCounter& dayCounter)
    : CallableBondVolatilityStructure(settlementDays, calendar),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(volatility))),
      dayCounter_(dayCounter), maxBondTenor_(100*Years) {}

    CallableBondConstantVolatility::CallableBondConstantVolatility(
                                              Natural settlementDays,
                                              const Calendar& calendar,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : CallableBondVolatilityStructure(settlementDays, calendar),
      volatility_(volatility), dayCounter_(dayCounter),
      maxBondTenor_(100*Years) {
        registerWith(volatility_);
    }

    Volatility CallableBondConstantVolatility::volatilityImpl(const Date&,
                                                              const Period&,
                                                              Rate) const {
        return volatility_->value();
    }

    Volatility CallableBondConstantVolatility::volatilityImpl(
                                                     Time, Time, Rate) const {
        return volatility_->value();
    }


    boost::shared_ptr<SmileSection>
    CallableBondConstantVolatility::smileSectionImpl(Time optionTime,
                                                     Time) const {
        Volatility atmVol = volatility_->value();
        return boost::shared_ptr<SmileSection>(
                                    new FlatSmileSection(optionTime,
                                                         atmVol,
                                                         dayCounter_));
    }

}

