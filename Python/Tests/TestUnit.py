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
    Revision 1.4  2001/03/08 14:48:55  marmar
    printDetails now takes multiple arguments

    Revision 1.3  2001/02/28 12:45:38  lballabio
    Print exception type besides the message

    Revision 1.2  2001/02/23 15:47:21  lballabio
    Added output flush before actual calculation

    Revision 1.1  2001/02/22 14:27:26  lballabio
    Implemented new test framework

"""

class TestUnit:
    """
    This class provides the framework for tests.
    A test can be implemented by deriving a class from TestUnit and 
    implementing a method doTest() which performs the actual test. doTest must 
    return 0 or None in case of success, or return a non-zero integer or raise 
    an exception in case of failure. 
    Detailed results can be output with the method printDetails(string) which 
    prints the string to screen if the -v switch was selected from the command 
    line, or prints nothing otherwise.
    """
    def printDetails(self, *args):
        if self.verbose:
            for item in args:
                print item,
            print
    def test(self, description):
        # parse arguments
        import sys
        self.verbose = 0
        self.batch = 0
        for i in sys.argv[1:]:
            if i == '-v':
                self.verbose = 1
            if i == '-b':
                self.batch = 1
        print 'Testing %s...' % description,
        sys.stdout.flush()
        if self.verbose:
            print
        # start clock
        import time
        startTime = time.time()
        # actual test
        try:
            errorCode = self.doTest()
        except Exception, e:
            if self.verbose:
                print str(e.__class__)[11:]+': '+str(e)
            errorCode = 1
        # stop clock
        stopTime = time.time()
        if errorCode == 0 or errorCode == None:
            print 'passed in %3.2f s.' % (stopTime-startTime)
        else:
            print 'not passed.'
            sys.exit(1)
        if not self.batch:
            print 'Press return to continue'
            raw_input()

