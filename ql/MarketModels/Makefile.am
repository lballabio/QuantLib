
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels

SUBDIRS = Products Models BrownianGenerators Evolvers ExerciseValues

include_HEADERS = \
    all.hpp \
    core.hpp \
    accountingengine.hpp \
    browniangenerator.hpp \
    curvestate.hpp \
    driftcalculator.hpp \
    duffsdeviceinnerproduct.hpp \
    evolutiondescription.hpp \
    exercisevalue.hpp \
    lsbasisfunctions.hpp \
    lsdatacollector.hpp \
    lsstrategy.hpp \
    marketmodelevolver.hpp \
    marketmodelproduct.hpp \
    marketmodel.hpp \
    swapforwardconversionmatrix.hpp \
    swapratetrigger.hpp \
    utilities.hpp

libMarketModels_la_SOURCES = \
    accountingengine.cpp \
    curvestate.cpp \
    driftcalculator.cpp \
    evolutiondescription.cpp \
    lsdatacollector.cpp \
    lsstrategy.cpp \
    swapforwardconversionmatrix.cpp \
    swapratetrigger.cpp \
    utilities.cpp


noinst_LTLIBRARIES = libMarketModels.la

libMarketModels_la_LIBADD = \
    BrownianGenerators/libMarketModelsBrownianGenerators.la \
    Evolvers/libMarketModelsEvolvers.la \
    ExerciseValues/libMarketModelsExerciseValues.la \
    Products/libMarketModelsProducts.la \
    Models/libMarketModelsModels.la

#EXTRA_DIST = makefile.mak

