
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License: see the file LICENSE.TXT for details.
 * Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file.
 * LICENCE.TXT is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/


#ifndef quantlib_forward_euler_h
#define quantlib_forward_euler_h

#include "qldefines.h"
#include "date.h"
#include "identity.h"
#include "operatortraits.h"

namespace QuantLib {

	namespace PDE {
	
		/*	Operators must be derived from either TimeConstantOperator or TimeDependentOperator.
			They must also implement at least the following interface:
		
			// copy constructor						// if no particular care is required, this one can be
			Operator(const Operator&);				// omitted. It will be provided by the compiler.
		
			// modifiers
			void setTime(Time t);					// only if derived from TimeDependentOperator.
			
			// operator interface
			arrayType applyTo(const arrayType&);
		*/
		
		template <class Operator>
		class ForwardEuler {
			friend class FiniteDifferenceModel<ForwardEuler<Operator> >;
		  private:
			// typedefs
			typedef typename OperatorTraits<Operator>::arrayType arrayType;
			typedef Operator operatorType;
			// constructors
			ForwardEuler(const operatorType& D) : D(D), dt(0.0) {}
			void step(arrayType& a, Time t) const;
			void setStep(Time dt) { this->dt = dt; }
			Operator D;
			Time dt;
		};
		
		// a bit of template metaprogramming to relax interface constraints on time-constant operators
		// see T. L. Veldhuizen, "Using C++ Template Metaprograms", C++ Report, Vol 7 No. 4, May 1995
		// http://extreme.indiana.edu/~tveldhui/papers/
		
		template <class Operator, int constant>
		class ForwardEulerTimeSetter {};
		
		// the following specialization will be instantiated if Operator is derived from TimeConstantOperator
		template<class Operator>
		class ForwardEulerTimeSetter<Operator,0> {
		  public:
			static inline void setTime(Operator& D, Time t) {}
		};
		
		// the following specialization will be instantiated if Operator is derived from TimeDependentOperator:
		// only in this case Operator will be required to implement void setTime(Time t)
		template<class Operator>
		class ForwardEulerTimeSetter<Operator,1> {
		  public:
			static inline void setTime(Operator& D, Time t) { D.setTime(t); }
		};
		
		
		// inline definitions
		
		template<class Operator>
		inline void ForwardEuler<Operator>::step(arrayType& a, Time t) const {
			ForwardEulerTimeSetter<Operator,Operator::isTimeDependent>::setTime(D,t);
			a -= dt*(D.applyTo(a));
		}

	}

}


#endif
