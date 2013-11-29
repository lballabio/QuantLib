
The documentation is automatically extracted from the source code using
Doxygen (http://www.doxygen.org).

The basic requirement to produce html documentation are:
1) Doxygen
2) Graphviz (http://www.research.att.com/sw/tools/graphviz/)
3) Aladdin Ghostscript (http://www.ghostscript.com/)


*********
* Win32 *
*********

To produce HTML documentation:
1) Install Doxygen. You will also need to add the doxygen\bin folder to your
   PATH environment variable.
2) Install Graphviz. Add Graphviz\bin to your PATH
3) Install Ghostscript. Add gs\gsx.xx\bin to your path
4) edit quantlib.doxy in the Docs directory:
    - replace ql_basepath with your QuantLib folder path
    - replace ql_version with the current version number
    - run 'doxygen quantlib.doxy'

**************
* Unix/Linux *
**************

From the base directory, you'll have to do

cd Docs
make docs-all

You'll end up with a directory Docs/html containing the HTML docs and a
directory Docs/man containing the man pages.

Look in Makefile.am for additional directives.
