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

    Revision 1.1  2001/03/07 09:34:02  marmar
    Everest option test added

"""

import QuantLib
import unittest

def initCovariance(corr, vol):
    n = len(vol)
    cov = QuantLib.Matrix(n,n)
    assert n == corr.rows(), \
        "correlation matrix and volatility vector have different size"
    for i in range(n):
        cov[i][i] = vol[i]*vol[i]
        for j in range(i):
            cov[i][j] = corr[i][j]*vol[i]*vol[j]
            cov[j][i] = cov[i][j]
    return cov

class EverestOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing Everest option pricer"
        cor = QuantLib.Matrix(4,4)
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

        everest = QuantLib.EverestOption(dividendYields, covariance,
                            riskFreeRate, resTime,
                            samples, seed)
        value = everest.value()
        error = everest.errorEstimate()
        storedValue = 0.363309646566
        storedError = 0.000501529768684
        assert abs(value-storedValue) <= 1e-10, \
            "calculated value: %g\n" % value + \
            "stored value:     %g\n" % storedValue + \
            "tolerance exceeded\n"
        assert abs(error-storedError) <= 1e-10, \
            "calculated error: %g\n" % error + \
            "stored error:     %g\n" % storedError + \
            "tolerance exceeded\n"


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(EverestOptionTest())
    unittest.TextTestRunner().run(suite)


