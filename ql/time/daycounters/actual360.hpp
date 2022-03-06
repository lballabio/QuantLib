/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file actual360.hpp
    \brief act/360 day counter
*/

#ifndef quantlib_actual360_day_counter_h
#define quantlib_actual360_day_counter_h

#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! Actual/360 day count convention

    /*! Actual/360 day count convention, also known as "Act/360", or "A/360".

        \ingroup daycounters
    */
    class Actual360 : public DayCounter {
      private:
        class Impl : public DayCounter::Impl {
          private:
              bool includeLastDay_;
          public:
            explicit Impl(const bool includeLastDay)
            : includeLastDay_(includeLastDay) {}
            std::string name() const override {
                return includeLastDay_ ?
                    std::string("Actual/360 (inc)")
                    : std::string("Actual/360");
            }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override {
                return (d2-d1) + (includeLastDay_ ? 1 : 0);
            }
            Time
            yearFraction(const Date& d1, const Date& d2, const Date&, const Date&) const override {
                return (daysBetween(d1,d2)
                        + (includeLastDay_ ? 1.0 : 0.0))/360.0;
            }
        };
      public:
        explicit Actual360(const bool includeLastDay = false)
        : DayCounter(ext::shared_ptr<DayCounter::Impl>(
            new Actual360::Impl(includeLastDay))) {}
    };

}

#endif


#ifndef id_aaa71cb9aac33c589e2af5e89c7db44f
#define id_aaa71cb9aac33c589e2af5e89c7db44f
inline bool test_aaa71cb9aac33c589e2af5e89c7db44f(const int* i) {
    return i != nullptr;
}
#endif
