
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

#include "ql/FiniteDifferences/boundarycondition.hpp"
#include "ql/array.hpp"
#include "ql/date.hpp"
#include "ql/handle.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! Base implementation for tridiagonal operator
        /*! \warning to use real time-dependant algebra, you must overload
            the corresponding operators in the inheriting time-dependent 
            class
        */
        class TridiagonalOperator {
            // unary operators
            friend TridiagonalOperator operator+(const TridiagonalOperator&);
            friend TridiagonalOperator operator-(const TridiagonalOperator&);
            // binary operators
            friend TridiagonalOperator operator+(const TridiagonalOperator&,
                const TridiagonalOperator&);
            friend TridiagonalOperator operator-(const TridiagonalOperator&,
                const TridiagonalOperator&);
            friend TridiagonalOperator operator*(double,
                const TridiagonalOperator&);
            friend TridiagonalOperator operator*(const TridiagonalOperator&,
                double);
            friend TridiagonalOperator operator/(const TridiagonalOperator&,
                double);
          public:
            typedef Array arrayType;
            // constructors
            TridiagonalOperator(size_t size = 0);
            TridiagonalOperator(const Array& low, const Array& mid,
                const Array& high);
            #if defined(QL_PATCH_MICROSOFT_BUGS)
                /* This copy constructor and assignment operator are here 
                   because somehow Visual C++ is not able to generate working 
                   ones. They are _not_ to be defined for other compilers 
                   which are able to generate correct ones.   */
                TridiagonalOperator(const TridiagonalOperator& L);
                TridiagonalOperator& operator=(const TridiagonalOperator& L);
            #endif
            //! \name Operator interface
            //@{
            //! apply operator to a given array
            Array applyTo(const Array& v) const;
            //! solve linear system for a given right-hand side
            Array solveFor(const Array& rhs) const;
            //! identity instance
            static TridiagonalOperator identity(size_t size);
            //@}
            //! \name Inspectors
            //@{
            size_t size() const;
            bool isTimeDependent();
            //@}
            //! \name Modifiers
            //@{
            void setLowerBC(const BoundaryCondition& bc);
            void setUpperBC(const BoundaryCondition& bc);
            void setFirstRow(double, double);
            void setMidRow(size_t, double, double, double);
            void setMidRows(double, double, double);
            void setLastRow(double, double);
            void setTime(Time t);
            //@}
            //! encapsulation of time-setting logic
            class TimeSetter {
              public:
                virtual ~TimeSetter() {}
                virtual void setTime(Time t, 
                                     TridiagonalOperator& L) const = 0;
            };
          protected:
            Array diagonal_, belowDiagonal_, aboveDiagonal_;
            BoundaryCondition lowerBC_, upperBC_;
            Handle<TimeSetter> timeSetter_;
        };


        // inline definitions

        #if defined(QL_PATCH_MICROSOFT_BUGS)
            inline TridiagonalOperator::TridiagonalOperator(
                const TridiagonalOperator& L) {
                    belowDiagonal_ = L.belowDiagonal_;
                    diagonal_      = L.diagonal_;
                    aboveDiagonal_ = L.aboveDiagonal_;
                    lowerBC_       = L.lowerBC_;
                    upperBC_       = L.upperBC_;
                    timeSetter_    = L.timeSetter_;
            }
            inline TridiagonalOperator& TridiagonalOperator::operator=(
                const TridiagonalOperator& L){
                    belowDiagonal_ = L.belowDiagonal_;
                    diagonal_      = L.diagonal_;
                    aboveDiagonal_ = L.aboveDiagonal_;
                    lowerBC_       = L.lowerBC_;
                    upperBC_       = L.upperBC_;
                    timeSetter_    = L.timeSetter_;
                    return *this;
            }
        #endif

        inline size_t TridiagonalOperator::size() const { 
            return diagonal_.size(); 
        }

        inline bool TridiagonalOperator::isTimeDependent() {
            return !timeSetter_.isNull();
        }

        inline void TridiagonalOperator::setFirstRow(double valB,
          double valC) {
            diagonal_[0]      = valB;
            aboveDiagonal_[0] = valC;
        }

        inline void TridiagonalOperator::setMidRow(size_t i, 
          double valA, double valB, double valC) {
            QL_REQUIRE(i>=1 && i<=size()-2,
                "out of range in TridiagonalSystem::setMidRow");
            belowDiagonal_[i-1] = valA;
            diagonal_[i]        = valB;
            aboveDiagonal_[i]   = valC;
        }

        inline void TridiagonalOperator::setMidRows(double valA,
          double valB, double valC){
            for (size_t i=1; i<=size()-2; i++) {
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

        inline void TridiagonalOperator::setTime(Time t) {
            if (!timeSetter_.isNull())
                timeSetter_->setTime(t,*this);
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

        inline TridiagonalOperator operator+(const TridiagonalOperator& D1,
          const TridiagonalOperator& D2) {
            QL_REQUIRE(D1.lowerBC_.type() == BoundaryCondition::None ||
                       D2.lowerBC_.type() == BoundaryCondition::None,
                "Adding operators with colliding boundary conditions");
            QL_REQUIRE(D1.upperBC_.type() == BoundaryCondition::None ||
                       D2.upperBC_.type() == BoundaryCondition::None,
                "Adding operators with colliding boundary conditions");
            Array low = D1.belowDiagonal_+D2.belowDiagonal_,
                mid = D1.diagonal_+D2.diagonal_,
                high = D1.aboveDiagonal_+D2.aboveDiagonal_;
            TridiagonalOperator result(low,mid,high);
            if (D1.lowerBC_.type() == BoundaryCondition::None)
                result.setLowerBC(D2.lowerBC_);
            else
                result.setLowerBC(D1.lowerBC_);
            if (D1.upperBC_.type() == BoundaryCondition::None)
                result.setUpperBC(D2.upperBC_);
            else
                result.setUpperBC(D1.upperBC_);
            return result;
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D1,
          const TridiagonalOperator& D2) {
            QL_REQUIRE(D1.lowerBC_.type() == BoundaryCondition::None ||
                       D2.lowerBC_.type() == BoundaryCondition::None,
                "Subtracting operators with colliding boundary conditions");
            QL_REQUIRE(D1.upperBC_.type() == BoundaryCondition::None ||
                       D2.upperBC_.type() == BoundaryCondition::None,
                "Subtracting operators with colliding boundary conditions");
            Array low = D1.belowDiagonal_-D2.belowDiagonal_,
                mid = D1.diagonal_-D2.diagonal_,
                high = D1.aboveDiagonal_-D2.aboveDiagonal_;
            TridiagonalOperator result(low,mid,high);
            if (D1.lowerBC_.type() == BoundaryCondition::None)
                result.setLowerBC(D2.lowerBC_);
            else
                result.setLowerBC(D1.lowerBC_);
            if (D1.upperBC_.type() == BoundaryCondition::None)
                result.setUpperBC(D2.upperBC_);
            else
                result.setUpperBC(D1.upperBC_);
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

    }

}


#endif
