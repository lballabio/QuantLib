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
    Revision 1.3  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.2  2001/03/15 13:49:35  marmar
    getCovariance function added

    Revision 1.1  2001/02/22 14:43:36  lballabio
    Renamed test script to follow a single naming scheme

    Revision 1.2  2001/02/22 14:27:26  lballabio
    Implemented new test framework

"""

import QuantLib
import unittest

class PlainBasketOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing plain basket option pricer"
        cor = QuantLib.Matrix(4,4)
        cor[0][0] = 1.00; cor[0][1] = 0.50; cor[0][2] = 0.30; cor[0][3] = 0.10
        cor[1][0] = 0.50; cor[1][1] = 1.00; cor[1][2] = 0.20; cor[1][3] = 0.40
        cor[2][0] = 0.30; cor[2][1] = 0.20; cor[2][2] = 1.00; cor[2][3] = 0.60
        cor[3][0] = 0.10; cor[3][1] = 0.40; cor[3][2] = 0.60; cor[3][3] = 1.00
        
        volatility = [ 0.3,  0.3,  0.3,  0.3]
        covariance = QuantLib.getCovariance(volatility, cor)
        
        assetValues = [100, 100, 100, 100]
        dividendYields = [0.0, 0.0, 0.0, 0]
        riskFreeRate = 0.0
        resTime = 1.0
        timeSteps = 1
        samples = 40000
        seed = 2417321
        
        pbo = QuantLib.PlainBasketOption(assetValues, dividendYields,
                covariance, riskFreeRate, resTime, timeSteps, samples, seed)
        value = pbo.value()
        error = pbo.errorEstimate()
        storedValue = 125.633568131
        storedError = 0.15961942869
        assert abs(value-storedValue) <= 1e-8, \
            "calculated value: %g\n" % value + \
            "stored value:     %g\n" % storedValue + \
            "tolerance exceeded\n"
        assert abs(error-storedError) <= 1e-8, \
            "calculated error: %g\n" % error + \
            "stored error:     %g\n" % storedError + \
            "tolerance exceeded\n"


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(PlainBasketOptionTest())
    unittest.TextTestRunner().run(suite)

    
