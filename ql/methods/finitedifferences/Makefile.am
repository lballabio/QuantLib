
SUBDIRS = meshers operators schemes solvers stepconditions utilities

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	americancondition.hpp \
	boundarycondition.hpp \
	bsmoperator.hpp \
	bsmtermoperator.hpp \
	cranknicolson.hpp \
	dminus.hpp \
	dplus.hpp \
	dplusdminus.hpp \
	dzero.hpp \
	expliciteuler.hpp \
	fdtypedefs.hpp \
	finitedifferencemodel.hpp \
	impliciteuler.hpp \
	mixedscheme.hpp\
	onefactoroperator.hpp \
	operatorfactory.hpp \
	operatortraits.hpp \
	parallelevolver.hpp \
	pde.hpp \
	pdebsm.hpp \
	pdeshortrate.hpp \
	shoutcondition.hpp \
	stepcondition.hpp \
	trbdf2.hpp \
	tridiagonaloperator.hpp \
	zerocondition.hpp

cpp_files = \
    boundarycondition.cpp \
    bsmoperator.cpp \
    tridiagonaloperator.cpp

if UNITY_BUILD

nodist_libFiniteDifferences_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFiniteDifferences_la_SOURCES = $(cpp_files)

endif

libFiniteDifferences_la_LIBADD = \
    meshers/libFdmMeshers.la \
    operators/libFdmOperators.la \
    schemes/libFdmSchemes.la \
	solvers/libFdmSolvers.la \
	stepconditions/libFdmStepConditions.la \
	utilities/libFdmUtils.la

noinst_LTLIBRARIES = libFiniteDifferences.la

include ${top_srcdir}/mk/all.mk

