
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

if AUTO_EXAMPLES
bin_PROGRAMS = Replication
TESTS = Replication$(EXEEXT)
else
noinst_PROGRAMS = Replication
endif
Replication_SOURCES = Replication.cpp
Replication_LDADD = ../../ql/libQuantLib.la ${BOOST_THREAD_LIB}

EXTRA_DIST = \
    CMakeLists.txt \
    Replication.vcxproj \
    Replication.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: Replication$(EXEEXT)

check-examples: examples
	./Replication$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

