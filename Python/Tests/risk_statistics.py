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
    Revision 1.4  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.3  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.2  2001/03/05 10:31:30  nando
    Expected Shortfall added to classes HVarTool and HRiskStatistics.
    Expected Shortfall included in python test.

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

import QuantLib
import unittest
from math import exp, sqrt, pi

# define a Gaussian
def gaussian(x, average, sigma):
    normFact = sigma * sqrt( 2 * pi )
    dx = x-average
    return exp( -dx*dx/(2.0*sigma*sigma) ) / normFact

class RiskStatisticsTest(unittest.TestCase):
    def runTest(self):
        "Testing risk statistics"
        s = QuantLib.RiskStatistics()
        averageRange = [-100.0, 0.0, 100.0]
        sigmaRange = [0.1, 1.0, 10]
        N = 25000
        numberOfSigma = 15

        for average in averageRange:
            for sigma in sigmaRange:
                #target cannot be changed:
                #it is a strong assumption to compute values to be checked
                target = average
                normal = QuantLib.NormalDistribution(average, sigma)

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

                assert s.samples() == N, \
                    'wrong number of samples\n' + \
                    'calculated: %d\n' % s.samples() + \
                    'expected  : %d\n' % N

                rightWeightSum = reduce(lambda x,y: x+y, weights)
                assert s.weightSum() == rightWeightSum, \
                    'wrong sum of weights\n' + \
                    'calculated: %f\n' % s.weightSum() + \
                    'expected  : %f\n' % rightWeightSum

                assert s.min() == dataMin, \
                    'wrong minimum value\n' + \
                    'calculated: %f\n' % s.min() + \
                    'expected  : %f\n' % dataMin

                assert abs(s.max()-dataMax) <= 1e-13, \
                    'wrong maximum value\n' + \
                    'calculated: %f\n' % s.max() + \
                    'expected  : %f\n' % dataMax

                if average == 0.0:
                    check = abs(s.mean()-average)
                else:
                    check = abs(s.mean()-average)/average
                assert check <= 1e-13, \
                    'wrong mean value\n' + \
                    'calculated: %f\n' % s.mean() + \
                    'expected  : %f\n' % average

                assert abs(s.variance()-sigma*sigma)/(sigma*sigma) <= 1e-4, \
                    'wrong variance\n' + \
                    'calculated: %f\n' % s.variance() + \
                    'expected  : %f\n' % sigma*sigma

                assert abs(s.standardDeviation()-sigma)/sigma <= 1e-4, \
                    'wrong standard deviation\n' + \
                    'calculated: %f\n' % s.standardDeviation() + \
                    'expected  : %f\n' % sigma

                assert abs(s.skewness()) <= 1e-4, \
                    'wrong skewness\n' + \
                    'calculated: %f\n' % s.skewness() + \
                    'expected  : 0.0\n'

                assert abs(s.kurtosis()) <= 1e-1, \
                    'wrong kurtosis\n' + \
                    'calculated: %f\n' % s.kurtosis() + \
                    'expected  : 0.0\n'

                rightVAR = -min(average-2.0*sigma, 0.0)
                VAR = s.valueAtRisk(0.9772)
                if rightVAR == 0.0:
                    check = abs(VAR-rightVAR)
                else:
                    check = abs(VAR-rightVAR)/rightVAR
                assert check <= 1e-3, \
                    'wrong valueAtRisk\n' + \
                    'calculated: %f\n' % VAR + \
                    'expected:   %f\n' % rightVAR

                tempVAR = average-2.0*sigma
                rightExShortfall = average - sigma*sigma*gaussian(tempVAR,
                    average, sigma)/(1.0-0.9772)
                rightExShortfall = -min(rightExShortfall, 0.0)
                exShortfall = s.expectedShortfall(0.9772)
                if rightExShortfall == 0.0:
                    check = abs(exShortfall)
                else:
                    check = abs(exShortfall-rightExShortfall)/rightExShortfall
                assert check <= 1e-3, \
                    'wrong expected shortfall\n' + \
                    'calculated: %f\n' % exShortFall + \
                    'expected:   %f\n' % rightExShortfall

                rightShortfall = 0.5
                shortfall = s.shortfall(target)
                assert abs(shortfall-rightShortfall)/rightShortfall <= 1e-8, \
                    'wrong shortfall\n' + \
                    'calculated: %f\n' % shortFall + \
                    'expected:   %f\n' % rightShortfall

                rightAvgShortfall = sigma/sqrt( 2 * pi )
                avgShortfall = s.averageShortfall(target)
                check = abs(avgShortfall-rightAvgShortfall)/rightAvgShortfall
                assert check <= 1e-4, \
                    'wrong average shortfall\n' + \
                    'calculated: %f\n' % avgShortFall + \
                    'expected:   %f\n' % rightAvgShortfall

                s.reset()


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(RiskStatisticsTest())
    unittest.TextTestRunner().run(suite)
