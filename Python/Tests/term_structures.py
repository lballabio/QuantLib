"""
 Copyright (C) 2000-2001 QuantLib Group

 This file is part of QuantLib.
 QuantLib is a C++ open source library for financial quantitative
 analysts and developers --- http://quantlib.sourceforge.net/

 QuantLib is free software and you are allowed to use, copy, modify, merge,
 publish, distribute, and/or sell copies of it under the conditions stated
 in the QuantLib License.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.

 You should have received a copy of the license along with this file;
 if not, contact ferdinando@ametrano.net
 The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT

 The members of the QuantLib Group are listed in the Authors.txt file, also
 available at http://quantlib.sourceforge.net/Authors.txt
"""

"""
    $Source$
    $Log$
    Revision 1.3  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

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

import QuantLib
import unittest
import time

class TermStructureTest(unittest.TestCase):
    def runTest(self):
        "Testing term structure bootstrapping"
        gmt = time.gmtime(time.time())
        today =  QuantLib.Date(gmt[2],gmt[1],gmt[0])

        currency = QuantLib.EUR()
        depoDayCount = QuantLib.Actual360()
        curveDayCount = QuantLib.Actual365()
        depoCalendar = QuantLib.TARGET()
        deposits = [
            QuantLib.DepositRate(depoCalendar.roll(today.plusWeeks(1)),
                0.0426,  depoDayCount),
            QuantLib.DepositRate(depoCalendar.roll(today.plusMonths(1)),
                0.04381, depoDayCount),
            QuantLib.DepositRate(depoCalendar.roll(today.plusMonths(2)),
                0.04423, depoDayCount),
            QuantLib.DepositRate(depoCalendar.roll(today.plusMonths(3)),
                0.04486, depoDayCount),
            QuantLib.DepositRate(depoCalendar.roll(today.plusMonths(6)),
                0.0464,  depoDayCount),
            QuantLib.DepositRate(depoCalendar.roll(today.plusMonths(9)),
                0.0481,  depoDayCount),
            QuantLib.DepositRate(depoCalendar.roll(today.plusYears(1)),
                0.04925, depoDayCount)
        ]

        curve = QuantLib.PiecewiseConstantForwards(currency,
                    curveDayCount,today,deposits)

        flatCurve =  QuantLib.FlatForward(currency,curveDayCount,today,0.05)

        days = 380
        settlement = curve.settlementDate()

        ytime     = map(lambda i : i/365.0, range(days))
        dates     = map(settlement.plusDays,range(days))

        discounts = map(lambda x,curve=curve: curve.discount(x, 1),  dates)
        zeros     = map(lambda x,curve=curve: curve.zeroYield(x, 1), dates)
        forwards  = map(lambda x,curve=curve: curve.forward(x, 1),   dates)

        # we might add some meaningful tests here


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(TermStructureTest())
    unittest.TextTestRunner().run(suite)

