
=begin
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

 QuantLib license is also available at:
 http://quantlib.sourceforge.net/LICENSE.TXT

 $Source$
 $Log$
 Revision 1.1  2001/04/03 14:50:28  lballabio
 Forgotten makefile

=end

require 'QuantLib'
require 'TestUnit'

TestUnit.test('random number generators') {
    seed = 576919
    samples = 1000000
    [QuantLib::UniformRandomGenerator,
     QuantLib::GaussianRandomGenerator].each { |rng|
        rn = rng.new(seed)
        s = QuantLib::Statistics.new
        samples.times { s.add(rn.next) }
        TestUnit.printDetails("#{rng.name[10..-1]}:")
        TestUnit.printDetails("    mean:            #{s.mean}")
        TestUnit.printDetails("    std. deviation:  #{s.standardDeviation}")
        TestUnit.printDetails("    skewness:        #{s.skewness}")
        TestUnit.printDetails("    exc. kurtosis:   #{s.kurtosis}")
        TestUnit.printDetails("    minimum:         #{s.min}")
        TestUnit.printDetails("    maximum:         #{s.max}")
    }
}

