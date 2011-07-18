/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2011 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2009 StatPro Italia srl

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

/*! \file schedule.hpp
    \brief date schedule
*/

#ifndef quantlib_schedule_hpp
#define quantlib_schedule_hpp

#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/utilities/null.hpp>
#include <ql/time/period.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/errors.hpp>
#include <boost/optional.hpp>

namespace QuantLib {

    //! Payment schedule
    /*! \ingroup datetime */
    class Schedule {
      public:
        Schedule(const std::vector<Date>&,
                 const Calendar& calendar = NullCalendar(),
                 BusinessDayConvention convention = Unadjusted);
        Schedule(Date effectiveDate,
                 const Date& terminationDate,
                 const Period& tenor,
                 const Calendar& calendar,
                 BusinessDayConvention convention,
                 BusinessDayConvention terminationDateConvention,
                 DateGeneration::Rule rule,
                 bool endOfMonth,
                 const Date& firstDate = Date(),
                 const Date& nextToLastDate = Date());
        Schedule() {}
        //! \name Date access
        //@{
        Size size() const { return dates_.size(); }
        const Date& operator[](Size i) const;
        const Date& at(Size i) const;
        const Date& date(Size i) const;
        Date previousDate(const Date& refDate) const;
        Date nextDate(const Date& refDate) const;
        const std::vector<Date>& dates() const { return dates_; }
        bool isRegular(Size i) const;
        //@}
        //! \name Other inspectors
        //@{
        bool empty() const { return dates_.empty(); }
        const Calendar& calendar() const;
        const Date& startDate() const;
        const Date& endDate() const;
        const Period& tenor() const;
        BusinessDayConvention businessDayConvention() const;
        BusinessDayConvention terminationDateBusinessDayConvention() const;
        DateGeneration::Rule rule() const;
        bool endOfMonth() const;
        //@}
        //! \name Iterators
        //@{
        typedef std::vector<Date>::const_iterator const_iterator;
        const_iterator begin() const { return dates_.begin(); }
        const_iterator end() const { return dates_.end(); }
        const_iterator lower_bound(const Date& d = Date()) const;
        //@}
        //! \name Utilities
        //@{
        //! truncated schedule
        Schedule until(const Date& truncationDate) const;
        //@}
      private:
        bool fullInterface_;
        Period tenor_;
        Calendar calendar_;
        BusinessDayConvention convention_, terminationDateConvention_;
        DateGeneration::Rule rule_;
        bool endOfMonth_;
        Date firstDate_, nextToLastDate_;
        std::vector<Date> dates_;
        std::vector<bool> isRegular_;
    };


    //! helper class
    /*! This class provides a more comfortable interface to the
        argument list of Schedule's constructor.
    */
    class MakeSchedule {
      public:
        MakeSchedule();
        MakeSchedule& from(const Date& effectiveDate);
        MakeSchedule& to(const Date& terminationDate);
        MakeSchedule& withTenor(const Period&);
        MakeSchedule& withFrequency(Frequency);
        MakeSchedule& withCalendar(const Calendar&);
        MakeSchedule& withConvention(BusinessDayConvention);
        MakeSchedule& withTerminationDateConvention(BusinessDayConvention);
        MakeSchedule& withRule(DateGeneration::Rule);
        MakeSchedule& forwards();
        MakeSchedule& backwards();
        MakeSchedule& endOfMonth(bool flag=true);
        MakeSchedule& withFirstDate(const Date& d);
        MakeSchedule& withNextToLastDate(const Date& d);
        operator Schedule() const;
      private:
        Calendar calendar_;
        Date effectiveDate_, terminationDate_;
        boost::optional<Period> tenor_;
        boost::optional<BusinessDayConvention> convention_;
        boost::optional<BusinessDayConvention> terminationDateConvention_;
        DateGeneration::Rule rule_;
        bool endOfMonth_;
        Date firstDate_, nextToLastDate_;
    };



    // inline definitions

    inline const Date& Schedule::date(Size i) const {
        return dates_.at(i);
    }

    inline const Date& Schedule::operator[](Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        return dates_.at(i);
        #else
        return dates_[i];
        #endif
    }

    inline const Date& Schedule::at(Size i) const {
        return dates_.at(i);
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

    inline const Period& Schedule::tenor() const {
        QL_REQUIRE(fullInterface_, "full interface not available");
        return tenor_;
    }

    inline BusinessDayConvention Schedule::businessDayConvention() const {
        return convention_;
    }

    inline BusinessDayConvention
    Schedule::terminationDateBusinessDayConvention() const {
        QL_REQUIRE(fullInterface_, "full interface not available");
        return terminationDateConvention_;
    }

    inline DateGeneration::Rule Schedule::rule() const {
        QL_REQUIRE(fullInterface_, "full interface not available");
        return rule_;
    }

    inline bool Schedule::endOfMonth() const {
        QL_REQUIRE(fullInterface_, "full interface not available");
        return endOfMonth_;
    }

}

#endif
