
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

/*! \file actualactual.cpp
    \brief act/act day counter

    $Source$
    $Log$
    Revision 1.21  2001/05/24 13:57:51  nando
    smoothing #include xx.hpp and cutting old Log messages

*/

#include "ql/DayCounters/actualactual.hpp"
#include "ql/DayCounters/thirty360european.hpp"

namespace QuantLib {

    namespace DayCounters {

        Time ActualActual::yearFraction(const Date& d1, const Date& d2,
          const Date& refPeriodStart, const Date& refPeriodEnd) const {
            QL_REQUIRE(refPeriodStart != Date() && refPeriodEnd != Date() &&
                refPeriodEnd > refPeriodStart && refPeriodEnd > d1,
                "Invalid reference period");
            // estimate roughly the length in months of a period
            int months = int(0.5+12*double(refPeriodEnd-refPeriodStart)/365);
            QL_REQUIRE(months != 0 && 12%months == 0,
                "number of months does not divide 12 exactly");
            double period = double(months)/12;
            if (d2 <= refPeriodEnd) {
                if (d1 >= refPeriodStart)
                    return period*double(dayCount(d1,d2)) /
                        dayCount(refPeriodStart,refPeriodEnd);
                else {
                    Date previousRef = refPeriodStart.plusMonths(-months);
                    return yearFraction(
                                d1,refPeriodStart,previousRef,refPeriodStart) +
                           yearFraction(
                                refPeriodStart,d2,refPeriodStart,refPeriodEnd);
                }
            } else {
                Date nextRef = refPeriodEnd.plusMonths(months);
                double sum =
                    yearFraction(d1,refPeriodEnd,refPeriodStart,refPeriodEnd);
                int i=0;
                Date newRefStart, newRefEnd;
                do {
                    newRefStart = refPeriodEnd.plusMonths(months*i);
                    newRefEnd   = refPeriodEnd.plusMonths(months*(i+1));
                    if (d2 < newRefEnd) {
                        break;
                    } else {
                        sum += period;
                        i++;
                    }
                } while (true);
                sum += yearFraction(newRefStart,d2,newRefStart,newRefEnd);
                return sum;
            }
        }

    }

}
