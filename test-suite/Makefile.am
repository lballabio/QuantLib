
QL_TESTS = quantlibtestsuite.cpp \
           qltestlistener.hpp qltestlistener.cpp \
           capfloor.hpp capfloor.cpp \
           covariance.hpp covariance.cpp \
           dates.hpp dates.cpp \
           daycounters.hpp daycounters.cpp \
           distributions.hpp distributions.cpp \
           europeanoption.hpp europeanoption.cpp \
           instruments.hpp instruments.cpp \
           integrals.hpp integrals.cpp \
           marketelements.hpp marketelements.cpp \
           operators.hpp operators.cpp \
           piecewiseflatforward.hpp piecewiseflatforward.cpp \
           riskstats.hpp riskstats.cpp \
           solvers.hpp solvers.cpp \
           stats.hpp stats.cpp \
           swap.hpp swap.cpp \
           swaption.hpp swaption.cpp \
           utilities.hpp

if CPPUNIT_FOUND

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} ${CPPUNIT_CFLAGS}
AM_LDFLAGS = ${CPPUNIT_LIBS}
bin_PROGRAMS = quantlib-test-suite
quantlib_test_suite_SOURCES = ${QL_TESTS}
quantlib_test_suite_LDADD = ../ql/libQuantLib.la
TESTS = quantlib-test-suite
EXTRA_DIST = README.txt CPPUNIT-COPYING

else

EXTRA_DIST = ${QL_TESTS} README.txt CPPUNIT-COPYING

endif

