
# $Id$
# $Source$
# $Log$
# Revision 1.13  2001/08/23 14:58:53  lballabio
# Doc fixes
#
# Revision 1.12  2001/08/22 17:54:35  lballabio
# Documentation revamped
#
# Revision 1.11  2001/07/24 16:59:33  nando
# documentation revised
#
# Revision 1.10  2001/07/19 16:40:10  lballabio
# Improved docs a bit
#
# Revision 1.9  2001/06/05 09:35:13  lballabio
# Updated docs to use Doxygen 1.2.8
#
# Revision 1.8  2001/05/24 12:52:01  nando
# smoothing #include xx.hpp
#
#
# makefile for QuantLib documentation under Borland C++
#

.autodepend
.silent

# Tools to be used
SED       = sed
DOXYGEN   = doxygen
LATEX     = latex
PDFLATEX  = pdflatex
MAKEINDEX = makeindex
DVIPS     = dvips

# Options
TEX_OPTS = --quiet --pool-size=1000000


# Primary target:
# all docs
all:: tex-files
    cd latex
    $(PDFLATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(PDFLATEX) $(TEX_OPTS) refman
    $(LATEX) $(TEX_OPTS) refman
    $(DVIPS) refman
    cd ..

# HTML documentation only
html::
    $(DOXYGEN) quantlib.win32.doxy
    copy images\*.jpg html
    copy images\*.pdf latex
    copy images\*.eps latex

# PDF documentation
pdf:: tex-files
    cd latex
    $(PDFLATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(PDFLATEX) $(TEX_OPTS) refman
    cd ..

# PostScript documentation
ps:: tex-files
    cd latex
    $(LATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(LATEX) $(TEX_OPTS) refman
    $(DVIPS) refman
    cd ..

# Correct LaTeX files to get the right layout
tex-files:: html
    cd latex
    ren refman.tex oldrefman.tex
    $(SED) -e "/Page Index/d" \
           -e "/input{pages}/d" \
           -e "/Page Documentation/d" \
           -e "/input{index}/d" \
           -e "/include{install}/d" \
           -e "/include{license}/d" \
           -e "/include{platforms}/d" \
           -e "/include{usage}/d" \
           -e "/include{where}/d" \
           -e "/include{todo}/d" \
           -e "s/i_ndex/index/" \
           -e "s/w_here/where/" \
           -e "s/i_nstall/install/" \
           -e "s/u_sage/usage/" \
           -e "s/p_latforms/platforms/" \
           -e "s/t_odo/todo/" \
           -e "s/l_icense/license/" \
           oldrefman.tex > refman.tex
    del oldrefman.tex
    cd ..

# Clean up
clean::
    if exist html  rmdir /S /Q html
    if exist latex rmdir /S /Q latex
