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
    Revision 1.2  2001/03/15 13:49:35  marmar
    getCovariance function added

"""

from QuantLib import Himalaya, Matrix, getCovariance
from TestUnit import TestUnit
from math import fabs


class HimalayaTest(TestUnit):
    def doTest(self):
        cor = Matrix(4,4)
        cor[0][0] = 1.00; cor[0][1] = 0.50; cor[0][2] = 0.30; cor[0][3] = 0.10
        cor[1][0] = 0.50; cor[1][1] = 1.00; cor[1][2] = 0.20; cor[1][3] = 0.40
        cor[2][0] = 0.30; cor[2][1] = 0.20; cor[2][2] = 1.00; cor[2][3] = 0.60
        cor[3][0] = 0.10; cor[3][1] = 0.40; cor[3][2] = 0.60; cor[3][3] = 1.00
        
        volatility = [ 0.3,  0.3,  0.3,  0.3]
        covariance = getCovariance(volatility, cor)
        
        assetValues = [100, 100, 100, 100]
        dividendYields = [0.0, 0.0, 0.0, 0]
        timeIncrements = [0.25, 0.25, 0.25, 0.25]
        riskFreeRate = 0.05
        strike = 100    
        samples = 40000
        seed = 3456789
        
        him = Himalaya(assetValues, dividendYields, covariance,
                       riskFreeRate, strike, timeIncrements, samples, seed)
        
        value = him.value()
        error = him.errorEstimate()
        self.printDetails(
            "Himalaya option price and error: %g %g" %
            (value, error)
        )
        self.printDetails(
            "Stored values:                   %g %g" %
            (7.30213127731,0.0521786123186)
        )
        if fabs(value-7.30213127731) > 1e-10 \
        or fabs(error-0.05217861232) > 1e-10:
            return 1
        else:
            return 0


if __name__ == '__main__':
    HimalayaTest().test('Himalaya option pricer')

    