
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
    \brief tridiagonal operator

    \fullpath
    ql/FiniteDifferences/%tridiagonaloperator.hpp
*/

// $Id$

#ifndef quantlib_tridiagonal_operator_h
#define quantlib_tridiagonal_operator_h

#include "ql/FiniteDifferences/operator.hpp"
#include "ql/FiniteDifferences/identity.hpp"
#include "ql/FiniteDifferences/boundarycondition.hpp"
#include "ql/array.hpp"
#include "ql/date.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        /*! Base implementation for tridiagonal operator
            \warning to use real time-dependant algebra, you must overload
            the corresponding operators in the inheriting time-dependent class
        */
        class TridiagonalOperator {
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
            TridiagonalOperator(unsigned int size = 0);
            TridiagonalOperator(const Array& low, const Array& mid,
                const Array& high);
            virtual ~TridiagonalOperator() {}
            // operator interface
            Array solveFor(const Array& rhs) const;
            Array applyTo(const Array& v) const;
            // inspectors
            unsigned int size() const { return diagonal_.size(); }
            // modifiers
            void setLowerBC(const BoundaryCondition& bc);
            void setUpperBC(const BoundaryCondition& bc);
            void setFirstRow(double, double);
            void setMidRow(unsigned int, double, double, double);
            void setMidRows(double, double, double);
            void setLastRow(double, double);
            virtual bool isTimeDependent() {return false;}
            virtual void setTime(Time t) {}
          protected:
            Array diagonal_, belowDiagonal_, aboveDiagonal_;
            BoundaryCondition lowerBC_, upperBC_;
        };

        // derived classes

        // time-constant

        class ConstantTridiagonalOperator : public TridiagonalOperator 
        {
          public:
            // constructors
            ConstantTridiagonalOperator() : TridiagonalOperator() {}
            ConstantTridiagonalOperator(unsigned int size) 
            : TridiagonalOperator(size) {}
            ConstantTridiagonalOperator(const Array& low, const Array& mid,
                const Array& high)
            : TridiagonalOperator(low,mid,high) {}
            #if defined(QL_PATCH_MICROSOFT_BUGS)
            /* This copy constructor and assignment operator are here because
               somehow Visual C++ is not able to generate working ones. They 
               are _not_ to be defined for other compilers which are able to
               generate correct ones.   */
                ConstantTridiagonalOperator(const ConstantTridiagonalOperator& op)
                : TridiagonalOperator(op.belowDiagonal_, op.diagonal_,
                    op.aboveDiagonal_) {
                        lowerBC_ = op.lowerBC_;
                        upperBC_ = op.upperBC_;
                }
                ConstantTridiagonalOperator& operator=(const ConstantTridiagonalOperator& op) {
                    belowDiagonal_ = op.belowDiagonal_;
                    diagonal_      = op.diagonal_;
                    aboveDiagonal_ = op.aboveDiagonal_;
                    lowerBC_       = op.lowerBC_;
                    upperBC_       = op.upperBC_;
                    return *this;
                }
            #endif
            virtual bool isTimeDependent() { return false;}    
        };

        // time-dependent

        class TimeDependentTridiagonalOperator :
          public TridiagonalOperator {
          public:
            // constructors
            TimeDependentTridiagonalOperator(unsigned int size=0)
            : TridiagonalOperator(size) {}
            TimeDependentTridiagonalOperator(const Array& low, 
                const Array& mid, const Array& high)
            : TridiagonalOperator(low,mid,high) {}
            virtual bool isTimeDependent() { return true;}
        };


        // inline definitions

        inline void TridiagonalOperator::setFirstRow(double valB,
          double valC) {
            diagonal_[0]      = valB;
            aboveDiagonal_[0] = valC;
        }

        inline void TridiagonalOperator::setMidRow(unsigned int i, 
          double valA, double valB, double valC) {
            QL_REQUIRE(i>=1 && i<=size()-2,
                "out of range in TridiagonalSystem::setMidRow");
            belowDiagonal_[i-1] = valA;
            diagonal_[i]        = valB;
            aboveDiagonal_[i]   = valC;
        }

        inline void TridiagonalOperator::setMidRows(double valA,
          double valB, double valC){
            for (unsigned int i=1; i<=size()-2; i++) {
                belowDiagonal_[i-1] = valA;
                diagonal_[i]        = valB;
                aboveDiagonal_[i]   = valC;
            }
        }

        inline void TridiagonalOperator::setLastRow(double valA,
          double valB) {
            belowDiagonal_[size()-2] = valA;
            diagonal_[size()-1]      = valB;
        }

        // Time constant algebra

        inline TridiagonalOperator operator+(const TridiagonalOperator& D) {
            return D;
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D) {
            Array low = -D.belowDiagonal_, mid = -D.diagonal_,
                high = -D.aboveDiagonal_;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setUpperBC(D.upperBC_);
            return result;
        }

        inline TridiagonalOperator operator*(double a,
          const TridiagonalOperator& D) {
            Array low = D.belowDiagonal_*a, mid = D.diagonal_*a,
                high = D.aboveDiagonal_*a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setUpperBC(D.upperBC_);
            return result;
        }

        inline TridiagonalOperator operator*(const TridiagonalOperator& D,
          double a) {
            Array low = D.belowDiagonal_*a, mid = D.diagonal_*a,
                high = D.aboveDiagonal_*a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setUpperBC(D.upperBC_);
            return result;
        }

        inline TridiagonalOperator operator/(const TridiagonalOperator& D,
          double a) {
            Array low = D.belowDiagonal_/a, mid = D.diagonal_/a,
                high = D.aboveDiagonal_/a;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setUpperBC(D.upperBC_);
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
            TridiagonalOperator result(D.belowDiagonal_, mid, 
                D.aboveDiagonal_);
            result.setLowerBC(D.lowerBC_);
            result.setUpperBC(D.upperBC_);
            return result;
        }

        inline TridiagonalOperator operator+(const Identity<Array>& I,
          const TridiagonalOperator& D) {
            Array mid = D.diagonal_+1.0;
            TridiagonalOperator result(D.belowDiagonal_, mid, 
                D.aboveDiagonal_);
            result.setLowerBC(D.lowerBC_);
            result.setUpperBC(D.upperBC_);
            return result;
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D,
          const Identity<Array>& I) {
            Array mid = D.diagonal_-1.0;
            TridiagonalOperator result(D.belowDiagonal_, mid, 
                D.aboveDiagonal_);
            result.setLowerBC(D.lowerBC_);
            result.setUpperBC(D.upperBC_);
            return result;
        }

        inline TridiagonalOperator operator-(const Identity<Array>& I,
          const TridiagonalOperator& D) {
            Array low = -D.belowDiagonal_, mid = 1.0-D.diagonal_,
                high = -D.aboveDiagonal_;
            TridiagonalOperator result(low,mid,high);
            result.setLowerBC(D.lowerBC_);
            result.setUpperBC(D.upperBC_);
            return result;
        }

    }

}


#endif
