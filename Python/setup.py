"""
/*
 * Copyright (C) 2001 QuantLib Group
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
#
# $Id$
#
# First version by Enrico Sirola
#

from distutils.core import setup, Extension
from distutils.cmd import Command

class test(Command):
    # Original version of this class posted
    # by Berthold Höllmann to distutils-sig@python.org
    description = "test the distribution prior to install"

    user_options = [
        ('test-dir=', None,
         "directory that contains the test definitions"),
        ('test-prefix=', None,
         "prefix to the testcase filename"),
        ('test-suffixes=', None,
         "a list of suffixes used to generate names the of the testcases")
        ]

    def initialize_options(self):
        self.build_base = 'build'
        # these are decided only after 'build_base' has its final value
        # (unless overridden by the user or client)
        self.test_dir = 'Tests'
        self.test_prefix = ''
        self.test_suffixes = None

    # initialize_options()

    def finalize_options(self):
        import os
        if self.test_suffixes is None:
            self.test_suffixes = []
            pref_len = len(self.test_prefix)
            for file in os.listdir(self.test_dir):
                if (file[-3:] == ".py" and
                    file[:pref_len]==self.test_prefix):
                    self.test_suffixes.append(file[pref_len:-3])

        build = self.get_finalized_command('build')
        self.build_purelib = build.build_purelib
        self.build_platlib = build.build_platlib

    # finalize_options()


    def run(self):
        import sys
        # Invoke the 'build' command to "build" pure Python modules
        # (ie. copy 'em into the build tree)
        self.run_command('build')

        # remember old sys.path to restore it afterwards
        old_path = sys.path[:]

        # extend sys.path
        sys.path.insert(0, self.build_purelib)
        sys.path.insert(0, self.build_platlib)
        sys.path.insert(0, self.test_dir)

        # build include path for test

        for case in self.test_suffixes:
            TEST = __import__(self.test_prefix+case,
                              globals(), locals(),
                              [''])
            try:
                tested_modules = TEST.tested_modules
            except AttributeError:
                tested_modules = None
            else:
                from code_coverage import Coverage
                coverage = Coverage(modules=tested_modules)
                sys.settrace(coverage.trace)

            TEST.test()

            if tested_modules is not None:
                # reload tested modules to get coverage of imports, etc.
                for name in tested_modules:
                    module = sys.modules.get(name)
                    if module:
                        reload(module)

                sys.settrace(None)
                sys.stdout.write("code coverage:\n")
                coverage.write_results(sys.stdout)

        # restore sys.path
        sys.path = old_path[:]

    # run()

# class test

# the directory where QL headers has been installed
predir = "/usr/local/include"

cmdclass = {'test': test}

# changes the compiler from gcc to g++
from distutils import sysconfig
save_init_posix = sysconfig._init_posix
def my_init_posix():
    print 'my_init_posix: changing gcc to g++'
    save_init_posix()
    g = sysconfig._config_vars
    g['CC'] = 'g++'
    g['LDSHARED'] = 'g++ -shared'
sysconfig._init_posix = my_init_posix


setup ( cmdclass = cmdclass, \
        name = "pyQuantLib", \
	version = "0.1.1", \
	maintainer = "Enrico Sirola", \
	maintainer_email = "enri@users.sourceforge.net", \
	url = "http://quantlib.sourceforge.net", \
	py_modules = ["QuantLib"],
	ext_modules = [ \
		Extension ( "QuantLibc", \
                            ["quantlib_wrap.cpp"], \
                            libraries = ["QuantLib"], \
                            include_dirs = [predir] \
                            ) \
                ] \
        )





