
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Asian

include_HEADERS = \
	all.hpp \
	analytic_cont_geom_av_price.hpp \
	analytic_discr_geom_av_price.hpp \
	mc_discr_arith_av_price.hpp \
	mc_discr_geom_av_price.hpp \
	mcdiscreteasianengine.hpp

EXTRA_DIST=makefile.mak

libAsianEngines_la_SOURCES = \
	analytic_cont_geom_av_price.cpp \
	analytic_discr_geom_av_price.cpp \
	mc_discr_arith_av_price.cpp \
	mc_discr_geom_av_price.cpp

noinst_LTLIBRARIES = libAsianEngines.la

