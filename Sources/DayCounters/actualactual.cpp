
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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file actualactual.cpp
    \brief act/act day counter

    $Source$
    $Log$
    Revision 1.15  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.14  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.13  2001/03/12 17:35:11  lballabio
    Removed global IsNull function - could have caused very vicious loops

    Revision 1.12  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.11  2000/12/27 17:18:35  lballabio
    Changes for compiling under Linux and Alpha Linux

    Revision 1.10  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.9  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "DayCounters/actualactual.hpp"
#include "DayCounters/thirty360european.hpp"
#include "null.hpp"

namespace QuantLib {

    namespace DayCounters {

        Time ActualActual::yearFraction(const Date& d1, const Date& d2, 
          const Date& refPeriodStart,
          const Date& refPeriodEnd) const {
            QL_REQUIRE(refPeriodStart != Date() && refPeriodEnd != Date() && 
                refPeriodEnd > refPeriodStart && refPeriodEnd > d1,
                "Invalid reference period");
            double period = Thirty360European().yearFraction(
                refPeriodStart,refPeriodEnd);
            int months;
            double temp;
            QL_ENSURE(QL_FABS(QL_MODF(period*12.0, &temp)) <= 1.0e-10,
                "non-integer number of months");
            months = (int)temp;

            if (d2 <= refPeriodEnd) {
                if (d1 >= refPeriodStart)
                    return (double)dayCount(d1,d2) / 
                        (double)dayCount(refPeriodStart,refPeriodEnd)*period;
                else {
                    Date previousBDate = refPeriodStart.plusMonths(-months);
                    double result = (double)(refPeriodStart-d1) / 
                        (double)dayCount(previousBDate,refPeriodStart);
                    result += (double)(d2-refPeriodStart) / 
                        (double)dayCount(refPeriodStart,refPeriodEnd);
                    return result*period;
                }
            } else {
                double result = (double)dayCount(d1,refPeriodStart) / 
                    (double)dayCount(refPeriodStart,refPeriodEnd)*period;
                int i = 0;
                Date tempd;
                do {
                    tempd = refPeriodEnd.plusMonths(months*(i++));
                } while (tempd <= d2);
                i--;
                Date previoustemp = refPeriodEnd.plusMonths(months*i);
                return result + i*period + 
                    yearFraction(previoustemp,d2,previoustemp,tempd);
            }
        }

    }

}
