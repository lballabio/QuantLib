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
    $Source$
    $Log$
    Revision 1.4  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.3  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.2  2001/03/15 10:29:10  lballabio
    Removed 1M list construction

    Revision 1.1  2001/02/22 14:43:36  lballabio
    Renamed test script to follow a single naming scheme

    Revision 1.6  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.5  2001/02/15 11:57:34  nando
    no message

    Revision 1.4  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.3  2001/01/08 15:33:23  nando
    improved

"""

import QuantLib
import unittest

class RNGTest(unittest.TestCase):
    def runTest(self):
        "Testing random number generators"
        seed = 576919
        samples = 1000000
        for RNG in [QuantLib.UniformRandomGenerator,
                    QuantLib.GaussianRandomGenerator]:
            rn = RNG(seed=seed)
            s = QuantLib.Statistics()
            while s.samples() < samples:
                s.add(rn.next())
            # we'll have to write some meaningful test here


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(RNGTest())
    unittest.TextTestRunner().run(suite)


