
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

/*! \file calendar.cpp
    \fullpath Sources/%calendar.cpp
    \brief Abstract calendar class

*/

// $Id$
// $Log$
// Revision 1.26  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.25  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.24  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.23  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.22  2001/05/29 15:12:48  lballabio
// Reintroduced RollingConventions (and redisabled default extrapolation on PFF curve)
//
// Revision 1.21  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/calendar.hpp"

namespace QuantLib {

    Date Calendar::roll(const Date& d , RollingConvention c) const {
        Date d1 = d;
        if (c == Following || c == ModifiedFollowing) {
            while (isHoliday(d1))
                d1++;
            if (c == ModifiedFollowing && d1.month() != d.month()) {
                return roll(d,Preceding);
            }
        } else if (c == Preceding || c == ModifiedPreceding) {
            while (isHoliday(d1))
                d1--;
            if (c == ModifiedPreceding && d1.month() != d.month()) {
                return roll(d,Following);
            }
        } else {
            throw Error("Unknown rolling convention");
        }
        return d1;
    }

    Date Calendar::advance(const Date& d, int n, TimeUnit unit,
      RollingConvention c) const {
        if (n == 0) {
            return roll(d,c);
        } else if (unit == Days) {
            Date d1 = d;
            if (n > 0) {
                while (n > 0) {
                    d1++;
                    while (isHoliday(d1))
                        d1++;
                    n--;
                }
            } else {
                while (n < 0) {
                    d1--;
                    while(isHoliday(d1))
                        d1--;
                    n++;
                }
            }
            return d1;
        } else {
            Date d1 = d.plus(n,unit);
            return roll(d1,c);
        }
        QL_DUMMY_RETURN(Date());
    }

}
