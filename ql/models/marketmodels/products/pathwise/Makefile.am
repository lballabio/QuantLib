
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    pathwiseproductcallspecified.hpp \
    pathwiseproductcaplet.hpp \
    pathwiseproductcashrebate.hpp \
    pathwiseproductinversefloater.hpp \
    pathwiseproductswap.hpp \
    pathwiseproductswaption.hpp

cpp_files = \
    pathwiseproductcallspecified.cpp \
    pathwiseproductcaplet.cpp \
    pathwiseproductcashrebate.cpp \
    pathwiseproductinversefloater.cpp \
    pathwiseproductswap.cpp \
    pathwiseproductswaption.cpp

if UNITY_BUILD

nodist_libMarketModelsProductsPathwise_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsProductsPathwise_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsProductsPathwise.la

include ${top_srcdir}/mk/all.mk

