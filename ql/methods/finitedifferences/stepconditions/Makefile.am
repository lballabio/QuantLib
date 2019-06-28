
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	fdmamericanstepcondition.hpp \
	fdmarithmeticaveragecondition.hpp \
	fdmbermudanstepcondition.hpp \
	fdmsimplestoragecondition.hpp \
	fdmsimpleswingcondition.hpp \
	fdmsnapshotcondition.hpp \
	fdmstepconditioncomposite.hpp

cpp_files = \
	fdmamericanstepcondition.cpp \
	fdmarithmeticaveragecondition.cpp \
	fdmbermudanstepcondition.cpp \
	fdmsimplestoragecondition.cpp \
	fdmsimpleswingcondition.cpp \
	fdmsnapshotcondition.cpp \
	fdmstepconditioncomposite.cpp

if UNITY_BUILD

nodist_libFdmStepConditions_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFdmStepConditions_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libFdmStepConditions.la

include ${top_srcdir}/mk/all.mk

