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
    Revision 1.2  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.1  2001/02/22 14:43:36  lballabio
    Renamed test script to follow a single naming scheme

    Revision 1.2  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.1  2001/02/20 11:13:41  marmar
    BarrierOption implements the analytical barrier option

"""

import QuantLib
import unittest
from math import fabs

class BarrierOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing barrier option pricer"
        maxErrorAllowed = 5e-5
        underPrice = 100
        rebate = 3
        resTime = 0.5
        rRate = 0.08
        divRate = 0.04
        vol = [0.25, 0.30]
        iv = []
        iv.append(("DownOut", "Call",  90,  95, (9.0246, 8.8334)))
        iv.append(("DownOut", "Call", 100,  95, (6.7924, 7.0285)))
        iv.append(("DownOut", "Call", 110,  95, (4.8759, 5.4137)))
        iv.append(("DownOut", "Call",  90, 100, (3.0000, 3.0000)))
        iv.append(("DownOut", "Call", 100, 100, (3.0000, 3.0000)))
        iv.append(("DownOut", "Call", 110, 100, (3.0000, 3.0000)))
        iv.append(("UpOut", "Call",  90, 105, (2.6789, 2.6341)))
        iv.append(("UpOut", "Call", 100, 105, (2.3580, 2.4389)))
        iv.append(("UpOut", "Call", 110, 105, (2.3453, 2.4315)))
        
        iv.append(("DownIn", "Call",  90,  95, (7.7627, 9.0093)))
        iv.append(("DownIn", "Call", 100,  95, (4.0109, 5.1370)))
        iv.append(("DownIn", "Call", 110,  95, (2.0576, 2.8517)))
        iv.append(("DownIn", "Call",  90, 100, (13.8333,14.8816)))
        iv.append(("DownIn", "Call", 100, 100, (7.8494, 9.2045)))
        iv.append(("DownIn", "Call", 110, 100, (3.9795, 5.3043)))
        iv.append(("UpIn", "Call",  90, 105, (14.1112,15.2098)))
        iv.append(("UpIn", "Call", 100, 105, (8.4482, 9.7278)))
        iv.append(("UpIn", "Call", 110, 105, (4.5910, 5.8350)))
        
        iv.append(("DownOut", "Put",  90,  95, (2.2798, 2.4170)))
        iv.append(("DownOut", "Put", 100,  95, (2.2947, 2.4258)))
        iv.append(("DownOut", "Put", 110,  95, (2.6252, 2.6246)))
        iv.append(("DownOut", "Put",  90, 100, (3.0000, 3.0000)))
        iv.append(("DownOut", "Put", 100, 100, (3.0000, 3.0000)))
        iv.append(("DownOut", "Put", 110, 100, (3.0000, 3.0000)))
        iv.append(("UpOut", "Put",  90, 105, (3.7760, 4.2293)))
        iv.append(("UpOut", "Put", 100, 105, (5.4932, 5.8032)))
        iv.append(("UpOut", "Put", 110, 105, (7.5187, 7.5649)))
        
        iv.append(("DownIn", "Put",  90,  95, (2.9586, 3.8769)))
        iv.append(("DownIn", "Put", 100,  95, (6.5677, 7.7989)))
        iv.append(("DownIn", "Put", 110,  95, (11.9752,13.3078)))
        iv.append(("DownIn", "Put",  90, 100, (2.2845, 3.3328)))
        iv.append(("DownIn", "Put", 100, 100, (5.9085, 7.2636)))
        iv.append(("DownIn", "Put", 110, 100, (11.6465,12.9713)))
        iv.append(("UpIn", "Put",  90, 105, (1.4653, 2.0658)))
        iv.append(("UpIn", "Put", 100, 105, (3.3721, 4.4226)))
        iv.append(("UpIn", "Put", 110, 105, (7.0846, 8.3686)))
        for inputPara in iv:
            barrType, optType, strike, barrier, results = inputPara
            for i in (0,1):
                op = QuantLib.BarrierOption(barrType, optType, underPrice,
                    strike, divRate, rRate, resTime, vol[i], barrier, rebate)
                val = op.value()
                error = fabs(val - results[i])
                assert error <= maxErrorAllowed, \
                    "%7s %5s %4i %4i\n" % \
                     (barrType, optType, strike, barrier) + \
                    "\tvalue:    %8.4f\n" % val + \
                    "\texpected: %8.4f\n" % results[i] + \
                    "\terror: %12.2e" % error


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(BarrierOptionTest())
    unittest.TextTestRunner().run(suite)






