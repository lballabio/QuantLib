
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_crank_nicolson_h
#define quantlib_crank_nicolson_h

#include "qldefines.h"
#include "date.h"
#include "identity.h"
#include "operatortraits.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(PDE)

// WARNING: the differential operator D must be linear for this evolver to work!

/*	Operators must be derived from either TimeConstantOperator or TimeDependentOperator.
	They must also implement at least the following interface:

	// copy constructor/assignment
	Operator(const Operator&);				// if no particular care is required, these two can be
	Operator& operator=(const Operator&);	// omitted. They will be provided by the compiler.

	// modifiers
	void setTime(Time t);					// only if derived from TimeDependentOperator.

	// operator interface
	arrayType applyTo(const arrayType&);
	arrayType solveFor(const arrayType&);

	// operator algebra
	Operator operator*(double,const Operator&);
	Operator operator+(const Identity<arrayType>&,const Operator&);
*/

template <class Operator>
class CrankNicolson {
	friend class FiniteDifferenceModel<CrankNicolson<Operator> >;
  private:
	// typedefs
	typedef typename OperatorTraits<Operator>::arrayType arrayType;
	typedef Operator operatorType;
	// constructors
	CrankNicolson(const operatorType& D) : D(D), dt(0.0) {}
	void step(arrayType& a, Time t);
	void setStep(Time dt) { this->dt = dt; system = Identity<arrayType>()+(dt/2)*D; }
	operatorType D, system;
	Time dt;
	// a bit of template metaprogramming to relax interface constraints on time-constant operators
	// see T. L. Veldhuizen, "Using C++ Template Metaprograms", C++ Report, Vol 7 No. 4, May 1995
	// http://extreme.indiana.edu/~tveldhui/papers/
	
	template <int constant>
	class CrankNicolsonTimeSetter {};
	
	// the following specialization will be instantiated if Operator is derived from TimeConstantOperator
	template<>
	class CrankNicolsonTimeSetter<0> {
	  public:
		static inline void setTime(Operator& D, Operator& system, Time t, Time dt) {}
	};
	
	// the following specialization will be instantiated if Operator is derived from TimeDependentOperator:
	// only in this case Operator will be required to implement void setTime(Time t)
	template<>
	class CrankNicolsonTimeSetter<1> {
		typedef typename OperatorTraits<Operator>::arrayType arrayType;
	  public:
		static inline void setTime(Operator& D, Operator& system, Time t, Time dt) {
			D.setTime(t);
			system = Identity<arrayType>()+(dt/2)*D;
		}
	};
};

// inline definitions

template <class Operator>
inline void CrankNicolson<Operator>::step(arrayType& a, Time t) {
	CrankNicolsonTimeSetter<Operator::isTimeDependent>::setTime(D,system,t,dt);
	a -= (dt/2)*(D.applyTo(a));
	a = system.solveFor(a); 
}


QL_END_NAMESPACE(PDE)

QL_END_NAMESPACE(QuantLib)


#endif
