
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_backward_euler_h
#define quantlib_backward_euler_h

#include "qldefines.h"
#include "date.h"
#include "identity.h"
#include "operatortraits.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(PDE)

/*	Operators must be derived from either TimeConstantOperator or TimeDependentOperator.
	They must also implement at least the following interface:

	// copy constructor/assignment
	Operator(const Operator&);				// if no particular care is required, these two can be
	Operator& operator=(const Operator&);	// omitted. They will be provided by the compiler.

	// modifiers
	void setTime(Time t);					// only if derived from TimeDependentOperator.

	// operator interface
	arrayType solveFor(const arrayType&);

	// operator algebra
	Operator operator*(double,const Operator&);
	Operator operator+(const Identity<arrayType>&,const Operator&);
*/

template <class Operator>
class BackwardEuler {
	friend class FiniteDifferenceModel<BackwardEuler<Operator> >;
  private:
	// typedefs
	typedef typename OperatorTraits<Operator>::arrayType arrayType;
	typedef Operator operatorType;
	// constructors
	BackwardEuler(const operatorType& D) : D(D), dt(0.0) {}
	void step(arrayType& a, Time t) const;
	void setStep(Time dt) { this->dt = dt; system = Identity<arrayType>()+dt*D; }
	operatorType system, D;
	Time dt;
};

// a bit of template metaprogramming to relax interface constraints on time-constant operators
// see T. L. Veldhuizen, "Using C++ Template Metaprograms", C++ Report, Vol 7 No. 4, May 1995
// http://extreme.indiana.edu/~tveldhui/papers/

template <class Operator, int constant>
class BackwardEulerTimeSetter {};

// the following specialization will be instantiated if Operator is derived from TimeConstantOperator
template<class Operator>
class BackwardEulerTimeSetter<Operator,0> {
  public:
	static inline void setTime(Operator& D, Time t) {}
};

// the following specialization will be instantiated if Operator is derived from TimeDependentOperator:
// only in this case Operator will be required to implement void setTime(Time t)
template<class Operator>
class BackwardEulerTimeSetter<Operator,1> {
	typedef typename OperatorTraits<Operator>::arrayType arrayType;
  public:
	static inline void setTime(Operator& D, Operator& system, Time t, Time dt) {
		D.setTime(t);
		system = Identity<arrayType>()+dt*D;
	}
};


// inline definitions

template <class Operator>
inline void BackwardEuler<Operator>::step(arrayType& a, Time t) const {
	BackwardEulerTimeSetter<Operator,Operator::isTimeDependent>::setTime(D,system,t,dt);
	a = system.solveFor(a);
}


QL_END_NAMESPACE(PDE)

QL_END_NAMESPACE(QuantLib)


#endif
