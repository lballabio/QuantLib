/*
*/

// $Id$
// $Log$
// Revision 1.3  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.2  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//


#ifndef quantlib_config_mwcw_h
#define quantlib_config_mwcw_h

#define REQUIRES_DUMMY_RETURN

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

#define HAVE_CCTYPE
#define QL_STRLEN   std::strlen
#define QL_TOLOWER  std::tolower
#define QL_TOUPPER  std::toupper

#define QL_MIN      std::min
#define QL_MAX      std::max

#define HAVE_EXPRESSION_TEMPLATES
#define HAVE_TEMPLATE_METAPROGRAMMING

#define QL_ITERATOR         std::iterator
#define QL_ITERATOR_TRAITS  std::iterator_traits


#endif
