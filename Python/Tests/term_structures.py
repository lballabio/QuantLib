"""
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
 *
 * QuantLib license is also available at:
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/
"""

"""
    $Source$
    $Log$
    Revision 1.1  2001/02/22 14:43:36  lballabio
    Renamed test script to follow a single naming scheme

    Revision 1.3  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.2  2001/02/01 11:10:29  nando
    almost a test

    Revision 1.2  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.1  2001/01/08 15:12:45  nando
    added test for date and distributions

"""

from QuantLib import *
from TestUnit import TestUnit
import time

class TermStructureTest(TestUnit):
    def doTest(self):
        
        gmt = time.gmtime(time.time())
        today =  Date(gmt[2],gmt[1],gmt[0])
        
        currency =  EUR()
        depoDayCount = Actual360()
        curveDayCount =  Actual365()
        depoCalendar =  TARGET()
        deposits = [
            DepositRate(depoCalendar.roll(today.plusWeeks(1)), 
                0.0426,  depoDayCount),
            DepositRate(depoCalendar.roll(today.plusMonths(1)),
                0.04381, depoDayCount),
            DepositRate(depoCalendar.roll(today.plusMonths(2)),
                0.04423, depoDayCount),
            DepositRate(depoCalendar.roll(today.plusMonths(3)),
                0.04486, depoDayCount),
            DepositRate(depoCalendar.roll(today.plusMonths(6)),
                0.0464,  depoDayCount),
            DepositRate(depoCalendar.roll(today.plusMonths(9)),
                0.0481,  depoDayCount),
            DepositRate(depoCalendar.roll(today.plusYears(1)), 
                0.04925, depoDayCount)
        ]
        
        curve = PiecewiseConstantForwards(currency,curveDayCount,today,deposits)
        
        flatCurve =  FlatForward(currency,curveDayCount,today,0.05)
        
        days = 380
        settlement = curve.settlementDate()
        
        ytime     = map(lambda i : i/365.0, range(days))
        dates     = map(settlement.plusDays,range(days))
        
        discounts = map(lambda x,curve=curve: curve.discount(x, 1),  dates)
        zeros     = map(lambda x,curve=curve: curve.zeroYield(x, 1), dates)
        forwards  = map(lambda x,curve=curve: curve.forward(x, 1),   dates)
        
        self.printDetails(
            ' time                dates          disc    zero   forward'
        )
        for i in range(days):
            self.printDetails(
                '%5.3f %20s %13.10f %7.3f %9.3f' % 
                (ytime[i], dates[i], discounts[i], zeros[i]*100, 
                forwards[i]*100)
            )


if __name__ == '__main__':
    TermStructureTest().test('term structure bootstrapping')

