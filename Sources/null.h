
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_null_h
#define quantlib_null_h

#include "qldefines.h"
#include <limits>

QL_BEGIN_NAMESPACE(QuantLib)

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
	operator int() const { return std::numeric_limits<int>::max(); }
};

template <>
class Null<double> {
  public:
	Null() {}
	operator double() const { return std::numeric_limits<double>::max(); }
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

QL_END_NAMESPACE(QuantLib)


#endif
