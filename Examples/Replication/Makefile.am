
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

if AUTO_EXAMPLES
bin_PROGRAMS = Replication
TESTS = Replication$(EXEEXT)
else
noinst_PROGRAMS = Replication
endif
Replication_SOURCES = Replication.cpp
Replication_LDADD = ../../ql/libQuantLib.la

EXTRA_DIST = \
    Replication.dev \
    Replication_vc8.vcproj \
    Replication_vc9.vcproj \
    Replication_vc10.vcxproj \
    Replication_vc10.vcxproj.filters \
    Replication_vc11.vcxproj \
    Replication_vc11.vcxproj.filters \
    ReadMe.txt

.PHONY: examples check-examples

examples: Replication$(EXEEXT)

check-examples: examples
	./Replication$(EXEEXT)

dist-hook:
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/build

