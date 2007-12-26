/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/time/schedule.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    Schedule::Schedule(const std::vector<Date>& dates,
                       const Calendar& calendar,
                       BusinessDayConvention convention)
    : fullInterface_(false),
      tenor_(Period()), calendar_(calendar),
      convention_(convention),
      terminationDateConvention_(convention),
      rule_(DateGeneration::Forward), endOfMonth_(false),
      finalIsRegular_(true),
      dates_(dates) {}

    Schedule::Schedule(const Date& effectiveDate,
                       const Date& terminationDate,
                       const Period& tenor,
                       const Calendar& calendar,
                       BusinessDayConvention convention,
                       BusinessDayConvention terminationDateConvention,
                       DateGeneration::Rule rule,
                       bool endOfMonth,
                       const Date& firstDate,
                       const Date& nextToLastDate)
    : fullInterface_(true),
      tenor_(tenor), calendar_(calendar),
      convention_(convention),
      terminationDateConvention_(terminationDateConvention),
      rule_(rule), endOfMonth_(endOfMonth),
      firstDate_(firstDate), nextToLastDate_(nextToLastDate),
      finalIsRegular_(true)
    {
        // sanity checks
        QL_REQUIRE(effectiveDate != Date(), "null effective date");
        QL_REQUIRE(terminationDate != Date(), "null termination date");
        QL_REQUIRE(effectiveDate < terminationDate,
                   "effective date (" << effectiveDate
                   << ") later than or equal to termination date ("
                   << terminationDate << ")");

        if (tenor.units()==0)
            rule_ = DateGeneration::Zero;
        else
            QL_REQUIRE(tenor.units()>0,
                       "non positive tenor (" << tenor << ") not allowed");


        if (firstDate != Date()) {
            switch (rule_) {
              case DateGeneration::Backward:
              case DateGeneration::Forward:
                QL_REQUIRE(firstDate > effectiveDate && firstDate < terminationDate,
                           "first date (" << firstDate
                           << ") out of range [effective date (" << effectiveDate
                           << "), termination date (" << terminationDate << ")]");
                break;
              case DateGeneration::Zero:
              case DateGeneration::ThirdWednesday:
                QL_FAIL("first date incompatible with " << rule_ <<
                        " date generation rule");
              default:
                QL_FAIL("unknown Rule (" << Integer(rule_) << ")");
            }
        }
        if (nextToLastDate != Date()) {
            switch (rule_) {
              case DateGeneration::Backward:
              case DateGeneration::Forward:
                QL_REQUIRE(nextToLastDate > effectiveDate && nextToLastDate < terminationDate,
                           "next to last date (" << nextToLastDate
                           << ") out of range [effective date (" << effectiveDate
                           << "), termination date (" << terminationDate << ")]");
                break;
              case DateGeneration::Zero:
              case DateGeneration::ThirdWednesday:
                QL_FAIL("next to last date incompatible with " << rule_ <<
                        " date generation rule");
              default:
                QL_FAIL("unknown Rule (" << Integer(rule_) << ")");
            }
        }


        // calendar needed for endOfMonth adjustment
        Calendar nullCalendar = NullCalendar();
        Integer periods = 1;
        Date seed, exitDate;
        switch (rule_) {

          case DateGeneration::Zero:
            tenor_ = 0*Days;
            dates_.push_back(effectiveDate);
            dates_.push_back(terminationDate);
            isRegular_.push_back(true);
            break;

          case DateGeneration::ThirdWednesday:
            QL_REQUIRE(!endOfMonth,
                       "endOfMonth convention incompatible with " << rule_ <<
                        " date generation rule");
          // fall through
          case DateGeneration::Backward:

            dates_.push_back(terminationDate);

            seed = terminationDate;
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

            exitDate = effectiveDate;
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
                    ++periods;
                }
            }

            if (endOfMonth && calendar.isEndOfMonth(seed))
                convention=Preceding;

            if (calendar.adjust(dates_.front(),convention)!=
                calendar.adjust(effectiveDate, convention)) {
                dates_.insert(dates_.begin(), effectiveDate);
                isRegular_.insert(isRegular_.begin(), false);
            }
            break;

          case DateGeneration::Forward:

            dates_.push_back(effectiveDate);

            seed = effectiveDate;
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

            exitDate = terminationDate;
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
                    ++periods;
                }
            }

            if (endOfMonth && calendar.isEndOfMonth(seed))
                convention=Preceding;

            if (calendar.adjust(dates_.back(),terminationDateConvention)!=
                calendar.adjust(terminationDate, terminationDateConvention)) {
                dates_.push_back(terminationDate);
                isRegular_.push_back(false);
            }

            break;

          default:
            QL_FAIL("unknown DateGeneration::Rule (" << Integer(rule_) << ")");
        }

        // adjustments
        if (rule_==DateGeneration::ThirdWednesday)
            for (Size i=1; i<dates_.size()-1; ++i)
                dates_[i] = Date::nthWeekday(3, Wednesday,
                                             dates_[i].month(),
                                             dates_[i].year());

        for (Size i=0; i<dates_.size()-1; ++i)
            dates_[i]=calendar.adjust(dates_[i], convention);

        // termination date is NOT adjusted as per ISDA specifications,
        // unless otherwise specified in the confirmation of the deal
        if (terminationDateConvention!=Unadjusted) {
            dates_[dates_.size()-1]=calendar.adjust(dates_[dates_.size()-1],
                                                    terminationDateConvention);
        }
    }

    std::vector<Date>::const_iterator
    Schedule::lower_bound(const Date& refDate) const {
        Date d = (refDate==Date() ?
                  Settings::instance().evaluationDate() :
                  refDate);
        return std::lower_bound(dates_.begin(), dates_.end(), d);
    }

    Date Schedule::nextDate(const Date& refDate) const {
        std::vector<Date>::const_iterator res = lower_bound(refDate);
        if (res!=dates_.end())
            return *res;
        else
            return Date();
    }

    Date Schedule::previousDate(const Date& refDate) const {
        std::vector<Date>::const_iterator res = lower_bound(refDate);
        if (res!=dates_.begin())
            return *(--res);
        else
            return Date();
    }

    bool Schedule::isRegular(Size i) const {
        QL_REQUIRE(fullInterface_, "full interface not available");
        QL_REQUIRE(i<=isRegular_.size() && i>0,
                   "index (" << i << ") must be in [1, " <<
                   isRegular_.size() <<"]");
        return isRegular_[i-1];
    }


    MakeSchedule::MakeSchedule(const Date& effectiveDate,
                               const Date& terminationDate,
                               const Period& tenor,
                               const Calendar& calendar,
                               BusinessDayConvention convention)
    : calendar_(calendar),
      effectiveDate_(effectiveDate), terminationDate_(terminationDate),
      tenor_(tenor),
      convention_(convention), terminationDateConvention_(convention),
      rule_(DateGeneration::Backward), endOfMonth_(false),
      stubDate_(Date()), firstDate_(Date()), nextToLastDate_(Date()) {}

    MakeSchedule& MakeSchedule::withTerminationDateConvention(
                                                BusinessDayConvention conv) {
        terminationDateConvention_ = conv;
        return *this;
    }

    MakeSchedule& MakeSchedule::withRule(DateGeneration::Rule r) {
        rule_ = r;
        return *this;
    }

    MakeSchedule& MakeSchedule::forwards() {
        rule_ = DateGeneration::Forward;
        return *this;
    }

    MakeSchedule& MakeSchedule::backwards() {
        rule_ = DateGeneration::Backward;
        return *this;
    }

    MakeSchedule& MakeSchedule::endOfMonth(bool flag) {
        endOfMonth_ = flag;
        return *this;
    }

    MakeSchedule& MakeSchedule::withFirstDate(const Date& d) {
        firstDate_ = d;
        return *this;
    }

    MakeSchedule& MakeSchedule::withNextToLastDate(const Date& d) {
        nextToLastDate_ = d;
        return *this;
    }

    MakeSchedule::operator Schedule() const {
        Date firstDate, nextToLastDate;
        if (stubDate_!=Date()) {
            switch (rule_) {
              case DateGeneration::Zero:
              case DateGeneration::ThirdWednesday:
              case DateGeneration::Backward:
                firstDate = firstDate_;
                nextToLastDate = stubDate_;
                break;
              case DateGeneration::Forward:
                firstDate = stubDate_;
                nextToLastDate = nextToLastDate_;
                break;
              default:
                QL_FAIL("unknown DateGeneration::Rule (" << Integer(rule_) << ")");
            }
        }
        return Schedule(effectiveDate_, terminationDate_, tenor_, calendar_,
                        convention_, terminationDateConvention_,
                        rule_, endOfMonth_, firstDate, nextToLastDate);
    }

}
