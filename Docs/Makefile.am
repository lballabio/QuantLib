
EXTRA_DIST = README.txt quantlib.doxy footer.html makefile.mak

if DOXYGEN_FOUND
docs:
	$(DOXYGEN) quantlib.doxy
else
docs:
	echo You do not have doxygen installed, so you cannot generate docs, sorry.
endif

