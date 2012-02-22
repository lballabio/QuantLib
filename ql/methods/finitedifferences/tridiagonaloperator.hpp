/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2011 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file tridiagonaloperator.hpp
    \brief tridiagonal operator
*/

#ifndef quantlib_tridiagonal_operator_hpp
#define quantlib_tridiagonal_operator_hpp

#include <ql/math/array.hpp>
#include <ql/math/comparison.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

    //! Base implementation for tridiagonal operator
    /*! \warning to use real time-dependant algebra, you must overload
                 the corresponding operators in the inheriting
                 time-dependent class.

        \ingroup findiff
    */
    class TridiagonalOperator {
        // unary operators
        friend Disposable<TridiagonalOperator>
        operator+(const TridiagonalOperator&);
        friend Disposable<TridiagonalOperator>
        operator-(const TridiagonalOperator&);
        // binary operators
        friend Disposable<TridiagonalOperator>
        operator+(const TridiagonalOperator&,
                  const TridiagonalOperator&);
        friend Disposable<TridiagonalOperator>
        operator-(const TridiagonalOperator&,
                  const TridiagonalOperator&);
        friend Disposable<TridiagonalOperator>
        operator*(Real,
                  const TridiagonalOperator&);
        friend Disposable<TridiagonalOperator>
        operator*(const TridiagonalOperator&,
                  Real);
        friend Disposable<TridiagonalOperator>
        operator/(const TridiagonalOperator&,
                  Real);
      public:
        typedef Array array_type;
        // constructors
        explicit TridiagonalOperator(Size size = 0);
        TridiagonalOperator(const Array& low,
                            const Array& mid,
                            const Array& high);
        TridiagonalOperator(const Disposable<TridiagonalOperator>&);
        TridiagonalOperator& operator=(const Disposable<TridiagonalOperator>&);
        //! \name Operator interface
        //@{
        //! apply operator to a given array
        Disposable<Array> applyTo(const Array& v) const;
        //! solve linear system for a given right-hand side
        Disposable<Array> solveFor(const Array& rhs) const;
        /*! solve linear system for a given right-hand side
            without result Array allocation. The rhs and result parameters
            can be the same Array, in which case rhs will be changed
        */
        void solveFor(const Array& rhs,
                      Array& result) const;
        //! solve linear system with SOR approach
        Disposable<Array> SOR(const Array& rhs,
                              Real tol) const;
        //! identity instance
        static Disposable<TridiagonalOperator> identity(Size size);
        //@}
        //! \name Inspectors
        //@{
        Size size() const { return n_; }
        bool isTimeDependent() const { return !!timeSetter_; }
        const Array& lowerDiagonal() const { return lowerDiagonal_; }
        const Array& diagonal() const { return diagonal_; }
        const Array& upperDiagonal() const { return upperDiagonal_; }
        //@}
        //! \name Modifiers
        //@{
        void setFirstRow(Real, Real);
        void setMidRow(Size, Real, Real, Real);
        void setMidRows(Real, Real, Real);
        void setLastRow(Real, Real);
        void setTime(Time t);
        //@}
        //! \name Utilities
        //@{
        void swap(TridiagonalOperator&);
        //@}
        //! encapsulation of time-setting logic
        class TimeSetter {
          public:
            virtual ~TimeSetter() {}
            virtual void setTime(Time t,
                                 TridiagonalOperator& L) const = 0;
        };
      protected:
        Size n_;
        Array diagonal_, lowerDiagonal_, upperDiagonal_;
        mutable Array temp_;
        boost::shared_ptr<TimeSetter> timeSetter_;
    };

    /* \relates TridiagonalOperator */
    void swap(TridiagonalOperator&, TridiagonalOperator&);


    // inline definitions

    inline TridiagonalOperator& TridiagonalOperator::operator=(
                                const Disposable<TridiagonalOperator>& from) {
        swap(const_cast<Disposable<TridiagonalOperator>&>(from));
        return *this;
    }

    inline void TridiagonalOperator::setFirstRow(Real valB,
                                                 Real valC) {
        diagonal_[0]      = valB;
        upperDiagonal_[0] = valC;
    }

    inline void TridiagonalOperator::setMidRow(Size i,
                                               Real valA,
                                               Real valB,
                                               Real valC) {
        QL_REQUIRE(i>=1 && i<=n_-2,
                   "out of range in TridiagonalSystem::setMidRow");
        lowerDiagonal_[i-1] = valA;
        diagonal_[i]        = valB;
        upperDiagonal_[i]   = valC;
    }

    inline void TridiagonalOperator::setMidRows(Real valA,
                                                Real valB,
                                                Real valC) {
        for (Size i=1; i<=n_-2; i++) {
            lowerDiagonal_[i-1] = valA;
            diagonal_[i]        = valB;
            upperDiagonal_[i]   = valC;
        }
    }

    inline void TridiagonalOperator::setLastRow(Real valA,
                                                Real valB) {
        lowerDiagonal_[n_-2] = valA;
        diagonal_[n_-1]      = valB;
    }

    inline void TridiagonalOperator::setTime(Time t) {
        if (timeSetter_)
            timeSetter_->setTime(t, *this);
    }

    inline void TridiagonalOperator::swap(TridiagonalOperator& from) {
        using std::swap;
        swap(n_, from.n_);
        diagonal_.swap(from.diagonal_);
        lowerDiagonal_.swap(from.lowerDiagonal_);
        upperDiagonal_.swap(from.upperDiagonal_);
        temp_.swap(from.temp_);
        swap(timeSetter_, from.timeSetter_);
    }


    // Time constant algebra

    inline Disposable<TridiagonalOperator>
    operator+(const TridiagonalOperator& D) {
        TridiagonalOperator D1 = D;
        return D1;
    }

    inline Disposable<TridiagonalOperator>
    operator-(const TridiagonalOperator& D) {
        Array low = -D.lowerDiagonal_,
            mid = -D.diagonal_,
            high = -D.upperDiagonal_;
        TridiagonalOperator result(low, mid, high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator+(const TridiagonalOperator& D1,
              const TridiagonalOperator& D2) {
        Array low = D1.lowerDiagonal_ + D2.lowerDiagonal_,
            mid = D1.diagonal_ + D2.diagonal_,
            high = D1.upperDiagonal_ + D2.upperDiagonal_;
        TridiagonalOperator result(low, mid, high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator-(const TridiagonalOperator& D1,
              const TridiagonalOperator& D2) {
        Array low = D1.lowerDiagonal_ - D2.lowerDiagonal_,
            mid = D1.diagonal_ - D2.diagonal_,
            high = D1.upperDiagonal_ - D2.upperDiagonal_;
        TridiagonalOperator result(low, mid, high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator*(Real a,
              const TridiagonalOperator& D) {
        Array low = D.lowerDiagonal_ * a,
            mid = D.diagonal_ * a,
            high = D.upperDiagonal_ * a;
        TridiagonalOperator result(low, mid, high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator*(const TridiagonalOperator& D,
              Real a) {
        Array low = D.lowerDiagonal_ * a,
            mid = D.diagonal_ * a,
            high = D.upperDiagonal_ * a;
        TridiagonalOperator result(low, mid, high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator/(const TridiagonalOperator& D,
              Real a) {
        Array low = D.lowerDiagonal_ / a,
            mid = D.diagonal_ / a,
            high = D.upperDiagonal_ / a;
        TridiagonalOperator result(low, mid, high);
        return result;
    }

    inline void swap(TridiagonalOperator& L1,
                     TridiagonalOperator& L2) {
        L1.swap(L2);
    }

}

#endif
