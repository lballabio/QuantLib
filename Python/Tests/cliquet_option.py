"""
/*
 * Copyright (C) 2001 QuantLib Group
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
    Revision 1.2  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.1  2001/04/04 10:01:36  marmar
    introducing cliquet option

"""

import QuantLib
import unittest
from math import exp

class CliquetOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing cliquet option pricer"
        spot = 100
        divYield = 0.01
        rRate = 0.06
        dates = [0.50, 1.00, 2.00]
        vol = 0.35
        euro1 = QuantLib.BSMEuropeanOption("Call", spot, spot, divYield,
                    rRate, dates[1]-dates[0], vol)
        w1 = exp(divYield*dates[0])
        
        euro2 = QuantLib.BSMEuropeanOption("Call", spot, spot, divYield,
                    rRate, dates[2]-dates[1], vol)
        w2 = exp(divYield*dates[1])
         
        cliquet = QuantLib.CliquetOption("Call", spot, divYield, rRate,
                      dates, vol)
        for method in ['value', 'delta', 'gamma', 'theta', 'rho', 'vega']:
            euro1Method = getattr(euro1, method)
            euro2Method = getattr(euro2, method)
            cliquetMethod = getattr(cliquet, method)
            expected = w1*apply(euro1Method,()) + w2 * apply(euro2Method,())
            calculated = apply(cliquetMethod,())
            error = abs(calculated-expected)
            assert error <= 1e-10, \
                'wrong %s\n' % method + \
                'calculated: %f\n' % calculated + \
                'expected  : %f\n' % expected


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(CliquetOptionTest())
    unittest.TextTestRunner().run(suite)

