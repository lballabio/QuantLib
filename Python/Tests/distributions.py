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
    Revision 1.11  2001/04/27 10:44:15  lballabio
    Support for unittest in Python 2.1

    Revision 1.10  2001/04/18 09:03:22  nando
    added/removed final
    raw_input('press any key to continue')

    Revision 1.9  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

    Revision 1.8  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.7  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.6  2001/03/05 17:18:21  lballabio
    Using math.pi

    Revision 1.5  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.4  2001/01/11 18:10:32  nando
    generalized to sigma<>1 and average<>0
    Also added NormalDistribution.derivative().
    Improved Python test now also uses Finite Difference
    first and second order operators

    Revision 1.3  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.2  2001/01/08 15:33:09  nando
    improved
"""

import QuantLib
import unittest
from math import exp, sqrt, pi

# define a Gaussian
def gaussian(x, average, sigma):
    normFact = sigma * sqrt( 2 * pi )
    dx = x-average
    return exp( -dx*dx/(2.0*sigma*sigma) ) / normFact

def gaussianDerivative(x, average, sigma):
    normFact = sigma * sigma * sigma * sqrt( 2 * pi )
    dx = x-average
    return - dx * exp( -dx*dx/(2.0*sigma*sigma) ) / normFact

# define the norm of a discretized function
def norm(f,h):
    # squared values
    f2 = map(lambda x: x*x, f)
    # numeric integral of f^2
    I = h*(reduce(lambda x,y: x+y, f2)-0.5*f2[0]-0.5*f2[-1])
    return sqrt(I)


class DistributionTest(unittest.TestCase):
    def runTest(self):
        "Testing distributions"
        average = 0.0
        sigma = 1.0
        normal = QuantLib.NormalDistribution(average, sigma)
        cum = QuantLib.CumulativeNormalDistribution(average, sigma)
        invCum = QuantLib.InvCumulativeNormalDistribution(average, sigma)

        xMin = average - 4*sigma
        xMax = average + 4*sigma
        # odd in include average
        N = 10001
        h = (xMax-xMin)/(N-1)

        x = [0]*N		# creates a list of N elements
        for i in range(N):
            x[i] = xMin+h*i

        y = map(lambda x,average=average,sigma=sigma:
            gaussian(x,average,sigma), x)

        yIntegrated = map(cum, x)
        yTemp = map(normal, x)
        y2Temp = map(cum.derivative, x)
        xTemp = map(invCum, yIntegrated)
        yd = map(normal.derivative, x)
        ydTemp = map(lambda x,average=average,sigma=sigma:
            gaussianDerivative(x,average,sigma), x)

        #check norm=gaussian
        e = norm(map(lambda x,y:x-y,yTemp,y),h)
        assert e <= 1.0e-16, \
            "norm of C++ NormalDistribution minus analytic gaussian: " + \
            "%5.2e\n" % e + \
            "tolerance exceeded\n"

        #check invCum(cum) = Identity
        e = norm(map(lambda x,y:x-y,xTemp,x),h)
        assert e <= 1.0e-3, \
            "norm of C++ invCum(cum(.)) minus identity: %5.2e\n" % e + \
            "tolerance exceeded\n"

        #check cum.derivative=normal
        e = norm(map(lambda x,y:x-y,y2Temp,y),h)
        assert e <= 1.0e-16, \
            "norm of C++ Cumulative.derivative minus analytic gaussian: " + \
            "%5.2e\n" % e + \
            "tolerance exceeded\n"

        #check normal.derivative=gaussianDerivative
        e = norm(map(lambda x,y:x-y,ydTemp,yd),h)
        assert e <= 1.0e-16, \
            "norm of C++ NormalDist.derivative minus " + \
            "analytic gaussian.derivative: %5.2e\n" % e + \
            "tolerance exceeded\n"

        # ... and now let's toy with finite difference
        # define the first derivative operators
        D = QuantLib.DZero(N,h)
        D2 = QuantLib.DPlusDMinus(N,h)
        # and calculate the derivatives
        y3Temp  = D.applyTo(yIntegrated)
        yd2Temp = D2.applyTo(yIntegrated)

        #check finite difference first order derivative operator = gaussian
        e = norm(map(lambda x,y:x-y,y3Temp,y),h)
        assert e <= 1.0e-6, \
           "norm of C++ FD 1st deriv. of cum minus analytic gaussian: " + \
           "%5.2e\n" % e + \
           "tolerance exceeded\n"

        # check finite difference second order derivative operator =
        # normal.derivative
        e = norm(map(lambda x,y:x-y,yd2Temp,yd),h)
        assert e <= 1.0e-4, \
            "norm of C++ FD 2nd deriv. of cum minus " + \
            "analytic gaussian.derivative: %5.2e\n" % e + \
            "tolerance exceeded\n"


if __name__ == '__main__':
    import sys
    suite = unittest.TestSuite()
    suite.addTest(DistributionTest())
    if sys.hexversion >= 0x020100f0:
        unittest.TextTestRunner(verbosity=2).run(suite)
    else:
        unittest.TextTestRunner().run(suite)
    raw_input('press any key to continue')

