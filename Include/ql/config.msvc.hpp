/*
*/

// $Id$
// $Log$
// Revision 1.13  2001/08/30 17:03:08  nando
// clock substitued by time
//
// Revision 1.12  2001/08/30 16:51:08  nando
// clock substitued by time
//
// Revision 1.11  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.10  2001/07/06 18:24:17  nando
// slight modifications to avoid VisualStudio warnings
//
// Revision 1.9  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.8  2001/06/01 16:50:16  lballabio
// Term structure on deposits and swaps
//
// Revision 1.7  2001/05/31 14:48:10  lballabio
// Worked around Visual C++ deficiencies
//
// Revision 1.6  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//


#ifndef quantlib_config_msvc_h
#define quantlib_config_msvc_h

// disable useless warning
// 'identifier' : decorated name length exceeded,
//                name was truncated in debug info
#pragma warning(disable: 4786)

#define REQUIRES_DUMMY_RETURN

#define HAVE_CMATH
#define QL_SQRT   sqrt
#define QL_FABS   fabs
#define QL_EXP    exp
#define QL_LOG    log
#define QL_SIN    sin
#define QL_COS    cos
#define QL_POW    pow
#define QL_MODF   modf

#define HAVE_LIMITS

#define HAVE_CTIME
#define QL_CLOCK    clock
#define QL_TIME     time

#define HAVE_CCTYPE
#define QL_STRLEN   strlen
#define QL_TOLOWER  tolower
#define QL_TOUPPER  toupper

#define QL_MIN      std::_cpp_min
#define QL_MAX      std::_cpp_max

#define BROKEN_TEMPLATE_SPECIALIZATION
#define BROKEN_TEMPLATE_METHOD_CALLS
#define HAVE_EXPRESSION_TEMPLATES
#define HAVE_TEMPLATE_METAPROGRAMMING

#define HAVE_INCOMPLETE_ITERATOR_SUPPORT
#define GARBLED_REVERSE_ITERATOR

#define QL_PATCH_MICROSOFT_BUGS


#endif
