
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file tridiagonaloperator.hpp
    \brief tridiagonal operator

    $Id$
*/

// $Source$
// $Log$
// Revision 1.5  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.4  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_tridiagonal_operator_h
#define quantlib_tridiagonal_operator_h

#include "ql/FiniteDifferences/operator.hpp"
#include "ql/FiniteDifferences/identity.hpp"
#include "ql/FiniteDifferences/boundarycondition.hpp"
#include "ql/array.hpp"
#include "ql/date.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! Base implementation for tridiagonal operator
        class TridiagonalOperatorCommon {
          public:
            // constructors
            TridiagonalOperatorCommon(int size = 0);
            TridiagonalOperatorCommon(const Array& low, const Array& mid,
                const Array& high);
            // operator interface
            Array solveFor(const Array& rhs) const;
            Array applyTo(const Array& v) const;
            // inspectors
            int size() const { return diagonal.size(); }
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
            Array diagonal, belowDiagonal, aboveDiagonal;
            BoundaryCondition theLowerBC, theHigherBC;
        };

        // derived classes

        // time-constant

        class TridiagonalOperator : public TridiagonalOperatorCommon,
          public TimeConstantOperator {
            friend TridiagonalOperator operator+(const TridiagonalOperator&);
            friend TridiagonalOperator operator-(const TridiagonalOperator&);
            friend TridiagonalOperator operator*(double,
                const TridiagonalOperator&);
            friend TridiagonalOperator operator*(const TridiagonalOperator&,
                double);
            friend TridiagonalOperator operator/(const TridiagonalOperator&,
                double);
            friend TridiagonalOperator operator+(const TridiagonalOperator&,
                const TridiagonalOperator&);
            friend TridiagonalOperator operator-(const TridiagonalOperator&,
                const TridiagonalOperator&);
            friend TridiagonalOperator operator+(const Identity<Array>&,
                const TridiagonalOperator&);
            friend TridiagonalOperator operator+(const TridiagonalOperator&,
                const Identity<Array>&);
            friend TridiagonalOperator operator-(const Identity<Array>&,
                const TridiagonalOperator&);
            friend TridiagonalOperator operator-(const TridiagonalOperator&,
                const Identity<Array>&);
          public:
            // constructors
            TridiagonalOperator() : TridiagonalOperatorCommon() {}
            TridiagonalOperator(int size) : TridiagonalOperatorCommon(size) {}
            TridiagonalOperator(const Array& low, const Array& mid,
                const Array& high)
            : TridiagonalOperatorCommon(low,mid,high) {}
            #if defined(QL_PATCH_MICROSOFT_BUGS)
            /* This copy constructor and assignment operator are here because
               somehow Visual C++ is not able to generate working ones. They are
               _not_ to be defined for other compilers which are able to
               generate correct ones.   */
                TridiagonalOperator(const TridiagonalOperator& op)
                : TridiagonalOperatorCommon(op.belowDiagonal, op.diagonal,
                    op.aboveDiagonal) {
                        theLowerBC = op.theLowerBC;
                        theHigherBC = op.theHigherBC;
                }
                TridiagonalOperator& operator=(const TridiagonalOperator& op) {
                    belowDiagonal = op.belowDiagonal;
                    diagonal = op.diagonal;
                    aboveDiagonal = op.aboveDiagonal;
                    theLowerBC = op.theLowerBC;
                    theHigherBC = op.theHigherBC;
                    return *this;
                }
            #endif
        };

        // time-dependent

        class TimeDependentTridiagonalOperator :
          public TridiagonalOperatorCommon, public TimeDependentOperator {
          public:
            // constructors
            TimeDependentTridiagonalOperator(int size = 0)
            : TridiagonalOperatorCommon(size) {}
            TimeDependentTridiagonalOperator(const Array& low, const Array& mid,
                const Array& high)
            : TridiagonalOperatorCommon(low,mid,high) {}
        };


        // inline definitions

        inline void TridiagonalOperatorCommon::setFirstRow(double valB,
          double valC) {
            diagonal[0]      = valB;
            aboveDiagonal[0] = valC;
        }

        inline void TridiagonalOperatorCommon::setMidRow(int i, double valA,
          double valB, double valC) {
            QL_REQUIRE(i>=1 && i<=size()-2,
                "out of range in TridiagonalSystem::setMidRow");
            belowDiagonal[i-1] = valA;
            diagonal[i]        = valB;
            aboveDiagonal[i]   = valC;
        }

        inline void TridiagonalOperatorCommon::setMidRows(double valA,
          double valB, double valC){
            for (int i=1; i<=size()-2; i++) {
                belowDiagonal[i-1] = valA;
                diagonal[i]        = valB;
                aboveDiagonal[i]   = valC;
            }
        }

        inline void TridiagonalOperatorCommon::setLastRow(double valA,
          double valB) {
            belowDiagonal[size()-2] = valA;
            diagonal[size()-1]      = valB;
        }

        // time-constant algebra

        inline TridiagonalOperator operator+(const TridiagonalOperator& D) {
            return D;
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D) {
            Array low = -D.belowDiagonal, mid = -D.diagonal,
                high = -D.aboveDiagonal;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.theLowerBC);
            result.setHigherBC(D.theHigherBC);
            return result;
        }

        inline TridiagonalOperator operator*(double a,
          const TridiagonalOperator& D) {
            Array low = D.belowDiagonal*a, mid = D.diagonal*a,
                high = D.aboveDiagonal*a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.theLowerBC);
            result.setHigherBC(D.theHigherBC);
            return result;
        }

        inline TridiagonalOperator operator*(const TridiagonalOperator& D,
          double a) {
            Array low = D.belowDiagonal*a, mid = D.diagonal*a,
                high = D.aboveDiagonal*a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.theLowerBC);
            result.setHigherBC(D.theHigherBC);
            return result;
        }

        inline TridiagonalOperator operator/(const TridiagonalOperator& D,
          double a) {
            Array low = D.belowDiagonal/a, mid = D.diagonal/a,
                high = D.aboveDiagonal/a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.theLowerBC);
            result.setHigherBC(D.theHigherBC);
            return result;
        }

        inline TridiagonalOperator operator+(const TridiagonalOperator& D1,
          const TridiagonalOperator& D2) {
            // to do: check boundary conditions
            Array low = D1.belowDiagonal+D2.belowDiagonal,
                mid = D1.diagonal+D2.diagonal,
                high = D1.aboveDiagonal+D2.aboveDiagonal;
            return TridiagonalOperator(low,mid,high);
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D1,
          const TridiagonalOperator& D2) {
            // to do: check boundary conditions
            Array low = D1.belowDiagonal-D2.belowDiagonal,
                mid = D1.diagonal-D2.diagonal,
                high = D1.aboveDiagonal-D2.aboveDiagonal;
            return TridiagonalOperator(low,mid,high);
        }

        inline TridiagonalOperator operator+(const TridiagonalOperator& D,
          const Identity<Array>& I) {
            Array mid = D.diagonal+1.0;
            TridiagonalOperator result(D.belowDiagonal,mid,D.aboveDiagonal);
            result.setLowerBC(D.theLowerBC);
            result.setHigherBC(D.theHigherBC);
            return result;
        }

        inline TridiagonalOperator operator+(const Identity<Array>& I,
          const TridiagonalOperator& D) {
            Array mid = D.diagonal+1.0;
            TridiagonalOperator result(D.belowDiagonal,mid,D.aboveDiagonal);
            result.setLowerBC(D.theLowerBC);
            result.setHigherBC(D.theHigherBC);
            return result;
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D,
          const Identity<Array>& I) {
            Array mid = D.diagonal-1.0;
            TridiagonalOperator result(D.belowDiagonal,mid,D.aboveDiagonal);
            result.setLowerBC(D.theLowerBC);
            result.setHigherBC(D.theHigherBC);
            return result;
        }

        inline TridiagonalOperator operator-(const Identity<Array>& I,
          const TridiagonalOperator& D) {
            Array low = -D.belowDiagonal, mid = 1.0-D.diagonal,
                high = -D.aboveDiagonal;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.theLowerBC);
            result.setHigherBC(D.theHigherBC);
            return result;
        }

    }

}


#endif
