
DOXYGEN = doxygen
LATEX = latex
PDFLATEX = pdflatex
MAKEINDEX	= makeindex
DVIPS		= dvips
DOXCFGDIR = ../Docs


alldocs:
	$(DOXYGEN) $(DOXCFGDIR)/doxygen_linux.cfg
	rm -Rf ../Docs/html
	mv html ../Docs
#	$(PDFLATEX) latex/refman
#	$(MAKEINDEX) latex/refman.idx
#	$(PDFLATEX) latex/refman
#	$(LATEX) latex/refman
#	$(MAKEINDEX) latex/refman.idx
#	$(LATEX) latex/refman
#	$(DVIPS) latex/refman
