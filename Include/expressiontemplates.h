
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_expression_templates_h
#define quantlib_expression_templates_h

#include "qldefines.h"
#include <cmath>

/* The concepts implemented here are from 
   T. L. Veldhuizen, "Expression templates", C++ Report, 7(5):26-31, June 1995
   http://extreme.indiana.edu/~tveldhui/papers/
*/

namespace QuantLib {

	// iterator on generic vectorial expression
	
	template <class Iter> 
	class VectorialExpression {
	  public:
		VectorialExpression(const Iter& i, int size) : i(i), n(size) {}
		double operator*() const { return *i; }
		VectorialExpression& operator++() { ++i; return *this; }
		// pre-increment operator not defined - inefficient
		int size() const { return n; }
	  private:
		Iter i;
		int n;
	};
	
	// dummy iterator on scalar
	
	class Scalar {
	  public:
		Scalar(double x) : x(x) {}
		double operator*() const { return x; }
		Scalar& operator++() { return *this; }
		// pre-increment operator not defined - inefficient
	  private:
		double x;
	};
	
	// iterator on unary vectorial expression
	
	template <class Iter1, class Operation> 
	class UnaryVectorialExpression {
	  public:
		UnaryVectorialExpression(const Iter1& i, int size) : i(i), n(size) {}
		double operator*() const { return Operation::apply(*i); }
		UnaryVectorialExpression& operator++() { ++i; return *this; }
		// pre-increment operator not defined - inefficient
		int size() const { return n; }
	  private:
		Iter1 i;
		int n;
	};
	
	// iterator on binary vectorial expression
	
	template <class Iter1, class Iter2, class Operation> 
	class BinaryVectorialExpression {
	  public:
		BinaryVectorialExpression(const Iter1& i, const Iter2& j, int size) : i(i), j(j), n(size) {}
		double operator*() const { return Operation::apply(*i,*j); }
		BinaryVectorialExpression& operator++() { ++i; ++j; return *this; }
		// pre-increment operator not defined - inefficient
		int size() const { return n; }
	  private:
		Iter1 i;
		Iter2 j;
		int n;
	};
	
	
	// unary operations
	
	class Plus {
	  public:
		static inline double apply(double a) { return +a; }
	};
	
	class Minus {
	  public:
		static inline double apply(double a) { return -a; }
	};
	
	
	// binary operations
	
	class Add {
	  public:
		static inline double apply(double a, double b) { return a+b; }
	};
	
	class Subtract {
	  public:
		static inline double apply(double a, double b) { return a-b; }
	};
	
	class Multiply {
	  public:
		static inline double apply(double a, double b) { return a*b; }
	};
	
	class Divide {
	  public:
		static inline double apply(double a, double b) { return a/b; }
	};
	
	
	// functions
	
	class AbsoluteValue {
	  public:
		static inline double apply(double a) { return QL_FABS(a); }
	};
	
	class SquareRoot {
	  public:
		static inline double apply(double a) { return QL_SQRT(a); }
	};
	
	class Sine {
	  public:
		static inline double apply(double a) { return QL_SIN(a); }
	};
	
	class Cosine {
	  public:
		static inline double apply(double a) { return QL_COS(a); }
	};
	
	class Logarithm {
	  public:
		static inline double apply(double a) { return QL_LOG(a); }
	};
	
	class Exponential {
	  public:
		static inline double apply(double a) { return QL_EXP(a); }
	};

}


#endif
