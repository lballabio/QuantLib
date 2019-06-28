
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	analytic_cont_geom_av_price.hpp \
	analytic_discr_geom_av_price.hpp \
	analytic_discr_geom_av_strike.hpp \
	fdblackscholesasianengine.hpp \
	mc_discr_arith_av_price.hpp \
	mc_discr_arith_av_strike.hpp \
	mc_discr_geom_av_price.hpp \
	mcdiscreteasianengine.hpp

cpp_files = \
	analytic_cont_geom_av_price.cpp \
	analytic_discr_geom_av_price.cpp \
	analytic_discr_geom_av_strike.cpp \
	fdblackscholesasianengine.cpp \
	mc_discr_arith_av_price.cpp \
	mc_discr_arith_av_strike.cpp \
	mc_discr_geom_av_price.cpp

if UNITY_BUILD

nodist_libAsianEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libAsianEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libAsianEngines.la

include ${top_srcdir}/mk/all.mk

