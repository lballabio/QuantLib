
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

/*! \file scheduler.hpp
    \brief date scheduler
*/

// $Id$

#ifndef quantlib_scheduler_hpp
#define quantlib_scheduler_hpp

#include <ql/calendar.hpp>
#include <vector>

namespace QuantLib {

    //! %Date scheduler
    class Scheduler {
      public:
        Scheduler(const Calendar& calendar,
            const Date& startDate, const Date& endDate,
            int frequency, RollingConvention rollingConvention,
            bool isAdjusted, const Date& stubDate = Date());
        // inspectors
        Size size() const { return dates_.size(); }
        const Date& date(int i) const;
        bool isRegular(Size i) const;
        // iterators
        typedef std::vector<Date>::const_iterator const_iterator;
        const_iterator begin() const { return dates_.begin(); }
        const_iterator end() const { return dates_.end(); }
      private:
        Calendar calendar_;
        Date startDate_, endDate_;
        int frequency_;
        RollingConvention rollingConvention_;
        bool isAdjusted_;
        Date stubDate_;
        bool lastIsRegular_;
        std::vector<Date> dates_;
        bool isEndOfMonth(const Date&) const;
    };


    inline const Date& Scheduler::date(int i) const {
        #if defined(QL_DEBUG)
            QL_REQUIRE(i >= 0 && i <= int(dates_.size()),
                "date index out of bounds");
        #endif
        return dates_[i];
    }

}


#endif
