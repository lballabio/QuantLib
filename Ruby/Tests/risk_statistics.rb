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
 Revision 1.1  2001/04/12 07:20:33  lballabio
 *** empty log message ***

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

# define a Gaussian
def gaussian(x, average, sigma)
    normFact = sigma * Math.sqrt( 2 * Math::PI )
    dx = x-average
    Math.exp( -dx*dx/(2.0*sigma*sigma) ) / normFact
end

class RiskStatisticsTest < RUNIT::TestCase
    def name
        "Testing risk statistics..."
    end
    def test
        s = QuantLib::RiskStatistics.new
        averageRange = [-100.0, 0.0, 100.0]
        sigmaRange = [0.1, 1.0, 10]
        n = 25000
        numberOfSigma = 15

        averageRange.each { |average|
            sigmaRange.each { |sigma| 
                #target cannot be changed:
                #it is a strong assumption to compute values to be checked
                target = average
                normal = QuantLib::NormalDistribution.new(average, sigma)

                dataMin = average - numberOfSigma*sigma
                dataMax = average + numberOfSigma*sigma
                # even NOT to include average
                h = (dataMax-dataMin)/(n-1)

                data = (0...n).map { |i| dataMin+h*i }
                weights = data.map { |x| gaussian(x,average,sigma) }

                s.addWeightedSequence(QuantLib::DoubleVector.new(data), 
                    QuantLib::DoubleVector.new(weights))

                unless s.samples == n
                    raise "wrong number of samples\n" + \
                          "calculated: #{s.samples}\n" + \
                          "expected  : #{n}\n"
                end

                rightWeightSum = weights.sum
                unless s.weightSum == rightWeightSum
                    raise "wrong sum of weights\n" + \
                          "calculated: #{s.weightSum}\n" + \
                          "expected  : #{rightWeightSum}\n"
                end

                unless s.min == dataMin
                    raise "wrong minimum value\n" + \
                          "calculated: #{s.min}\n" + \
                          "expected  : #{dataMin}\n"
                end

                unless (s.max-dataMax).abs <= 1.0e-13
                    raise "wrong maximum value\n" + \
                          "calculated: #{s.max}\n" + \
                          "expected  : #{dataMax}\n"
                end

                if average == 0.0
                    check = (s.mean-average).abs
                else
                    check = ((s.mean-average).abs)/average
                end
                unless check <= 1.0e-13
                    raise "wrong mean value\n" + \
                          "calculated: #{s.mean}\n" + \
                          "expected  : #{average}\n"
                end

                unless ((s.variance-sigma*sigma).abs)/(sigma*sigma) <= 1.0e-4
                    raise "wrong variance\n" + \
                          "calculated: #{s.variance}\n" + \
                          "expected  : #{sigma*sigma}\n"
                end

                unless ((s.standardDeviation-sigma).abs)/sigma <= 1.0e-4
                    raise "wrong standard deviation\n" + \
                          "calculated: #{s.standardDeviation}\n" + \
                          "expected  : #{sigma}\n"
                end

                unless s.skewness.abs <= 1.0e-4
                    raise "wrong skewness\n" + \
                          "calculated: #{s.skewness}\n" + \
                          "expected  : 0.0\n"
                end

                unless s.kurtosis.abs <= 1.0e-1
                    raise "wrong kurtosis\n" + \
                          "calculated: #{s.kurtosis}\n" + \
                          "expected  : 0.0\n"
                end

                rightVar = -[average-2.0*sigma, 0.0].min
                var = s.valueAtRisk(0.9772)
                if rightVar == 0.0
                    check = (var-rightVar).abs
                else
                    check = ((var-rightVar).abs)/rightVar
                end
                unless check <= 1.0e-3
                    raise "wrong value at risk\n" + \
                          "calculated: #{var}\n" + \
                          "expected:   #{rightVar}\n"
                end

                tempVar = average-2.0*sigma
                rightExSF = average - sigma*sigma*gaussian(tempVar,
                    average, sigma)/(1.0-0.9772)
                rightExSF = -[rightExSF, 0.0].min
                exShortfall = s.expectedShortfall(0.9772)
                if rightExSF == 0.0
                    check = exShortfall.abs
                else
                    check = ((exShortfall-rightExSF).abs)/rightExSF
                end
                unless check <= 1.0e-3
                    raise "wrong expected shortfall\n" + \
                          "calculated: #{exShortfall}\n" + \
                          "expected:   #{rightExSF}\n"
                end

                rightSF = 0.5
                shortfall = s.shortfall(target)
                unless ((shortfall-rightSF).abs)/rightSF <= 1.0e-8
                    raise "wrong shortfall\n" + \
                          "calculated: #{shortfall}\n" + \
                          "expected:   #{rightSF}\n"
                end

                rightAvgSF = sigma/Math.sqrt( 2 * Math::PI )
                avgShortfall = s.averageShortfall(target)
                check = ((avgShortfall-rightAvgSF).abs)/rightAvgSF
                unless check <= 1.0e-4
                    raise "wrong average shortfall\n" + \
                          "calculated: #{avgShortfall}\n" + \
                          "expected:   #{rightAvgSF}\n"
                end

                s.reset
            }
        }
    end
end

if $0 == __FILE__
    RUNIT::CUI::TestRunner.run(RiskStatisticsTest.suite)
end

