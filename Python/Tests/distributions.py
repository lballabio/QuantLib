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

from QuantLib import *
import time
from math import exp, sqrt


# define a Gaussian
def gaussian(x, average, sigma):
    normFact = sigma * sqrt( 2 * 3.14159265358979323846 )
    dx = x-average
    return exp( -dx*dx/(2.0*sigma*sigma) ) / normFact

def gaussianDerivative(x, average, sigma):
    normFact = sigma * sigma * sigma * sqrt( 2 * 3.14159265358979323846 )
    dx = x-average
    return - dx * exp( -dx*dx/(2.0*sigma*sigma) ) / normFact

# define the norm of a discretized function
def norm(f,h):
    # squared values
    f2 = map(lambda x: x*x, f)
    # numeric integral of f^2
    I = h*(reduce(lambda x,y: x+y, f2)-0.5*f2[0]-0.5*f2[-1])
    return sqrt(I)


startTime = time.time()

print 'Testing distributions'

average = 0.0
sigma = 1.0
print 'gaussian distribution with average =', average, 'and sigma =', sigma

normal = NormalDistribution(average, sigma)
cum = CumulativeNormalDistribution(average, sigma)
invCum = InvCumulativeNormalDistribution(average, sigma)

xMin = average - 4*sigma
xMax = average + 4*sigma
# odd in include average
N = 10001
h = (xMax-xMin)/(N-1)

x = [0]*N		# creates a list of N elements
for i in range(N):
	x[i] = xMin+h*i

y = map(lambda x: gaussian(x,average,sigma), x)

yIntegrated = map(cum, x)
yTemp = map(normal, x)
y2Temp = map(cum.derivative, x)
xTemp = map(invCum, yIntegrated)
yd = map(normal.derivative, x)
ydTemp = map(lambda x: gaussianDerivative(x,average,sigma), x)


print 'norm of:'

#check norm=gaussian
e = norm(map(lambda x,y:x-y,yTemp,y),h)
print 'C++ NormalDistribution    MINUS            analityc gaussian: %5.2e' % e
if e > 1e-16:
    raise "Tolerance exceeded"

#check invCum(cum) = Identity
e = norm(map(lambda x,y:x-y,xTemp,x),h)
print 'C++ invCum(cum(.))        MINUS                     identity: %5.2e' % e
if e > 1e-3:
    raise "Tolerance exceeded"

#check cum.derivative=normal
e = norm(map(lambda x,y:x-y,y2Temp,y),h)
print 'C++ Cumulative.derivative MINUS            analytic gaussian: %5.2e' % e
if e > 1e-16:
    raise "Tolerance exceeded"

#check normal.derivative=gaussianDerivative
e = norm(map(lambda x,y:x-y,ydTemp,yd),h)
print 'C++ NormalDist.derivative MINUS analytic gaussian.derivative: %5.2e' % e
if e > 1e-16:
    raise "Tolerance exceeded"


# ... and now let's toy with finite difference
# define the first derivative operators
D = DZero(N,h)
D2 = DPlusDMinus(N,h)
# and calculate the derivatives
y3Temp  = D.applyTo(yIntegrated)
yd2Temp = D2.applyTo(yIntegrated)

#check finite difference first order derivative operator = gaussian
e = norm(map(lambda x,y:x-y,y3Temp,y),h)
print 'C++ FD 1st deriv. of cum  MINUS            analytic gaussian: %5.2e' % e
if e > 1e-6:
    raise "Tolerance exceeded"

#check finite difference second order derivative operator = normal.derivative
e = norm(map(lambda x,y:x-y,yd2Temp,yd),h)
print 'C++ FD 2nd deriv. of cum  MINUS analytic gaussian.derivative: %5.2e' % e
if e > 1e-4:
    raise "Tolerance exceeded"

print
print 'Test passed (elapsed time', time.time() - startTime, ')'
print 'Press return to end this test'
raw_input()
