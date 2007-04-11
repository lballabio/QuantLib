
SUBDIRS = browniangenerators curvestates driftcomputation evolvers \
          exercisestrategies exercisevalues models products

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    core.hpp \
    accountingengine.hpp \
    browniangenerator.hpp \
    curvestate.hpp \
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
    nodedataprovider.hpp \
    parametricexercise.hpp \
    parametricexerciseadapter.hpp \
    parametricswapexercise.hpp \
    proxygreekengine.hpp \
    swapbasissystem.hpp \
    swapforwardconversionmatrix.hpp \
    swapforwardmappings.hpp \
    upperboundengine.hpp \
    utilities.hpp

libMarketModels_la_SOURCES = \
    accountingengine.cpp \
    curvestate.cpp \
    evolutiondescription.cpp \
    lsdatacollector.cpp \
    marketmodeldiscounter.cpp \
    marketmodel.cpp \
    parametricexerciseadapter.cpp \
    parametricswapexercise.cpp \
    proxygreekengine.cpp \
    swapbasissystem.cpp \
    swapforwardconversionmatrix.cpp \
    swapforwardmappings.cpp \
    upperboundengine.cpp \
    utilities.cpp

noinst_LTLIBRARIES = libMarketModels.la

libMarketModels_la_LIBADD = \
    browniangenerators/libMarketModelsBrownianGenerators.la \
    curvestates/libMarketModelsCurveStates.la \
    driftcomputation/libMarketModelsDriftComputation.la \
    evolvers/libMarketModelsEvolvers.la \
    exercisevalues/libMarketModelsExerciseValues.la \
    exercisestrategies/libMarketModelsExerciseStrategies.la \
    models/libMarketModelsModels.la \
    products/libMarketModelsProducts.la


