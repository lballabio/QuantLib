
"""
    $Source$
    $Log$
    Revision 1.1  2001/02/22 14:43:36  lballabio
    Renamed test script to follow a single naming scheme

    Revision 1.2  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.1  2001/01/19 09:34:25  nando
    RiskTool is now RiskStatistics everywhere

    Revision 1.6  2001/01/18 14:40:17  nando
    no message

    Revision 1.5  2001/01/18 09:12:12  nando
    improved RiskStatistics test


"""

from QuantLib import RiskStatistics, NormalDistribution
from TestUnit import TestUnit
from math import exp, sqrt


# define a Gaussian
def gaussian(x, average, sigma):
    normFact = sigma * sqrt( 2 * 3.14159265358979323846 )
    dx = x-average
    return exp( -dx*dx/(2.0*sigma*sigma) ) / normFact

class RiskStatisticsTest(TestUnit):
    def doTest(self):
        
        s = RiskStatistics()
        averageRange = [-100.0, 0.0, 100.0]
        sigmaRange = [0.1, 1.0, 10]
        N = 25000
        numberOfSigma = 15
        
        for average in averageRange:
            for sigma in sigmaRange:
                
                #target cannot be changed:
                #it is a strong assumption to compute values to be checked
                target = average
                self.printDetails(
                    'Gaussian distribution with average %g and sigma %g' %
                    (average,sigma)
                )
                
                normal = NormalDistribution(average, sigma)
                
                dataMin = average - numberOfSigma*sigma
                dataMax = average + numberOfSigma*sigma
                # even NOT to include average
                h = (dataMax-dataMin)/(N-1)
                
                data = [0]*N		# creates a list of N elements
                for i in range(N):
                	data[i] = dataMin+h*i
                
                weights = map(lambda x,average=average,sigma=sigma: 
                    gaussian(x,average,sigma), data)
                s.addWeightedSequence(data, weights)
                
                samples = s.samples()
                self.printDetails('\tsamples:            %9d' % samples)
                if (samples!=N):
                    self.printDetails(
                        'wrong number of samples (%f): should be %f' % 
                        (samples, N)
                    )
                    raise Exception('wrong number of sample')
                
                rightWeightSum = reduce(lambda x,y: x+y, weights)
                weightSum = s.weightSum()
                self.printDetails('\tsum of the weights: %9.2f' % weightSum)
                if (weightSum!=rightWeightSum):
                    self.printDetails(
                        'wrong weigth sum (%f): should be %f' % 
                        (weightSum, rightWeightSum)
                    )
                    raise Exception('wrong weigth sum')
                
                minDist = s.min()
                self.printDetails('\tminimum value:      %9.3g' % minDist)
                if (s.min()!=dataMin):
                    self.printDetails(
                        'wrong min value (%f): should be %f' %
                        (minDist, dataMin)
                    )
                    raise Exception('wrong min value')
                
                maxDist = s.max()
                self.printDetails('\tmaximum value:      %9.3g' % maxDist)
                if (abs(maxDist-dataMax)>1e-13):
                    self.printDetails(
                        'wrong max value (%f): should be %f' % 
                        (maxDist, dataMax)
                    )
                    raise Exception('wrong max value')
                
                mean = s.mean()
                if average == 0.0:
                    check = abs(mean-average)
                else:
                    check = abs(mean-average)/average
                self.printDetails('\tmean value:         %9.3f' % mean)
                if (check>1e-13):
                    self.printDetails(
                        'wrong mean value (%f): should be %f' % 
                        (mean, average)
                    )
                    raise Exception('wrong mean value')
                
                self.printDetails(
                    '\terror estimate:     %9.3g (not checked)' % 
                    s.errorEstimate()
                )
                
                variance = s.variance()
                self.printDetails('\tvariance:           %9.3f' % variance)
                if (abs(variance-sigma*sigma)/(sigma*sigma)>1e-4):
                    self.printDetails(
                        'wrong variance (%f): should be %f' % 
                        (variance, sigma*sigma)
                    )
                    raise Exception('wrong variance')
                
                stdev = s.standardDeviation()
                self.printDetails('\tstandard deviation: %9.3f' % stdev)
                if (abs(stdev-sigma)/sigma>1e-4):
                    self.printDetails(
                        'wrong standard deviation (%f): should be %f' % 
                        (stdev, sigma)
                    )
                    raise Exception('wrong standard deviation')
                
                skew = s.skewness()
                self.printDetails('\tskewness:           %+9.1e' % skew)
                if abs(skew)>1e-4:
                    self.printDetails(
                        'wrong skewness (%f): should be %f' % 
                        (skew, 0.0)
                    )
                    raise Exception('wrong skewness')
                
                kurt = s.kurtosis()
                self.printDetails('\texcess kurtosis:    %+9.1e' % kurt)
                if (abs(kurt)>1e-1):
                    self.printDetails(
                        'wrong kurtosis (%f): should be %f' % 
                        (kurt, 0.0)
                    )
                    raise Exception('wrong kurtosis')
                
                rightVAR = -min(average-2.0*sigma, 0.0)
                VAR = s.valueAtRisk(0.9772)
                if rightVAR == 0.0:
                    check = abs(VAR-rightVAR)
                else:
                    check = abs(VAR-rightVAR)/rightVAR
                self.printDetails('\tValue-at-Risk:      %9.3f' % VAR)
                if (check>1e-3):
                    self.printDetails(
                        'wrong valueAtRisk (%f): should be %f' % 
                        (VAR, rightVAR)
                    )
                    raise Exception('wrong valueAtRisk')
                
                rightShortfall = 0.5
                shortfall = s.shortfall(target)
                self.printDetails('\tshortfall:          %9.3f' % shortfall)
                if abs(shortfall-rightShortfall)/rightShortfall>1e-8:
                    self.printDetails(
                        'wrong shortfall (%f): should be %f' % 
                        (shortfall, rightShortfall)
                    )
                    raise Exception('wrong shortfall')
                
                rightAvgShortfall = sigma/sqrt( 2 * 3.14159265358979323846 )
                avgShortfall = s.averageShortfall(target)
                self.printDetails('\taverageShortfall:   %9.3f' % avgShortfall)
                if abs(avgShortfall-rightAvgShortfall)/rightAvgShortfall>1e-4:
                    self.printDetails(
                        'wrong average shortfall (%f): should be %f' % 
                        (avgShortfall, rightAvgShortfall)
                    )
                    raise Exception('wrong average shortfall')
                
                s.reset()


if __name__ == '__main__':
    RiskStatisticsTest().test('risk statistics')
