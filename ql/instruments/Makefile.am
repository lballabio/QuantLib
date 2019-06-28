
SUBDIRS = bonds

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    asianoption.hpp \
    assetswap.hpp \
    averagetype.hpp \
    barrieroption.hpp \
    barriertype.hpp \
    basketoption.hpp \
    bmaswap.hpp \
    bond.hpp \
    callabilityschedule.hpp \
    capfloor.hpp \
    claim.hpp \
    cliquetoption.hpp \
    compositeinstrument.hpp \
    cpiswap.hpp \
    cpicapfloor.hpp \
    creditdefaultswap.hpp \
	dividendbarrieroption.hpp \
    dividendschedule.hpp \
    dividendvanillaoption.hpp \
    europeanoption.hpp \
    fixedratebondforward.hpp \
    floatfloatswap.hpp \
    floatfloatswaption.hpp \
    forward.hpp \
    forwardrateagreement.hpp \
    forwardvanillaoption.hpp \
    futures.hpp \
    impliedvolatility.hpp \
    inflationcapfloor.hpp \
    lookbackoption.hpp \
    makecapfloor.hpp \
    makecds.hpp \
    makecms.hpp \
    makeois.hpp \
    makeswaption.hpp \
    makevanillaswap.hpp \
    makeyoyinflationcapfloor.hpp \
    multiassetoption.hpp \
    nonstandardswap.hpp \
    nonstandardswaption.hpp \
    oneassetoption.hpp \
    overnightindexedswap.hpp \
    payoffs.hpp \
    quantobarrieroption.hpp \
    quantoforwardvanillaoption.hpp \
    quantovanillaoption.hpp \
    stickyratchet.hpp \
    stock.hpp \
    swap.hpp \
    swaption.hpp \
    vanillaoption.hpp \
	vanillastorageoption.hpp \
	vanillaswingoption.hpp \
    vanillaswap.hpp \
    varianceswap.hpp \
    yearonyearinflationswap.hpp \
    zerocouponinflationswap.hpp

cpp_files = \
    asianoption.cpp \
    assetswap.cpp \
    averagetype.cpp \
    barrieroption.cpp \
    barriertype.cpp \
    basketoption.cpp \
    bmaswap.cpp \
    bond.cpp \
    capfloor.cpp \
    claim.cpp \
    cliquetoption.cpp \
    compositeinstrument.cpp \
    cpiswap.cpp \
    cpicapfloor.cpp \
    creditdefaultswap.cpp \
	dividendbarrieroption.cpp \
    dividendvanillaoption.cpp \
    europeanoption.cpp \
    fixedratebondforward.cpp \
    floatfloatswap.cpp \
    floatfloatswaption.cpp \
    forward.cpp \
    forwardrateagreement.cpp \
    forwardvanillaoption.cpp \
    futures.cpp \
    impliedvolatility.cpp \
    inflationcapfloor.cpp \
    lookbackoption.cpp \
    makecapfloor.cpp \
    makecds.cpp \
    makecms.cpp \
    makeois.cpp \
    makeswaption.cpp \
    makevanillaswap.cpp \
    makeyoyinflationcapfloor.cpp \
    multiassetoption.cpp \
    nonstandardswap.cpp \
    nonstandardswaption.cpp \
    oneassetoption.cpp \
    overnightindexedswap.cpp \
    payoffs.cpp \
    quantobarrieroption.cpp \
    quantoforwardvanillaoption.cpp \
    quantovanillaoption.cpp \
    stickyratchet.cpp \
    stock.cpp \
    swap.cpp \
    swaption.cpp \
    vanillaoption.cpp \
    vanillaswap.cpp \
    vanillaswingoption.cpp \
    varianceswap.cpp \
    yearonyearinflationswap.cpp \
    zerocouponinflationswap.cpp

if UNITY_BUILD

nodist_libInstruments_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libInstruments_la_SOURCES = $(cpp_files)

endif

libInstruments_la_LIBADD = \
    bonds/libBonds.la

noinst_LTLIBRARIES = libInstruments.la

include ${top_srcdir}/mk/all.mk

