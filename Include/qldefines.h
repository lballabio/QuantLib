
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file qldefines.h
	\brief Global definitions and compiler switches.
	
	This file contains a few global definitions and quite a few macros 
	which help porting the code to different compilers (each with its own
	non-standard behaviors and implementations).
*/

/*! \namespace QuantLib
	\brief a.k.a. the QuantLib Foundation
	
	The root namespace <tt>QuantLib</tt> contains what can be considered
	the <b>QuantLib foundation</b>, i.e., the core of abstract classes upon
	which the rest of the library is built.
*/

#ifndef quantlib_defines_h
#define quantlib_defines_h

//! global trace level (may be superseded locally by a greater value)
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
	#define QL_BROKEN_TEMPLATE_SPECIALIZATION	1
	#define QL_GARBLED_MIN_AND_MAX				1
	#define QL_GARBLED_REVERSE_ITERATORS		1
	#define QL_GARBLED_PTR_CONST				1
	#define QL_INCLUDE_IN_SUBFOLDERS			1

#elif defined(__BORLANDC__)				// Borland C++
	// set switches
	#define QL_CMATH_IN_STD						1
	#define QL_CCHAR_IN_STD						1
	#define QL_TEMPLATE_METAPROGRAMMING_WORKS	0
	#define QL_EXPRESSION_TEMPLATES_WORK		0
	#define QL_REQUIRES_DUMMY_RETURN			0
	#define QL_BROKEN_TEMPLATE_SPECIALIZATION	0
	#define QL_GARBLED_MIN_AND_MAX				0
	#define QL_GARBLED_REVERSE_ITERATORS		0
	#define QL_GARBLED_PTR_CONST				0
	#define QL_INCLUDE_IN_SUBFOLDERS			1

#elif defined(__MWERKS__)					// Metrowerks CodeWarrior
	// set switches
	#define QL_CMATH_IN_STD						1
	#define QL_CCHAR_IN_STD						1
	#define QL_TEMPLATE_METAPROGRAMMING_WORKS	1
	#define QL_EXPRESSION_TEMPLATES_WORK		0
	#define QL_REQUIRES_DUMMY_RETURN			1
	#define QL_BROKEN_TEMPLATE_SPECIALIZATION	0
	#define QL_GARBLED_MIN_AND_MAX				0
	#define QL_GARBLED_REVERSE_ITERATORS		0
	#define QL_GARBLED_PTR_CONST				0
	#define QL_INCLUDE_IN_SUBFOLDERS			0

#elif defined(__GNUC__)						// GNU C++
	// set switches
	#define QL_CMATH_IN_STD						0
	#define QL_CCHAR_IN_STD						0
	#define QL_TEMPLATE_METAPROGRAMMING_WORKS	1
	#define QL_EXPRESSION_TEMPLATES_WORK		0
	#define QL_REQUIRES_DUMMY_RETURN			0
	#define QL_BROKEN_TEMPLATE_SPECIALIZATION	0
	#define QL_GARBLED_MIN_AND_MAX				0
	#define QL_GARBLED_REVERSE_ITERATORS		0
	#define QL_GARBLED_PTR_CONST				0
	#define QL_INCLUDE_IN_SUBFOLDERS			1

#else										// Generic ANSI C++ compliant compiler
	// set switches
	#define QL_CMATH_IN_STD						1
	#define QL_CCHAR_IN_STD						1
	#define QL_TEMPLATE_METAPROGRAMMING_WORKS	1
	#define QL_EXPRESSION_TEMPLATES_WORK		1
	#define QL_REQUIRES_DUMMY_RETURN			0
	#define QL_BROKEN_TEMPLATE_SPECIALIZATION	0
	#define QL_GARBLED_MIN_AND_MAX				0
	#define QL_GARBLED_REVERSE_ITERATORS		0
	#define QL_GARBLED_PTR_CONST				0
	#define QL_INCLUDE_IN_SUBFOLDERS			1

#endif


// Switch-dependent #definitions

