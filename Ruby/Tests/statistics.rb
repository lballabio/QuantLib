
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
 The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT

 The members of the QuantLib Group are listed in the Authors.txt file, also
 available at http://quantlib.sourceforge.net/Authors.txt
=end

=begin
 $Id$
 $Source$
 $Log$
 Revision 1.3  2001/04/06 18:46:21  nando
 changed Authors, Contributors, Licence and copyright header

=end

require 'QuantLib'
require 'runit/testcase'
require 'runit/testsuite'
require 'runit/cui/testrunner'

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

class StatisticsTest < RUNIT::TestCase
    def name
        "Testing statistics..."
    end
    def test
        tol = 1e-9
        data =    QuantLib::DoubleVector.new(
            [  3,   4,   5,   2,   3,   4,   5,   6,   4,   7])
        weights = QuantLib::DoubleVector.new(
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0])

        s = QuantLib::Statistics.new
        s.addWeightedSequence(data, weights)

        unless s.samples == data.length
            raise "Wrong number of samples\n" + \
                  "calculated: #{s.samples}\n" + \
                  "expected:   #{data.length}\n"
        end

        unless s.weightSum == weights.sum
            raise "Wrong sum of weights\n" + \
                  "calculated: #{s.weightSum}\n" + \
                  "expected:   #{weights.sum}\n"
        end

        unless s.min == data.min
            raise "Wrong minimum value\n" + \
                  "calculated: #{s.min}\n" + \
                  "expected:   #{data.min}\n"
        end

        unless s.max == data.max
            raise "Wrong maximum value\n" + \
                  "calculated: #{s.max}\n" + \
                  "expected:   #{data.max}\n"
        end

        unless (s.mean-data.times(weights).sum/weights.sum).abs <= tol
            raise "Wrong mean value\n" + \
                  "calculated: #{s.mean}\n" + \
                  "expected:   #{data.times(weights).sum/weights.sum}\n"
        end

        unless (s.variance-2.23333333333).abs <= tol
            raise "Wrong variance\n" + \
                  "calculated: #{s.variance}\n" + \
                  "expected:   2.23333333333\n"
        end

        unless (s.standardDeviation-1.4944341181).abs <= tol
            raise "Wrong std. deviation\n" + \
                  "calculated: #{s.standardDeviation}\n" + \
                  "expected:   1.4944341181\n"
        end

        unless (s.skewness-0.359543071407).abs <= tol
            raise "Wrong skewness\n" + \
                  "calculated: #{s.skewness}\n" + \
                  "expected:   0.359543071407\n"
        end

        unless (s.kurtosis+0.151799637209).abs <= tol
            raise "Wrong kurtosis\n" + \
                  "calculated: #{s.kurtosis}\n" + \
                  "expected:   -0.151799637209\n"
        end
    end
end

if $0 == __FILE__
    RUNIT::CUI::TestRunner.run(StatisticsTest.suite)
end


