
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*
    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file actual365.hpp
    \brief act/365 day counter
*/

#ifndef quantlib_actual365_day_counter_h
#define quantlib_actual365_day_counter_h

#include "qldefines.hpp"
#include "daycounter.hpp"

namespace QuantLib {

    namespace DayCounters {

        class Actual365 : public DayCounter {
          public:
            std::string name() const { return std::string("act/365"); }
            int dayCount(const Date& d1, const Date& d2) const { return (d2-d1); }
            Time yearFraction(const Date& d1, const Date& d2,
              const Date& refPeriodStart = Date(), const Date& refPeriodEnd = Date()) const {
                return dayCount(d1,d2)/365.0;
            }
        };

    }

}


#endif
