
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
    copy userman.tex latex
    cd latex
    ren refman.tex oldrefman.tex
    $(SED) -e "/Page Index/d" \
           -e "/input{pages}/d" \
           -e "/Page Documentation/d" \
           -e "s/input{index}/ /" \
           -e "s/include{index}/ /" \
           -e "s/input{overview}/ /" \
           -e "s/include{overview}/ /" \
           -e "s/input{where}/ /" \
           -e "s/include{where}/ /" \
           -e "s/input{install}/ /" \
           -e "s/include{install}/ /" \
           -e "s/input{usage}/ /" \
           -e "s/include{usage}/ /" \
           -e "s/input{platforms}/ /" \
           -e "s/include{platforms}/ /" \
           -e "s/input{history}/ /" \
           -e "s/include{history}/ /" \
           -e "s/input{todo}/ /" \
           -e "s/include{todo}/ /" \
           -e "s/input{resources}/ /" \
           -e "s/include{resources}/ /" \
           -e "s/input{group}/ /" \
           -e "s/include{group}/ /" \
           -e "s/input{license}/ /" \
           -e "s/include{license}/ /" \
           -e "s/input{coreclasses}/ /" \
           -e "s/include{coreclasses}/ /" \
           -e "s/input{findiff}/ /" \
           -e "s/include{findiff}/ /" \
           -e "s/input{mcarlo}/ /" \
           -e "s/include{mcarlo}/ /" \
           -e "s/ple Documentation}/ple Documentation}\\\\label{exchap}/" \
           oldrefman.tex > refman.tex
    del oldrefman.tex
    cd ..

# Clean up
clean::
    if exist html  rmdir /S /Q html
    if exist latex rmdir /S /Q latex
