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

    Revision 1.7  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.6  2001/02/15 11:57:20  nando
    impliedVol require targetValue>0.0

    Revision 1.5  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.4  2001/01/08 15:33:23  nando
    improved

"""

# this file tests the method impliedVolatility
# (it actually tests the Brent 1D solver used)

import QuantLib
import unittest

class ImpliedVolatilityTest(unittest.TestCase):
    def runTest(self):
        "Testing implied volatility and Brent 1D solver"
        typRange = ['Call', 'Put', 'Straddle']
        underRange = [80, 95, 99.90, 100, 100.10, 105, 120]
        strikeRange = [80, 95, 99.90, 100, 100.10, 105, 120]
        qRateRange = [0.01, 0.05, 0.10]
        rRateRange = [0.01, 0.05, 0.10]
        resTimeRange = [0.001, 0.1, 0.5, 1.0, 3.0]
        volRange = [0.01, 0.2, 0.3, 0.7, 0.9]
        dVolRange = [0.5, 0.999, 1.0, 1.001, 1.5]

        maxEval = 100
        tol = 1e-6

        for typ in typRange:
          for under in underRange:
            for strike in strikeRange:
              for qRate in qRateRange:
                for rRate in rRateRange:
                  for resTime in resTimeRange:
                    for vol in volRange:
                      bsm = QuantLib.BSMEuropeanOption(typ, under, \
                          strike, qRate, rRate, resTime, vol)
                      bsmValue = bsm.value()
                      if bsmValue==0.0 :
                        continue
                      for dVol in dVolRange:
                        vol2 = vol*dVol
                        bsm2 = QuantLib.BSMEuropeanOption(typ, under, \
                            strike, qRate, rRate, resTime, vol2)
                        try:
                            implVol = bsm2.impliedVolatility(bsmValue, tol,
                              maxEval)
                        except Exception, e:
                            raise str(e) + "\n" + \
                                "Option details: %s %f %f %f %f %f\n" % \
                                (typ, under, strike, qRate, rRate, resTime) + \
                                "volatility:   %18.16f\n" % vol2 + \
                                "option value: %20.12e\n" % bsm2.value() + \
                                "while trying to calculate implied vol " + \
                                "from value %20.12e\n" % bsmValue
                        if abs(implVol-vol) > tol:
                          bsm3 = QuantLib.BSMEuropeanOption(typ, under, \
                            strike, qRate, rRate, resTime, implVol)
                          bsm3Value = bsm3.value()
                          assert abs(bsm3Value-bsmValue)/under<=1.0e-3, \
                           "Option details: %s %f %f %f %f %f\n" % \
                             (typ, under, strike, qRate, rRate, resTime) + \
                           "at %18.16f vol the option value is %20.12e\n" % \
                             (vol, bsmValue) + \
                           "at %18.16f vol the option value is %20.12e\n" % \
                             (vol2, bsm2.value()) + \
                           "at %20.12e value the implied vol is  %20.16f\n" % \
                             (bsmValue, implVol) + \
                           "the error is %10.2e (tolerance is %10.2e)\n" % \
                             (err, tol) + \
                           "at %18.16f vol the option value is %20.12e\n" % \
                             (implVol, bsm3Value) + \
                           "which is %g above target value\n" % \
                             (bsm3Value - bsmValue)


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(ImpliedVolatilityTest())
    unittest.TextTestRunner().run(suite)
