
# makefile for QuantLib documentation under Borland C++
#
# $Id$

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
html: html-offline

html-offline::
    $(DOXYGEN) quantlib.doxy
    copy images\*.jpg html
    copy images\*.png html
    copy images\*.pdf latex
    copy images\*.eps latex

html-online::
    $(SED) -e "s/quantlibfooter.html/quantlibfooteronline.html/" \
           quantlib.doxy > quantlib.doxy.temp
    $(DOXYGEN) quantlib.doxy.temp
    del /Q quantlib.doxy.temp
    copy images\*.jpg html
    copy images\*.png html
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
           -e "54,$s/input{index}/ /" \
           -e "54,$s/include{index}/ /" \
           -e "54,$s/input{install}/ /" \
           -e "54,$s/include{install}/ /" \
           -e "54,$s/input{license}/ /" \
           -e "54,$s/include{license}/ /" \
           -e "54,$s/input{platforms}/ /" \
           -e "54,$s/include{platforms}/ /" \
           -e "54,$s/input{usage}/ /" \
           -e "54,$s/include{usage}/ /" \
           -e "54,$s/input{where}/ /" \
           -e "54,$s/include{where}/ /" \
           -e "54,$s/input{todo}/ /" \
           -e "54,$s/include{todo}/ /" \
           -e "54,$s/input{core}/ /" \
           -e "54,$s/include{core}/ /" \
           -e "54,$s/input{findiff}/ /" \
           -e "54,$s/include{findiff}/ /" \
           -e "54,$s/input{mcarlo}/ /" \
           -e "54,$s/include{mcarlo}/ /" \
           oldrefman.tex > refman.tex
    del oldrefman.tex
    cd ..

# Clean up
clean::
    if exist html  rmdir /S /Q html
    if exist latex rmdir /S /Q latex
