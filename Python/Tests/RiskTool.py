
"""
    $Source$
    $Log$
    Revision 1.4  2001/01/17 16:34:31  nando
    new improved version

    Revision 1.3  2001/01/16 11:33:23  nando
    updated RiskTool.
    now constructor doesn't require target, while
    shortfall and averageShortfall require target as input parameter

    Revision 1.2  2001/01/15 13:27:07  aleppo
    improved Python test

    Revision 1.1  2001/01/12 17:35:17  nando
    added RiskTool.
    It offres VAR, shortfall, average shortfall methods.
    It still needs test

"""

from QuantLib import RiskTool, NormalDistribution
import time
from math import exp, sqrt


# define a Gaussian
def gaussian(x, average, sigma):
    normFact = sigma * sqrt( 2 * 3.14159265358979323846 )
    dx = x-average
    return exp( -dx*dx/(2.0*sigma*sigma) ) / normFact


startTime = time.time()
print 'Testing RiskTool'
s = RiskTool()

average = -100.0
sigma = 1.0
#cannot be changed, it is a strong assumption to compute values to be checked
target = average


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

maxDist = s.max()
print 'max .............. %9.3g' % maxDist,
if (maxDist==dataMax):
    print 'OK'
else:
    print
    print 'wrong max value %f should be %f' % (maxDist, dataMax)
    print 'error %e' % (abs(maxDist-dataMax))
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
if (abs(s.standardDeviation()-sigma)<1e-3):
    print 'OK'
else:
    print
    print 'wrong standard deviation %f should be %f' % (s.standardDeviation(), \
                                                                         sigma)
    print 'error %e' % (abs(s.standardDeviation()))
    raise 'wrong standard deviation'

print 'skewness ......... %+9.1e' % s.skewness(),
if abs(s.skewness())<1e-7:
    print 'OK'
else:
    print
    print 'wrong skewness %f should be %f' % (s.skewness(), 0.0)
    print 'error %e' % (abs(s.skewness()))
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

rightVAR = -min(average-2.0*sigma, 0.0)
VAR = s.valueAtRisk(0.9772)
print 'Value-at-Risk .... %9.3f' % VAR,
if (abs(VAR-rightVAR)<1e-3):
    print 'OK'
else:
    print
    print 'wrong valueAtRisk %f should be %f' % (VAR, rightVAR)
    print 'error %e' % (abs(VAR-rightVAR))
    raise 'wrong valueAtRisk'


rightShortfall = 0.5
shortfall = s.shortfall(target)
print 'shortfall ........ %9.3f' % shortfall,
if abs(shortfall-rightShortfall)<1e-9:
    print 'OK'
else:
    print
    print 'wrong shortfall %f should be %f' % (shortfall, rightShortfall)
    print 'error %e' % (abs(shortfall-rightShortfall))
    raise 'wrong shortfall'


rightAverageShortfall = sigma/sqrt( 2 * 3.14159265358979323846 )
averageShortfall = s.averageShortfall(target)
print 'averageShortfall . %9.3f' % averageShortfall,
if abs(averageShortfall-rightAverageShortfall)<1e-4:
    print 'OK'
else:
    print
    print 'wrong average shortfall %f should be %f' % (averageShortfall,
                                                    rightAverageShortfall)
    print 'error %e' % ( abs(averageShortfall - rightAverageShortfall) )
    raise 'wrong average shortfall'


s.reset()

print
print 'Test passed (elapsed time', time.time() - startTime, ')'
print 'Press return to end this test'
raw_input()
