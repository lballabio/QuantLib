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
    Revision 1.1  2001/03/15 13:49:35  marmar
    getCovariance function added


"""

import QuantLib
from TestUnit import TestUnit

def initCovariance(vol, corr):
    n = len(vol)
    cov = QuantLib.Matrix(n,n)
    if(n != corr.rows()):
        raise Exception(
            "correlation matrix and volatility vector have different size")
    for i in range(n):
        cov[i][i] = vol[i]*vol[i]
        for j in range(i):
            cov[i][j] = corr[i][j]*vol[i]*vol[j]
            cov[j][i] = cov[i][j]
    return cov


class GetCovarianceTest(TestUnit):
    def doTest(self):

        vol = QuantLib.Array([0.1, 0.5, 1.0])
        corr = QuantLib.Matrix(3, 3)
        corr[0][0] = 1.0; corr[0][1] = 0.2; corr[0][2] = 0.5
        corr[1][0] = 0.2; corr[1][1] = 1.0; corr[1][2] = 0.8
        corr[2][0] = 0.5; corr[2][1] = 0.8; corr[2][2] = 1.0
            
        expectedCov = initCovariance(vol, corr) 
        cov = QuantLib.getCovariance(vol, corr)
            
        numErrors = 0
        for i in range(3):
            for j in range(3):
                if abs(cov[i][j] - expectedCov[i][j]) > 1e-10:
                    numErrors  = numErrors + 1
            
        self.printDetails("QuantLib.getCovariance", cov)            
        self.printDetails("Python.getCovariance  ", expectedCov)            
        return numErrors

if __name__ == '__main__':
    GetCovarianceTest().test("getCovariance")
