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
    Revision 1.5  2001/04/18 09:03:22  nando
    added/removed final
    raw_input('press any key to continue')

    Revision 1.4  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

    Revision 1.3  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.1  2001/02/22 14:43:36  lballabio
    Renamed test script to follow a single naming scheme

    Revision 1.6  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.5  2001/02/13 10:04:25  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.4  2001/02/05 16:57:14  marmar
    McAsianPricer replaced by AveragePriceAsian and AverageStrikeAsian

    Revision 1.3  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.2  2001/01/08 15:33:23  nando
    improved

"""

import QuantLib
import unittest

class MonteCarloPricerTest(unittest.TestCase):
    def runTest(self):
        "Testing MonteCarlo pricers"
        type = "Call"
        underlying = 100
        strike = 100
        dividendYield = 0.0
        riskFreeRate = 0.05
        residualTime = 1.0
        volatility = 0.3
        timesteps = 100
        numIte = 10000
        seed = 3456789
        cases = [[QuantLib.McEuropeanPricer,  14.209699846520,0.226121398649],
                 [QuantLib.AverageStrikeAsian, 8.201621371225,0.124035566770],
                 [QuantLib.AveragePriceAsian,  8.190052086091,0.008114392632]]

        for (pricer,storedValue,storedError) in cases:
            p = pricer(type, underlying, strike, dividendYield, riskFreeRate,
                residualTime, volatility, timesteps, numIte, seed=seed)
            assert abs(p.value()-storedValue) <= 1e-10, \
                "calculated value: %g\n" % p.value() + \
                "stored value:     %g\n" % storedValue + \
                "tolerance exceeded\n"
            assert abs(p.errorEstimate()-storedError) <= 1e-10, \
                "calculated error: %g\n" % p.errorEstimate() + \
                "stored error:     %g\n" % storedError + \
                "tolerance exceeded\n"


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(MonteCarloPricerTest())
    unittest.TextTestRunner().run(suite)
    raw_input('press any key to continue')





