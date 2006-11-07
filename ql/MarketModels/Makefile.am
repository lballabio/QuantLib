
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels

SUBDIRS = BrownianGenerators Evolvers ExerciseStrategies \
    ExerciseValues Models Products

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
    marketmodelconstrainedevolver.hpp \
    marketmodeldiscounter.hpp \
    marketmodelevolver.hpp \
    marketmodelproduct.hpp \
    marketmodel.hpp \
    swapbasissystem.hpp \
    swapforwardconversionmatrix.hpp \
    utilities.hpp

libMarketModels_la_SOURCES = \
    accountingengine.cpp \
    curvestate.cpp \
    driftcalculator.cpp \
    evolutiondescription.cpp \
    lsdatacollector.cpp \
    marketmodeldiscounter.cpp \
    swapbasissystem.cpp \
    swapforwardconversionmatrix.cpp \
    utilities.cpp


noinst_LTLIBRARIES = libMarketModels.la

libMarketModels_la_LIBADD = \
    BrownianGenerators/libMarketModelsBrownianGenerators.la \
    Evolvers/libMarketModelsEvolvers.la \
    ExerciseValues/libMarketModelsExerciseValues.la \
    ExerciseStrategies/libMarketModelsExerciseStrategies.la \
    Models/libMarketModelsModels.la \
    Products/libMarketModelsProducts.la

EXTRA_DIST = makefile.mak

