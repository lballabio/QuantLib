
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
 Revision 1.1  2001/03/30 15:45:42  lballabio
 Still working on make dist (and added IntVector and DoubleVector to Ruby module)

=end

require 'QuantLib'
require 'TestUnit'

module Enumerable
    def sum
        s = 0.0
        each { |i| s += i }
        s
    end
end

module QuantLib
    class DoubleVector
        include Enumerable
        def times(v)
            a = []
	    0.upto(length-1) { |i| a << self[i]*v[i] }
            a
        end
    end
end

TestUnit.test('statistics') {
    tol = 1e-9
    data =    QuantLib::DoubleVector.new(
        [  3,   4,   5,   2,   3,   4,   5,   6,   4,   7])
    weights = QuantLib::DoubleVector.new(
        [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0])
        
    s = QuantLib::Statistics.new
    s.addWeightedSequence(data, weights)
        
    TestUnit.printDetails("samples:             #{s.samples}")
    raise "Wrong number of samples" if s.samples != data.length
        
    TestUnit.printDetails("sum of the weights:  #{s.weightSum}")
    raise "Wrong sum of weights" if s.weightSum != weights.sum
        
    TestUnit.printDetails("minimum value:        #{s.min}")
    raise "Wrong minimum value" if s.min != data.min
        
    TestUnit.printDetails("maximum value:        #{s.max}")
    raise "Wrong maximum value" if s.max != data.max

    TestUnit.printDetails("mean value:           #{s.mean}")
    raise "Wrong mean value" \
        if (s.mean-data.times(weights).sum/weights.sum).abs > tol

    TestUnit.printDetails("variance:             #{s.variance}")
    raise "Wrong variance" if (s.variance-2.23333333333).abs>tol
        
    TestUnit.printDetails("std. deviation:       #{s.standardDeviation}")
    raise "Wrong std. deviation" if (s.standardDeviation-1.4944341181).abs>tol
        
    TestUnit.printDetails("skewness:             #{s.skewness}")
    raise "Wrong skewness" if (s.skewness-0.359543071407).abs>tol
        
    TestUnit.printDetails("excess kurtosis:     #{s.kurtosis}")
    raise "Wrong kurtosis" if (s.kurtosis+0.151799637209).abs>tol
        
    TestUnit.printDetails(
	"error estimate:       #{s.errorEstimate} (not checked)"
    )
}

