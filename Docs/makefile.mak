
# makefile for QuantLib documentation under Borland C++

.autodepend
#.silent

MAKE = $(MAKE) 

# Tools to be used
SED       = sed
!ifdef NODOXYGEN
    DOXYGEN   = type
!else
    DOXYGEN   = doxygen
!endif
LATEX     = latex
PDFLATEX  = pdflatex
MAKEINDEX = makeindex
DVIPS     = dvips

# Options
TEX_OPTS = --quiet --pool-size=2000000

# Primary target:
# all docs
all:: htmlhelp-config tex-config tex-files
    cd latex
    $(PDFLATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(PDFLATEX) $(TEX_OPTS) refman
    $(LATEX) $(TEX_OPTS) refman
    $(DVIPS) refman
    cd ..
    copy latex\refman.pdf QuantLib-docs-$(VERSION).pdf
    copy latex\refman.ps  QuantLib-docs-$(VERSION).ps

generic-config::
    $(SED) -e "s|ql_basepath|D:/Projects/QuantLib|" \
           -e "s|ql_version|$(VERSION)|" \
           quantlib.doxy > quantlib.doxy.temp

html-config:: generic-config
    $(SED) -e "s|GENERATE_HTML          = NO|GENERATE_HTML          = YES|" \
           quantlib.doxy.temp > quantlib.doxy.temp2
    del /Q quantlib.doxy.temp
    ren quantlib.doxy.temp2 quantlib.doxy.temp
    if not exist .\html md .\html
    copy images\*.jpg html
    copy images\*.png html

htmlhelp-config:: html-config
    $(SED) -e "s|GENERATE_HTMLHELP      = NO|GENERATE_HTMLHELP      = YES|" \
           quantlib.doxy.temp > quantlib.doxy.temp2
    del /Q quantlib.doxy.temp
    ren quantlib.doxy.temp2 quantlib.doxy.temp

html-online-config:: generic-config
    $(SED) -e "s/_OUTPUT            = html/_OUTPUT            = html-online/" \
           -e "s/quantlibfooter.html/quantlibfooteronline.html/" \
           -e "s|GENERATE_HTML          = NO|GENERATE_HTML          = YES|" \
           quantlib.doxy.temp > quantlib.doxy.temp2
    del /Q quantlib.doxy.temp
    ren quantlib.doxy.temp2 quantlib.doxy.temp
    if not exist .\html-online md .\html-online
    copy images\*.jpg html-online
    copy images\*.png html-online

tex-config:: generic-config
    $(SED) -e "s|GENERATE_LATEX         = NO|GENERATE_LATEX         = YES|" \
           quantlib.doxy.temp > quantlib.doxy.temp2
    del /Q quantlib.doxy.temp
    ren quantlib.doxy.temp2 quantlib.doxy.temp
    if not exist .\latex md .\latex
	copy qlintro.tex latex
    copy images\*.pdf latex
    copy images\*.eps latex

html:: html-config
    $(DOXYGEN) quantlib.doxy.temp
    del /Q quantlib.doxy.temp

htmlhelp:: htmlhelp-config
    $(DOXYGEN) quantlib.doxy.temp
    del /Q quantlib.doxy.temp

html-online:: html-online-config
    $(DOXYGEN) quantlib.doxy.temp
    del /Q quantlib.doxy.temp

# Correct LaTeX files to get the right layout
tex-files:: tex-config
    $(DOXYGEN) quantlib.doxy.temp
    del /Q quantlib.doxy.temp
    cd latex
    ren refman.tex oldrefman.tex
    $(SED) -e "/Page Index/d" \
           -e "/input{pages}/d" \
           -e "/Page Documentation/d" \
           -e "/include{config}/d" \
           -e "/input{faq}/d" \
           -e "/include{group}/d" \
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
	del oldbug.tex

	ren caveats.tex oldcaveats.tex
	$(SED) -e "s/section/chapter/" oldcaveats.tex > caveats.tex
	del oldcaveats.tex

	ren test.tex oldtest.tex
	$(SED) -e "s/section/chapter/" oldtest.tex > test.tex
	del oldtest.tex

	ren todo.tex oldtodo.tex
	$(SED) -e "s/section/chapter/" oldtodo.tex > todo.tex
	del oldtodo.tex

    cd ..

# PDF documentation
pdf:: tex-files
    cd latex
    $(PDFLATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(PDFLATEX) $(TEX_OPTS) refman
    cd ..
    copy latex\refman.pdf QuantLib-docs-$(VERSION).pdf

# PostScript documentation
ps:: tex-files
    cd latex
    $(LATEX) $(TEX_OPTS) refman
    $(MAKEINDEX) refman.idx
    $(LATEX) $(TEX_OPTS) refman
    $(DVIPS) refman
    cd ..
    copy latex\refman.ps QuantLib-docs-$(VERSION).ps


# Clean up
clean::
    if exist html            rmdir /S /Q html
    if exist html-online     rmdir /S /Q html-online
    if exist latex           rmdir /S /Q latex
    if exist QuantLib-docs-* rm -f QuantLib-docs-*
    if exist *.temp          rm -f *.temp
    if exist *.temp2         rm -f *.temp2
