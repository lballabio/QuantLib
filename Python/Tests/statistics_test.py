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
    Revision 1.10  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.9  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.8  2001/01/08 15:33:23  nando
    improved

"""

from QuantLib import Statistics
from TestUnit import TestUnit

class StatisticsTest(TestUnit):
    def doTest(self):
        tol = 1e-9
        
        data =    [  3,   4,   5,   2,   3,   4,   5,   6,   4,   7]
        weights = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
        
        s = Statistics()
        s.addWeightedSequence(data, weights)
        
        self.printDetails('samples:                    %d' % s.samples())
        if s.samples()!=len(data):
            self.printDetails('wrong')
            return -1
        
        self.printDetails('sum of the weights:  %f' % s.weightSum())
        if s.weightSum()!=reduce(lambda x,y:x+y, weights):
            self.printDetails('wrong')
            return -1
        
        self.printDetails('minimum value:        %f' % s.min())
        if s.min()!=min(data):
            self.printDetails('wrong')
            return -1
        
        self.printDetails('maximum value:        %f' % s.max())
        if s.max()!=max(data):
            self.printDetails('wrong')
            return -1

        self.printDetails('mean value:           %f' % s.mean())
        mean = reduce(lambda x,y:x+y, map(lambda x,y:x*y, data, weights)) \
            / reduce(lambda x,y:x+y, weights)
        if abs(s.mean()-mean)>tol:
            self.printDetails('wrong')
            return -1
        
        self.printDetails('variance:             %f' % s.variance())
        if abs(s.variance()-2.23333333333)>tol:
            self.printDetails('wrong')
            return -1
        
        self.printDetails('standard deviation:   %f' % s.standardDeviation())
        if abs(s.standardDeviation()-1.4944341181)>tol:
            self.printDetails('wrong')
            return -1
        
        self.printDetails('skewness:             %f' % s.skewness())
        if abs(s.skewness()-0.359543071407)>tol:
            self.printDetails('wrong')
            return -1
        
        self.printDetails('excess kurtosis:     %f' % s.kurtosis())
        if abs(s.kurtosis()+0.151799637209)>tol:
            self.printDetails('wrong')
            return -1
        
        self.printDetails(
            'error estimate:       %f (not checked)' % s.errorEstimate()
        )


if __name__ == '__main__':
    StatisticsTest().test('statistics')
