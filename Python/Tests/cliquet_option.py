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
    Revision 1.1  2001/04/04 10:01:36  marmar
    introducing cliquet option

"""

# Make sure that Python path contains the directory of QuantLib and
# that of this file

from QuantLib import BSMEuropeanOption, CliquetOption
from TestUnit import TestUnit
import math

class CliquetOptionTest(TestUnit):
    def doTest(self):
        spot = 100
        divYield = 0.01
        rRate = 0.06
        dates = [0.50, 1.00, 2.00]
        vol =0.35
        euro1 = BSMEuropeanOption("Call", spot, spot, divYield, rRate,
                                  dates[1]-dates[0], vol)
        w1 = math.exp(divYield*dates[0])
         
        euro2 = BSMEuropeanOption("Call", spot, spot, divYield, rRate,
                                  dates[2]-dates[1], vol)
        w2 = math.exp(divYield*dates[1])
         
        cliquet = CliquetOption("Call", spot, divYield, rRate,
                                dates, vol)
        total_number_of_error = 0
        for method in ['value', 'delta', 'gamma', 'theta', 'rho', 'vega']:
            euro1Method = getattr(euro1, method)
            euro2Method = getattr(euro2, method)
            cliquetMethod = getattr(cliquet, method)
            error = w1*apply(euro1Method,()) \
                    + w2 * apply(euro2Method,()) \
                    - apply(cliquetMethod,())
            if math.fabs(error) > 1e-10:
               total_number_of_error = total_number_of_error + 1
        if total_number_of_error >= 1:
            self.printDetails(
                "total number of failures: %d" %
                total_number_of_error
            )
            return 1
        else:
            return 0


if __name__ == '__main__':
    CliquetOptionTest().test('cliquet option pricer')

