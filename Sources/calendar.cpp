
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file calendar.cpp
    \brief Abstract calendar class

    $Source$
    $Name$
    $Log$
    Revision 1.14  2001/03/01 11:37:07  lballabio
    Fixed bug in advance(...,Days)

    Revision 1.13  2001/01/24 13:17:46  marmar
    style redefined

    Revision 1.12  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.11  2000/12/20 18:26:15  enri
    test

    Revision 1.10  2000/12/20 16:42:38  enri
    commit test

    Revision 1.9  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "calendar.h"

namespace QuantLib {

    Date Calendar::roll(const Date& d , bool modified) const {
        Date d1 = d;
        while (isHoliday(d1))
            d1++;
        if (modified && d1.month() != d.month()) {
            d1 = d;
            while (isHoliday(d1))
                d1--;
        }
        return d1;
    }

    Date Calendar::advance(const Date& d, int n, TimeUnit unit, 
      bool modified) const {
        if (n == 0) {
            return roll(d,modified);
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
                    n--;
                }
            }
            return d1;
        } else {
            Date d1 = d.plus(n,unit);
            if (modified && d.month() != roll(d.plusDays(1)).month() // EOM
              && (unit == Months || unit == Years)) {
                Month m = d1.month();
                Date firstOfNextMonth = (m == December ? 
                    Date(1,January,d1.year()+1) : Date(1,Month(m+1),d1.year()));
                Date d1 = firstOfNextMonth.plusDays(-1);    // last of month
                while (isHoliday(d1))
                    d1--;
            } else {
                d1 = roll(d1,modified);
            }
            return d1;
        }
        QL_DUMMY_RETURN(Date());
    }

}
