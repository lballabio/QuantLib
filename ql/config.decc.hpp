/*
*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:54:20  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.5  2001/08/30 17:03:08  nando
// clock substitued by time
//
// Revision 1.4  2001/08/30 16:57:46  nando
// clock substitued by time
//
// Revision 1.3  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.2  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//


#ifndef quantlib_config_decc_h
#define quantlib_config_decc_h

#define HAVE_MATH_H
#define QL_SQRT   sqrt
#define QL_FABS   fabs
#define QL_EXP    exp
#define QL_LOG    log
#define QL_SIN    sin
#define QL_COS    cos
#define QL_POW    pow
#define QL_MODF   modf

#define HAVE_LIMITS

#define HAVE_TIME_H
#define QL_CLOCK  clock
#define QL_TIME   time

#define HAVE_CTYPE_H
#define QL_STRLEN   strlen
#define QL_TOLOWER  tolower
#define QL_TOUPPER  toupper

#define QL_ITERATOR         std::iterator
#define QL_ITERATOR_TRAITS  std::iterator_traits


#endif
