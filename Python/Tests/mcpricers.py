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
print "Testing Monte Carlo pricers"

from QuantLib import McEuropeanPricer
from QuantLib import McAsianPricer

type = "Call"
underlying = 100
strike = 100
underlyingGrowthRate = 0.0
riskFreeRate = 0.05
residualTime = 1.0
volatility = 0.3
timesteps = 100
numIte = 10000
seed = 6919789

print "Pricer                          iterations   Value     Error Estimate "
for pricer in [McEuropeanPricer, McAsianPricer]:
  p = pricer(type, underlying, strike, underlyingGrowthRate, riskFreeRate,
             residualTime, volatility, timesteps, numIte, seed=seed)
  print "%30s: %7i %12.6f %12.6f" %(pricer, numIte, p.value(), p.errorEstimate())

print 'Press return to end this test'
raw_input()
