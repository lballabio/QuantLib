
******************************************************
 $Source$
 $Log$
 Revision 1.7  2001/03/15 16:08:15  nando
 doxy config file renamed and unified (Win32 and Linux)
 html doc footer modified

******************************************************

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
4) a) Borland user: make HTML
   b) VC user: go to the QuantLib\Docs directory and type:
        doxygen quantlib.doxy
Just ignore TeX/LaTeX warning/error.

If you want to produce TeX/PDF/PS documentation:
4) install MiKTeX from http://www.miktex.org
Choose to create your "Local TEXMF Tree".
Add texmf\miktex\bin to your PATH.
5) under your "Local TEXMF Tree" create a folder \tex\latex\fancyhdr
that includes fancyhdr.sty, extramarks.sty and fixmarks.sty (the 3 files can be
downloaded from http://toocool.calpoly.edu/latex/fancy_header.html)
6) download http://www.tex.ac.uk/tex-archive/support/epstopdf/epstopdf.exe
and copy it into a directory on your path

