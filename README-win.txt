***********************
***  Borland users  ***
***********************

The makefile is to be used with the free Borland C++ Compiler available at
http://www.borland.com/bcppbuilder/freecompiler/

The command 'make' builds the library.

To compile the Python extension you need to set the environment variable 
PYTHON_HOME to your Python installation dir (e.g. C:\Python20).

'make python' builds the extension (SWIG wrap files are automatically generated 
if needed);
'make python-install' copies the *.py and *.dll files into your Python 
installation;
'make python-test' runs the Python test (QuantLib\Python\Tests)

The file QuantLib\Docs\README.txt details the requirements for producing 
documentation.

'make docs-html' produces the HTML documentation, whose main page will be 
QuantLib\Docs\html\index.htlm
'make docs-pdf' produces the PDF documentation, which will be available at
QuantLib\Docs\latex\refman.pdf
'make docs-all' produces both

**************************
***  Visual C++ users  ***
**************************

Visual C++ 6.0 projects files are supplied.

QuantLib\QuantLib.dsp is the project for building the QuantLib static library.

QuantLib\Python\PyQuanLib.dsw is the Python extension workspace that includes 
PyQuantLib.dsp along with QuantLib.dsp. To compile the Python extension you need 
a Python installation. Python is available at:
http://www.python.org/
http://www.activestate.com/Products/ActivePython/Download.html
http://sourceforge.net/projects/python/
In Visual C++ under Tools | Options | Directories, C:\Python20\include) to the 
Include directory list and C:\Python20\libs to the Library directory list
(replace C:\Python20 with the path to your Python installation).
In order to (re)generate the wrapper files (which is not needed if you 
downloaded a QuantLib release) you will need a SWIG installation. You can refer 
to http://quantlib.sourceforge.net/FAQ.html for help about installing SWIG.
The batch file QuantLib\Win\PyWrap.bat builds the wrapper files.


**************************
***  All users         ***
**************************

To produce documentation see QuantLib\Docs\README.txt
