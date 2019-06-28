
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	fdm2dblackscholesop.hpp \
	fdmbatesop.hpp \
	fdmblackscholesop.hpp \
	fdmcevop.hpp \
	fdmg2op.hpp \
	fdmhestonhullwhiteop.hpp \
	fdmhestonop.hpp \
	fdmhullwhiteop.hpp \
	fdmlinearopcomposite.hpp \
	fdmornsteinuhlenbeckop.hpp \
	fdmlinearop.hpp \
	fdmlinearopiterator.hpp \
	fdmlinearoplayout.hpp \
	fdmsabrop.hpp \
	firstderivativeop.hpp \
	ninepointlinearop.hpp \
	nthorderderivativeop.hpp \
	numericaldifferentiation.hpp \
	secondderivativeop.hpp \
	secondordermixedderivativeop.hpp \
	triplebandlinearop.hpp

cpp_files = \
	fdm2dblackscholesop.cpp \
	fdmbatesop.cpp \
	fdmblackscholesop.cpp \
	fdmcevop.cpp \
	fdmg2op.cpp \
	fdmhestonhullwhiteop.cpp \
	fdmhestonop.cpp \
	fdmhullwhiteop.cpp \
	fdmlinearoplayout.cpp \
	fdmornsteinuhlenbeckop.cpp \
	fdmsabrop.cpp \
	firstderivativeop.cpp \
	ninepointlinearop.cpp \
	nthorderderivativeop.cpp \
	numericaldifferentiation.cpp \
	secondderivativeop.cpp \
	secondordermixedderivativeop.cpp \
	triplebandlinearop.cpp

if UNITY_BUILD

nodist_libFdmOperators_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFdmOperators_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libFdmOperators.la

include ${top_srcdir}/mk/all.mk

