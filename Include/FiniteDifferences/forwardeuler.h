
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

/*! \file forwardeuler.h
	\brief forward Euler scheme for time evolution
	
	$Source$
	$Name$
	$Log$
	Revision 1.4  2000/12/14 12:32:30  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_forward_euler_h
#define quantlib_forward_euler_h

#include "qldefines.h"
#include "date.h"
#include "identity.h"
#include "operatortraits.h"

namespace QuantLib {

	namespace FiniteDifferences {
	
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
			void setStep(Time dt) {
				this->dt = dt;
				explicitPart = Identity<arrayType>()-dt*D;
			}
			Operator D, explicitPart;
			Time dt;
			#if QL_TEMPLATE_METAPROGRAMMING_WORKS
				// a bit of template metaprogramming to relax interface constraints on time-constant operators
				// see T. L. Veldhuizen, "Using C++ Template Metaprograms", C++ Report, Vol 7 No. 4, May 1995
				// http://extreme.indiana.edu/~tveldhui/papers/
				template <int constant>
				class ForwardEulerTimeSetter {};
				// the following specialization will be instantiated if Operator is derived from TimeConstantOperator
				template<>
				class ForwardEulerTimeSetter<0> {
				  public:
					static inline void setTime(Operator& D, Operator& explicitPart, Time t, Time dt) {}
				};
				// the following specialization will be instantiated if Operator is derived from TimeDependentOperator:
				// only in this case Operator will be required to implement void setTime(Time t)
				template<>
				class ForwardEulerTimeSetter<1> {
				  public:
					static inline void setTime(Operator& D, Operator& explicitPart, Time t, Time dt) {
						D.setTime(t);
						explicitPart = Identity<arrayType>()-dt*D;
					}
				};
			#endif
		};
		
		// inline definitions
		
		template<class Operator>
		inline void ForwardEuler<Operator>::step(arrayType& a, Time t) const {
			#if QL_TEMPLATE_METAPROGRAMMING_WORKS
				ForwardEulerTimeSetter<Operator::isTimeDependent>::setTime(D,explicitPart,t,dt);
			#else
				if (Operator::isTimeDependent) {
					D.setTime(t);
					explicitPart = Identity<arrayType>()-dt*D;
				}
			#endif
			a = explicitPart.applyTo(a);
		}

	}

}


#endif
