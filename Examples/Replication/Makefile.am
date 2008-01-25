
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = Replication
TESTS = Replication
else
EXTRA_PROGRAMS = Replication
CLEANFILES = Replication
endif
Replication_SOURCES = Replication.cpp
Replication_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    Replication.dev \
    Replication_vc7.vcproj \
    Replication_vc8.vcproj \
    ReadMe.txt

.PHONY: examples check-examples

examples: Replication$(EXEEXT)

check-examples: examples
	./Replication$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

