
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

/*! \file thirty360.hpp
    \brief 30/360 day counter

    \fullpath
    ql/DayCounters/%thirty360.hpp
*/

// $Id$

#ifndef quantlib_thirty360_day_counter_h
#define quantlib_thirty360_day_counter_h

#include "ql/daycounter.hpp"

namespace QuantLib {

    namespace DayCounters {

        //! 30/360 day count convention
        class Thirty360 : public DayCounter {
          public:
            enum Convention { USA, European, Italian };
            explicit Thirty360(Convention c = USA)
            : convention_(c) {}
            //! \name DayCounter interface
            //@{
            std::string name() const;
            int dayCount(const Date& d1, const Date& d2) const;
            Time yearFraction(const Date& d1, const Date& d2,
              const Date& refPeriodStart = Date(), 
              const Date& refPeriodEnd = Date()) const;
            //@}
          private:
            class Thirty360Factory : public factory {
              public:
                Thirty360Factory(Convention c) : convention_(c) {}
                Handle<DayCounter> create() const {
                    return Handle<DayCounter>(new Thirty360(convention_));
                }
              private:
                Convention convention_;
            };
          public:
            //! returns a factory of 30/360 day counters
            Handle<factory> getFactory(Convention c) const {
                return Handle<factory>(new Thirty360Factory(c));
            }
          private:
            Convention convention_;
        };

    }

}


#endif
