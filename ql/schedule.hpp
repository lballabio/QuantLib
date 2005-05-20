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

/*! \file schedule.hpp
    \brief date schedule
*/

#ifndef quantlib_schedule_hpp
#define quantlib_schedule_hpp

#include <ql/calendar.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/Utilities/null.hpp>
#include <vector>

namespace QuantLib {

    //! Payment schedule
    class Schedule {
      public:
        Schedule(const Calendar& calendar,
                 const Date& startDate, const Date& endDate,
                 Frequency frequency, BusinessDayConvention convention,
                 const Date& stubDate = Date(),
                 bool startFromEnd = false, bool longFinal = false);
        Schedule(const std::vector<Date>&,
                 const Calendar& calendar = NullCalendar(), 
                 BusinessDayConvention convention = Unadjusted);
        //! \name Date access
        //@{
        Size size() const { return dates_.size(); }
        const Date& operator[](Size i) const;
        const Date& date(Size i) const;
        const std::vector<Date>& dates() const { return dates_; }
        bool isRegular(Size i) const;
        //@}
        //! \name Other inspectors
        //@{
        const Calendar& calendar() const;
        const Date& startDate() const;
        const Date& endDate() const;
        Frequency frequency() const;
        BusinessDayConvention businessDayConvention() const;
        //@}
        //! \name Iterators
        //@{
        typedef std::vector<Date>::const_iterator const_iterator;
        const_iterator begin() const { return dates_.begin(); }
        const_iterator end() const { return dates_.end(); }
        //@}
      private:
        Calendar calendar_;
        Frequency frequency_;
        BusinessDayConvention convention_;
        Date stubDate_;
        bool startFromEnd_;
        bool longFinal_;
        bool finalIsRegular_;
        std::vector<Date> dates_;
    };


    //! helper class
    /*! This class provides a more comfortable interface to the
        argument list of Schedule's constructor.
    */
    class MakeSchedule {
      public:
        MakeSchedule(const Calendar& calendar,
                     const Date& startDate, const Date& endDate,
                     Frequency frequency,
                     BusinessDayConvention convention)
        : calendar_(calendar), startDate_(startDate), endDate_(endDate),
          frequency_(frequency), convention_(convention),
          stubDate_(Date()), startFromEnd_(false), longFinal_(false) {}
        MakeSchedule& withStubDate(const Date& d) {
            stubDate_ = d;
            return *this;
        }
        MakeSchedule& backwards(bool flag=true) {
            startFromEnd_ = flag;
            return *this;
        }
        MakeSchedule& forwards(bool flag=true) {
            startFromEnd_ = !flag;
            return *this;
        }
        MakeSchedule& longFinalPeriod(bool flag=true) {
            longFinal_ = flag;
            return *this;
        }
        MakeSchedule& shortFinalPeriod(bool flag=true) {
            longFinal_ = !flag;
            return *this;
        }
        operator Schedule() {
            return Schedule(calendar_,startDate_,endDate_,frequency_,
                            convention_,stubDate_,startFromEnd_,longFinal_);
        }
      private:
        Calendar calendar_;
        Date startDate_, endDate_;
        Frequency frequency_;
        BusinessDayConvention convention_;
        Date stubDate_;
        bool startFromEnd_;
        bool longFinal_;
    };



    // inline definitions

    inline Schedule::Schedule(const std::vector<Date>& dates,
                              const Calendar& calendar,
                              BusinessDayConvention convention)
    : calendar_(calendar), frequency_(Frequency(-1)), convention_(convention),
      startFromEnd_(false), longFinal_(false), finalIsRegular_(true),
      dates_(dates) {}

    inline const Date& Schedule::date(Size i) const {
        QL_REQUIRE(i <= dates_.size(),
                   "date index out of bounds");
        return dates_[i];
    }

    inline const Date& Schedule::operator[] (Size i) const {
        #if defined(QL_DEBUG)
        QL_REQUIRE(i <= dates_.size(),
                   "date index out of bounds");
        #endif
        return dates_[i];
    }

    inline const Calendar& Schedule::calendar() const {
        return calendar_;
    }

    inline const Date& Schedule::startDate() const {
        return dates_.front();
    }

    inline const Date& Schedule::endDate() const {
        return dates_.back();
    }

    inline Frequency Schedule::frequency() const {
        QL_REQUIRE(Integer(frequency_) != -1,
                   "frequency not available");
        return frequency_;
    }

    inline BusinessDayConvention Schedule::businessDayConvention() const {
        return convention_;
    }

}


#endif
