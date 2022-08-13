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

/*! \file actual36525.hpp
    \brief act/365.25 day counter
*/

#ifndef quantlib_actual365_25_day_counter_h
#define quantlib_actual365_25_day_counter_h

#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! Actual/365.25 day count convention

    /*! Actual/365.25 day count convention, also known as "Act/365.25", or "A/365.25".
        \ingroup daycounters
    */
    class Actual36525 : public DayCounter {
      private:
        class Impl : public DayCounter::Impl {
          private:
              bool includeLastDay_;
          public:
            explicit Impl(const bool includeLastDay)
            : includeLastDay_(includeLastDay) {}
            std::string name() const override {
                return includeLastDay_ ?
                    std::string("Actual/365.25 (inc)")
                    : std::string("Actual/365.25");
            }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override {
                return (d2-d1) + (includeLastDay_ ? 1 : 0);
            }
            Time
            yearFraction(const Date& d1, const Date& d2, const Date&, const Date&) const override {
                return (daysBetween(d1,d2)
                        + (includeLastDay_ ? 1.0 : 0.0))/365.25;
            }
        };
      public:
        explicit Actual36525(const bool includeLastDay = false)
        : DayCounter(ext::shared_ptr<DayCounter::Impl>(
            new Actual36525::Impl(includeLastDay))) {}
    };

}

#endif
