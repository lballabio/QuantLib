
// $Id$

#ifndef quantlib_config_bcc_h
#define quantlib_config_bcc_h

#define HAVE_CSTDLIB
#define QL_SIZE_T std::size_t

#define HAVE_CMATH
#define QL_SQRT   std::sqrt
#define QL_FABS   std::fabs
#define QL_EXP    std::exp
#define QL_LOG    std::log
#define QL_SIN    std::sin
#define QL_COS    std::cos
#define QL_POW    std::pow
#define QL_MODF   std::modf

#define HAVE_LIMITS

#define HAVE_CTIME
#define QL_CLOCK    std::clock
#define QL_TIME     std::time

#define HAVE_CCTYPE
#define QL_STRLEN   std::strlen
#define QL_TOLOWER  std::tolower
#define QL_TOUPPER  std::toupper

#define HAVE_CSTDIO
#define QL_SPRINTF  std::sprintf

#define QL_MIN      std::min
#define QL_MAX      std::max

#define HAVE_EXPRESSION_TEMPLATES

#define QL_ITERATOR         std::iterator
#define QL_ITERATOR_TRAITS  std::iterator_traits


#endif
