
"""
    $Source$
    $Log$
    Revision 1.6  2001/01/18 14:40:17  nando
    no message

    Revision 1.5  2001/01/18 09:12:12  nando
    improved RiskTool test


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

averageRange = [-100.0, 0.0, 100.0]
sigmaRange = [0.1, 1.0, 10]
N = 25000
numberOfSigma = 15


for average in averageRange:
    for sigma in sigmaRange:

        #target cannot be changed:
        #it is a strong assumption to compute values to be checked
        target = average

        print 'gaussian distribution with average =', average,
        print 'and sigma =', sigma


        normal = NormalDistribution(average, sigma)

        dataMin = average - numberOfSigma*sigma
        dataMax = average + numberOfSigma*sigma
        # even NOT to include average
        h = (dataMax-dataMin)/(N-1)

        data = [0]*N		# creates a list of N elements
        for i in range(N):
        	data[i] = dataMin+h*i

        weights = map(lambda x: gaussian(x,average,sigma), data)

        s.addWeightedSequence(data, weights)

        samples = s.samples()
        print 'samples .......... %9d' % samples,
        if (samples==N):
            print 'OK'
        else:
            print
            print 'wrong number of samples %f should be %f' % (samples, N)
            print 'error %e' % (abs(samples-N))
            raise 'wrong number of sample'


        rightWeightSum = reduce(lambda x,y: x+y, weights)
        weightSum = s.weightSum()
        print 'Sum of the weights %9.2f' % weightSum,
        if (weightSum==rightWeightSum):
            print 'OK'
        else:
            print
            print 'wrong weigth sum %f should be %f' % (weightSum, \
                                                    rightWeightSum)
            print 'error %e' % (abs(weightSum-rightWeightSum))
            raise 'wrong weigth sum'


        minDist = s.min()
        print 'min .............. %9.3g' % minDist,
        if (s.min()==dataMin):
            print 'OK'
        else:
            print
            print 'wrong min value %f should be %f' % (minDist, dataMin)
            print 'error %e' % (abs(minDist-dataMin))
            raise 'wrong min value'


        maxDist = s.max()
        print 'max .............. %9.3g' % maxDist,
        if (abs(maxDist-dataMax)<1e-13):
            print 'OK'
        else:
            print
            print 'wrong max value %f should be %f' % (maxDist, dataMax)
            print 'error %e' % (abs(maxDist-dataMax))
            raise 'wrong max value'



        mean = s.mean()
        if average == 0.0:
            check = abs(mean-average)
        else:
            check = abs(mean-average)/average
        print 'mean ............. %9.3f' % mean,
        if (check<1e-13):
            print 'OK'
        else:
            print
            print 'wrong mean value %f should be %f' % (mean, average)
            print 'error %e' % (abs(mean-average))
            raise 'wrong mean value'

        print 'error estimate ... %9.3g' % s.errorEstimate(),
        if (1):
            print 'not checked'
        else:
            raise 'wrong'

        variance = s.variance()
        print 'variance ......... %9.3f' % variance,
        if (abs(variance-sigma*sigma)/(sigma*sigma)<1e-4):
            print 'OK'
        else:
            print
            print 'wrong variance %f should be %f' % (variance, sigma*sigma)
            print 'error %e' % (abs(variance-sigma*sigma))
            raise 'wrong variance'

        stdev = s.standardDeviation()
        print 'standard Deviation %9.3f' % stdev,
        if (abs(stdev-sigma)/sigma<1e-4):
            print 'OK'
        else:
            print
            print 'wrong standard deviation %f should be %f' % (stdev, sigma)
            print 'error %e' % abs(stdev-sigma)
            raise 'wrong standard deviation'

        skew = s.skewness()
        print 'skewness ......... %+9.1e' % skew,
        if abs(skew)<1e-4:
            print 'OK'
        else:
            print
            print 'wrong skewness %f should be %f' % (skew, 0.0)
            print 'error %e' % (abs(skew))
            raise 'wrong skewness'

        kurt = s.kurtosis()
        print 'excess kurtosis .. %+9.1e' % kurt,
        if (abs(kurt)<1e-1):
            print 'OK'
        else:
            print
            print 'wrong kurtosis %f should be %f' % (kurt, 0.0)
            print 'error %e' % (abs(kurt))
            raise 'wrong kurtosis'

        rightVAR = -min(average-2.0*sigma, 0.0)
        VAR = s.valueAtRisk(0.9772)
        if rightVAR == 0.0:
            check = abs(VAR-rightVAR)
        else:
            check = abs(VAR-rightVAR)/rightVAR
        print 'Value-at-Risk .... %9.3f' % VAR,
        if (check<1e-3):
            print 'OK'
        else:
            print
            print 'wrong valueAtRisk %f should be %f' % (VAR, rightVAR)
            print 'error %e' % (abs(VAR-rightVAR))
            raise 'wrong valueAtRisk'


        rightShortfall = 0.5
        shortfall = s.shortfall(target)
        print 'shortfall ........ %9.3f' % shortfall,
        if abs(shortfall-rightShortfall)/rightShortfall<1e-8:
            print 'OK'
        else:
            print
            print 'wrong shortfall %f should be %f' % (shortfall, \
                                                    rightShortfall)
            print 'error %e' % (abs(shortfall-rightShortfall))
            raise 'wrong shortfall'


        rightAvrgShortfall = sigma/sqrt( 2 * 3.14159265358979323846 )
        avrgShortfall = s.averageShortfall(target)
        print 'averageShortfall . %9.3f' % avrgShortfall,
        if abs(avrgShortfall-rightAvrgShortfall)/rightAvrgShortfall<1e-4:
            print 'OK'
        else:
            print
            print 'wrong average shortfall %f should be %f' % (avrgShortfall, \
                                                            rightAvrgShortfall)
            print 'error %e' % ( abs(avrgShortfall - rightAvrgShortfall) )
            raise 'wrong average shortfall'


        s.reset()







print
print 'Test passed (elapsed time', time.time() - startTime, ')'
print 'Press return to end this test'
raw_input()
