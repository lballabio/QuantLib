
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_defines_h
#define quantlib_defines_h

// global trace level (may be superseded locally by a greater value)

#define QL_TRACE_LEVEL 0

// Compiler-dependent switches

#if defined(_MSC_VER)					// Microsoft Visual C++
	// disable useless warnings
	#pragma warning(disable: 4786)  // identifier truncated in debug info
	// set switches
	#define QL_CMATH_IN_STD						0
	#define QL_CCHAR_IN_STD						0
	#define QL_EXPRESSION_TEMPLATES_WORK		1
	#define QL_TEMPLATE_METAPROGRAMMING_WORKS	1
	#define QL_REQUIRES_DUMMY_RETURN			1
	#define QL_GARBLED_MIN_AND_MAX				1
	#define QL_GARBLED_REVERSE_ITERATORS		1
	#define QL_GARBLED_PTR_CONST				1

#elif defined(__BORLANDC__)				// Borland C++
	// set switches
	#define QL_CMATH_IN_STD						1
	#define QL_CCHAR_IN_STD						1
	#define QL_TEMPLATE_METAPROGRAMMING_WORKS	0
	#define QL_EXPRESSION_TEMPLATES_WORK		0
	#define QL_REQUIRES_DUMMY_RETURN			0
	#define QL_GARBLED_MIN_AND_MAX				0
	#define QL_GARBLED_REVERSE_ITERATORS		0
	#define QL_GARBLED_PTR_CONST				0

#elif defined(__MWERKS__)					// Metrowerks CodeWarrior
	// set switches
	#define QL_CMATH_IN_STD						1
	#define QL_CCHAR_IN_STD						1
	#define QL_TEMPLATE_METAPROGRAMMING_WORKS	1
	#define QL_EXPRESSION_TEMPLATES_WORK		0
	#define QL_REQUIRES_DUMMY_RETURN			1
	#define QL_GARBLED_MIN_AND_MAX				0
	#define QL_GARBLED_REVERSE_ITERATORS		0
	#define QL_GARBLED_PTR_CONST				0

#else										// Generic ANSI C++ compliant compiler
	// set switches
	#define QL_CMATH_IN_STD						1
	#define QL_CCHAR_IN_STD						1
	#define QL_TEMPLATE_METAPROGRAMMING_WORKS	1
	#define QL_EXPRESSION_TEMPLATES_WORK		1
	#define QL_REQUIRES_DUMMY_RETURN			0
	#define QL_GARBLED_MIN_AND_MAX				0
	#define QL_GARBLED_REVERSE_ITERATORS		0
	#define QL_GARBLED_PTR_CONST				0

#endif


// Switch-dependent #definitions

// Are math functions defined in the std namespace?
#if QL_CMATH_IN_STD
	#define QL_SQRT	std::sqrt
	#define QL_FABS	std::fabs
	#define QL_EXP	std::exp
	#define QL_LOG	std::log
	#define QL_SIN	std::sin
	#define QL_COS	std::cos
	#define QL_POW	std::pow
	#define QL_MODF	std::modf
#else
	#define QL_SQRT	sqrt
	#define QL_FABS	fabs
	#define QL_EXP	exp
	#define QL_LOG	log
	#define QL_SIN	sin
	#define QL_COS	cos
	#define QL_POW	pow
	#define QL_MODF	modf
#endif

// Are string functions defined in the std namespace?
#if QL_CCHAR_IN_STD
	#define QL_STRLEN	std::strlen
	#define QL_TOLOWER	std::tolower
	#define QL_TOUPPER	std::toupper
#else
	#define QL_STRLEN	strlen
	#define QL_TOLOWER	tolower
	#define QL_TOUPPER	toupper
#endif

// Is a return statement required at the end of a function even though it can never be reached?
#if QL_REQUIRES_DUMMY_RETURN
	#define QL_DUMMY_RETURN(x)		return x;
#else
	#define QL_DUMMY_RETURN(x) 
#endif

// Do std::min and std::max have the names you would expect? Blame Microsoft for this one
#if QL_GARBLED_MIN_AND_MAX
	#define QL_MIN	std::_cpp_min
	#define QL_MAX	std::_cpp_max
#else
	#define QL_MIN	std::min
	#define QL_MAX	std::max
#endif

// Does std::reverse_iterator need an extra template argument? Blame Microsoft for this one too
#if QL_GARBLED_REVERSE_ITERATORS
	#define QL_REVERSE_ITERATOR(iterator,type)	std::reverse_iterator<iterator,type>
#else
	#define QL_REVERSE_ITERATOR(iterator,type)	std::reverse_iterator<iterator>
#endif

// Does * const have the syntax you would expect? Blame Microsoft for this one
#if QL_GARBLED_PTR_CONST
	#define QL_PTR_CONST	const *
#else
	#define QL_PTR_CONST	* const
#endif


#endif
