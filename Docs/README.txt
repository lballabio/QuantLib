
The documentation is automatically extracted from the source code using
Doxygen (http://www.doxygen.org). The suggested version is 1.3.6; versions
as back as 1.2.14 should still work.

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
1) Install Doxygen. You will also need to add to your PATH doxygen-x.x.x\bin.
2) Install Graphviz. Add to your PATH Graphviz\bin
3) Install Ghostscript. Add to your path gs\gsx.xx\bin
4) a) Borland user: type 'make html' in the Docs directory
   b) VC user: go to the Docs directory and type:
      doxygen quantlib.doxy

If you want to produce TeX/PDF/PS documentation:
5) install MiKTeX 2 or higher from http://www.miktex.org
6) download http://www.tex.ac.uk/tex-archive/support/epstopdf/epstopdf.exe
   and copy it into a directory on your path
7) a) Borland users: type 'make ps' or 'make pdf' from the Docs
      directory
   b) VC user: go to the Docs\latex directory and type:
        latex refman
        makeindex refman.idx
        latex refman
      for PostScript output or
        pdflatex refman
        makeindex refman.idx
        pdflatex refman
      for PDF output.
   Just ignore TeX/LaTeX warning/error.

If you want to compile the HTML documentation into Microsoft Help format:
8) download and install the HTML Help Workshop from
   msdn.microsoft.com/library/default.asp?url=/library/en-us/htmlhelp/html/vsconhh1start.asp
    and add to your path the directory created by the installer
9) type 'hhc index.hhp' from the Docs\html directory. The file
    index.chm will be created.




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

