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
    Revision 1.1  2001/03/07 17:19:25  marmar
    Example of european option using finite differences

"""
# Make sure that Python path contains the directory of QuantLib and
# that of this file

from QuantLib import BSMEuropeanOption
from QuantLib import FiniteDifferenceEuropean
from TestUnit import TestUnit

class FDEuropeanOptionTest(TestUnit):
    def doTest(self):
        rangeUnder = [100]
        rangeStrike = [66, 99.5, 100, 100.5, 150]
        rangeRrate = [0.01, 0.05, 0.15]
        rangeQrate = [0.0, 0.01]
        rangeResTime = [1.0, 2.0]
        rangeVol = [0.05, 0.5, 1.2]
        
        tollerance =6.69e-3
        total_number_of_error = 0
        maxError = 0
        
        for under in rangeUnder:
          for Qrate in rangeQrate:
            for resTime in rangeResTime:
              for Rrate in rangeRrate:
                for strike in rangeStrike:
                  for vol in rangeVol:
                    for optType in ['Call', 'Put', 'Straddle']:
                        anValue = BSMEuropeanOption(
                                    optType, under, strike, Qrate,
                                    Rrate, resTime, vol).value()
                        numValue = FiniteDifferenceEuropean(
                                    optType, under, strike, Qrate,
                                    Rrate, resTime, vol, 100, 400).value()
                        absErr = abs(anValue - numValue)
                        if absErr > tollerance:
                            self.printDetails("Error = %12.2e " % absErr)
                            maxError = max(maxError, absErr)
                            total_number_of_error = total_number_of_error + 1
         
        if total_number_of_error >= 1:
            self.printDetails("total number of failures: %d, maxError %g"
                              % (total_number_of_error, maxError))
            return 1
        else:
            return 0
         
if __name__ == '__main__':
    FDEuropeanOptionTest().test('finite-difference european option pricer')

