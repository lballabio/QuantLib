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
    Revision 1.5  2001/04/18 09:03:22  nando
    added/removed final
    raw_input('press any key to continue')

    Revision 1.4  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

    Revision 1.3  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

"""

import QuantLib
import unittest

class PagodaOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing pagoda option pricer"
        c = QuantLib.Matrix(7,7,0.0)
        c[0][0] = 1.0  ; c[0][1] = -0.02 ; c[0][2] =  0.315; c[0][3] =  0.127;\
                         c[0][4] =  0.348; c[0][5] =  0.357; c[0][6] =  0.112
        c[1][0] =-0.02 ; c[1][1] =  1.0  ; c[1][2] =  0.203; c[1][3] = -0.739;\
                         c[1][4] = -0.254; c[1][5] =  0.406; c[1][6] = -0.244
        c[2][0] = 0.315; c[2][1] =  0.203; c[2][2] =  1.0  ; c[2][3] = -0.312;\
                         c[2][4] =  0.302; c[2][5] = -0.216; c[2][6] = -0.616
        c[3][0] = 0.127; c[3][1] = -0.739; c[3][2] = -0.312; c[3][3] =  1.0  ;\
                         c[3][4] =  0.238; c[3][5] = -0.154; c[3][6] =  0.362
        c[4][0] = 0.348; c[4][1] = -0.254; c[4][2] =  0.302; c[4][3] =  0.238;\
                         c[4][4] =  1.0  ; c[4][5] = -0.35 ; c[4][6] = -0.439
        c[5][0] = 0.357; c[5][1] =  0.406; c[5][2] = -0.216; c[5][3] = -0.154;\
                         c[5][4] = -0.35 ; c[5][5] =  1.0  ; c[5][6] =  0.381
        c[6][0] = 0.112; c[6][1] = -0.244; c[6][2] = -0.616; c[6][3] =  0.362;\
                         c[6][4] = -0.439; c[6][5] =  0.381; c[6][6] =  1.0

        volatilities = [0.325, 0.365, 0.235, 0.426, 0.365, 0.377, 0.228]
        divYield  = [0.03807, 0.01227, 0.02489, 0.09885, \
                     0.01244, 0.00466, 0.10827]
        divYield  = [0.0003807, 0.0001227, 0.0002489, 0.0009885, \
                     0.0001244, 0.0000466, 0.0010827]
        portfolio = [0.10, 0.20, 0.20, 0.20, 0.10, 0.10, 0.10]

        fraction = 0.62
        roof = 0.20
        residualTime = 1
        riskFreeRate = 0.05
        timesteps = 12
        samples = 2000
        seed = 86421

        cov = QuantLib.getCovariance(volatilities, c)
        storedValue = 0.0411470297914
        storedError = 0.000936103530233

        pagoda = QuantLib.PagodaOption(portfolio,
                    fraction, roof, residualTime,
                    cov, divYield, riskFreeRate,
                    timesteps, samples, seed);

        value = pagoda.value()
        error = pagoda.errorEstimate()
        assert abs(value-storedValue) <= 1e-10, \
            "calculated value: %g\n" % value + \
            "stored value:     %g\n" % storedValue + \
            "tolerance exceeded\n"
        assert abs(error-storedError) <= 1e-10, \
            "calculated error: %g\n" % error + \
            "stored error:     %g\n" % storedError + \
            "tolerance exceeded\n"

if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(PagodaOptionTest())
    unittest.TextTestRunner().run(suite)
    raw_input('press any key to continue')


