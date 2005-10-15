/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file tridiagonaloperator.hpp
    \brief tridiagonal operator
*/

#ifndef quantlib_tridiagonal_operator_hpp
#define quantlib_tridiagonal_operator_hpp

#include <ql/Math/array.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

    //! Base implementation for tridiagonal operator
    /*! \warning to use real time-dependant algebra, you must overload
                 the corresponding operators in the inheriting time-dependent
                 class

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
        TridiagonalOperator(Size size = 0);
        TridiagonalOperator(const Array& low, const Array& mid,
                            const Array& high);
        #if defined(QL_PATCH_MSVC6)
        /* This copy constructor and assignment operator are here
           because somehow Visual C++ is not able to generate working
           ones. They are _not_ to be defined for other compilers
           which are able to generate correct ones.   */
        TridiagonalOperator(const TridiagonalOperator&);
        TridiagonalOperator& operator=(const TridiagonalOperator&);
        #endif
        TridiagonalOperator(const Disposable<TridiagonalOperator>&);
        TridiagonalOperator& operator=(
                                       const Disposable<TridiagonalOperator>&);
        //! \name Operator interface
        //@{
        //! apply operator to a given array
        template <class T>
        Disposable<T> applyTo(const T& v) const {
            QL_REQUIRE(v.size()==size(),
                       "vector of the wrong size (" << v.size()
                       << "instead of " << size() << ")"  );
            T result(size());
            std::transform(diagonal_.begin(),
                           diagonal_.end(),
                           v.begin(),
                           result.begin(),
                           std::multiplies<Real>());
            
            // matricial product
            result[0] += upperDiagonal_[0]*v[1];
            for (Size j=1;j<=size()-2;j++)
                result[j] += lowerDiagonal_[j-1]*v[j-1]+ 
                    upperDiagonal_[j]*v[j+1];
            result[size()-1] += lowerDiagonal_[size()-2]*v[size()-2];
            
            return result;
        }

        //! solve linear system for a given right-hand side
        template <class T>
        Disposable<T> solveFor(const T& rhs) const  {
            QL_REQUIRE(rhs.size()==size(), "rhs has the wrong size");
            
            T result(size()), tmp(size());
            
            Real bet=diagonal_[0];
            QL_REQUIRE(bet != 0.0, "division by zero");
            result[0] = rhs[0]/bet;
            Size j;
            for (j=1;j<=size()-1;j++){
                tmp[j]=upperDiagonal_[j-1]/bet;
                bet=diagonal_[j]-lowerDiagonal_[j-1]*tmp[j];
                QL_ENSURE(bet != 0.0, "division by zero");
                result[j] = (rhs[j]-lowerDiagonal_[j-1]*result[j-1])/bet;
            }
            // cannot be j>=0 with Size j
            for (j=size()-2;j>0;j--)
                result[j] -= tmp[j+1]*result[j+1];
            result[0] -= tmp[1]*result[1];
            return result;
        }

        //! solve linear system with SOR approach
        template <class T>
        Disposable<T> SOR(const T& rhs, Real tol) const {
            QL_REQUIRE(rhs.size()==size(), "rhs has the wrong size");

            // initial guess
            T result = rhs;
            
            // solve tridiagonal system with SOR technique
            Size sorIteration, i;
            Real omega = 1.5;
            Real err = 2.0*tol;
            Real temp;
            for (sorIteration=0; err>tol ; sorIteration++) {
                QL_REQUIRE(sorIteration<100000,
                           "tolerance (" << tol << ") not reached in "
                           << sorIteration << " iterations. "
                           << "The error still is " << err);
                err=0.0;
                for (i=1; i<size()-2 ; i++) {
                    temp = omega * (rhs[i]     -
                                    upperDiagonal_[i]   * result[i+1]-
                                    diagonal_[i]        * result[i] -
                                    lowerDiagonal_[i-1] * result[i-1]) /
                        diagonal_[i];
                    err += temp * temp;
                    result[i] += temp;
                }
            }
            return result;
        }


        //! identity instance
        static Disposable<TridiagonalOperator> identity(Size size);
        //@}
        //! \name Inspectors
        //@{
        Size size() const;
        bool isTimeDependent();
        const Array& lowerDiagonal() const;
        const Array& diagonal() const;
        const Array& upperDiagonal() const;
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
        Array diagonal_, lowerDiagonal_, upperDiagonal_;
        boost::shared_ptr<TimeSetter> timeSetter_;
    };

    /* \relates TridiagonalOperator */
    void swap(TridiagonalOperator&, TridiagonalOperator&);

    // inline definitions

    #if defined(QL_PATCH_MSVC6)
    inline TridiagonalOperator::TridiagonalOperator(
                                               const TridiagonalOperator& L) {
        lowerDiagonal_ = L.lowerDiagonal_;
        diagonal_      = L.diagonal_;
        upperDiagonal_ = L.upperDiagonal_;
        timeSetter_    = L.timeSetter_;
    }
    inline TridiagonalOperator& TridiagonalOperator::operator=(
                                               const TridiagonalOperator& L) {
        lowerDiagonal_ = L.lowerDiagonal_;
        diagonal_      = L.diagonal_;
        upperDiagonal_ = L.upperDiagonal_;
        timeSetter_    = L.timeSetter_;
        return *this;
    }
    #endif

    inline TridiagonalOperator::TridiagonalOperator(
                                const Disposable<TridiagonalOperator>& from) {
        swap(const_cast<Disposable<TridiagonalOperator>&>(from));
    }

    inline TridiagonalOperator& TridiagonalOperator::operator=(
                                const Disposable<TridiagonalOperator>& from) {
        swap(const_cast<Disposable<TridiagonalOperator>&>(from));
        return *this;
    }

    inline Size TridiagonalOperator::size() const {
        return diagonal_.size();
    }

    inline bool TridiagonalOperator::isTimeDependent() {
        return !!timeSetter_;
    }

    inline const Array& TridiagonalOperator::lowerDiagonal() const{
        return lowerDiagonal_;
    }

    inline const Array& TridiagonalOperator::diagonal() const{
        return diagonal_;
    }

    inline const Array& TridiagonalOperator::upperDiagonal() const{
        return upperDiagonal_;
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
        QL_REQUIRE(i>=1 && i<=size()-2,
                   "out of range in TridiagonalSystem::setMidRow");
        lowerDiagonal_[i-1] = valA;
        diagonal_[i]        = valB;
        upperDiagonal_[i]   = valC;
    }

    inline void TridiagonalOperator::setMidRows(Real valA,
                                                Real valB,
                                                Real valC) {
        for (Size i=1; i<=size()-2; i++) {
            lowerDiagonal_[i-1] = valA;
            diagonal_[i]        = valB;
            upperDiagonal_[i]   = valC;
        }
    }

    inline void TridiagonalOperator::setLastRow(Real valA,
                                                Real valB) {
        lowerDiagonal_[size()-2] = valA;
        diagonal_[size()-1]      = valB;
    }

    inline void TridiagonalOperator::setTime(Time t) {
        if (timeSetter_)
            timeSetter_->setTime(t,*this);
    }

    inline void TridiagonalOperator::swap(TridiagonalOperator& from) {
        using std::swap;
        diagonal_.swap(from.diagonal_);
        lowerDiagonal_.swap(from.lowerDiagonal_);
        upperDiagonal_.swap(from.upperDiagonal_);
        swap(timeSetter_,from.timeSetter_);
    }


    // Time constant algebra

    inline Disposable<TridiagonalOperator>
    operator+(const TridiagonalOperator& D) {
        TridiagonalOperator D1 = D;
        return D1;
    }

    inline Disposable<TridiagonalOperator>
    operator-(const TridiagonalOperator& D) {
        Array low = -D.lowerDiagonal_, mid = -D.diagonal_,
            high = -D.upperDiagonal_;
        TridiagonalOperator result(low,mid,high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator+(const TridiagonalOperator& D1,
              const TridiagonalOperator& D2) {
        Array low = D1.lowerDiagonal_+D2.lowerDiagonal_,
            mid = D1.diagonal_+D2.diagonal_,
            high = D1.upperDiagonal_+D2.upperDiagonal_;
        TridiagonalOperator result(low,mid,high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator-(const TridiagonalOperator& D1,
              const TridiagonalOperator& D2) {
        Array low = D1.lowerDiagonal_-D2.lowerDiagonal_,
            mid = D1.diagonal_-D2.diagonal_,
            high = D1.upperDiagonal_-D2.upperDiagonal_;
        TridiagonalOperator result(low,mid,high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator*(Real a,
              const TridiagonalOperator& D) {
        Array low = D.lowerDiagonal_*a, mid = D.diagonal_*a,
            high = D.upperDiagonal_*a;
        TridiagonalOperator result(low,mid,high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator*(const TridiagonalOperator& D,
              Real a) {
        Array low = D.lowerDiagonal_*a, mid = D.diagonal_*a,
            high = D.upperDiagonal_*a;
        TridiagonalOperator result(low,mid,high);
        return result;
    }

    inline Disposable<TridiagonalOperator>
    operator/(const TridiagonalOperator& D,
              Real a) {
        Array low = D.lowerDiagonal_/a, mid = D.diagonal_/a,
            high = D.upperDiagonal_/a;
        TridiagonalOperator result(low,mid,high);
        return result;
    }

    inline void swap(TridiagonalOperator& L1, TridiagonalOperator& L2) {
        L1.swap(L2);
    }

}


#endif
