
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
print "Testing random number generators"

from QuantLib import Statistics
from QuantLib import UniformRandomGenerator
from QuantLib import GaussianRandomGenerator

tol = 1e-9
seed = 576919
numIte = 100000
print "Generator                         mean    sigma   skewness kurtosis   min     max"
for RNG in [UniformRandomGenerator, GaussianRandomGenerator]:
  rn = RNG(seed=seed)
  s = Statistics()
  for ite in range(numIte):
    s.add(rn.next())
  print "%35s: %7.4f %8.4f %8.4f %8.4f %7.3f %7.3f " %  (RNG, s.mean(),
    s.standardDeviation(), s.skewness(), s.kurtosis(), s.min(), s.max())

print 'Press return to end this test'
raw_input()
