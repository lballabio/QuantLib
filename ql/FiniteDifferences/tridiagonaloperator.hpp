
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file tridiagonaloperator.hpp
    \brief tridiagonal operator

    \fullpath
    ql/FiniteDifferences/%tridiagonaloperator.hpp
*/

// $Id$

#ifndef quantlib_tridiagonal_operator_h
#define quantlib_tridiagonal_operator_h

#include <ql/array.hpp>
#include <ql/handle.hpp>

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
            TridiagonalOperator(Size size = 0);
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
            //! solve linear system with SOR approach
            Array SOR(const Array& rhs, double tol) const;
            //! identity instance
            static TridiagonalOperator identity(Size size);
            //@}
            //! \name Inspectors
            //@{
            Size size() const;
            bool isTimeDependent();
            //@}
            //! \name Modifiers
            //@{
            void setFirstRow(double, double);
            void setMidRow(Size, double, double, double);
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
            Handle<TimeSetter> timeSetter_;
        };


        // inline definitions

        #if defined(QL_PATCH_MICROSOFT_BUGS)
            inline TridiagonalOperator::TridiagonalOperator(
                const TridiagonalOperator& L) {
                    belowDiagonal_ = L.belowDiagonal_;
                    diagonal_      = L.diagonal_;
                    aboveDiagonal_ = L.aboveDiagonal_;
                    timeSetter_    = L.timeSetter_;
            }
            inline TridiagonalOperator& TridiagonalOperator::operator=(
                const TridiagonalOperator& L){
                    belowDiagonal_ = L.belowDiagonal_;
                    diagonal_      = L.diagonal_;
                    aboveDiagonal_ = L.aboveDiagonal_;
                    timeSetter_    = L.timeSetter_;
                    return *this;
            }
        #endif

        inline Size TridiagonalOperator::size() const {
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

        inline void TridiagonalOperator::setMidRow(Size i,
          double valA, double valB, double valC) {
            QL_REQUIRE(i>=1 && i<=size()-2,
                "out of range in TridiagonalSystem::setMidRow");
            belowDiagonal_[i-1] = valA;
            diagonal_[i]        = valB;
            aboveDiagonal_[i]   = valC;
        }

        inline void TridiagonalOperator::setMidRows(double valA,
          double valB, double valC){
            for (Size i=1; i<=size()-2; i++) {
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
            return result;
        }

        inline TridiagonalOperator operator+(const TridiagonalOperator& D1,
          const TridiagonalOperator& D2) {
            Array low = D1.belowDiagonal_+D2.belowDiagonal_,
                mid = D1.diagonal_+D2.diagonal_,
                high = D1.aboveDiagonal_+D2.aboveDiagonal_;
            TridiagonalOperator result(low,mid,high);
            return result;
        }

        inline TridiagonalOperator operator-(const TridiagonalOperator& D1,
          const TridiagonalOperator& D2) {
            Array low = D1.belowDiagonal_-D2.belowDiagonal_,
                mid = D1.diagonal_-D2.diagonal_,
                high = D1.aboveDiagonal_-D2.aboveDiagonal_;
            TridiagonalOperator result(low,mid,high);
            return result;
        }

        inline TridiagonalOperator operator*(double a,
          const TridiagonalOperator& D) {
            Array low = D.belowDiagonal_*a, mid = D.diagonal_*a,
                high = D.aboveDiagonal_*a;
            TridiagonalOperator result(low,mid,high);
            return result;
        }

        inline TridiagonalOperator operator*(const TridiagonalOperator& D,
          double a) {
            Array low = D.belowDiagonal_*a, mid = D.diagonal_*a,
                high = D.aboveDiagonal_*a;
            TridiagonalOperator result(low,mid,high);
            return result;
        }

        inline TridiagonalOperator operator/(const TridiagonalOperator& D,
          double a) {
            Array low = D.belowDiagonal_/a, mid = D.diagonal_/a,
                high = D.aboveDiagonal_/a;
            TridiagonalOperator result(low,mid,high);
            return result;
        }

    }

}


#endif
