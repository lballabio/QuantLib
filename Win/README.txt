***********************
***  Borland users  ***
***********************

The makefile is to be used with the free Borland C++ Compiler available at
http://www.borland.com/bcppbuilder/freecompiler/

The command 'Make' build the library.

To compile the Python extension you need to set the environment variable PYTHON_HOME to your Python installation dir (e.g. C:\Python20).
'Make Python' builds the extension (SWIG wrap files are automatically generated if needed).
'Make install' copies the *.py and *.dll files into your Python installation.
'Make test' runs the Python test (QuantLib\Python\Tests)

To produce documentation see QuantLib  \Docs\README.txt.
Win32 direction follows.
You need Doxygen: download it from http://www.stack.nl/~dimitri/doxygen/download.html#latestsrc and copy the binaries in a directory in your PATH.
You also need Graphviz (http://www.research.att.com/sw/tools/graphviz/gviz15.exe) and Aladdin Ghostscript (ftp://ftp.cs.wisc.edu/ghost/aladdin/gs650/gs650w32.exe).
Install them.
'Make HTML' produce HTML documentation

**************************
***  Visual C++ users  ***
**************************

Visual C++ 6.0 projects files are supplied.

QuantLib.dsp is the project included in the workspace QuantLi.dsw.
Unfortunately the compiled LIB do not work corectly due to, probably, a bug in the compiler. In particular, broken object code is generated for the BSMAmericanPricer class.
Please let us know whether it works on your configuration, or if you 
are aware of any workarounds.

PyQuanLib.dsw is the Python extension workspace that includes PyQuantLib.dsp along with QuantLib.dsp. To compile the Python extension you need a Python installation.
Python 1.5.2 is available from http://www.python.org/1.5/
Python 2.0 is available from http://www.activestate.com/Products/ActivePython/Download.html or http://sourceforge.net/projects/python/
In Visual C++ under Tools | Options | Directories add C:\Python20\include to the Include directory list and C:\Python20\libs to the Library directory list (change C:\Python20 accordingly to your Python installation dir).
If the wrapper files are out of synch (this is possible if you get a snapshot of the current QuantLib CVS) you need to re-generate it with SWIG.
Refer to http://quantlib.sourceforge.net/FAQ.html for help about installing SWIG.
The batch QuantLib\Win\PyWrap.bat build the wrapper files.
The broken object code generated for the BSMAmericanPricer class is evident in the test Python/Tests/greeks_in_american.py which fails.
