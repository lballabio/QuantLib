
$Id$

The documentation is automatically extracted from the source code using
DoxyGen (http://www.stack.nl/~dimitri/doxygen/).

The basic requirement to produce html documentation are:
1) DoxyGen (latest release at
http://www.stack.nl/~dimitri/doxygen/download.html#latestsrc)
2) Graphviz (http://www.research.att.com/sw/tools/graphviz/)
3) Aladdin Ghostscript (http://www.cs.wisc.edu/~ghost/)

If you want to produce PDF and/or PS documentation you will need also:
4) TeX/LaTeX
5) LaTeX Fancy Header (http://toocool.calpoly.edu/latex/fancy_header.html)
6) epstopdf. It's available as Perl script (you will also need Perl, of
course), C source to be compiled or Windows executable.
http://www.tex.ac.uk/tex-archive/help/Catalogue/entries/epstopdf.html
or http://www.ctan.org/tex-archive/support/epstopdf/



*********
* Win32 *
*********

To produce HTML documentation:
1) Install Doxygen. You will also need to add to your PATH doxygen-x.x.x\bin.
2) Install Graphviz. Add to your PATH Graphviz\bin
3) Install Ghostscript. Add to your path gs\gsx.xx\bin
4) a) Borland user: type 'make html' in the QuantLib\Docs directory
   b) VC user: go to the QuantLib\Docs directory and type:
        doxygen quantlib.win32.doxy

If you want to produce TeX/PDF/PS documentation:
5) install MiKTeX 2 or higher from http://www.miktex.org
Choose to create your "Local TEXMF Tree".
Add miktex\bin (where latex.exe is) to your PATH.
6) under your "Local TEXMF Tree" create a folder \tex\latex\fancyhdr
that includes fancyhdr.sty, extramarks.sty and fixmarks.sty (the 3 files can be
downloaded from http://toocool.calpoly.edu/latex/fancy_header.html)
7) download http://www.tex.ac.uk/tex-archive/support/epstopdf/epstopdf.exe
and copy it into a directory on your path
8) Open MikTeX Option panel from Programs/MiKTeX 2 list and click on the
"Refresh Now" button in the "General" folder.
9) a) Borland users: type 'make ps' or 'make pdf' in the QuantLib\Docs directory
   b) VC user: go to the QuantLib\Docs\latex directory and type:
        latex refman
        makeindex refman.idx
        latex refman
      for PostScript output or
        pdflatex refman
        makeindex refman.idx
        pdflatex refman
      for PDF output.
Just ignore TeX/LaTeX warning/error.

*********
* Win32 *
*********

From the QuantLib base directory, you'll have to do

cd Docs
./bootstrap
./configure
make docs-all

You'll end up with:
- a directory Docs/html containing the HTML docs
- a directory Docs/latex containing a lot of latex files you can ignore,
  plus refman.pdf and refman.ps which are their final result
- and a directory Docs/man containing the man pages.

Look in MAkefile.am for additional directives
