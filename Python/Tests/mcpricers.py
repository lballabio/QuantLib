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
    Revision 1.5  2001/02/13 10:04:25  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.4  2001/02/05 16:57:14  marmar
    McAsianPricer replaced by AveragePriceAsian and AverageStrikeAsian

    Revision 1.3  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.2  2001/01/08 15:33:23  nando
    improved

"""

import QuantLib

def main():
   import time
   startTime = time.time()

   print "Testing Monte Carlo pricers"

   type = "Call"
   underlying = 100
   strike = 100
   dividendYield = 0.0
   riskFreeRate = 0.05
   residualTime = 1.0
   volatility = 0.3
   timesteps = 100
   numIte = 10000
   seed = 3456789

   print "Pricer                          iterations   Value     Error Estimate "
   for pricer in [QuantLib.McEuropeanPricer,
                  QuantLib.AverageStrikeAsian,
                  QuantLib.AveragePriceAsian]:
     p = pricer(type, underlying, strike, dividendYield, riskFreeRate,
                residualTime, volatility, timesteps, numIte, seed=seed)
     print "%30s: %7i %12.6f %12.6f" %(pricer, numIte, p.value(), p.errorEstimate())

   print
   print 'Test passed (elapsed time', time.time() - startTime, ')'

main()
print 'Press return to end this test'
raw_input()
