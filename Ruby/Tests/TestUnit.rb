
# Copyright (C) 2000-2001 QuantLib Group
=begin 
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
 Revision 1.1  2001/03/22 18:07:35  lballabio
 Framework for Ruby tests

=end

class TestUnit
    # This class provides the framework for tests.
    # A test can be run by using the test() class method, as in 
    # TestUnit.test("What I'm testing") { 
    #     code to be tested here
    # }
    # The tested code must raise an exception in case of failure.
    # Detailed results can be output with the class method printDetails() which 
    # prints the string to screen if the -v switch was selected from the  
    # command line, or prints nothing otherwise.
    @@verbose = false
    @@batch = false
    def TestUnit.printDetails(*args)
        puts args.join(' ') if @@verbose
    end
    def TestUnit.test(description)
        # parse arguments
        @@verbose = false
        @@batch = false
        ARGV.each { |s| 
            @@verbose = true if s == '-v'
            @@batch = true   if s == '-b'
        }
        print "Testing #{description}..."
        $stdout.flush
        print "\n" if @@verbose
        startTime = Time.now
        begin
            yield
        rescue Exception => e
            puts "#{e.type}: #{e}" if @@verbose
            print "not passed\n"
            exit(1)
        end
        stopTime = Time.now
        print "passed in #{stopTime-startTime} s.\n"
        unless @@batch
            puts 'Press return to continue'
            ans = $stdin.gets
        end
    end
end

