
The QuantLib test suite is implemented on top of CppUnit, available from
http://cppunit.sourceforge.net. Version 1.8.0 or later of CppUnit is required.

CppUnit is licensed under the GNU Lesser General Public License---see the file
CPPUNIT-COPYING for details.

*******************************************
*** BORLAND COMMAND LINE COMPILER USERS ***
*******************************************

The makefile provided has been used with CppUnit 1.9.10
(available from http://cppunit.sourceforge.net/snapshot/)
which provides a Borland makefile under the contrib folder.
Build the libraries and copy cppunit_bc5.lib and cppunitd_bc5.lib under your
<cppunit>-install_dir/lib

*******************************************
***     VISUAL C++ COMPILER USERS       ***
*******************************************

CppUnit VC++ projects use the '(Debug) Multithread DLL' setting.
Switch to '(Debug) Multithread' in order to make it work with QuantLib (or viceversa, switch QuantLib to '(Debug) Multithread DLL')
