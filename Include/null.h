
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

/*! \file null.h
	\brief null values
	
	$Source$
	$Name$
	$Log$
	Revision 1.6  2000/12/20 15:27:48  lballabio
	Using new defines for helping Linux port

	Revision 1.5  2000/12/14 12:32:29  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#ifndef quantlib_null_h
#define quantlib_null_h

#include "qldefines.h"

namespace QuantLib {

	//! template class providing a null value.
	template <class Type>
	class Null {
	  public:
		Null() {}
		operator Type() const { return Type(); }
	};
	
	template <>
	class Null<int> {
	  public:
		Null() {}
		operator int() const { return QL_MAX_INT; }
	};
	
	template <>
	class Null<double> {
	  public:
		Null() {}
		operator double() const { return QL_MAX_DOUBLE; }
	};
	
	
	template <class Type> bool IsNull(const Type&);
	// specializations shouldn't be necessary.
	// However, the dumb VC++ compiler gets stuck on Type(Null<Type>()).
	bool IsNull(int);
	bool IsNull(double);
	
	
	// inline definitions
	
	template <class Type>
	inline bool IsNull(const Type& x) {
		return (x == Type());
	}
	
	inline bool IsNull(int x) {
		return (x == Null<int>());
	}
	
	inline bool IsNull(double x) {
		return (x == Null<double>());
	}

}


#endif
