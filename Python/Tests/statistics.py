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
    Revision 1.4  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

    Revision 1.3  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.1  2001/02/22 14:43:36  lballabio
    Renamed test script to follow a single naming scheme

    Revision 1.10  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.9  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.8  2001/01/08 15:33:23  nando
    improved

"""

import QuantLib
import unittest

class StatisticsTest(unittest.TestCase):
    def runTest(self):
        "Testing statistics"
        tol = 1e-9

        data =    [  3,   4,   5,   2,   3,   4,   5,   6,   4,   7]
        weights = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]

        s = QuantLib.Statistics()
        s.addWeightedSequence(data, weights)

        assert s.samples() == len(data), \
            'wrong number of samples\n' + \
            'calculated: %d\n' % s.samples() + \
            'expected  : %d\n' % len(data)

        rightWeightSum = reduce(lambda x,y:x+y, weights)
        assert s.weightSum() == rightWeightSum, \
            'wrong sum of weights\n' + \
            'calculated: %f\n' % s.weightSum() + \
            'expected  : %f\n' % rightWeightSum

        assert s.min() == min(data), \
            'wrong minimum value\n' + \
            'calculated: %f\n' % s.min() + \
            'expected  : %f\n' % min(data)

        assert s.max() == max(data), \
            'wrong maximum value\n' + \
            'calculated: %f\n' % s.max() + \
            'expected  : %f\n' % max(data)

        mean = reduce(lambda x,y:x+y, map(lambda x,y:x*y, data, weights)) \
            / reduce(lambda x,y:x+y, weights)
        assert abs(s.mean()-mean) <= tol, \
            'wrong mean value\n' + \
            'calculated: %f\n' % s.mean() + \
            'expected  : %f\n' % mean

        assert abs(s.variance()-2.23333333333) <= tol, \
            'wrong variance\n' + \
            'calculated: %f\n' % s.variance() + \
            'expected  : 2.23333333333\n'

        assert abs(s.standardDeviation()-1.4944341181) <= tol, \
            'wrong standard deviation\n' + \
            'calculated: %f\n' % s.standardDeviation() + \
            'expected  : 1.4944341181\n'

        assert abs(s.skewness()-0.359543071407) <= tol, \
            'wrong skewness\n' + \
            'calculated: %f\n' % s.skewness() + \
            'expected  : 0.359543071407\n'

        assert abs(s.kurtosis()+0.151799637209) <= tol, \
            'wrong kurtosis\n' + \
            'calculated: %f\n' % s.kurtosis() + \
            'expected  : -0.151799637209\n'


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(StatisticsTest())
    unittest.TextTestRunner().run(suite)

