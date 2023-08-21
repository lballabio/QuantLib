/* ql/config.hpp.  Generated from config.hpp.in by configure.  */
/* ql/config.hpp.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "QuantLib"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "quantlib-dev@lists.sourceforge.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "QuantLib"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "QuantLib 1.32-dev"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "QuantLib"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.32-dev"

/* Define this if you want to enable the parallel unit test runner. */
/* #undef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER */

/* Define this if you want to enable sessions. */
/* #undef QL_ENABLE_SESSIONS */

/* Define this if you want to enable thread-safe observer pattern. */
/* #undef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN */

/* Define this if tracing messages should allowed (whether they are actually
   emitted will depend on run-time settings.) */
/* #undef QL_ENABLE_TRACING */

/* Define this if error messages should include current function information.
   */
/* #undef QL_ERROR_FUNCTIONS */

/* Define this if error messages should include file and line information. */
/* #undef QL_ERROR_LINES */

/* Define this if extra safety checks should be performed. This can degrade
   performance. */
/* #undef QL_EXTRA_SAFETY_CHECKS */

/* Define this if you want lazy objects to forward fewer notifications. */
#define QL_FASTER_LAZY_OBJECTS 1

/* Define this if you want to enable high resolution date class. */
/* #undef QL_HIGH_RESOLUTION_DATE */

/* Define this if you want to enable the implementation of Null as template
   functions. */
/* #undef QL_NULL_AS_FUNCTIONS */

/* Define if running on a Sun Solaris machine. */
/* #undef QL_PATCH_SOLARIS */

/* Define this if you want to throw an exception when a notification loop is
   detected. */
/* #undef QL_THROW_IN_CYCLES */

/* Define this to use indexed coupons instead of par coupons in floating legs.
   */
/* #undef QL_USE_INDEXED_COUPON */

/* Define this if you want to use std::any. */
/* #undef QL_USE_STD_ANY */

/* Define this if you want to use std::function and std::bind. */
#define QL_USE_STD_FUNCTION 1

/* Define this if you want to use std::optional. */
/* #undef QL_USE_STD_OPTIONAL */

/* Define this if you want to use standard smart pointers. */
/* #undef QL_USE_STD_SHARED_PTR */

/* Define this if you want to use std::tuple. */
#define QL_USE_STD_TUPLE 1

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1.32-dev"
