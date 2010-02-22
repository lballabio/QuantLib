
The documentation is automatically extracted from the source code using
Doxygen (http://www.doxygen.org). The suggested version is the latest one;
versions as back as 1.2.14 should still work.

The basic requirement to produce html documentation are:
1) Doxygen
2) Graphviz (http://www.research.att.com/sw/tools/graphviz/)
3) Aladdin Ghostscript (http://www.ghostscript.com/)

If you want to produce PDF and/or PS documentation you will need also:
4) TeX/LaTeX
5) epstopdf. It's available as Perl script (you will also need Perl, of
   course), C source to be compiled or Windows executable.
The above, and any additional packages your LaTeX installation might need,
are available through the CTAN (http://www.ctan.org/)


*********
* Win32 *
*********

To produce HTML documentation:
1) Install Doxygen. You will also need to add the doxygen\bin folder to your
   PATH environment variable.
2) Install Graphviz. Add Graphviz\bin to your PATH
3) Install Ghostscript. Add gs\gsx.xx\bin to your path
4) a) Borland user: type 'make html' in the Docs directory
   b) VC user: edit quantlib.doxy in the Docs directory:
      - replace ql_basepath with your QuantLib folder path
      - replace ql_version with the current version number
      - change GENERATE_HTML = NO to GENERATE_HTML = YES
      - run 'doxygen quantlib.doxy'

If you want to produce TeX/PDF/PS documentation:
5) install MiKTeX 2 or higher from http://www.miktex.org
6) download http://www.tex.ac.uk/tex-archive/support/epstopdf/epstopdf.exe
   and copy it into a directory on your path (recent version of MikTeX already
   includes epstopdf, so this step could be skipped)
7) a) Borland users: type 'make ps' or 'make pdf' from the Docs
      directory
   b) VC user: TeX/PDF/PS documentation cannot be easily generated without a
      make facility. See Borland Makefile.mak for directions.

If you want to compile the HTML documentation into Microsoft Help format:
8) download and install the HTML Help Workshop from
   msdn.microsoft.com/library/default.asp?url=/library/en-us/htmlhelp/html/vsconhh1start.asp
    and add to your path the directory created by the installer
9) a) Borland user: type 'make htmlhelp' in the Docs directory
   b) VC user: edit quantlib.doxy in the Docs directory:
      - replace ql_basepath with your QuantLib folder path
      - replace ql_version with the current version number
      - change GENERATE_HTML = NO to GENERATE_HTML = YES
      - change GENERATE_HTMLHELP = NO to GENERATE_HTMLHELP = YES
      - run 'doxygen quantlib.doxy'

Borland users can look in Makefile.mak for additional directives.


**************
* Unix/Linux *
**************

From the base directory, you'll have to do

cd Docs
make docs-all

You'll end up with:
- a directory Docs/html containing the HTML docs
- a directory Docs/latex containing a lot of latex files you can ignore,
  plus refman.pdf and refman.ps which are their final result
- and a directory Docs/man containing the man pages.

Look in Makefile.am for additional directives
