
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file actual365.hpp
    \brief act/365 day counter

    \fullpath
    ql/DayCounters/%actual365.hpp
*/

// $Id$

#ifndef quantlib_actual365_day_counter_h
#define quantlib_actual365_day_counter_h

#include <ql/daycounter.hpp>

namespace QuantLib {

    namespace DayCounters {

        //! Actual/365 day count convention
        class Actual365 : public DayCounter {
          private:
            class Act365Factory : public DayCounter::factory {
              public:
                DayCounter create() const { return Actual365(); }
            };
            class Act365Impl : public DayCounter::DayCounterImpl {
              public:
                std::string name() const { return std::string("act/365"); }
                int dayCount(const Date& d1, const Date& d2) const {
                    return (d2-d1); }
                Time yearFraction(const Date& d1, const Date& d2,
                    const Date&, const Date&) const {
                        return dayCount(d1,d2)/365.0;
                }
            };
          public:
            Actual365()
            : DayCounter(Handle<DayCounterImpl>(new Act365Impl)) {}
            //! returns a factory of actual/365 day counters
            Handle<factory> getFactory() const {
                return Handle<factory>(new Act365Factory);
            }
        };

    }

}


#endif
