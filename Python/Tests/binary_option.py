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
    Revision 1.8  2001/04/18 09:03:22  nando
    added/removed final
    raw_input('press any key to continue')

    Revision 1.7  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

    Revision 1.6  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.5  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.4  2001/03/05 11:41:32  lballabio
    Cosmetic change

    Revision 1.3  2001/03/02 10:19:50  marmar
    One failure is enough to halt the test

    Revision 1.2  2001/03/02 08:27:25  lballabio
    Fixed initial message

    Revision 1.1  2001/03/01 12:57:39  enri
    class BinaryOption added, test it with binary_option.py


"""

# Make sure that Python path contains the directory of QuantLib and
# that of this file

import QuantLib
import unittest
from math import exp


def relErr(x1, x2, reference):
    if reference != 0.0:
        return abs(x1-x2)/reference
    else:
        return 10e10

class BinaryOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing binary option pricer"
        pricer = QuantLib.BinaryOption

        rangeUnder = [100]
        rangeQrate = [0.05]
        rangeResTime = [1.0]
        rangeStrike = [50, 99.5, 100, 100.5, 150]
        rangeVol = [ 0.11, 0.5, 1.2]
        rangeRrate = [ 0.01, 0.05, 0.15]

        err_delta = 5e-5
        err_gamma = 5e-5
        err_theta = 5e-5
        err_rho  =  5e-5
        err_vega =  5e-5

        for typ in ['Call','Put','Straddle']:
          for under in rangeUnder:
            for Qrate in rangeQrate:
              for resTime in rangeResTime:
                for Rrate in rangeRrate:
                  for strike in rangeStrike:
                    for vol in rangeVol:
                      #Check Greeks
                      dS = under/10000.0
                      dT = resTime/10000.0
                      dVol = vol/10000.0
                      dR = Rrate/10000.0
                      opt = pricer(typ,under,strike,Qrate,Rrate,resTime,vol)
                      opt_val = opt.value()
                      if opt_val > 1e-6:
                        optPs = pricer(typ, under+dS, strike, Qrate,
                          Rrate,    resTime ,   vol)
                        optMs = pricer(typ, under-dS, strike, Qrate,
                          Rrate,    resTime ,   vol)
                        optPt = pricer(typ, under   , strike, Qrate,
                          Rrate,    resTime+dT, vol)
                        optMt = pricer(typ, under   , strike, Qrate,
                          Rrate,    resTime-dT, vol)
                        optPr = pricer(typ, under   , strike, Qrate,
                          Rrate+dR, resTime   , vol)
                        optMr = pricer(typ, under   , strike, Qrate,
                          Rrate-dR, resTime   , vol)
                        optPv = pricer(typ, under   , strike, Qrate,
                          Rrate   , resTime   , vol+dVol)
                        optMv = pricer(typ, under   , strike, Qrate,
                          Rrate   , resTime   , vol-dVol)

                        deltaNum = (optPs.value()-optMs.value())/(2*dS)
                        gammaNum = (optPs.delta()-optMs.delta())/(2*dS)
                        thetaNum =-(optPt.value()-optMt.value())/(2*dT)
                        rhoNum   = (optPr.value()-optMr.value())/(2*dR)
                        vegaNum  = (optPv.value()-optMv.value())/(2*dVol)

                        assert (relErr(opt.delta(),deltaNum,under)<=err_delta \
                            and relErr(opt.gamma(),gammaNum,under)<=err_gamma \
                            and relErr(opt.theta(),thetaNum,under)<=err_theta \
                            and relErr(opt.rho(),  rhoNum,  under)<=err_rho \
                            and relErr(opt.vega(), vegaNum, under)<=err_vega),\
                            'Option details: %s %f %f %f %f %f %f\n' % \
                             (typ,under,strike,Qrate,Rrate,resTime,vol) + \
                             '\tvalue=%+9.5f\n' % (opt_val) + \
                             '\tdelta=%+9.5f, deltaNum=%+9.5f err=%7.2e\n' % \
                              (opt.delta(), deltaNum,
                               relErr(opt.delta(),deltaNum,under)) + \
                             '\tgamma=%+9.5f, gammaNum=%+9.5f err=%7.2e\n' % \
                              (opt.gamma(), gammaNum,
                               relErr(opt.gamma(),gammaNum,under)) + \
                             '\ttheta=%+9.5f, thetaNum=%+9.5f err=%7.2e\n' % \
                              (opt.theta(), thetaNum,
                               relErr(opt.theta(),thetaNum,under)) + \
                             '\trho  =%+9.5f,   rhoNum=%+9.5f err=%7.2e\n' % \
                              (opt.rho(), rhoNum,
                               relErr(opt.rho(),rhoNum,under)) + \
                             '\tvega =%+9.5f, vegaNum =%+9.5f err=%7.2e\n' % \
                              (opt.vega(), vegaNum,
                               relErr(opt.vega(),vegaNum,under))


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(BinaryOptionTest())
    unittest.TextTestRunner().run(suite)
    raw_input('press any key to continue')





