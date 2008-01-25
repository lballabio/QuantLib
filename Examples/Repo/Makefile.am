
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = Repo
TESTS = Repo
else
EXTRA_PROGRAMS = Repo
CLEANFILES = Repo
endif
Repo_SOURCES = Repo.cpp
Repo_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    Repo.dev \
    Repo_vc7.vcproj \
    Repo_vc8.vcproj \
    ReadMe.txt

.PHONY: examples check-examples

examples: Repo$(EXEEXT)

check-examples: examples
	./Repo$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

