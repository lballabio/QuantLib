"""
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/
"""

# Make sure that Python path contains the directory of QuantLib and that of this file
from QuantLib  import BSMAmericanOption
import copy
import math

pricer = BSMAmericanOption
nstp = 100
ngrd = 100

def relErr(x1, x2, reference):
    if reference != 0.0:
	return abs(x1-x2)/reference
    else:
    	return 10e10
    	
rangeUnder = [100]
rangeQrate = [0.05]
rangeResTime = [1.0]
#rangeStrike = [50, 99.5, 100, 100.5, 150]
rangeStrike = [100]
rangeVol = [ 0.11, 0.5, 1.2]
rangeRrate = [ 0.01, 0.05, 0.15]

maxNumDerErrorList=[]; maxCPSerrorList=[];
resuCPSvalue = [];     resuCPSdelta = [];   resuCPSgamma = [];     resuCPStheta = [];
resuCPSrho   = [];     resuCPSvega  = [];   resuCPparity = []

err_delta = 5e-5
err_gamma = 5e-5
err_theta = 5e-5
err_rho  =  5e-5
err_vega =  5e-5
total_number_of_error = 0

print "Test of the class BSMEuropeanOption, maximum-error"
print "     Type  items err-value err-delta err-gamma err-theta  err-rho  err-vega "

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
              option = pricer(typ,under,strike,Qrate,Rrate,resTime,vol,nstp,ngrd) 
              opt_val = option.value()
              if opt_val>0.00001*under:
                optionPs = pricer(typ,under+dS,strike,Qrate,Rrate   ,resTime   ,vol,nstp,ngrd) 
                optionMs = pricer(typ,under-dS,strike,Qrate,Rrate   ,resTime   ,vol,nstp,ngrd) 
                optionPt = pricer(typ,under   ,strike,Qrate,Rrate   ,resTime+dT,vol,nstp,ngrd) 
                optionMt = pricer(typ,under   ,strike,Qrate,Rrate   ,resTime-dT,vol,nstp,ngrd) 
                optionPr = pricer(typ,under   ,strike,Qrate,Rrate+dR,resTime   ,vol,nstp,ngrd) 
                optionMr = pricer(typ,under   ,strike,Qrate,Rrate-dR,resTime   ,vol,nstp,ngrd) 
                optionPv = pricer(typ,under   ,strike,Qrate,Rrate   ,resTime   ,vol+dVol,nstp,ngrd) 
                optionMv = pricer(typ,under   ,strike,Qrate,Rrate   ,resTime   ,vol-dVol,nstp,ngrd) 
                
                deltaNum = (optionPs.value()-optionMs.value())/(2*dS)
                gammaNum = (optionPs.delta()-optionMs.delta())/(2*dS)
                thetaNum =-(optionPt.value()-optionMt.value())/(2*dT)
                rhoNum   = (optionPr.value()-optionMr.value())/(2*dR)
                vegaNum  = (optionPv.value()-optionMv.value())/(2*dVol)
		            
                resuDelta.append(relErr(option.delta(), deltaNum, under))
                resuGamma.append(relErr(option.gamma(), gammaNum, under))
                resuTheta.append(relErr(option.theta(), thetaNum, under))
                resuRho.append(  relErr(option.rho(),   rhoNum,   under))
                resuVega.append( relErr(option.vega(),  vegaNum,  under))
                                   
                if(relErr(option.delta(),deltaNum,under) > err_delta or
                   relErr(option.gamma(),gammaNum,under) > err_gamma or
                   relErr(option.theta(),thetaNum,under) > err_theta or
                   relErr(option.rho(),  rhoNum,  under) > err_rho   or
                   relErr(option.vega(), vegaNum, under) > err_vega  ):
                  total_number_of_error = total_number_of_error + 1
                  print "Attention required ",typ,under,strike,Qrate,Rrate,resTime,vol
                  print '\tvalue=%+9.5f' % (opt_val)
                  print '\tdelta=%+9.5f, deltaNum=%+9.5f err=%7.2e' % (option.delta(),deltaNum,relErr(option.delta(),deltaNum,under))
                  print '\tgamma=%+9.5f, gammaNum=%+9.5f err=%7.2e' % (option.gamma(),gammaNum,relErr(option.gamma(),gammaNum,under))
                  print '\ttheta=%+9.5f, thetaNum=%+9.5f err=%7.2e' % (option.theta(),thetaNum,relErr(option.theta(),thetaNum,under))
                  print '\trho  =%+9.5f,   rhoNum=%+9.5f err=%7.2e' % (option.rho(),  rhoNum,  relErr(option.rho(),  rhoNum,  under))
                  print '\tvega =%+9.5f, vegaNum =%+9.5f err=%7.2e' % (option.vega(), vegaNum, relErr(option.vega(), vegaNum, under))
  print "%9s   %d            %7.2e %7.2e %7.2e %7.2e %7.2e" % (typ, len(resuDelta), 
        max(resuDelta), max(resuGamma), max(resuTheta), max(resuRho), max(resuVega))
	       
  maxNumDerErrorList.append(max(resuDelta))
  maxNumDerErrorList.append(max(resuGamma))
  maxNumDerErrorList.append(max(resuTheta))
  maxNumDerErrorList.append(max(resuRho))
  maxNumDerErrorList.append(max(resuVega))

print 	
print "Final maximum global error on numerical derivatives = %g" % max(maxNumDerErrorList)
print 
if total_number_of_error > 1:
        print "Test not passed, total noumber of failures:",total_number_of_error
else:
        print "Test passed!!"

raw_input()
