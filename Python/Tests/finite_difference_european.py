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
    Revision 1.2  2001/03/08 14:50:35  marmar
    Revised form of test with random parameters

    Revision 1.1  2001/03/07 17:19:25  marmar
    Example of european option using finite differences

"""
# Make sure that Python path contains the directory of QuantLib and
# that of this file

from QuantLib import UniformRandomGenerator
from QuantLib import BSMEuropeanOption
from QuantLib import FiniteDifferenceEuropean
from TestUnit import TestUnit
import math

class FDEuropeanOptionTest(TestUnit):
    def doTest(self):
        under = 100
        strikeMin = 60
        rangeStrike = 100
        rangeRrate = 0.18
        rangeQrate = 0.02
        rangeVol = 1.2
        timeMin = 0.5
        rangeTime = 2
        
        tollerance = 8.42e-3
        total_number_of_error = 0
        maxError = 0
        L2err = 0

        totCases = 200
        rng = UniformRandomGenerator(56789012)
        for ite in range(totCases):
            strike = strikeMin + rangeStrike * rng.next()
            Qrate =              rangeQrate * rng.next()
            Rrate =              rangeRrate * rng.next()
            vol =                rangeVol * rng.next()            
            resTime = timeMin +  rangeTime * rng.next()
            for optType in ['Call', 'Put', 'Straddle']:
                anValue = BSMEuropeanOption(
                           optType, under, strike, Qrate,
                           Rrate, resTime, vol).value()
                numValue = FiniteDifferenceEuropean(
                           optType, under, strike, Qrate,
                           Rrate, resTime, vol, 100, 400).value()
                absErr = abs(anValue - numValue)
                L2err = L2err + absErr*absErr
                maxError = max(maxError, absErr)
                if absErr > tollerance:
                    self.printDetails(optType, under, strike, Qrate, Rrate, resTime, vol,
                                      "Error = %12.2e " % absErr)
                    total_number_of_error = total_number_of_error + 1
         
        L2err = math.sqrt(L2err/totCases/3)
        self.printDetails(
           "total number of failures: %d/%d, maxError =%g, L2err =%g"
                  % (total_number_of_error, totCases*3, maxError, L2err))

        if total_number_of_error >= 1:
            return 1
        else:
            return 0
         
if __name__ == '__main__':
    FDEuropeanOptionTest().test('finite-difference european option pricer')

