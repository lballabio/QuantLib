/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

#include <ql/instruments/inflationswap.hpp>
#include <ql/event.hpp>

namespace QuantLib {

    InflationSwap::InflationSwap(const Date& start, const Date& maturity,
                                 const Period& lag, const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 const DayCounter& dayCounter)
    : start_(start), maturity_(maturity), lag_(lag), calendar_(calendar),
      bdc_(convention), dayCounter_(dayCounter) {

        baseDate_ = calendar_.adjust(start_ - lag_, bdc_);
        maturity_ = calendar_.adjust(maturity_, bdc_);
    }

    Date InflationSwap::baseDate() const {
        return baseDate_;
    }

    Period InflationSwap::lag() const {
        return lag_;
    }

    Date InflationSwap::startDate() const {
        return start_;
    }

    Date InflationSwap::maturityDate() const {
        return maturity_;
    }

    Calendar InflationSwap::calendar() const {
        return calendar_;
    }

    BusinessDayConvention InflationSwap::businessDayConvention() const {
        return bdc_;
    }

    DayCounter InflationSwap::dayCounter() const {
        return dayCounter_;
    }

    Rate InflationSwap::fairRate() const {
        calculate();
        QL_REQUIRE(fairRate_ != Null<Real>(), "fair rate not provided");
        return fairRate_;
    }

    bool InflationSwap::isExpired() const {
        return detail::simple_event(maturity_).hasOccurred();
    }

    void InflationSwap::setupArguments(PricingEngine::arguments* args) const {
        InflationSwap::arguments* arguments =
            dynamic_cast<InflationSwap::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->start = start_;
        arguments->maturity = maturity_;
        arguments->lag = lag_;
        arguments->calendar = calendar_;
        arguments->bdc = bdc_;
        arguments->dayCounter = dayCounter_;
        arguments->baseDate = baseDate_;
    }

    void InflationSwap::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const InflationSwap::results* results =
            dynamic_cast<const InflationSwap::results*>(r);
        QL_REQUIRE(results != 0, "wrong result type");

        fairRate_ = results->fairRate;
    }

    void InflationSwap::setupExpired() const {
        Instrument::setupExpired();
        fairRate_ = 0.0;
    }


    void InflationSwap::arguments::validate() const {
        QL_REQUIRE(start != Date(), "start date not provided");
        QL_REQUIRE(maturity != Date(), "maturity date not provided");
        QL_REQUIRE(lag != Period(), "lag not provided");
        QL_REQUIRE(!calendar.empty(), "calendar not provided");
        QL_REQUIRE(!dayCounter.empty(), "day counter not provided");
        QL_REQUIRE(baseDate != Date(), "base date not provided");
    }


    void InflationSwap::results::reset() {
        Instrument::results::reset();
        fairRate = Null<Rate>();
    }

}
