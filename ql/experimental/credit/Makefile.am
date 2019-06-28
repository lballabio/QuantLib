
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    basecorrelationlossmodel.hpp \
    basecorrelationstructure.hpp \
    basket.hpp \
    binomiallossmodel.hpp \
    blackcdsoptionengine.hpp \
    cdo.hpp \
    cdsoption.hpp \
    constantlosslatentmodel.hpp \
    correlationstructure.hpp \
    defaultevent.hpp \
    defaultlossmodel.hpp \
    defaultprobabilitykey.hpp \
    defaultprobabilitylatentmodel.hpp \
    defaulttype.hpp \
    distribution.hpp \
    factorspreadedhazardratecurve.hpp \
    gaussianlhplossmodel.hpp \
    homogeneouspooldef.hpp \
    inhomogeneouspooldef.hpp \
    integralcdoengine.hpp \
    integralntdengine.hpp \
    interpolatedaffinehazardratecurve.hpp \
    issuer.hpp \
    loss.hpp \
    lossdistribution.hpp \
    midpointcdoengine.hpp \
    nthtodefault.hpp \
    onefactoraffinesurvival.hpp \
    onefactorcopula.hpp \
    onefactorgaussiancopula.hpp \
    onefactorstudentcopula.hpp \
    pool.hpp \
    randomdefaultlatentmodel.hpp \
    randomdefaultmodel.hpp \
    randomlosslatentmodel.hpp \
    recoveryratemodel.hpp \
    recoveryratequote.hpp \
    recursivelossmodel.hpp \
    riskyassetswap.hpp \
    riskyassetswapoption.hpp \
    riskybond.hpp \
    saddlepointlossmodel.hpp \
    spotlosslatentmodel.hpp \
    spreadedhazardratecurve.hpp \
    syntheticcdo.hpp

cpp_files = \
    basecorrelationstructure.cpp \
    basket.cpp \
    blackcdsoptionengine.cpp \
    cdo.cpp \
    cdsoption.cpp \
    correlationstructure.cpp \
    defaultevent.cpp \
    defaultprobabilitykey.cpp \
    defaulttype.cpp \
    distribution.cpp \
    gaussianlhplossmodel.cpp \
    integralcdoengine.cpp \
    integralntdengine.cpp \
    issuer.cpp \
    lossdistribution.cpp \
    midpointcdoengine.cpp \
    nthtodefault.cpp \
    onefactorcopula.cpp \
    onefactorgaussiancopula.cpp \
    onefactorstudentcopula.cpp \
    pool.cpp \
    randomdefaultmodel.cpp \
    recoveryratemodel.cpp \
    recoveryratequote.cpp \
    riskyassetswap.cpp \
    riskyassetswapoption.cpp \
    riskybond.cpp \
    syntheticcdo.cpp

if UNITY_BUILD

nodist_libCredit_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCredit_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCredit.la

include ${top_srcdir}/mk/all.mk

