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
    Revision 1.6  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.5  2001/02/15 11:57:34  nando
    no message

    Revision 1.4  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.3  2001/01/08 15:33:23  nando
    improved

"""

from QuantLib import Statistics, UniformRandomGenerator, GaussianRandomGenerator
from TestUnit import TestUnit

class RNGTest(TestUnit):
    def doTest(self):
        tol = 1e-9
        seed = 576919
        numIte = 1000000
        self.printDetails(
            "Generator                          "
            "mean    sigma    skew    kurt   min   max"
        )
        for RNG in [UniformRandomGenerator, GaussianRandomGenerator]:
            rn = RNG(seed=seed)
            s = Statistics()
            for ite in range(numIte):
                s.add(rn.next())
            self.printDetails(
                "%32s %+8.4f %7.4f %7.3f %7.3f %5.2f %5.2f " % 
                (RNG, s.mean(), s.standardDeviation(), s.skewness(), 
                s.kurtosis(), s.min(), s.max())
            )


if __name__ == '__main__':
    RNGTest().test('random number generators')

