/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
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
    : calendar_(calendar), frequency_(frequency),
      tenor_(Period()),
      convention_(convention),
      startFromEnd_(startFromEnd), longFinal_(longFinal),
      endOfMonth_(false), finalIsRegular_(true),
      isRegular_(std::vector<bool>()) {

        firstDate_ = (startFromEnd ? Date() : stubDate);
        nextToLastDate_ = (startFromEnd ? stubDate : Date());

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
            tenor_=Period(frequency);
            // calculations
            Date seed = endDate;
            Date first = calendar.adjust(startDate, convention);
            // add end date
            dates_.push_back(calendar.adjust(endDate, convention));

            // add stub date if given
            if (nextToLastDate_ != Date()) {
                seed = nextToLastDate_;
                dates_.insert(dates_.begin(),
                              calendar.adjust(nextToLastDate_, convention));
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
                if (N > 1 && longFinal) {
                    dates_.erase(dates_.begin()+1);
                    N--;
                }
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
            tenor_=Period(frequency);
            // calculations
            Date seed = startDate;
            Date last = calendar.adjust(endDate, convention);
            // add start date
            dates_.push_back(calendar.adjust(startDate, convention));

            // add stub date if given
            if (firstDate_ != Date()) {
                seed = firstDate_;
                dates_.push_back(calendar.adjust(firstDate_, convention));
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
                if (N > 1 && longFinal) {
                    dates_.pop_back();
                    N--;
                }
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


    Schedule::Schedule(const Calendar& calendar,
                       const Date& startDate, const Date& endDate,
                       const Period& tenor,
                       BusinessDayConvention convention,
                       const Date& stubDate, bool startFromEnd,
                       bool longFinal)
    : calendar_(calendar), frequency_(tenor.frequency()),
      tenor_(tenor),
      convention_(convention),
      startFromEnd_(startFromEnd), longFinal_(longFinal),
      endOfMonth_(false), finalIsRegular_(true),
      isRegular_(std::vector<bool>()) {

        firstDate_ = (startFromEnd ? Date() : stubDate);
        nextToLastDate_ = (startFromEnd ? stubDate : Date());

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
        QL_REQUIRE(frequency_ == 0 || 12 % frequency_ == 0,
                   "frequency (" << Integer(frequency_)
                   << " per year) does not correspond to "
                   << "a whole number of months");

        if (frequency_ == 0) {
            QL_REQUIRE(stubDate == Date(),
                       "stub date incompatible with frequency_ 'once'");
            dates_.push_back(calendar.adjust(startDate, convention));
            dates_.push_back(calendar.adjust(endDate, convention));
        } else if (startFromEnd) {
            tenor_=Period(frequency_);
            // calculations
            Date seed = endDate;
            Date first = calendar.adjust(startDate, convention);
            // add end date
            dates_.push_back(calendar.adjust(endDate, convention));

            // add stub date if given
            if (nextToLastDate_ != Date()) {
                seed = nextToLastDate_;
                dates_.insert(dates_.begin(),
                              calendar.adjust(nextToLastDate_, convention));
            }

            // add subsequent dates
            Integer periods = 1, months = 12/frequency_;
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
                if (N > 1 && longFinal) {
                    dates_.erase(dates_.begin()+1);
                    N--;
                }
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
            tenor_=Period(frequency_);
            // calculations
            Date seed = startDate;
            Date last = calendar.adjust(endDate, convention);
            // add start date
            dates_.push_back(calendar.adjust(startDate, convention));

            // add stub date if given
            if (firstDate_ != Date()) {
                seed = firstDate_;
                dates_.push_back(calendar.adjust(firstDate_, convention));
            }

            // add subsequent dates
            Integer periods = 1, months = 12/frequency_;
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
                if (N > 1 && longFinal) {
                    dates_.pop_back();
                    N--;
                }
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

    Schedule::Schedule(const Date& effectiveDate,
                       const Date& terminationDate,
                       const Period& tenor,
                       const Calendar& calendar,
                       BusinessDayConvention convention,
                       BusinessDayConvention terminationDateConvention,
                       bool backward,
                       bool endOfMonth,
                       const Date& firstDate,
                       const Date& nextToLastDate)
    : calendar_(calendar),
      frequency_(tenor.frequency()),
      tenor_(tenor),
      convention_(convention),
      firstDate_(firstDate), nextToLastDate_(nextToLastDate),
      startFromEnd_(backward), longFinal_(false),
      endOfMonth_(endOfMonth), finalIsRegular_(true)
    {
        // sanity checks
        QL_REQUIRE(effectiveDate != Date(), "null effective date");
        QL_REQUIRE(terminationDate != Date(), "null termination date");
        QL_REQUIRE(effectiveDate < terminationDate,
                   "effective date (" << effectiveDate
                   << ") later than termination date (" << terminationDate << ")");
        if (firstDate != Date()) {
            QL_REQUIRE((firstDate > effectiveDate && firstDate < terminationDate),
                       "first date (" << firstDate
                       << ") out of range (effective date (" << effectiveDate
                       << "), termination date (" << terminationDate << "))");
        }
        if (nextToLastDate != Date()) {
            QL_REQUIRE((nextToLastDate > effectiveDate && nextToLastDate < terminationDate),
                       "next to last date (" << nextToLastDate
                       << ") out of range (effective date (" << effectiveDate
                       << "), termination date (" << terminationDate << "))");
        }

        if (tenor_ < Period(1, Days)) {
            QL_REQUIRE(firstDate == Date(),
                       "first date incompatible with zero coupon date");
            QL_REQUIRE(nextToLastDate == Date(),
                       "next to last date incompatible with zero coupon date");
            dates_.push_back(effectiveDate);
            dates_.push_back(terminationDate);
            isRegular_.push_back(true);
        } else if (backward) { // backward roll date convention
            // calendar needed for endOfMonth adjustment
            Calendar nullCalendar = NullCalendar();
            Integer periods = 1;

            dates_.push_back(terminationDate);

            Date seed = terminationDate;
            if (nextToLastDate != Date()) {
                dates_.insert(dates_.begin(), nextToLastDate);
                Date temp = nullCalendar.advance(seed,
                    -periods*tenor_, convention, endOfMonth);
                if (temp!=nextToLastDate)
                    isRegular_.insert(isRegular_.begin(), false);
                else 
                    isRegular_.insert(isRegular_.begin(), true);
                seed = nextToLastDate;
            }

            Date exitDate = effectiveDate;
            if (firstDate != Date())
                exitDate = firstDate;

            while (true) {
                Date temp = nullCalendar.advance(seed,
                    -periods*tenor_, convention, endOfMonth);
                if (temp < exitDate)
                    break;
                else {
                    dates_.insert(dates_.begin(), temp);
                    isRegular_.insert(isRegular_.begin(), true);
                    periods++;
                }
            }

            if (endOfMonth && calendar.isEndOfMonth(seed))
                convention=Preceding;

            if (dates_.front()!=calendar.adjust(effectiveDate, convention)) {
                dates_.insert(dates_.begin(), effectiveDate);
                isRegular_.insert(isRegular_.begin(), false);
            }


        } else { // forward roll date convention
            // calendar needed for endOfMonth adjustment
            Calendar nullCalendar = NullCalendar();
            Integer periods = 1;

            dates_.push_back(effectiveDate);

            Date seed = effectiveDate;
            if (firstDate!=Date()) {
                dates_.push_back(firstDate);
                Date temp = nullCalendar.advance(seed,
                    periods*tenor_, convention, endOfMonth);
                if (temp!=firstDate)
                    isRegular_.push_back(false);
                else 
                    isRegular_.push_back(true);
                seed = firstDate;
            }

            Date exitDate = terminationDate;
            if (nextToLastDate != Date())
                exitDate = nextToLastDate;

            while (true) {
                Date temp = nullCalendar.advance(seed,
                    periods*tenor_, convention, endOfMonth);
                if (temp > exitDate)
                    break;
                else {
                    dates_.push_back(temp);
                    isRegular_.push_back(true);
                    periods++;
                }
            }

            if (endOfMonth && calendar.isEndOfMonth(seed))
                convention=Preceding;

            if (dates_.back()!=calendar.adjust(terminationDate, terminationDateConvention)) {
                dates_.push_back(terminationDate);
                isRegular_.push_back(false);
            }

        }

        for (Size i=0; i<dates_.size()-1; i++)
            dates_[i]=calendar.adjust(dates_[i], convention);

        // termination date is NOT adjusted as per ISDA specifications,
        // unless otherwise specified in the confirmation of the deal 
        if (terminationDateConvention!=Unadjusted) {                
            dates_[dates_.size()-1]=calendar.adjust(dates_[dates_.size()-1],
                                                    terminationDateConvention);
        }
    }

    bool Schedule::isRegular(Size i) const {
        if (isRegular_.size()==0) {
            if (frequency_ == 0) {
                return true;
            } else if (startFromEnd_) {
                if (i == 1)
                    return finalIsRegular_;
                else if (i == size()-1)
                    return (nextToLastDate_ == Date());
                else
                    return true;
            } else {
                if (i == 1)
                    return (firstDate_ == Date());
                else if (i == size()-1)
                    return finalIsRegular_;
                else
                    return true;
            }
        } else {
            QL_REQUIRE(i<=isRegular_.size(),
                       "i (" << i << ") must be less than or equal to "
                       << isRegular_.size());
            if (tenor_==Period()) return true;
            return isRegular_[i-1];
        }
        QL_DUMMY_RETURN(true);
    }

}
