***********************
***  Borland users  ***
***********************

The makefile is to be used with the free Borland C++ Compiler available at
<http://www.borland.com/bcppbuilder/freecompiler/>.

The command 'make' builds the library.

The file Docs\README.txt details the requirements for producing
documentation.

'make docs-html' produces the HTML documentation, whose main page will be
Docs\html\index.html
'make docs-pdf' produces the PDF documentation, which will be available at
Docs\latex\refman.pdf
'make docs-all' produces both

**************************
***  Visual C++ users  ***
**************************

Visual C++ 6.0 projects files are supplied.

QuantLib\QuantLib.dsp is the project for building the QuantLib static library.

A few suggestions if you want to use QuantLib into your own application:
1) your main() must be compiled with the same options that were used in
compiling the QuantLib library - namely, you'll have to set the run-time library
to "Multithreaded DLL" or "Debug Multithreaded DLL" depending on whether you're
linking to QuantLib.lib or QuantLib_d.lib, respectively.
The setting is in the project settings, "C/C++" tab, "Code Generation" category.
You'll probably want to check the "Use RTTI" option under the "C++ Language"
category, too.
2) as long as you include ql/quantlib.hpp you don't have to explicitly link QuantLib.lib (or QuantLib_d.lib). This is automatically done by quantlib.hpp
using a pragma statement: #pragma comment(lib,"QuantLib.lib")

**************************
***  All users         ***
**************************

To produce documentation see Docs\README.txt
