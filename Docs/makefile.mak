
# makefile for QuantLib documentation under Borland C++

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
    $(SED) -e "s|ql_basepath|$(QL_DIR)\\\\|" \
           quantlib.doxy > quantlib.doxy.temp
    $(DOXYGEN) quantlib.doxy.temp
    del /Q quantlib.doxy.temp
    copy images\*.jpg html
    copy images\*.png html
    copy images\*.pdf latex
    copy images\*.eps latex

html-online::
    $(SED) -e "s/quantlibfooter.html/quantlibfooteronline.html/" \
           -e "s|ql_basepath|$(QL_DIR)\\\\|" \
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
           -e "/input{group}/d" \
           -e "/include{history}/d" \
           -e "/include{index}/d" \
           -e "/include{install}/d" \
           -e "/include{license}/d" \
           -e "/include{overview}/d" \
           -e "/include{resources}/d" \
           -e "/include{usage}/d" \
           -e "/include{where}/d" \
           -e "s/ple Documentation}/ple Documentation}\\\\label{exchap}/" \
           oldrefman.tex > refman.tex
    del oldrefman.tex
	ren deprecated.tex olddeprecated.tex
	$(SED) -e "s/section/chapter/" olddeprecated.tex > deprecated.tex
	del olddeprecated.tex
	ren bug.tex oldbug.tex
	$(SED) -e "s/section/chapter/" oldbug.tex > bug.tex
	rm -f oldbug.tex
    cd ..

# Clean up
clean::
    if exist html  rmdir /S /Q html
    if exist latex rmdir /S /Q latex