/*! \def QL_EXPRESSION_TEMPLATES_WORK
	Expression templates techniques (see T. L. Veldhuizen, "Expression templates", 
	C++ Report, 7(5):26-31, June 1995, available at http://extreme.indiana.edu/~tveldhui/papers)
	are sometimes too advanced for the template implementation of current compilers.
*/

/*! \def QL_TEMPLATE_METAPROGRAMMING_WORKS
	Template metaprogramming techniques (see T. L. Veldhuizen, "Using C++ Template Metaprograms", 
	C++ Report, Vol 7 No. 4, May 1995, available at http://extreme.indiana.edu/~tveldhui/papers)
	are sometimes too advanced for the template implementation of current compilers.
*/

/*! \def QL_INCLUDE_IN_SUBFOLDERS
	Defined as true if the compiler supports the <path/header.h> syntax, as false otherwise.
*/

/*! \def QL_CMATH_IN_STD
	\brief Are math functions defined in the std namespace?
	
	Some compilers still define them in the global namespace.
*/
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

/*! \def QL_CCHAR_IN_STD
	\brief Are string functions defined in the std namespace?
	
	Some compilers still define them in the global namespace.
*/
#if QL_CCHAR_IN_STD
	#define QL_STRLEN	std::strlen
	#define QL_TOLOWER	std::tolower
	#define QL_TOUPPER	std::toupper
#else
	#define QL_STRLEN	strlen
	#define QL_TOLOWER	tolower
	#define QL_TOUPPER	toupper
#endif

/*! \def QL_REQUIRES_DUMMY_RETURN
	\brief Is a dummy return statement required?
	
	Some compilers will issue a warning if it is missing even though
	it could never be reached during execution, e.g., after a block like
	\code
	if (condition)
		return validResult;
	else
		throw HideousError();
	\endcode
*/
#if QL_REQUIRES_DUMMY_RETURN
	#define QL_DUMMY_RETURN(x)		return x;
#else
	#define QL_DUMMY_RETURN(x) 
#endif

/*! \def QL_BROKEN_TEMPLATE_SPECIALIZATION
	\brief Blame Microsoft for this one...
	
	They decided that a declaration and a definition of a specialized template function amount
	to a redefinition and should issue a linker error.
*/
#if QL_BROKEN_TEMPLATE_SPECIALIZATION
	#define QL_DECLARE_TEMPLATE_SPECIALIZATION(x)
	#define QL_TEMPLATE_SPECIALIZATION
#else
	#define QL_DECLARE_TEMPLATE_SPECIALIZATION(x)	template<> x;
	#define QL_TEMPLATE_SPECIALIZATION				template<>
#endif

/*! \def QL_GARBLED_MIN_AND_MAX
	\brief Blame Microsoft for this one...
	
	They decided to call them <tt>std::_cpp_min</tt> and <tt>std::_cpp_max</tt> to avoid
	the hassle of rewriting their code.
*/
#if QL_GARBLED_MIN_AND_MAX
	#define QL_MIN	std::_cpp_min
	#define QL_MAX	std::_cpp_max
#else
	#define QL_MIN	std::min
	#define QL_MAX	std::max
#endif

/*! \def QL_GARBLED_REVERSE_ITERATORS
	\brief Blame Microsoft for this one...
	
	They decided that <tt>std::reverse_iterator<iterator></tt> needed an extra 
	template argument.
*/
#if QL_GARBLED_REVERSE_ITERATORS
	#define QL_REVERSE_ITERATOR(iterator,type)	std::reverse_iterator<iterator,type>
#else
	#define QL_REVERSE_ITERATOR(iterator,type)	std::reverse_iterator<iterator>
#endif

/*! \def QL_GARBLED_PTR_CONST
	\brief Blame Microsoft for this one...
	
	They decided to rewrite ANSI C++ and use <tt>const *</tt> instead of <tt>* const</tt>.
*/
#if QL_GARBLED_PTR_CONST
	#define QL_PTR_CONST	const *
#else
	#define QL_PTR_CONST	* const
#endif


#endif
