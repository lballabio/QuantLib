"""
/*
 * Copyright (C) 2001 QuantLib Group
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
    Revision 1.1  2001/03/07 09:34:02  marmar
    Everest option test added

"""

from QuantLib import Matrix, EverestOption
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

class EverestTest(TestUnit):
    def doTest(self):
        
        cor = Matrix(4,4)
        cor[0][0] = 1.00
        cor[1][0] = 0.50; cor[1][1] = 1.00
        cor[2][0] = 0.30; cor[2][1] = 0.20; cor[2][2] = 1.00
        cor[3][0] = 0.10; cor[3][1] = 0.40; cor[3][2] = 0.60; cor[3][3] = 1.00
        
        volatility = [ 0.3,  0.3,  0.3,  0.3]
        covariance = initCovariance(cor,volatility)
        
        dividendYields = [0.010, 0.005, 0.008, 0.011]
        riskFreeRate = 0.05
        resTime = 10.0
        samples = 400000
        seed = 765432123
        
        everest = EverestOption(dividendYields, covariance,
                            riskFreeRate, resTime,
                            samples, seed)
        value = everest.value()
        error = everest.errorEstimate()
        storedValue = 0.363309646566 
        storedError = 0.000501529768684
        self.printDetails(
            "Everest option price and error: %g %g" %
            (value, error)
        )
        self.printDetails(
            "Stored values:                  %g %g" %
            (storedValue, storedError)
        )
        if fabs(value-storedValue) > 1e-10 \
        or fabs(error-storedError) > 1e-10:
            return 1
        else:
            return 0


if __name__ == '__main__':
    EverestTest().test('everest option pricer')

    