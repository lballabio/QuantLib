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
    Revision 1.8  2001/04/27 10:44:15  lballabio
    Support for unittest in Python 2.1

    Revision 1.7  2001/04/18 09:03:22  nando
    added/removed final
    raw_input('press any key to continue')

    Revision 1.6  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

    Revision 1.5  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.4  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.3  2001/03/28 12:50:32  marmar
    Dates are now used for input instead of time delays

    Revision 1.2  2001/03/15 13:49:35  marmar
    getCovariance function added
"""

import QuantLib
import unittest
from math import fabs

class HimalayaOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing Himalaya option pricer"
        cor = QuantLib.Matrix(4,4)
        cor[0][0] = 1.00; cor[0][1] = 0.50; cor[0][2] = 0.30; cor[0][3] = 0.10
        cor[1][0] = 0.50; cor[1][1] = 1.00; cor[1][2] = 0.20; cor[1][3] = 0.40
        cor[2][0] = 0.30; cor[2][1] = 0.20; cor[2][2] = 1.00; cor[2][3] = 0.60
        cor[3][0] = 0.10; cor[3][1] = 0.40; cor[3][2] = 0.60; cor[3][3] = 1.00

        volatility = [ 0.3,  0.3,  0.3,  0.3]
        covariance = QuantLib.getCovariance(volatility, cor)

        assetValues = [100, 100, 100, 100]
        dividendYields = [0.0, 0.0, 0.0, 0]
        timeIncrements = [0.25, 0.5, 0.75, 1.0]
        riskFreeRate = 0.05
        strike = 100
        samples = 40000
        seed = 3456789

        him = QuantLib.Himalaya(assetValues, dividendYields, covariance,
                       riskFreeRate, strike, timeIncrements, samples, seed)

        value = him.value()
        error = him.errorEstimate()
        storedValue = 7.30213127731
        storedError = 0.0521786123186
        assert fabs(value-storedValue) <= 1e-10, \
            "calculated value: %g\n" % value + \
            "stored value:     %g\n" % storedValue + \
            "tolerance exceeded\n"
        assert fabs(error-storedError) <= 1e-10, \
            "calculated error: %g\n" % error + \
            "stored error:     %g\n" % storedError + \
            "tolerance exceeded\n"


if __name__ == '__main__':
    import sys
    suite = unittest.TestSuite()
    suite.addTest(HimalayaOptionTest())
    if sys.hexversion >= 0x020100f0:
        unittest.TextTestRunner(verbosity=2).run(suite)
    else:
        unittest.TextTestRunner().run(suite)
    raw_input('press any key to continue')

