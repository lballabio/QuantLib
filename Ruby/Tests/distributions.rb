
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
 Revision 1.2  2001/04/04 14:10:30  lballabio
 Ruby tests moved on top of RubyUnit

 Revision 1.1  2001/03/27 17:39:39  lballabio
 Making sure dist target is complete (and added distributions to Ruby module)

=end

require 'QuantLib'
require 'runit/testcase'
require 'runit/testsuite'
require 'runit/cui/testrunner'

# define a Gaussian
def gaussian(x,average,sigma)
    normFact = sigma * Math.sqrt(2*Math::PI)
    dx = x-average
    Math.exp(-dx*dx/(2.0*sigma*sigma))/normFact
end

def gaussianDerivative(x,average,sigma)
    normFact = sigma*sigma*sigma*Math.sqrt(2*Math::PI)
    dx = x-average
    -dx * Math.exp(-dx*dx/(2.0*sigma*sigma))/normFact
end

# define the norm of a discretized function and the difference between two
class Array
    def sum
        result = 0
        each { |i| result += i }
        result
    end
    def norm(h)
        f2 = map { |i| i*i }
        # numeric integral of f^2
        Math.sqrt(h*(f2.sum-0.5*f2[0]-0.5*f2[-1]))
    end
    def diff(a)
        result = Array.new
        each_with_index { |x,i| result << x-a[i] }
        result
    end
end

# Test

class DistributionTest < RUNIT::TestCase
    def name
        "Testing distributions..."
    end
    def test
        average = 0.0
        sigma = 1.0

        normal = QuantLib::NormalDistribution.new(average, sigma)
        cum =    QuantLib::CumulativeNormalDistribution.new(average, sigma)
        invCum = QuantLib::InvCumulativeNormalDistribution.new(average, sigma)
        
        xMin = average - 4*sigma
        xMax = average + 4*sigma

        n = 10001
        h = (xMax-xMin)/(n-1)

        x = Array.new(n)        # creates a list of N elements
        0.upto(n-1) { |i| x[i] = xMin+h*i }

        y = x.map { |z| gaussian(z,average,sigma) }
        
        yIntegrated = x.map { |z| cum.call(z) }
        yTemp       = x.map { |z| normal.call(z) }
        y2Temp      = x.map { |z| cum.derivative(z) }
        xTemp       = yIntegrated.map { |z| invCum.call(z) }
        yd          = x.map { |z| normal.derivative(z) }
        ydTemp      = x.map { |z| gaussianDerivative(z,average,sigma) }
        
        #check norm=gaussian
        e = yTemp.diff(y).norm(h)
        unless e <= 1.0e-16
            raise "tolerance exceeded\n" + \
                  "norm of C++ NormalDistribution " + \
                  "minus analytic gaussian: #{e}\n"
        end
        
        #check invCum(cum) = Identity
        e = xTemp.diff(x).norm(h)
        unless e <= 1.0e-3
            raise "tolerance exceeded\n" + \
                  "norm of C++ invCum(cum(.)) " + \
                  "minus identity: #{e}\n"
        end
        
        #check cum.derivative=normal
        e = y2Temp.diff(y).norm(h)
        unless e <= 1.0e-16
            raise "tolerance exceeded\n" + \
                  "norm of C++ Cumulative.derivative " + \
                  "minus analytic gaussian: #{e}\n"
        end
        
        #check normal.derivative=gaussianDerivative
        e = ydTemp.diff(yd).norm(h)
        unless e <= 1.0e-16
            raise "tolerance exceeded\n" + \
                  "norm of C++ NormalDist.derivative " + \
                  "minus analytic gaussian derivative: #{e}\n"
        end
    end
end

if $0 == __FILE__
    RUNIT::CUI::TestRunner.run(DistributionTest.suite)
end

