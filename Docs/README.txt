The documentation is automatically extracted from the source code using
DoxyGen (http://www.stack.nl/~dimitri/doxygen/).

The basic requirement to produce html documentation are:
1) DoxyGen (latest release at http://www.stack.nl/~dimitri/doxygen/download.html#latestsrc)
2) Graphviz (http://www.research.att.com/sw/tools/graphviz/)
3) Aladdin Ghostscript (http://www.cs.wisc.edu/~ghost/)

If you want to produce PDF and/or PS documentation you will need also:
4) TeX/LaTeX
5) LaTeX Fancy Header (http://toocool.calpoly.edu/latex/fancy_header.html)
6) epstopdf. It's available as .exe for Windows and C sources to be compiled for every other platform. Non-Windows users might consider using the equivalent epstopdf.pl Perl script (in this case you will need Perl, of course).
http://www.tex.ac.uk/tex-archive/help/Catalogue/entries/epstopdf.html
    
      

*********
* Win32 *
*********


1) Install Doxygen. You will also need to add to your PATH doxygen-x.x.x\bin.
2) Install Graphviz. Add to your PATH Graphviz\bin
2) Install Ghostscript. Add to your path gs\gsx.xx\bin
3) To produce HTML documentation:
    a) Borland user: make HTML
    b) VC user will have to go to the QuantLib\Docs directory and there type:
        doxygen offline.doxy
Just ignore TeX/LaTeX warning/error.

If you want to produce TeX/PDF/PS documentation
4) install MiKTeX from http://www.miktex.org
Choose to create your "Local TEXMF Tree".
Add texmf\miktex\bin to your PATH.
5) create a folder fancyhdr that includes fancyhdr.sty, extramarks.sty and fixmarks.sty under your "Local TEXMF Tree" (the 3 files can be downloaded from http://toocool.calpoly.edu/latex/fancy_header.html)
6) download http://www.tex.ac.uk/tex-archive/support/epstopdf/epstopdf.exe and copy it into a directory on your path
