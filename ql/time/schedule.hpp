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
 <https://www.quantlib.org/license.shtml>.

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
#include <ql/optional.hpp>

namespace QuantLib {

    //! Payment schedule
    /*! \ingroup datetime */
    class Schedule {
      public:
        /*! constructor that takes any list of dates, and optionally
            meta information that can be used by client classes. Note
            that neither the list of dates nor the meta information is
            checked for plausibility in any sense. */
        Schedule(
            const std::vector<Date>&,
            Calendar calendar = NullCalendar(),
            BusinessDayConvention convention = Unadjusted,
            const ext::optional<BusinessDayConvention>& terminationDateConvention = ext::nullopt,
            const ext::optional<Period>& tenor = ext::nullopt,
            const ext::optional<DateGeneration::Rule>& rule = ext::nullopt,
            const ext::optional<bool>& endOfMonth = ext::nullopt,
            std::vector<bool> isRegular = std::vector<bool>(0));
        /*! rule based constructor */
        Schedule(Date effectiveDate,
                 const Date& terminationDate,
                 const Period& tenor,
                 Calendar calendar,
                 BusinessDayConvention convention,
                 BusinessDayConvention terminationDateConvention,
                 DateGeneration::Rule rule,
                 bool endOfMonth,
                 const Date& firstDate = Date(),
                 const Date& nextToLastDate = Date());
        Schedule() = default;
        //! \name Element access
        //@{
        Size size() const { return dates_.size(); }
        const Date& operator[](Size i) const;
        const Date& at(Size i) const;
        const Date& date(Size i) const;
        const std::vector<Date>& dates() const { return dates_; }
        bool empty() const { return dates_.empty(); }
        const Date& front() const;
        const Date& back() const;
        //@}
        //! \name Other inspectors
        //@{
        Date previousDate(const Date& refDate) const;
        Date nextDate(const Date& refDate) const;
        bool hasIsRegular() const;
        bool isRegular(Size i) const;
        const std::vector<bool>& isRegular() const;
        const Calendar& calendar() const;
        const Date& startDate() const;
        const Date& endDate() const;
        bool hasTenor() const;
        const Period& tenor() const;
        BusinessDayConvention businessDayConvention() const;
        bool hasTerminationDateBusinessDayConvention() const;
        BusinessDayConvention terminationDateBusinessDayConvention() const;
        bool hasRule() const;
        DateGeneration::Rule rule() const;
        bool hasEndOfMonth() const;
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
        Schedule after(const Date& truncationDate) const;
        Schedule until(const Date& truncationDate) const;
        //@}
      private:
        ext::optional<Period> tenor_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        ext::optional<BusinessDayConvention> terminationDateConvention_;
        ext::optional<DateGeneration::Rule> rule_;
        ext::optional<bool> endOfMonth_;
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
        ext::optional<Period> tenor_;
        ext::optional<BusinessDayConvention> convention_;
        ext::optional<BusinessDayConvention> terminationDateConvention_;
        DateGeneration::Rule rule_ = DateGeneration::Backward;
        bool endOfMonth_ = false;
        Date firstDate_, nextToLastDate_;
    };

    /*! Helper function for returning the date on or before date \p d that is the 20th of the month and obeserves the 
        given date generation \p rule if it is relevant.
    */
    Date previousTwentieth(const Date& d, DateGeneration::Rule rule);

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

    inline const Date& Schedule::front() const {
        QL_REQUIRE(!dates_.empty(), "no front date for empty schedule");
        return dates_.front();
    }

    inline const Date& Schedule::back() const {
        QL_REQUIRE(!dates_.empty(), "no back date for empty schedule");
        return dates_.back();
    }

    inline const Calendar& Schedule::calendar() const {
        return calendar_;
    }

    inline const Date& Schedule::startDate() const {
        QL_REQUIRE(!dates_.empty(), "empty Schedule: no start date"); 
        return dates_.front();
    }

    inline const Date &Schedule::endDate() const {
        // Checks to avoid segfault, issue #2302
        QL_REQUIRE(!dates_.empty(), "empty Schedule: no end date"); 
        return dates_.back(); 
    }

    inline bool Schedule::hasTenor() const {
        return static_cast<bool>(tenor_);
    }

    inline const Period& Schedule::tenor() const {
        QL_REQUIRE(hasTenor(),
                   "full interface (tenor) not available");
        return *tenor_;  // NOLINT(bugprone-unchecked-optional-access)
    }

    inline BusinessDayConvention Schedule::businessDayConvention() const {
        return convention_;
    }

    inline bool
    Schedule::hasTerminationDateBusinessDayConvention() const {
        return static_cast<bool>(terminationDateConvention_);
    }

    inline BusinessDayConvention
    Schedule::terminationDateBusinessDayConvention() const {
        QL_REQUIRE(hasTerminationDateBusinessDayConvention(),
                   "full interface (termination date bdc) not available");
        return *terminationDateConvention_;  // NOLINT(bugprone-unchecked-optional-access)
    }

    inline bool Schedule::hasRule() const {
        return static_cast<bool>(rule_);
    }

    inline DateGeneration::Rule Schedule::rule() const {
        QL_REQUIRE(hasRule(), "full interface (rule) not available");
        return *rule_;  // NOLINT(bugprone-unchecked-optional-access)
    }

    inline bool Schedule::hasEndOfMonth() const {
        return static_cast<bool>(endOfMonth_);
    }

    inline bool Schedule::endOfMonth() const {
        QL_REQUIRE(hasEndOfMonth(),
                   "full interface (end of month) not available");
        return *endOfMonth_;  // NOLINT(bugprone-unchecked-optional-access)
    }

}

#endif
