***********************
***  Borland users  ***
***********************

The makefile is to be used with the free Borland C++ Compiler available at
http://www.borland.com/bcppbuilder/freecompiler/

To compile the Python extension you need to set the environment variable
PYTHON_HOME to your Python installation dir (e.g. C:\Python20).
The command 'make' builds the extension (SWIG wrap files are automatically
generated if needed);
'make install' copies the *.py and *.dll files into your Python
installation;
'make test' runs the Python test (QuantLib\Python\Tests)

**************************
***  Visual C++ users  ***
**************************

Visual C++ 6.0 projects files are supplied.

QuantLib\Python\PyQuanLib.dsw is the Python extension workspace that includes
PyQuantLib.dsp along with QuantLib.dsp.
In Visual C++ under Tools | Options | Directories, C:\Python20\include) to the
Include directory list and C:\Python20\libs to the Library directory list
(replace C:\Python20 with the path to your Python installation).
In order to (re)generate the wrapper files (which is not needed if you
downloaded a QuantLib release) you will need a SWIG installation. You can refer
to http://quantlib.sourceforge.net/FAQ.html for help about installing SWIG.
The batch file QuantLib\Win\PyQuantLibWrap.bat builds the wrapper files.

The suite of tests in the Tests folder is implemented on top of the PyUnit
framework, available from http://pyunit.sourceforge.net/ and scheduled to be
included in the Python standard library beginning with the Python 2.1 release.
The tests can be run from the Tests folder by launching the QuantLibTestSuite.py
script. For platforms with a make program, they can also be run from the base
QuantLib folder by typing "make python-test".

