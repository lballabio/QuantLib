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
    Revision 1.6  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.5  2001/03/02 10:19:50  marmar
    One failure is enough to halt the test

    Revision 1.4  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.3  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.2  2001/01/08 15:33:23  nando
    improved

"""

import QuantLib
import unittest

def relErr(x1, x2, reference):
    if reference != 0.0:
        return abs(x1-x2)/reference
    else:
        return 10e10

class DividendEuropeanOptionTest(unittest.TestCase):
    def runTest(self):
        "Testing European option pricer with dividends"
        pricer = QuantLib.DividendEuropeanOption
        nstp = 150
        ngrd = nstp+1
        
        div=[3.92,4.21]
        dates=[0.333,0.667]
        
        rangeUnder = [100]
        rangeQrate = [0.0, 0.05,0.15]
        rangeResTime = [1.0,2.0]
        rangeStrike = [50, 99.5, 100, 100.5, 150]
        rangeVol = [0.04,0.2,0.7]
        rangeRrate = [0.01,0.1,0.3]
        
        err_delta = 1e-4
        err_gamma = 1e-4
        err_theta = 1e-4
        err_rho  =  1e-4
        err_vega =  1e-4
        
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
                        vol, div, dates) 
                      opt_val = opt.value()
                      if opt_val>0.00001*under:
                        optPs = pricer(typ, under+dS, strike, Qrate, 
                          Rrate   , resTime   , vol,      div, dates) 
                        optMs = pricer(typ, under-dS, strike, Qrate, 
                          Rrate   , resTime   , vol,      div, dates) 
                        optPt = pricer(typ, under   , strike, Qrate, 
                          Rrate   , resTime+dT, vol,      div, dates) 
                        optMt = pricer(typ, under   , strike, Qrate, 
                          Rrate   , resTime-dT, vol,      div, dates) 
                        optPr = pricer(typ, under   , strike, Qrate, 
                          Rrate+dR, resTime   , vol,      div, dates) 
                        optMr = pricer(typ, under   , strike, Qrate, 
                          Rrate-dR, resTime   , vol,      div, dates) 
                        optPv = pricer(typ, under   , strike, Qrate, 
                          Rrate   , resTime   , vol+dVol, div, dates) 
                        optMv = pricer(typ, under   , strike, Qrate, 
                          Rrate   , resTime   , vol-dVol, div, dates) 
                        
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
                            '\tvega =%+9.5f, vegaNum =%+9.5f err=%7.2e' % \
                              (opt.vega(), vegaNum, 
                               relErr(opt.vega(),vegaNum,under))


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(DividendEuropeanOptionTest())
    unittest.TextTestRunner().run(suite)


