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
    Revision 1.2  2001/02/22 14:27:26  lballabio
    Implemented new test framework

"""

from QuantLib import Matrix, PlainBasketOption
from TestUnit import TestUnit
from math import fabs

def initCovariance(corr, vol):
    n = len(vol)
    cov = Matrix(n,n)
    if(n != corr.rows()):
        print "correlation matrix and volatility vector have different size"
    for i in range(n):
        cov[i][i] = vol[i]*vol[i]
        for j in range(i):
            cov[i][j] = corr[i][j]*vol[i]*vol[j]
            cov[j][i] = cov[i][j]
    return cov

class PlainBasketTest(TestUnit):
    def doTest(self):
        
        cor = Matrix(4,4)
        cor[0][0] = 1.00
        cor[1][0] = 0.50; cor[1][1] = 1.00
        cor[2][0] = 0.30; cor[2][1] = 0.20; cor[2][2] = 1.00
        cor[3][0] = 0.10; cor[3][1] = 0.40; cor[3][2] = 0.60; cor[3][3] = 1.00
        
        volatility = [ 0.3,  0.3,  0.3,  0.3]
        covariance = initCovariance(cor,volatility)
        
        assetValues = [100, 100, 100, 100]
        dividendYields = [0.0, 0.0, 0.0, 0]
        riskFreeRate = 0.0
        resTime = 1.0
        timeSteps = 1
        samples = 40000
        seed = 2417321
        
        pbo = PlainBasketOption(assetValues, dividendYields, covariance,
                    riskFreeRate, resTime, timeSteps, samples, seed)
        value = pbo.value()
        error = pbo.errorEstimate()
        self.printDetails(
            "Basket option price and error: %g %g" %
            (value, error)
        )
        self.printDetails(
            "Stored values:                 %g %g" %
            (125.633568131,0.15961942869)
        )
        if fabs(value-125.633568131) > 1e-8 \
        or fabs(error-0.15961942869) > 1e-8:
            return 1
        else:
            return 0


if __name__ == '__main__':
    PlainBasketTest().test('plain basket option pricer')

    