
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_expression_templates_h
#define quantlib_expression_templates_h

#include "qldefines.h"
#include "array.h"

/* The concepts implemented here are from 
   T. L. Veldhuizen, "Expression templates", C++ Report, 7(5):26-31, June 1995
   http://extreme.indiana.edu/~tveldhui/papers/
*/

QL_BEGIN_NAMESPACE(QuantLib)

// iterator on generic vectorial expression

template <class Type, class Iter> 
class VectorialExpression {
  public:
	VectorialExpression(const Iter& i, int size) : i(i), n(size) {}
	Type operator*() const { return *i; }
	VectorialExpression& operator++() { ++i; return *this; }
	// pre-increment operator not defined - inefficient
	int size() const { return n; }
  private:
	Iter i;
	int n;
};

// dummy iterator on scalar

template <class Type> 
class Scalar {
  public:
	Scalar(const Type& x) : x(x) {}
	Type operator*() const { return x; }
	Scalar& operator++() { return *this; }
	// pre-increment operator not defined - inefficient
  private:
	Type x;
};

// iterator on unary vectorial expression

template <class Type, class Iter1, class Operation> 
class UnaryVectorialExpression {
  public:
	UnaryVectorialExpression(const Iter1& i, int size) : i(i), n(size) {}
	Type operator*() const { return Operation::apply(*i); }
	UnaryVectorialExpression& operator++() { ++i; return *this; }
	// pre-increment operator not defined - inefficient
	int size() const { return n; }
  private:
	Iter1 i;
	int n;
};

// iterator on binary vectorial expression

template <class Type, class Iter1, class Iter2, class Operation> 
class BinaryVectorialExpression {
  public:
	BinaryVectorialExpression(const Iter1& i, const Iter2& j, int size) : i(i), j(j), n(size) {}
	Type operator*() const { return Operation::apply(*i,*j); }
	BinaryVectorialExpression& operator++() { ++i; ++j; return *this; }
	// pre-increment operator not defined - inefficient
	int size() const { return n; }
  private:
	Iter1 i;
	Iter2 j;
	int n;
};


// unary operations

template <class Type> 
class Plus {
  public:
	static inline Type apply(Type a) { return +a; }
};

template <class Type> 
class Minus {
  public:
	static inline Type apply(Type a) { return -a; }
};


// binary operations

template <class Type> 
class Add {
  public:
	static inline Type apply(Type a, Type b) { return a+b; }
};

template <class Type> 
class Subtract {
  public:
	static inline Type apply(Type a, Type b) { return a-b; }
};

template <class Type> 
class Multiply {
  public:
	static inline Type apply(Type a, Type b) { return a*b; }
};

template <class Type> 
class Divide {
  public:
	static inline Type apply(Type a, Type b) { return a/b; }
};


// functions

template <class Type>
class AbsoluteValue {
  public:
	static inline Type apply(Type a) { return QL_ABS(a); }
};

template <class Type>
class SquareRoot {
  public:
	static inline Type apply(Type a) { return QL_SQRT(a); }
};

template <class Type>
class Sinus {
  public:
	static inline Type apply(Type a) { return QL_SIN(a); }
};

template <class Type>
class Cosinus {
  public:
	static inline Type apply(Type a) { return QL_COS(a); }
};

template <class Type>
class Logarithm {
  public:
	static inline Type apply(Type a) { return QL_LOG(a); }
};

template <class Type>
class Exponential {
  public:
	static inline Type apply(Type a) { return QL_EXP(a); }
};

QL_END_NAMESPACE(QuantLib)

#endif
