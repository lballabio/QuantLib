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
    Revision 1.3  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.2  2001/01/08 15:33:09  nando
    improved
"""

from QuantLib import *
import time
startTime = time.time()


print 'Testing distributions'

tolerance = 1e-3
steps = 100000
cum = CumulativeNormalDistribution()
invCum = InverseCumulativeNormalDistribution()

try:
    dx = 1.0/steps
    pOld =  invCum(dx)
    tOld = cum(pOld)
    abscissaOld = dx
    
    for x in range(1, steps):
        abscissa = x*dx

        p = invCum(abscissa)
        if (p<pOld):
            print 'invCum not monotone'
            print 'invCum(', abscissa, ') = ', p
            print 'invCum(', abscissaOld, ') = ', pOld
            raise

        t = cum(p)
        if (t<tOld):
            print 'cum not monotone'
            print 'cum(', p, ') = ', t
            print 'cum(', pOld, ') = ', tOld
            raise

        if abs(t-abscissa)>tolerance:
            print 'invCum(', abscissa, ') = ', p,
            print 'cum(', p, ') = ', t,
            print 'error:', abs(t-abscissa), 'out of tolerance'
            raise

            pOld = p
            tOld = t
            abscissaOld = abscissa
    
except Exception, e:
    print 'iter n.', x, 'increment', dx,
    raise e

print
print 'Test passed (elapsed time', time.time() - startTime, ')'
print 'Press return to end this test'
raw_input()
