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
    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/05/16 14:16:09  lballabio
    Using more meaningful rates

    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

"""

from QuantLib import *
import unittest

class PiecewiseFlatForwardTest(unittest.TestCase):
    def setUp(self):
        import time
        gmt = time.gmtime(time.time())
        self.today =  Date(gmt[2],gmt[1],gmt[0])
        self.ns = \
            [     1,      1,       2,       3,       6,       9,     1]
        self.units = \
            ['week','month','months','months','months','months','year']
        self.rates = \
            [ 4.591,  4.593,   4.583,   4.572,   4.490,   4.455, 4.433]
        self.modified = 1
        self.dayCount = Actual360()
        self.instruments = map(
            lambda n,unit,rate,mod=self.modified,dayCount=self.dayCount: 
                DepositRateHelper(n,unit,mod,rate/100,dayCount),
            self.ns,
            self.units,
            self.rates)
    def runTest(self):
        "Testing piecewise flat forward curve"
        termStructure = \
            PiecewiseFlatForward(EUR(),Actual360(),self.today,self.instruments)
        LiborManager.setTermStructure(EUR(),termStructure)
        # check
        settlement = termStructure.settlementDate()
        index = Euribor()
        for i in range(len(self.rates)):
            estimatedRate = index.fixing(settlement,self.ns[i],self.units[i])
            assert abs(estimatedRate - self.rates[i]/100) <= 1.0e-9, \
                "estimated rate: %12.10f\n" % estimatedRate + \
                "input rate:     %12.10f\n" % (self.rates[i]/100) + \
                "tolerance exceeded\n"

if __name__ == '__main__':
    import sys
    suite = unittest.TestSuite()
    suite.addTest(PiecewiseFlatForwardTest())
    if sys.hexversion >= 0x020100f0:
        unittest.TextTestRunner(verbosity=2).run(suite)
    else:
        unittest.TextTestRunner().run(suite)
    raw_input('press any key to continue')

    