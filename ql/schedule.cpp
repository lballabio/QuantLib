/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/schedule.hpp>

namespace QuantLib {

    Schedule::Schedule(const Calendar& calendar,
                       const Date& startDate, const Date& endDate,
                       Frequency frequency,
                       BusinessDayConvention convention,
                       const Date& stubDate, bool startFromEnd,
                       bool longFinal)
    : calendar_(calendar), frequency_(Frequency(frequency)),
      convention_(convention), stubDate_(stubDate),
      startFromEnd_(startFromEnd), longFinal_(longFinal),
      finalIsRegular_(true) {

        // sanity checks
        QL_REQUIRE(startDate != Date(), "null start date");
        QL_REQUIRE(endDate != Date(),   "null end date");
        QL_REQUIRE(startDate < endDate,
                   "start date (" << startDate
                   << ") later than end date (" << endDate << ")");
        if (stubDate != Date()) {
            QL_REQUIRE((stubDate > startDate && stubDate < endDate),
                       "stub date (" << stubDate
                       << ") out of range (start date (" << startDate
                       << "), end date (" << endDate << "))");
        }
        QL_REQUIRE(frequency == 0 || 12 % frequency == 0,
                   "frequency (" << Integer(frequency)
                   << " per year) does not correspond to "
                   << "a whole number of months");

        if (frequency == 0) {
            QL_REQUIRE(stubDate == Date(),
                       "stub date incompatible with frequency 'once'");
            dates_.push_back(calendar.adjust(startDate, convention));
            dates_.push_back(calendar.adjust(endDate, convention));
        } else if (startFromEnd) {
            // calculations
            Date seed = endDate;
            Date first = calendar.adjust(startDate, convention);
            // add end date
            dates_.push_back(calendar.adjust(endDate, convention));

            // add stub date if given
            if (stubDate != Date()) {
                seed = stubDate;
                dates_.insert(dates_.begin(),
                              calendar.adjust(stubDate, convention));
            }

            // add subsequent dates
            Integer periods = 1, months = 12/frequency;
            while (true) {
                Date temp = calendar.advance(seed, -periods*months, Months,
                                             convention);
                dates_.insert(dates_.begin(),temp);
                // check exit condition
                if (temp <= first)
                    break;
                else
                    periods++;
            }

            Size N = dates_.size();
            // possibly correct first inserted date
            if (dates_[0] < first) {
                dates_[0] = first;
                if (N > 1 && longFinal)
                    dates_.erase(dates_.begin()+1);
                finalIsRegular_ = false;
            }

            // possibly collapse first two dates
            if (N > 1 && calendar.adjust(dates_[0], convention) ==
                calendar.adjust(dates_[1], convention)) {
                dates_[1] = dates_[0];
                dates_.erase(dates_.begin());
                finalIsRegular_ = true;
            }
        } else {
            // calculations
            Date seed = startDate;
            Date last = calendar.adjust(endDate, convention);
            // add start date
            dates_.push_back(calendar.adjust(startDate, convention));

            // add stub date if given
            if (stubDate != Date()) {
                seed = stubDate;
                dates_.push_back(calendar.adjust(stubDate, convention));
            }

            // add subsequent dates
            Integer periods = 1, months = 12/frequency;
            while (true) {
                Date temp = calendar.advance(seed, periods*months, Months,
                                             convention);
                dates_.push_back(temp);
                // check exit condition
                if (temp >= last)
                    break;
                else
                    periods++;
            }

            Size N = dates_.size();
            // possibly correct last inserted date
            if (dates_.back() > last) {
                if (N > 1 && longFinal)
                    dates_.pop_back();
                dates_.back() = last;
                finalIsRegular_ = false;
            }

            // possibly collapse last two dates
            if (N > 1 && calendar.adjust(dates_[N-2], convention) ==
                calendar.adjust(dates_[N-1], convention)) {
                dates_[N-2] = dates_[N-1];
                dates_.pop_back();
                finalIsRegular_ = true;
            }
        }
    }


    bool Schedule::isRegular(Size i) const {
        if (frequency_ == 0) {
            return true;
        } else if (startFromEnd_) {
            if (i == 1)
                return finalIsRegular_;
            else if (i == size()-1)
                return (stubDate_ == Date());
            else
                return true;
        } else {
            if (i == 1)
                return (stubDate_ == Date());
            else if (i == size()-1)
                return finalIsRegular_;
            else
                return true;
        }
        QL_DUMMY_RETURN(true);
    }

}
