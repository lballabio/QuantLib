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
    Revision 1.6  2001/04/18 09:03:22  nando
    added/removed final
    raw_input('press any key to continue')

    Revision 1.5  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

    Revision 1.4  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.3  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.2  2001/03/08 14:50:35  marmar
    Revised form of test with random parameters

    Revision 1.1  2001/03/07 17:19:25  marmar
    Example of european option using finite differences

"""
import QuantLib
import unittest
import math

class FDEuropeanOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing finite-difference European option pricer"
        under = 100
        strikeMin = 60
        rangeStrike = 100
        rangeRrate = 0.18
        rangeQrate = 0.02
        rangeVol = 1.2
        timeMin = 0.5
        rangeTime = 2

        tolerance = 8.42e-3
        maxError = 0
        L2err = 0

        totCases = 200
        rng = QuantLib.UniformRandomGenerator(56789012)
        for ite in range(totCases):
            strike = strikeMin + rangeStrike * rng.next()
            Qrate =              rangeQrate * rng.next()
            Rrate =              rangeRrate * rng.next()
            vol =                rangeVol * rng.next()
            resTime = timeMin +  rangeTime * rng.next()
            for optType in ['Call', 'Put', 'Straddle']:
                anValue = QuantLib.BSMEuropeanOption(
                           optType, under, strike, Qrate,
                           Rrate, resTime, vol).value()
                numValue = QuantLib.FiniteDifferenceEuropean(
                           optType, under, strike, Qrate,
                           Rrate, resTime, vol, 100, 400).value()
                assert abs(anValue - numValue) <= tolerance, \
                    "Option details: %s %g %g %g %g %g %g\n" % \
                      (optType, under, strike, Qrate, Rrate, resTime, vol) + \
                    "Error = %12.2e \n" % abs(anValue - numValue)


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(FDEuropeanOptionTest())
    unittest.TextTestRunner().run(suite)
    raw_input('press any key to continue')

