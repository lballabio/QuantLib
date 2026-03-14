/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

/*! \file dateinterval.hpp
    \brief Date interval
*/

#ifndef quantlib_date_interval_hpp
#define quantlib_date_interval_hpp

#include <ql/time/date.hpp>
#include <ql/errors.hpp>
#include <algorithm>

namespace QuantLib {

    //! Date interval described by a number of a given time unit
    /*! \ingroup datetime */
    class DateInterval {
        friend std::ostream& operator<<(std::ostream&, const DateInterval&);

      private:
        Date startDate_;
        Date endDate_;
      public:
        DateInterval() = default;
        DateInterval(const Date& startDate, const Date& endDate)
        : startDate_(startDate), endDate_(endDate) {
            QL_REQUIRE(endDate_ >= startDate_,
                       "end date must be >= start date");
        }
        const Date& startDate() const { return startDate_; }
        const Date& endDate() const { return endDate_; }

        bool isDateBetween(Date date,
                           bool includeFirst = true,
                           bool includeLast = true) const {
            if (includeFirst && !(date >= startDate_))
                return false;
            else if (!(date > startDate_))
                return false;
            if (includeLast && !(date <= endDate_))
                return false;
            else if (!(date < endDate_))
                return false;
            return true;
        }

        DateInterval intersection(const DateInterval& di) const {
            if ((startDate_ < di.startDate_ && endDate_ < di.startDate_) ||
                (startDate_ > di.endDate_ && endDate_ > di.endDate_))
                return {};
            return {std::max(startDate_, di.startDate_), std::min(endDate_, di.endDate_)};
        }

        bool operator==(const DateInterval& rhs) const {
            return startDate_ == rhs.startDate_ && endDate_ == rhs.endDate_;
        }

        bool operator!=(const DateInterval& rhs) const {
            return !(*this == rhs);
        }
    };

}

#endif
