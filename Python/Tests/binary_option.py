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
    Revision 1.1  2001/03/01 12:57:39  enri
    class BinaryOption added, test it with binary_option.py


"""

# Make sure that Python path contains the directory of QuantLib and
# that of this file

from QuantLib import BinaryOption
from TestUnit import TestUnit
from math import exp


def relErr(x1, x2, reference):
    if reference != 0.0:
        return abs(x1-x2)/reference
    else:
        return 10e10

class BinaryOptionTest(TestUnit):
    def doTest(self):
        pricer = BinaryOption
        
        rangeUnder = [100]
        rangeQrate = [0.05]
        rangeResTime = [1.0]
        rangeStrike = [50, 99.5, 100, 100.5, 150]
        rangeVol = [ 0.11, 0.5, 1.2]
        rangeRrate = [ 0.01, 0.05, 0.15]
        
        maxNumDerErrorList=[]; maxCPSerrorList=[];
        resuCPSvalue = [];     resuCPSdelta = [];   resuCPSgamma = []
        resuCPStheta = [];     resuCPSrho   = [];   resuCPSvega  = []
        resuCPparity = []
        
        err_delta = 5e-5
        err_gamma = 5e-5
        err_theta = 5e-5
        err_rho  =  5e-5
        err_vega =  5e-5
        total_number_of_error = 0

        self.printDetails(
          "     Type  items err-value err-delta err-gamma " +
          "err-theta  err-rho  err-vega "
        )
        for under in rangeUnder:
          for Qrate in rangeQrate:
            for resTime in rangeResTime:
                for Rrate in rangeRrate:
                    for strike in rangeStrike:
                        for vol in rangeVol:
                            #Check straddle
                            call     = pricer(
                              'Call'    ,under,strike,Qrate,Rrate,resTime,vol)
                            put      = pricer(
                              'Put'     ,under,strike,Qrate,Rrate,resTime,vol)
                            straddle = pricer(
                              'Straddle',under,strike,Qrate,Rrate,resTime,vol)
                            #CPparity = call.value() - put.value() - \
                            #  under*exp(-Qrate*resTime) + \
                            #  strike*exp(-Rrate*resTime)
                            resuCPSvalue.append(
                              call.value()+put.value()-straddle.value())
                            resuCPSdelta.append(
                              call.delta()+put.delta()-straddle.delta())
                            resuCPSgamma.append(
                              call.gamma()+put.gamma()-straddle.gamma())
                            resuCPStheta.append(
                              call.theta()+put.theta()-straddle.theta())
                            resuCPSrho.append(
                              call.rho()  +put.rho()  -straddle.rho())
                            resuCPSvega.append(
                              call.vega() +put.vega() -straddle.vega())
                            #resuCPparity.append(CPparity)
                
        typ = 'C+P-S'
        self.printDetails(
          "%9s   %d  %7.2e %7.2e %7.2e %7.2e %7.2e %7.2e" % 
          (typ, len(resuCPSvalue), max(resuCPSvalue), max(resuCPSdelta), 
          max(resuCPSgamma), max(resuCPStheta), max(resuCPSrho), 
          max(resuCPSvega))
        )

        maxCPSerrorList.append(max(resuCPSvalue))
        maxCPSerrorList.append(max(resuCPSdelta))
        maxCPSerrorList.append(max(resuCPSgamma))
        maxCPSerrorList.append(max(resuCPStheta))
        maxCPSerrorList.append(max(resuCPSrho))
        maxCPSerrorList.append(max(resuCPSvega))

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
                            
                        resuDelta.append(
                          relErr(opt.delta(), deltaNum, under))
                        resuGamma.append(
                          relErr(opt.gamma(), gammaNum, under))
                        resuTheta.append(
                          relErr(opt.theta(), thetaNum, under))
                        resuRho.append(
                          relErr(opt.rho(),   rhoNum,   under))
                        resuVega.append(
                          relErr(opt.vega(),  vegaNum,  under))
                                           
                        if(relErr(opt.delta(),deltaNum,under) > err_delta or
                           relErr(opt.gamma(),gammaNum,under) > err_gamma or
                           relErr(opt.theta(),thetaNum,under) > err_theta or
                           relErr(opt.rho(),  rhoNum,  under) > err_rho   or
                           relErr(opt.vega(), vegaNum, under) > err_vega  ):
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
          self.printDetails(
            "%9s %6d %7.2e %7.2e %7.2e %7.2e %7.2e" % 
            (typ, len(resuDelta), max(resuDelta), max(resuGamma), 
            max(resuTheta), max(resuRho), max(resuVega))
          )
                   
          maxNumDerErrorList.append(max(resuDelta))
          maxNumDerErrorList.append(max(resuGamma))
          maxNumDerErrorList.append(max(resuTheta))
          maxNumDerErrorList.append(max(resuRho))
          maxNumDerErrorList.append(max(resuVega))

        self.printDetails('')
        #maxCPparityError = max(resuCPparity)
        #self.printDetails("C-P parity err = %g" % (maxCPparityError))
        self.printDetails("Final maximum C+P-S = %g" % max(maxCPSerrorList))
        self.printDetails(
          "Final maximum global error on numerical derivatives = %g" % 
          max(maxNumDerErrorList)
        )
        if total_number_of_error > 1:
            self.printDetails(
                "total number of failures: %d" %
                total_number_of_error
            )
            return 1
        else:
            return 0


if __name__ == '__main__':
    BinaryOptionTest().test('European option pricer')






