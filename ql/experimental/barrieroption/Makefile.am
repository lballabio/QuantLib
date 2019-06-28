
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    analyticdoublebarrierbinaryengine.hpp \
	analyticdoublebarrierengine.hpp \
    binomialdoublebarrierengine.hpp \
    discretizeddoublebarrieroption.hpp \
    doublebarrieroption.hpp \
    doublebarriertype.hpp \
    perturbativebarrieroptionengine.hpp \
    quantodoublebarrieroption.hpp \
    vannavolgabarrierengine.hpp \
    vannavolgadoublebarrierengine.hpp \
    vannavolgainterpolation.hpp \
    wulinyongdoublebarrierengine.hpp

cpp_files = \
    analyticdoublebarrierbinaryengine.cpp \
	analyticdoublebarrierengine.cpp \
    discretizeddoublebarrieroption.cpp \
    doublebarrieroption.cpp \
    doublebarriertype.cpp \
    perturbativebarrieroptionengine.cpp \
    quantodoublebarrieroption.cpp \
    vannavolgabarrierengine.cpp \
    wulinyongdoublebarrierengine.cpp

if UNITY_BUILD

nodist_libBarrierOption_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libBarrierOption_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libBarrierOption.la

include ${top_srcdir}/mk/all.mk

