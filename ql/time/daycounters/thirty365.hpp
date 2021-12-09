/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 StatPro Italia srl

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

/*! \file thirty365.hpp
    \brief 30/365 day counters
*/

#ifndef quantlib_thirty_365_day_counter_hpp
#define quantlib_thirty_365_day_counter_hpp

#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! 30/365 day count convention
    /*! \ingroup daycounters */
    class Thirty365 : public DayCounter {
      private:
        class Impl : public DayCounter::Impl {
          public:
            std::string name() const override { return std::string("30/365"); }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override;
            Time
            yearFraction(const Date& d1, const Date& d2, const Date&, const Date&) const override {
                return dayCount(d1,d2)/365.0;
            }
        };
      public:
        Thirty365();
    };

}

#endif
