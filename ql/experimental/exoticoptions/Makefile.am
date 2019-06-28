
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    analyticamericanmargrabeengine.hpp \
    analyticcomplexchooserengine.hpp \
    analyticcompoundoptionengine.hpp \
    analyticeuropeanmargrabeengine.hpp \
    analyticholderextensibleoptionengine.hpp \
    analyticpartialtimebarrieroptionengine.hpp \
    analyticpdfhestonengine.hpp \
    analyticsimplechooserengine.hpp \
    analytictwoassetbarrierengine.hpp \
    analytictwoassetcorrelationengine.hpp \
    analyticwriterextensibleoptionengine.hpp \
    complexchooseroption.hpp \
    compoundoption.hpp \
    continuousarithmeticasianlevyengine.hpp \
    continuousarithmeticasianvecerengine.hpp \
    everestoption.hpp \
    himalayaoption.hpp \
    holderextensibleoption.hpp \
    kirkspreadoptionengine.hpp \
    margrabeoption.hpp \
    mceverestengine.hpp \
    mchimalayaengine.hpp \
    mcpagodaengine.hpp \
    pagodaoption.hpp \
    partialtimebarrieroption.hpp \
    simplechooseroption.hpp \
    spreadoption.hpp \
    twoassetbarrieroption.hpp \
    twoassetcorrelationoption.hpp \
    writerextensibleoption.hpp

cpp_files = \
    analyticamericanmargrabeengine.cpp \
    analyticcomplexchooserengine.cpp \
    analyticcompoundoptionengine.cpp \
    analyticeuropeanmargrabeengine.cpp \
    analyticholderextensibleoptionengine.cpp \
    analyticpartialtimebarrieroptionengine.cpp \
    analyticpdfhestonengine.cpp \
    analyticsimplechooserengine.cpp \
    analytictwoassetbarrierengine.cpp \
    analytictwoassetcorrelationengine.cpp \
    analyticwriterextensibleoptionengine.cpp \
    complexchooseroption.cpp \
    compoundoption.cpp \
    continuousarithmeticasianlevyengine.cpp \
    continuousarithmeticasianvecerengine.cpp \
    everestoption.cpp \
    himalayaoption.cpp \
    holderextensibleoption.cpp \
    kirkspreadoptionengine.cpp \
    margrabeoption.cpp \
    mceverestengine.cpp \
    mchimalayaengine.cpp \
    mcpagodaengine.cpp \
    pagodaoption.cpp \
    partialtimebarrieroption.cpp \
    simplechooseroption.cpp \
    twoassetbarrieroption.cpp \
    twoassetcorrelationoption.cpp \
    writerextensibleoption.cpp

if UNITY_BUILD

nodist_libExoticOptions_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libExoticOptions_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libExoticOptions.la

include ${top_srcdir}/mk/all.mk

