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
    Revision 1.8  2001/03/02 15:44:03  lballabio
    Cosmetic change

    Revision 1.7  2001/03/02 14:37:03  marmar
    Shout option included in tests

    Revision 1.6  2001/03/02 10:19:50  marmar
    One failure is enough to halt the test

    Revision 1.5  2001/03/02 09:02:48  marmar
    AmericanOption replaced BSMAmericanOption

    Revision 1.4  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.3  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.2  2001/01/08 15:33:23  nando
    improved

"""
# Make sure that Python path contains the directory of QuantLib
# and that of this file

import QuantLib 
from TestUnit import TestUnit

def relErr(x1,x2,reference):
    if reference != 0.0:
        return abs(x1-x2)/reference
    else:
        return 10e10

class AmericanOptionTest(TestUnit):
  def doTest(self):
    nstp = 145
    ngrd = nstp + 1
    # ranges
    rangeUnder = [100]
    rangeQrate = [0.05]
    rangeResTime = [1.0]
    rangeStrike = [50, 100, 150]
    rangeVol = [ 0.05, 0.5, 1.2]
    rangeRrate = [ 0.01, 0.05, 0.15]
    maxNumDerErrorList=[]; maxCPSerrorList=[];
    resuCPSvalue = []; resuCPSdelta = []; resuCPSgamma = [] 
    resuCPStheta = []; resuCPSrho   = []; resuCPSvega  = []
    resuCPparity = []
    err_delta = 2e-3
    err_gamma = 2e-3
    err_theta = 2e-3
    err_rho  =  2e-3
    err_vega =  2e-3
    total_number_of_error = 0
    for pricer in [QuantLib.AmericanOption, QuantLib.ShoutOption]:
      self.printDetails(pricer)
      # table header
      self.printDetails(
        "     Type  items err-delta err-gamma err-theta  err-rho  err-vega ")
      # test
      for typ in ['Call','Put','Straddle']:
        resuDelta = [];  resuGamma = [];  resuTheta = []
        resuRho   = [];  resuVega  = []
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
                       # store results
                       resuDelta.append(relErr(opt.delta(),deltaNum,under))
                       resuGamma.append(relErr(opt.gamma(),gammaNum,under))
                       resuTheta.append(relErr(opt.theta(),thetaNum,under))
                       resuRho.append(relErr(opt.rho(),rhoNum,under))
                       resuVega.append(relErr(opt.vega(),vegaNum,under))
                                           
                       if(relErr(opt.delta(),deltaNum,under)>err_delta or
                         relErr(opt.gamma(),gammaNum,under)>err_gamma or
                         relErr(opt.theta(),thetaNum,under)>err_theta or
                         relErr(opt.rho(),  rhoNum,  under)>err_rho   or
                         relErr(opt.vega(), vegaNum, under)>err_vega):
                           total_number_of_error = total_number_of_error + 1
                           self.printDetails(
                             'Attention required: %s %f %f %f %f %f %f' % 
                             (typ,under,strike,Qrate,Rrate,resTime,vol)
                           )
                           self.printDetails('\tvalue=%+9.5f' % (opt_val))
                           self.printDetails(
                             '\tdelta=%+9.5f, deltaNum=%+9.5f err=%7.2e' % 
                             (opt.delta(), deltaNum, 
                             relErr(opt.delta(),deltaNum,under))
                           )
                           self.printDetails(
                             '\tgamma=%+9.5f, gammaNum=%+9.5f err=%7.2e' % 
                             (opt.gamma(), gammaNum, 
                             relErr(opt.gamma(),gammaNum,under))
                           )
                           self.printDetails(
                             '\ttheta=%+9.5f, thetaNum=%+9.5f err=%7.2e' % 
                             (opt.theta(), thetaNum, 
                             relErr(opt.theta(),thetaNum,under))
                           )
                           self.printDetails(
                             '\trho  =%+9.5f,   rhoNum=%+9.5f err=%7.2e' % 
                             (opt.rho(), rhoNum, 
                             relErr(opt.rho(),rhoNum,under))
                           )
                           self.printDetails(
                             '\tvega =%+9.5f, vegaNum =%+9.5f err=%7.2e' % 
                             (opt.vega(), vegaNum, 
                             relErr(opt.vega(),vegaNum,under))
                           )
        self.printDetails("%9s %6d %7.2e %7.2e %7.2e %7.2e %7.2e" % 
            (typ, len(resuDelta), max(resuDelta), max(resuGamma), 
            max(resuTheta), max(resuRho), max(resuVega)))
        maxNumDerErrorList.append(max(resuDelta))
        maxNumDerErrorList.append(max(resuGamma))
        maxNumDerErrorList.append(max(resuTheta))
        maxNumDerErrorList.append(max(resuRho))
        maxNumDerErrorList.append(max(resuVega))

      self.printDetails(
        "Maximum global error on numerical derivatives = %g\n" % 
        max(maxNumDerErrorList))
    if total_number_of_error >= 1:
        self.printDetails("total number of failures: %d" % 
          total_number_of_error)
        return 1
    else:
        return 0


if __name__ == '__main__':
    AmericanOptionTest().test("step condition options")

