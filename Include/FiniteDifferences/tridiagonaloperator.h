
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

/*! \file tridiagonaloperator.h
	\brief tridiagonal operator

	$Source$
	$Name$
	$Log$
	Revision 1.5  2001/01/08 10:28:16  lballabio
	Moved Array to Math namespace

	Revision 1.4  2000/12/27 14:05:56  lballabio
	Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros
	
	Revision 1.3  2000/12/14 12:32:30  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#ifndef quantlib_tridiagonal_operator_h
#define quantlib_tridiagonal_operator_h

#include "qldefines.h"
#include "operator.h"
#include "identity.h"
#include "boundarycondition.h"
#include "array.h"
#include "date.h"

namespace QuantLib {

	namespace FiniteDifferences {

		class TridiagonalOperatorCommon {
		  public:
			// constructors
			TridiagonalOperatorCommon() : theSize(0) {}
			TridiagonalOperatorCommon(int size);
			TridiagonalOperatorCommon(const Math::Array& low, const Math::Array& mid, const Math::Array& high);
			// operator interface
			Math::Array solveFor(const Math::Array& rhs) const;
			Math::Array applyTo(const Math::Array& v) const;
			// inspectors
			int size() const { return theSize; }
			// modifiers
			void setLowerBC(const BoundaryCondition& bc);
			void setHigherBC(const BoundaryCondition& bc);
			void setFirstRow(double, double);
			void setMidRow(int, double, double, double);
			void setMidRows(double, double, double);
			void setLastRow(double, double);
			#if !(QL_TEMPLATE_METAPROGRAMMING_WORKS)
				void setTime(Time t) {}
			#endif
		  protected:
			Math::Array diagonal, belowDiagonal, aboveDiagonal;
			BoundaryCondition theLowerBC, theHigherBC;
		  private:
			int theSize;
		};
		
		// derived classes
		
		// time-constant

		class TridiagonalOperator : public TridiagonalOperatorCommon, public TimeConstantOperator {
			friend TridiagonalOperator operator+(const TridiagonalOperator&);
			friend TridiagonalOperator operator-(const TridiagonalOperator&);
			friend TridiagonalOperator operator*(double, const TridiagonalOperator&);
			friend TridiagonalOperator operator*(const TridiagonalOperator&, double);
			friend TridiagonalOperator operator+(const TridiagonalOperator&, const TridiagonalOperator&);
			friend TridiagonalOperator operator-(const TridiagonalOperator&, const TridiagonalOperator&);
			friend TridiagonalOperator operator+(const Identity<Math::Array>&, const TridiagonalOperator&);
			friend TridiagonalOperator operator+(const TridiagonalOperator&, const Identity<Math::Array>&);
			friend TridiagonalOperator operator-(const Identity<Math::Array>&, const TridiagonalOperator&);
			friend TridiagonalOperator operator-(const TridiagonalOperator&, const Identity<Math::Array>&);
		  public:
			// constructors
			TridiagonalOperator() : TridiagonalOperatorCommon() {}
			TridiagonalOperator(int size) : TridiagonalOperatorCommon(size) {}
			TridiagonalOperator(const Math::Array& low, const Math::Array& mid, const Math::Array& high)
			: TridiagonalOperatorCommon(low,mid,high) {}
		};

		// time-dependent
		
		class TimeDependentTridiagonalOperator : public TridiagonalOperatorCommon, public TimeDependentOperator {
		  public:
			// constructors
			TimeDependentTridiagonalOperator() : TridiagonalOperatorCommon() {}
			TimeDependentTridiagonalOperator(int size) : TridiagonalOperatorCommon(size) {}
			TimeDependentTridiagonalOperator(const Math::Array& low, const Math::Array& mid, const Math::Array& high)
			: TridiagonalOperatorCommon(low,mid,high) {}
		};


		// inline definitions
		
		inline TridiagonalOperatorCommon::TridiagonalOperatorCommon(int size) 
		: theSize(size) {
			QL_REQUIRE(theSize >= 3, "invalid size for tridiagonal operator (must be >= 3)");
			belowDiagonal = Math::Array(theSize-1);
			diagonal = Math::Array(theSize);
			aboveDiagonal = Math::Array(theSize-1);
		}
		
		inline TridiagonalOperatorCommon::TridiagonalOperatorCommon(const Math::Array& low, const Math::Array& mid, const Math::Array& high)
		: diagonal(mid), belowDiagonal(low), aboveDiagonal(high), theSize(mid.size()) {
			QL_REQUIRE(belowDiagonal.size() == theSize-1, "wrong size for lower diagonal vector");
			QL_REQUIRE(aboveDiagonal.size() == theSize-1, "wrong size for upper diagonal vector");
		}
		
		inline void TridiagonalOperatorCommon::setFirstRow(double valB, double valC){
			diagonal[0]      = valB;
			aboveDiagonal[0] = valC;
		}
		
		inline void TridiagonalOperatorCommon::setMidRow(int i, double valA, double valB, double valC){
			QL_REQUIRE(i>=1 && i<=theSize-2, "out of range in TridiagonalSystem::setMidRow");
			belowDiagonal[i-1] = valA;
			diagonal[i]        = valB;
			aboveDiagonal[i]   = valC;
		}
		
		inline void TridiagonalOperatorCommon::setMidRows(double valA, double valB, double valC){
			for(int i=1; i<=theSize-2; i++){
				belowDiagonal[i-1] = valA;
				diagonal[i]        = valB;
				aboveDiagonal[i]   = valC;
			}
		}
		
		inline void TridiagonalOperatorCommon::setLastRow(double valA, double valB){	
			belowDiagonal[theSize-2] = valA;
			diagonal[theSize-1]      = valB;
		}
		
		// time-constant algebra
		
		inline TridiagonalOperator operator+(const TridiagonalOperator& D) {
			return D;
		}
		
		inline TridiagonalOperator operator-(const TridiagonalOperator& D) {
			Math::Array low = -D.belowDiagonal, mid = -D.diagonal, high = -D.aboveDiagonal;
			TridiagonalOperator result(low,mid,high);
			result.setLowerBC(D.theLowerBC);
			result.setHigherBC(D.theHigherBC);
			return result;
		}
		
		inline TridiagonalOperator operator*(double a, const TridiagonalOperator& D) {
			Math::Array low = D.belowDiagonal*a, mid = D.diagonal*a, high = D.aboveDiagonal*a;
			TridiagonalOperator result(low,mid,high);
			result.setLowerBC(D.theLowerBC);
			result.setHigherBC(D.theHigherBC);
			return result;
		}
		
		inline TridiagonalOperator operator*(const TridiagonalOperator& D, double a) {
			Math::Array low = D.belowDiagonal*a, mid = D.diagonal*a, high = D.aboveDiagonal*a;
			TridiagonalOperator result(low,mid,high);
			result.setLowerBC(D.theLowerBC);
			result.setHigherBC(D.theHigherBC);
			return result;
		}
		
		inline TridiagonalOperator operator+(const TridiagonalOperator& D1, const TridiagonalOperator& D2) {
			// to do: check boundary conditions
			Math::Array low = D1.belowDiagonal+D2.belowDiagonal, mid = D1.diagonal+D2.diagonal,
			  high = D1.aboveDiagonal+D2.aboveDiagonal;
			return TridiagonalOperator(low,mid,high);
		}
		
		inline TridiagonalOperator operator-(const TridiagonalOperator& D1, const TridiagonalOperator& D2) {
			// to do: check boundary conditions
			Math::Array low = D1.belowDiagonal-D2.belowDiagonal, mid = D1.diagonal-D2.diagonal,
			  high = D1.aboveDiagonal-D2.aboveDiagonal;
			return TridiagonalOperator(low,mid,high);
		}
		
		inline TridiagonalOperator operator+(const TridiagonalOperator& D, const Identity<Math::Array>& I) {
			Math::Array mid = D.diagonal+1.0;
			TridiagonalOperator result(D.belowDiagonal,mid,D.aboveDiagonal);
			result.setLowerBC(D.theLowerBC);
			result.setHigherBC(D.theHigherBC);
			return result;
		}

		inline TridiagonalOperator operator+(const Identity<Math::Array>& I, const TridiagonalOperator& D) {
			Math::Array mid = D.diagonal+1.0;
			TridiagonalOperator result(D.belowDiagonal,mid,D.aboveDiagonal);
			result.setLowerBC(D.theLowerBC);
			result.setHigherBC(D.theHigherBC);
			return result;
		}

		inline TridiagonalOperator operator-(const TridiagonalOperator& D, const Identity<Math::Array>& I) {
			Math::Array mid = D.diagonal-1.0;
			TridiagonalOperator result(D.belowDiagonal,mid,D.aboveDiagonal);
			result.setLowerBC(D.theLowerBC);
			result.setHigherBC(D.theHigherBC);
			return result;
		}

		inline TridiagonalOperator operator-(const Identity<Math::Array>& I, const TridiagonalOperator& D) {
			Math::Array low = -D.belowDiagonal, mid = 1.0-D.diagonal, high = -D.aboveDiagonal;
			TridiagonalOperator result(low,mid,high);
			result.setLowerBC(D.theLowerBC);
			result.setHigherBC(D.theHigherBC);
			return result;
		}

	}

}


#endif
