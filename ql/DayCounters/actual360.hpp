

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file actual360.hpp
    \brief act/360 day counter

    \fullpath
    ql/DayCounters/%actual360.hpp
*/

// $Id$

#ifndef quantlib_actual360_day_counter_h
#define quantlib_actual360_day_counter_h

#include <ql/daycounter.hpp>

namespace QuantLib {

    namespace DayCounters {

        //! Actual/360 day count convention
        class Actual360 : public DayCounter {
          private:
            class Act360Impl : public DayCounter::DayCounterImpl {
              public:
                std::string name() const { return std::string("act/360"); }
                int dayCount(const Date& d1, const Date& d2) const {
                    return (d2-d1); }
                Time yearFraction(const Date& d1, const Date& d2,
                    const Date&, const Date&) const {
                        return dayCount(d1,d2)/360.0;
                }
            };
          public:
            Actual360()
            : DayCounter(Handle<DayCounterImpl>(new Act360Impl)) {}
        };

    }

}


#endif
