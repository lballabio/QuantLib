
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    aonia.hpp \
    audlibor.hpp \
    bbsw.hpp \
    bibor.hpp \
    bkbm.hpp \
    cadlibor.hpp \
    cdor.hpp \
    chflibor.hpp \
    dkklibor.hpp \
    eonia.hpp \
    euribor.hpp \
    eurlibor.hpp \
    fedfunds.hpp \
    gbplibor.hpp \
    jibar.hpp \
    jpylibor.hpp \
    libor.hpp \
    mosprime.hpp \
    nzdlibor.hpp \
    nzocr.hpp \
    pribor.hpp \
    robor.hpp \
    seklibor.hpp \
    shibor.hpp \
    sofr.hpp \
    sonia.hpp \
    tibor.hpp \
    thbfix.hpp \
    trlibor.hpp \
    usdlibor.hpp \
    wibor.hpp \
    zibor.hpp

cpp_files = \
    bibor.cpp \
    eonia.cpp \
    euribor.cpp \
    eurlibor.cpp \
    fedfunds.cpp \
    libor.cpp \
    shibor.cpp \
    sofr.cpp \
    sonia.cpp


if UNITY_BUILD

nodist_libIborIndexes_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libIborIndexes_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libIborIndexes.la

include ${top_srcdir}/mk/all.mk

