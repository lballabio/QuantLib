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

#include <ql/instruments/yearonyearinflationswap.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    YearOnYearInflationSwap::YearOnYearInflationSwap(
                                         const Date& start,
                                         const Date& maturity,
                                         const Period& lag,
                                         Rate fixedRate,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         bool allowAmbiguousPayments,
                                         const Period& ambiguousPaymentPeriod)
    : InflationSwap(start, maturity, lag, calendar, convention, dayCounter),
      fixedRate_(fixedRate) {

        Schedule temp = MakeSchedule(start_, maturity_,
                                     Period(1,Years),
                                     calendar_, bdc_);
        paymentDates_.clear();
        paymentDates_.reserve(temp.size()-1);

        // the first payment date is the _second_ date in the schedule,
        // so we start from index 1
        for (Size i=1; i<temp.size(); ++i) {
            if (!allowAmbiguousPayments) {
                if (temp[i] > start_ + ambiguousPaymentPeriod) {
                    paymentDates_.push_back(temp[i]);
                }
            } else {
                paymentDates_.push_back(temp[i]);
            }
        }

        QL_REQUIRE(!paymentDates_.empty(),
                   " no payments dates, start " << start_
                    << ", maturity: " <<  maturity_);
    }

    Rate YearOnYearInflationSwap::fixedRate() const {
        return fixedRate_;
    }

    std::vector<Date> YearOnYearInflationSwap::paymentDates() const {
        return paymentDates_;
    }

    void YearOnYearInflationSwap::setupArguments(
                                       PricingEngine::arguments* args) const {
        InflationSwap::setupArguments(args);

        YearOnYearInflationSwap::arguments* arguments =
            dynamic_cast<YearOnYearInflationSwap::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->fixedRate = fixedRate_;
        arguments->paymentDates = paymentDates_;
    }


    void YearOnYearInflationSwap::arguments::validate() const {
        InflationSwap::arguments::validate();

        QL_REQUIRE(fixedRate != Null<Rate>(), "fixed rate not provided");
        QL_REQUIRE(!paymentDates.empty(), "payment dates not provided");
    }

}

