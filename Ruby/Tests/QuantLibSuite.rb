
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
 Revision 1.1  2001/04/04 14:10:30  lballabio
 Ruby tests moved on top of RubyUnit


=end

require 'runit/testcase'
require 'runit/testsuite'
require 'runit/cui/testrunner'

require 'dates'
require 'distributions'
require 'random_generators'
require 'statistics'

suite = RUNIT::TestSuite.new
suite.add_test(DateTest.suite)
suite.add_test(DistributionTest.suite)
suite.add_test(RNGTest.suite)
suite.add_test(StatisticsTest.suite)

result = RUNIT::CUI::TestRunner.run(suite)
unless result.succeed?
    exit(1)
end


