
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file tridiagonaloperator.hpp
    \fullpath Include/ql/FiniteDifferences/%tridiagonaloperator.hpp
    \brief tridiagonal operator

*/

// $Id$
// $Log$
// Revision 1.10  2001/08/28 17:23:30  nando
// unsigned int instead of int
//
// Revision 1.9  2001/08/28 13:37:35  nando
// unsigned int instead of int
//
// Revision 1.8  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.7  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.6  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
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
            TridiagonalOperatorCommon(unsigned int size = 0);
            TridiagonalOperatorCommon(const Array& low, const Array& mid,
                const Array& high);
            // operator interface
            Array solveFor(const Array& rhs) const;
            Array applyTo(const Array& v) const;
            // inspectors
            unsigned int size() const { return diagonal_.size(); }
            // modifiers
            void setLowerBC(const BoundaryCondition& bc);
            void setHigherBC(const BoundaryCondition& bc);
            void setFirstRow(double, double);
            void setMidRow(unsigned int, double, double, double);
            void setMidRows(double, double, double);
            void setLastRow(double, double);
            #if !(QL_TEMPLATE_METAPROGRAMMING_WORKS)
                void setTime(Time t) {}
            #endif
          protected:
            Array diagonal_, belowDiagonal_, aboveDiagonal_;
            BoundaryCondition lowerBC_, higherBC_;
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
            TridiagonalOperator(unsigned int size) : TridiagonalOperatorCommon(size) {}
            TridiagonalOperator(const Array& low, const Array& mid,
                const Array& high)
            : TridiagonalOperatorCommon(low,mid,high) {}
            #if defined(QL_PATCH_MICROSOFT_BUGS)
            /* This copy constructor and assignment operator are here because
               somehow Visual C++ is not able to generate working ones. They are
               _not_ to be defined for other compilers which are able to
               generate correct ones.   */
                TridiagonalOperator(const TridiagonalOperator& op)
                : TridiagonalOperatorCommon(op.belowDiagonal_, op.diagonal_,
                    op.aboveDiagonal_) {
                        lowerBC_  = op.lowerBC_;
                        higherBC_ = op.higherBC_;
                }
                TridiagonalOperator& operator=(const TridiagonalOperator& op) {
                    belowDiagonal_ = op.belowDiagonal_;
                    diagonal_      = op.diagonal_;
                    aboveDiagonal_ = op.aboveDiagonal_;
                    lowerBC_       = op.lowerBC_;
                    higherBC_      = op.higherBC_;
                    return *this;
                }
            #endif
        };

        // time-dependent

        class TimeDependentTridiagonalOperator :
          public TridiagonalOperatorCommon, public TimeDependentOperator {
          public:
            // constructors
            TimeDependentTridiagonalOperator(unsigned int size = 0)
            : TridiagonalOperatorCommon(size) {}
            TimeDependentTridiagonalOperator(const Array& low, const Array& mid,
                const Array& high)
            : TridiagonalOperatorCommon(low,mid,high) {}
        };


        // inline definitions

        inline void TridiagonalOperatorCommon::setFirstRow(double valB,
          double valC) {
            diagonal_[0]      = valB;
            aboveDiagonal_[0] = valC;
        }

        inline void TridiagonalOperatorCommon::setMidRow(unsigned int i, double valA,
          double valB, double valC) {
            QL_REQUIRE(i>=1 && i<=size()-2,
                "out of range in TridiagonalSystem::setMidRow");
            belowDiagonal_[i-1] = valA;
            diagonal_[i]        = valB;
            aboveDiagonal_[i]   = valC;
        }

        inline void TridiagonalOperatorCommon::setMidRows(double valA,
          double valB, double valC){
            for (unsigned int i=1; i<=size()-2; i++) {
                belowDiagonal_[i-1] = valA;
                diagonal_[i]        = valB;
                aboveDiagonal_[i]   = valC;
            }
        }

        inline void TridiagonalOperatorCommon::setLastRow(double valA,
          double valB) {
            belowDiagonal_[size()-2] = valA;
            diagonal_[size()-1]      = valB;
        }

        // time-constant algebra

        inline TridiagonalOperator operator+(const TridiagonalOperator& D) {
            return D;
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D) {
            Array low = -D.belowDiagonal_, mid = -D.diagonal_,
                high = -D.aboveDiagonal_;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setHigherBC(D.higherBC_);
            return result;
        }

        inline TridiagonalOperator operator*(double a,
          const TridiagonalOperator& D) {
            Array low = D.belowDiagonal_*a, mid = D.diagonal_*a,
                high = D.aboveDiagonal_*a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setHigherBC(D.higherBC_);
            return result;
        }

        inline TridiagonalOperator operator*(const TridiagonalOperator& D,
          double a) {
            Array low = D.belowDiagonal_*a, mid = D.diagonal_*a,
                high = D.aboveDiagonal_*a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setHigherBC(D.higherBC_);
            return result;
        }

        inline TridiagonalOperator operator/(const TridiagonalOperator& D,
          double a) {
            Array low = D.belowDiagonal_/a, mid = D.diagonal_/a,
                high = D.aboveDiagonal_/a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setHigherBC(D.higherBC_);
            return result;
        }

        inline TridiagonalOperator operator+(const TridiagonalOperator& D1,
          const TridiagonalOperator& D2) {
            // to do: check boundary conditions
            Array low = D1.belowDiagonal_+D2.belowDiagonal_,
                mid = D1.diagonal_+D2.diagonal_,
                high = D1.aboveDiagonal_+D2.aboveDiagonal_;
            return TridiagonalOperator(low,mid,high);
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D1,
          const TridiagonalOperator& D2) {
            // to do: check boundary conditions
            Array low = D1.belowDiagonal_-D2.belowDiagonal_,
                mid = D1.diagonal_-D2.diagonal_,
                high = D1.aboveDiagonal_-D2.aboveDiagonal_;
            return TridiagonalOperator(low,mid,high);
        }

        inline TridiagonalOperator operator+(const TridiagonalOperator& D,
          const Identity<Array>& I) {
            Array mid = D.diagonal_+1.0;
            TridiagonalOperator result(D.belowDiagonal_,mid,D.aboveDiagonal_);
            result.setLowerBC(D.lowerBC_);
            result.setHigherBC(D.higherBC_);
            return result;
        }

        inline TridiagonalOperator operator+(const Identity<Array>& I,
          const TridiagonalOperator& D) {
            Array mid = D.diagonal_+1.0;
            TridiagonalOperator result(D.belowDiagonal_,mid,D.aboveDiagonal_);
            result.setLowerBC(D.lowerBC_);
            result.setHigherBC(D.higherBC_);
            return result;
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D,
          const Identity<Array>& I) {
            Array mid = D.diagonal_-1.0;
            TridiagonalOperator result(D.belowDiagonal_,mid,D.aboveDiagonal_);
            result.setLowerBC(D.lowerBC_);
            result.setHigherBC(D.higherBC_);
            return result;
        }

        inline TridiagonalOperator operator-(const Identity<Array>& I,
          const TridiagonalOperator& D) {
            Array low = -D.belowDiagonal_, mid = 1.0-D.diagonal_,
                high = -D.aboveDiagonal_;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setHigherBC(D.higherBC_);
            return result;
        }

    }

}


#endif
