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
    Revision 1.1  2001/01/08 15:12:45  nando
    added test for date and distributions

"""

from QuantLib import *

tol = 1e-3
steps = 10000
cum = CumulativeNormalDistribution()
invCum = InverseCumulativeNormalDistribution()

try:
    dx = 1.0/steps
    for x in range(1, steps):
        t = cum(invCum(x*dx))
        if abs(t-x*dx)>tol:
            print 'iter n.', x, 'increment', dx,
            print 'abscissa', x*dx, t, abs(t-x*dx)
            raise
    
except Exception, e:
    print 'cum(invCum(', x, ')) =', t, ': out of tolerance'
    raise e

print 'test passed'
print 'Press return to end this test'
raw_input()
