
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file scheduler.cpp
    \brief date scheduler
*/

#include <ql/scheduler.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    Schedule::Schedule(const Calendar& calendar,
                       const Date& startDate, const Date& endDate, 
                       int frequency,
                       RollingConvention rollingConvention, 
                       bool isAdjusted,
                       const Date& stubDate, bool startFromEnd, 
                       bool longFinal)
    : calendar_(calendar), frequency_(frequency), 
      rollingConvention_(rollingConvention),
      isAdjusted_(isAdjusted), stubDate_(stubDate),
      startFromEnd_(startFromEnd),longFinal_(longFinal),
      finalIsRegular_(true) {
        // sanity checks
        QL_REQUIRE(startDate != Date(), "null start date");
        QL_REQUIRE(endDate != Date(),   "null end date");
        QL_REQUIRE(startDate < endDate,
                   "start date (" +
                   DateFormatter::toString(startDate) +
                   ") later than end date (" +
                   DateFormatter::toString(endDate) +
                   ")");
        if (stubDate_ != Date()) {
            QL_REQUIRE((stubDate_ > startDate && stubDate_ < endDate),
                       "stub date (" +
                       DateFormatter::toString(stubDate_) +
                       ") out of range (start date (" +
                       DateFormatter::toString(startDate) +
                       "), end date (" +
                       DateFormatter::toString(endDate) + "))");
        }
        QL_REQUIRE(12 % frequency_ == 0,
                   "frequency (" +
                   IntegerFormatter::toString(frequency_) +
                   " per year) does not correspond to "
                   "a whole number of months");

        if (startFromEnd_) {
            // calculations
            Date seed = endDate;
            Date first = (isAdjusted_ ?
                          calendar_.roll(startDate,rollingConvention_) :
                          startDate);
            // add end date
            dates_.push_back(endDate);
	   
            // add stub date if given
            if (stubDate_ != Date()) {
                seed = stubDate_;
                dates_.insert(dates_.begin(),
                              isAdjusted_ ? 
                              calendar_.roll(stubDate_) : 
                              stubDate_);
            }
	   
            // add subsequent dates
            int periods = 1, months = 12/frequency_;
            while (true) {
                Date temp = seed.plus(-periods*months,Months);
                if (isAdjusted_)
                    temp = calendar_.roll(temp,rollingConvention_);
                dates_.insert(dates_.begin(),temp);
                // check exit condition
                if (temp <= first)
                    break;
                else
                    periods++;
            }
	   
            // possibly correct first inserted date
            if (dates_[0] < first) {
                dates_[0] = first;
                if (longFinal_)
                    dates_.erase(dates_.begin()+1);
                finalIsRegular_ = false;
            }

            // possibly collapse first two dates
            if (calendar_.roll(dates_[0],rollingConvention_) ==
                calendar_.roll(dates_[1],rollingConvention_)) {
                dates_[1] = dates_[0];
                dates_.erase(dates_.begin());
                finalIsRegular_ = true;
            }
        } else {
            // calculations
            Date seed = startDate;
            Date last = (isAdjusted_ ?
                         calendar_.roll(endDate,rollingConvention_) :
                         endDate);
            // add start date
            dates_.push_back(startDate);
	   
            // add stub date if given
            if (stubDate_ != Date()) {
                seed = stubDate_;
                dates_.push_back(isAdjusted_ ?
                                 calendar_.roll(stubDate_) :
                                 stubDate_);
            }
	   
            // add subsequent dates
            int periods = 1, months = 12/frequency_;
            while (true) {
                Date temp = seed.plus(periods*months,Months);
                if (isAdjusted_)
                    temp = calendar_.roll(temp,rollingConvention_);
                dates_.push_back(temp);
                // check exit condition
                if (temp >= last)
                    break;
                else
                    periods++;
            }
	   
            // possibly correct last inserted date
            if (dates_.back() > last) {
                if (longFinal_)
                    dates_.pop_back();
                dates_.back() = last;
                finalIsRegular_ = false;
            }

            // possibly collapse last two dates
            int N = dates_.size();
            if (calendar_.roll(dates_[N-2],rollingConvention_) ==
                calendar_.roll(dates_[N-1],rollingConvention_)) {
                dates_[N-2] = dates_[N-1];
                dates_.pop_back();
                finalIsRegular_ = true;
            }
        }
    }


    bool Schedule::isRegular(Size i) const {
        if (startFromEnd_) {
            if (i == 1)
                return finalIsRegular_;
            else if (i == size()-1)
                return (stubDate_ == Date());;
        } else {
            if (i == 1)
                return (stubDate_ == Date());
            else if (i == size()-1)
                return finalIsRegular_;
        }
        return true;
    }

}
