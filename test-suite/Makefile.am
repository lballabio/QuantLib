
QL_TESTS = \
           americanoption.hpp americanoption.cpp \
           asianoptions.hpp asianoptions.cpp \
           barrieroption.hpp barrieroption.cpp \
           basketoption.hpp basketoption.cpp \
           calendars.hpp calendars.cpp \
           capfloor.hpp capfloor.cpp \
           compoundforward.hpp compoundforward.cpp \
           covariance.hpp covariance.cpp \
           dates.hpp dates.cpp \
           daycounters.hpp daycounters.cpp \
           digitaloption.hpp digitaloption.cpp \
           distributions.hpp distributions.cpp \
           europeanoption.hpp europeanoption.cpp \
           factorial.hpp factorial.cpp \
           instruments.hpp instruments.cpp \
           integrals.hpp integrals.cpp \
           interpolations.hpp interpolations.cpp \
           jumpdiffusion.hpp jumpdiffusion.cpp \
           lowdiscrepancysequences.hpp lowdiscrepancysequences.cpp \
           marketelements.hpp marketelements.cpp \
           matrices.hpp matrices.cpp \
           mersennetwister.hpp mersennetwister.cpp \
           old_pricers.hpp old_pricers.cpp \
           operators.hpp operators.cpp \
           piecewiseflatforward.hpp piecewiseflatforward.cpp \
           qltestlistener.hpp qltestlistener.cpp \
           quantlibtestsuite.cpp \
           riskstats.hpp riskstats.cpp \
           solvers.hpp solvers.cpp \
           stats.hpp stats.cpp \
           swap.hpp swap.cpp \
           swaption.hpp swaption.cpp \
           termstructures.hpp termstructures.cpp \
           utilities.hpp utilities.cpp

if CPPUNIT_FOUND

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir} ${CPPUNIT_CFLAGS}
AM_LDFLAGS = ${CPPUNIT_LIBS}
bin_PROGRAMS = quantlib-test-suite
quantlib_test_suite_SOURCES = ${QL_TESTS}
quantlib_test_suite_LDADD = ../ql/libQuantLib.la
TESTS = quantlib-test-suite
EXTRA_DIST = README.txt CPPUNIT-COPYING testsuite.dsp makefile.mak

else

EXTRA_DIST = \
    ${QL_TESTS} \
    README.txt \
    CPPUNIT-COPYING \
    testsuite.dsp \
    makefile.mak

endif

