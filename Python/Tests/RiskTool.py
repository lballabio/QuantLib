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
    Revision 1.1  2001/01/12 17:35:17  nando
    added RiskTool.
    It offres VAR, shortfall, average shortfall methods.
    It still needs test

"""

from QuantLib import *
import time
from math import exp, sqrt


# define a Gaussian
def gaussian(x, average, sigma):
    normFact = sigma * sqrt( 2 * 3.14159265358979323846 )
    dx = x-average
    return exp( -dx*dx/(2.0*sigma*sigma) ) / normFact


startTime = time.time()
print 'Testing RiskTool'

average = 0.0
sigma = 1.0

print 'gaussian distribution with average =', average, 'and sigma =', sigma


normal = NormalDistribution(average, sigma)

dataMin = average - 10*sigma
dataMax = average + 10*sigma
# even NOT to include average
N = 10000
h = (dataMax-dataMin)/(N-1)

data = [0]*N		# creates a list of N elements
for i in range(N):
	data[i] = dataMin+h*i

weights = map(lambda x: gaussian(x,average,sigma), data)

s = RiskTool(average/2.0)
s.addWeightedSequence(data, weights)

print 'samples .......... %9d' % s.samples(),
if (s.samples()==N):
    print 'OK'
else:
    raise 'wrong number of sample'

print 'Sum of the weights %9.2f' % s.weightSum(),
if (s.weightSum()==reduce(lambda x,y: x+y, weights)):
    print 'OK'
else:
    raise 'wrong weigth sum'

print 'min .............. %9.3g' % s.min(),
if (s.min()==dataMin):
    print 'OK'
else:
    raise 'wrong min value'

print 'max .............. %9.3g' % s.max(),
if (s.max()==dataMax):
    print 'OK'
else:
    raise 'wrong max value'

print 'mean ............. %9.3f' % s.mean(),
if (abs(s.mean()-average)<1e-3):
    print 'OK'
else:
    raise 'wrong mean value'

print 'variance ......... %9.3f' % s.variance(),
if (abs(s.variance()-sigma*sigma)<1e-3):
    print 'OK'
else:
    raise 'wrong variance'

print 'standard Deviation %9.3f' % s.standardDeviation(),
if (abs(s.standardDeviation()-sigma)<1e-4):
    print 'OK'
else:
    raise 'wrong standard deviation'

print 'skewness ......... %+9.1e' % s.skewness(),
if (abs(s.skewness())<1e-15):
    print 'OK'
else:
    raise 'wrong skewness'

print 'excess kurtosis .. %+9.1e' % s.kurtosis(), 
if (abs(s.kurtosis())<1e-3):
    print 'OK'
else:
    raise 'wrong kurtosis'

print 'error estimate ... %9.3g' % s.errorEstimate(),
if (1):
    print 'not checked'
else:
    raise 'wrong'

print 'VAR .............. %9.3f' % s.valueAtRisk(0.99),
if (1):
    print 'not checked'
else:
    raise 'wrong'

print 'shortfall ........ %9.3f' % s.shortfall(),
if (abs(s.shortfall()-s.weightSum()/2.0)<1e-10):
    print 'OK'
else:
    print
    print 'wrong shortfall %f should be %f' % (s.shortfall(), s.weightSum()/2.0)
    raise 'wrong shortfall'
  
print 'averageShortfall . %9.3f' % s.averageShortfall(),
if (1):
    print 'not checked'
else:
    raise 'wrong'

print
print 'Test passed (elapsed time', time.time() - startTime, ')'
print 'Press return to end this test'
raw_input()
