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
    Revision 1.11  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

"""

# Make sure that Python path contains the directory of QuantLib
# and that of this file

import QuantLib
import unittest

def relErr(x1,x2,reference):
    if reference != 0.0:
        return abs(x1-x2)/reference
    else:
        return 10e10

class AmericanOptionTest(unittest.TestCase):
  def runTest(self):
    "Testing step condition options"
    nstp = 145
    ngrd = nstp + 1
    # ranges
    rangeUnder = [100]
    rangeQrate = [0.05]
    rangeResTime = [1.0]
    rangeStrike = [50, 100, 150]
    rangeVol = [ 0.05, 0.5, 1.2]
    rangeRrate = [ 0.01, 0.05, 0.15]
    err_delta = 2e-3
    err_gamma = 2e-3
    err_theta = 2e-3
    err_rho  =  2e-3
    err_vega =  2e-3
    for pricer in [QuantLib.AmericanOption, QuantLib.ShoutOption]:
      for typ in ['Call','Put','Straddle']:
        for under in rangeUnder:
          for Qrate in rangeQrate:
            for resTime in rangeResTime:
              for Rrate in rangeRrate:
                for strike in rangeStrike:
                  for vol in rangeVol:
                     #Check Greeks
                     dS = under/10000.0
                     dT = resTime/nstp
                     dVol = vol/10000.0
                     dR = Rrate/10000.0
                     opt = pricer(typ, under, strike, Qrate, Rrate, resTime,
                       vol, nstp, ngrd)
                     opt_val = opt.value()
                     if opt_val>0.00001*under:
                       optPs = pricer(typ, under+dS, strike, Qrate, Rrate,
                         resTime ,vol, nstp, ngrd)
                       optMs = pricer(typ, under-dS, strike, Qrate, Rrate,
                         resTime, vol, nstp, ngrd)
                       optPt = pricer(typ, under, strike, Qrate, Rrate,
                         resTime+dT, vol, nstp+1, ngrd)
                       optMt = pricer(typ, under, strike, Qrate, Rrate,
                         resTime-dT, vol, nstp-1, ngrd)
                       optPr = pricer(typ, under, strike, Qrate, Rrate+dR,
                         resTime, vol, nstp, ngrd)
                       optMr = pricer(typ, under, strike, Qrate, Rrate-dR,
                         resTime, vol, nstp, ngrd)
                       optPv = pricer(typ, under, strike, Qrate, Rrate,
                         resTime, vol+dVol, nstp, ngrd)
                       optMv = pricer(typ, under, strike, Qrate, Rrate,
                         resTime, vol-dVol, nstp, ngrd)
                       # numeric values
                       deltaNum = (optPs.value()-optMs.value())/(2*dS)
                       gammaNum = (optPs.delta()-optMs.delta())/(2*dS)
                       thetaNum =-(optPt.value()-optMt.value())/(2*dT)
                       rhoNum   = (optPr.value()-optMr.value())/(2*dR)
                       vegaNum  = (optPv.value()-optMv.value())/(2*dVol)

                       assert (relErr(opt.delta(),deltaNum,under)<=err_delta \
                           and relErr(opt.gamma(),gammaNum,under)<=err_gamma \
                           and relErr(opt.theta(),thetaNum,under)<=err_theta \
                           and relErr(opt.rho(),  rhoNum,  under)<=err_rho \
                           and relErr(opt.vega(), vegaNum, under)<=err_vega), \
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
    suite.addTest(AmericanOptionTest())
    unittest.TextTestRunner().run(suite)

















