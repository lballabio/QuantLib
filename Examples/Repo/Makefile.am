
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = Repo
TESTS = Repo$(EXEEXT)
else
noinst_PROGRAMS = Repo
endif
Repo_SOURCES = Repo.cpp
Repo_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    Repo.dev \
    Repo_vc8.vcproj \
    Repo_vc9.vcproj \
    Repo_vc10.vcxproj \
    Repo_vc10.vcxproj.filters \
    Repo_vc11.vcxproj \
    Repo_vc11.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: Repo$(EXEEXT)

check-examples: examples
	./Repo$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

