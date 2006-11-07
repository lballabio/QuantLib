
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/ExerciseValues

include_HEADERS = \
	all.hpp \
	bermudanswaptionexercisevalue.hpp \
	nothingexercisevalue.hpp

libMarketModelsExerciseValues_la_SOURCES = \
	bermudanswaptionexercisevalue.cpp \
	nothingexercisevalue.cpp

noinst_LTLIBRARIES = libMarketModelsExerciseValues.la

EXTRA_DIST = makefile.mak

