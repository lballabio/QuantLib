
# $Id$
# $Source$
#
# makefile for QuantLib documentation under Borland C++
#

.autodepend
.silent

# Tools to be used
DOXYGEN   = doxygen
LATEX     = latex
PDFLATEX  = pdflatex
MAKEINDEX = makeindex
DVIPS     = dvips

# Options
TEX_OPTS     = --quiet --pool-size=1000000


# Primary target:
# all docs
all::
    $(DOXYGEN) quantlib.doxy
    cd latex
    $(PDFLATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(PDFLATEX) $(TEX_OPTS) refman
    $(LATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(LATEX) $(TEX_OPTS) refman
    $(DVIPS) refman
    cd ..

# HTML documentation only
html::
    $(DOXYGEN) quantlib.doxy

# PDF documentation
pdf::
    $(DOXYGEN) quantlib.doxy
    cd latex
    $(PDFLATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(PDFLATEX) $(TEX_OPTS) refman
    cd ..

# PostScript documentation
ps::
    $(DOXYGEN) quantlib.doxy
    cd latex
    $(LATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(LATEX) $(TEX_OPTS) refman
    $(DVIPS) refman
    cd ..

# Clean up
clean::
    if exist .\html     rd /s /q .\html
    if exist .\latex    rd /s /q .\latex
