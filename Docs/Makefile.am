
EXTRA_DIST = README.txt quantlib.doxy footer.html makefile.mak

docs-html:
	$(DOXYGEN) quantlib.doxy
docs-ps: docs-html
	$(MAKE) -C latex ps
docs-pdf: docs-html
	$(MAKE) -C latex pdf
docs-man: docs-html

