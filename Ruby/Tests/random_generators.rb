
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

class RNGTest < RUNIT::TestCase
    def name
        "Testing random number generators..."
    end
    def test
        seed = 576919
        samples = 1000000
        [QuantLib::UniformRandomGenerator,
         QuantLib::GaussianRandomGenerator].each { |rng|
            rn = rng.new(seed)
            s = QuantLib::Statistics.new
            samples.times { s.add(rn.next) }
            # add some meaningful test here
        }
    end
end

if $0 == __FILE__
    RUNIT::CUI::TestRunner.run(RNGTest.suite)
end

