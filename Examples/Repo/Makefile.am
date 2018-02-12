
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = Repo
TESTS = Repo$(EXEEXT)
else
noinst_PROGRAMS = Repo
endif
Repo_SOURCES = Repo.cpp
Repo_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    Repo_vc9.vcproj \
    Repo.vcxproj \
    Repo.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: Repo$(EXEEXT)

check-examples: examples
	./Repo$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

