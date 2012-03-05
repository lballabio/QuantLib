/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

/*! \file complexarray.hpp
    \brief 1-D array used in linear algebra.
*/

#ifndef quantlib_complex_array_hpp
#define quantlib_complex_array_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/utilities/disposable.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/scoped_array.hpp>
#include <functional>
#include <numeric>
#include <iomanip>
#include <complex>

namespace QuantLib {

    //! 1-D array used in linear algebra.
    /*! This class implements the concept of vector as used in linear
        algebra.
        As such, it is <b>not</b> meant to be used as a container -
        <tt>std::vector</tt> should be used instead.

        \test construction of arrays is checked in a number of cases
    */
    class ComplexArray {
      public:
        //! \name Constructors, destructor, and assignment
        //@{
        //! creates the array with the given dimension
        explicit ComplexArray(Size size = 0);
        //! creates the array and fills it with <tt>value</tt>
        ComplexArray(Size size, std::complex<Real>  value);
        /*! \brief creates the array and fills it according to
            \f$ a_{0} = value, a_{i}=a_{i-1}+increment \f$
        */
        ComplexArray(Size size, std::complex<Real>  value, std::complex<Real>  increment);
        ComplexArray(const ComplexArray&);
        ComplexArray(const Disposable<ComplexArray>&);
        ComplexArray& operator=(const ComplexArray&);
        ComplexArray& operator=(const Disposable<ComplexArray>&);
        //@}
        /*! \name Vector algebra

            <tt>v += x</tt> and similar operation involving a scalar value
            are shortcuts for \f$ \forall i : v_i = v_i + x \f$

            <tt>v *= w</tt> and similar operation involving two vectors are
            shortcuts for \f$ \forall i : v_i = v_i \times w_i \f$

            \pre all arrays involved in an algebraic expression must have
            the same size.
        */
        //@{
        const ComplexArray& operator+=(const ComplexArray&);
        const ComplexArray& operator+=(std::complex<Real> );
        const ComplexArray& operator-=(const ComplexArray&);
        const ComplexArray& operator-=(std::complex<Real> );
        const ComplexArray& operator*=(const ComplexArray&);
        const ComplexArray& operator*=(std::complex<Real> );
        const ComplexArray& operator/=(const ComplexArray&);
        const ComplexArray& operator/=(std::complex<Real> );
        //@}
        //! \name Element access
        //@{
        //! read-only
        std::complex<Real>  operator[](Size) const;
        std::complex<Real>  at(Size) const;
        std::complex<Real>  front() const;
        std::complex<Real>  back() const;
        //! read-write
        std::complex<Real> & operator[](Size);
        std::complex<Real> & at(Size);
        std::complex<Real> & front();
        std::complex<Real> & back();
        //@}
        //! \name Inspectors
        //@{
        //! dimension of the array
        Size size() const;
        //! whether the array is empty
        bool empty() const;
        //@}
        typedef std::complex<Real> * iterator;
        typedef const std::complex<Real> * const_iterator;
        typedef boost::reverse_iterator<iterator> reverse_iterator;
        typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;
        //! \name Iterator access
        //@{
        const_iterator begin() const;
        iterator begin();
        const_iterator end() const;
        iterator end();
        const_reverse_iterator rbegin() const;
        reverse_iterator rbegin();
        const_reverse_iterator rend() const;
        reverse_iterator rend();
        //@}
        //! \name Utilities
        //@{
        void swap(ComplexArray&);  // never throws
        //@}

      private:
        boost::scoped_array<std::complex<Real> > data_;
        Size n_;
    };

    /*! \relates ComplexArray */
    std::complex<Real>  DotProduct(const ComplexArray&, const ComplexArray&);

    // unary operators
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator+(const ComplexArray& v);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator-(const ComplexArray& v);

    // binary operators
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator+(const ComplexArray&, const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator+(const ComplexArray&, std::complex<Real> );
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator+(std::complex<Real> , const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator-(const ComplexArray&, const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator-(const ComplexArray&, std::complex<Real> );
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator-(std::complex<Real> , const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator*(const ComplexArray&, const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator*(const ComplexArray&, std::complex<Real> );
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator*(std::complex<Real> , const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator/(const ComplexArray&, const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator/(const ComplexArray&, std::complex<Real> );
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> operator/(std::complex<Real> , const ComplexArray&);

    // math functions
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> Abs(const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> Sqrt(const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> Log(const ComplexArray&);
    /*! \relates ComplexArray */
    const Disposable<ComplexArray> Exp(const ComplexArray&);

    // utilities
    /*! \relates ComplexArray */
    void swap(ComplexArray&, ComplexArray&);

    // format
    /*! \relates ComplexArray */
    std::ostream& operator<<(std::ostream&, const ComplexArray&);


    // inline definitions

    inline ComplexArray::ComplexArray(Size size)
    : data_(size ? new std::complex<Real> [size] : (std::complex<Real> *)(0)), n_(size) {}

    inline ComplexArray::ComplexArray(Size size, std::complex<Real>  value)
    : data_(size ? new std::complex<Real> [size] : (std::complex<Real> *)(0)), n_(size) {
        std::fill(begin(),end(),value);
    }

    inline ComplexArray::ComplexArray(Size size, std::complex<Real>  value, std::complex<Real>  increment)
    : data_(size ? new std::complex<Real> [size] : (std::complex<Real> *)(0)), n_(size) {
        for (iterator i=begin(); i!=end(); i++,value+=increment)
            *i = value;
    }

    inline ComplexArray::ComplexArray(const ComplexArray& from)
    : data_(from.n_ ? new std::complex<Real> [from.n_] : (std::complex<Real> *)(0)), n_(from.n_) {
        std::copy(from.begin(),from.end(),begin());
    }

    inline ComplexArray::ComplexArray(const Disposable<ComplexArray>& from)
    : data_((std::complex<Real> *)(0)), n_(0) {
        swap(const_cast<Disposable<ComplexArray>&>(from));
    }

    inline ComplexArray& ComplexArray::operator=(const ComplexArray& from) {
        // strong guarantee
        ComplexArray temp(from);
        swap(temp);
        return *this;
    }

    inline ComplexArray& ComplexArray::operator=(const Disposable<ComplexArray>& from) {
        swap(const_cast<Disposable<ComplexArray>&>(from));
        return *this;
    }

    inline const ComplexArray& ComplexArray::operator+=(const ComplexArray& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes (" << n_ << ", "
                   << v.n_ << ") cannot be added");
        std::transform(begin(),end(),v.begin(),begin(),
                       std::plus<std::complex<Real> >());
        return *this;
    }


    inline const ComplexArray& ComplexArray::operator+=(std::complex<Real>  x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::plus<std::complex<Real> >(),x));
        return *this;
    }

    inline const ComplexArray& ComplexArray::operator-=(const ComplexArray& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes (" << n_ << ", "
                   << v.n_ << ") cannot be subtracted");
        std::transform(begin(),end(),v.begin(),begin(),
                       std::minus<std::complex<Real> >());
        return *this;
    }

    inline const ComplexArray& ComplexArray::operator-=(std::complex<Real>  x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::minus<std::complex<Real> >(),x));
        return *this;
    }

    inline const ComplexArray& ComplexArray::operator*=(const ComplexArray& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes (" << n_ << ", "
                   << v.n_ << ") cannot be multiplied");
        std::transform(begin(),end(),v.begin(),begin(),
                       std::multiplies<std::complex<Real> >());
        return *this;
    }

    inline const ComplexArray& ComplexArray::operator*=(std::complex<Real>  x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::multiplies<std::complex<Real> >(),x));
        return *this;
    }

    inline const ComplexArray& ComplexArray::operator/=(const ComplexArray& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes (" << n_ << ", "
                   << v.n_ << ") cannot be divided");
        std::transform(begin(),end(),v.begin(),begin(),
                       std::divides<std::complex<Real> >());
        return *this;
    }

    inline const ComplexArray& ComplexArray::operator/=(std::complex<Real>  x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::divides<std::complex<Real> >(),x));
        return *this;
    }

    inline std::complex<Real>  ComplexArray::operator[](Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<n_,
                   "index (" << i << ") must be less than " << n_ <<
                   ": array access out of range");
        #endif
        return data_.get()[i];
    }

    inline std::complex<Real>  ComplexArray::at(Size i) const {
        QL_REQUIRE(i<n_,
                   "index (" << i << ") must be less than " << n_ <<
                   ": array access out of range");
        return data_.get()[i];
    }

    inline std::complex<Real>  ComplexArray::front() const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(n_>0, "null ComplexArray: array access out of range");
        #endif
        return data_.get()[0];
    }

    inline std::complex<Real>  ComplexArray::back() const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(n_>0, "null ComplexArray: array access out of range");
        #endif
        return data_.get()[n_-1];
    }

    inline std::complex<Real> & ComplexArray::operator[](Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<n_,
                   "index (" << i << ") must be less than " << n_ <<
                   ": array access out of range");
        #endif
        return data_.get()[i];
    }

    inline std::complex<Real> & ComplexArray::at(Size i) {
        QL_REQUIRE(i<n_,
                   "index (" << i << ") must be less than " << n_ <<
                   ": array access out of range");
        return data_.get()[i];
    }

    inline std::complex<Real> & ComplexArray::front() {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(n_>0, "null ComplexArray: array access out of range");
        #endif
        return data_.get()[0];
    }

    inline std::complex<Real> & ComplexArray::back() {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(n_>0, "null ComplexArray: array access out of range");
        #endif
        return data_.get()[n_-1];
    }

    inline Size ComplexArray::size() const {
        return n_;
    }

    inline bool ComplexArray::empty() const {
        return n_ == 0;
    }

    inline ComplexArray::const_iterator ComplexArray::begin() const {
        return data_.get();
    }

    inline ComplexArray::iterator ComplexArray::begin() {
        return data_.get();
    }

    inline ComplexArray::const_iterator ComplexArray::end() const {
        return data_.get()+n_;
    }

    inline ComplexArray::iterator ComplexArray::end() {
        return data_.get()+n_;
    }

    inline ComplexArray::const_reverse_iterator ComplexArray::rbegin() const {
        return const_reverse_iterator(end());
    }

    inline ComplexArray::reverse_iterator ComplexArray::rbegin() {
        return reverse_iterator(end());
    }

    inline ComplexArray::const_reverse_iterator ComplexArray::rend() const {
        return const_reverse_iterator(begin());
    }

    inline ComplexArray::reverse_iterator ComplexArray::rend() {
        return reverse_iterator(begin());
    }

    inline void ComplexArray::swap(ComplexArray& from) {
        using std::swap;
        data_.swap(from.data_);
        swap(n_,from.n_);
    }

    // dot product

    /*    inline std::complex<Real>  DotProduct(const ComplexArray& v1, const ComplexArray& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be multiplied");
        return std::inner_product(v1.begin(),v1.end(),v2.begin(),0.0);
        }*/

    // overloaded operators

    // unary

    inline const Disposable<ComplexArray> operator+(const ComplexArray& v) {
        ComplexArray result = v;
        return result;
    }

    inline const Disposable<ComplexArray> operator-(const ComplexArray& v) {
        ComplexArray result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::negate<std::complex<Real> >());
        return result;
    }


    // binary operators

    inline const Disposable<ComplexArray> operator+(const ComplexArray& v1,
                                             const ComplexArray& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be added");
        ComplexArray result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                       std::plus<std::complex<Real> >());
        return result;
    }

    inline const Disposable<ComplexArray> operator+(const ComplexArray& v1, std::complex<Real>  a) {
        ComplexArray result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::plus<std::complex<Real> >(),a));
        return result;
    }

    inline const Disposable<ComplexArray> operator+(std::complex<Real>  a, const ComplexArray& v2) {
        ComplexArray result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::plus<std::complex<Real> >(),a));
        return result;
    }

    inline const Disposable<ComplexArray> operator-(const ComplexArray& v1,
                                             const ComplexArray& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be subtracted");
        ComplexArray result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                       std::minus<std::complex<Real> >());
        return result;
    }

    inline const Disposable<ComplexArray> operator-(const ComplexArray& v1, std::complex<Real>  a) {
        ComplexArray result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::minus<std::complex<Real> >(),a));
        return result;
    }

    inline const Disposable<ComplexArray> operator-(std::complex<Real>  a, const ComplexArray& v2) {
        ComplexArray result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::minus<std::complex<Real> >(),a));
        return result;
    }

    inline const Disposable<ComplexArray> operator*(const ComplexArray& v1,
                                             const ComplexArray& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be multiplied");
        ComplexArray result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                       std::multiplies<std::complex<Real> >());
        return result;
    }

    inline const Disposable<ComplexArray> operator*(const ComplexArray& v1, std::complex<Real>  a) {
        ComplexArray result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::multiplies<std::complex<Real> >(),a));
        return result;
    }

    inline const Disposable<ComplexArray> operator*(std::complex<Real>  a, const ComplexArray& v2) {
        ComplexArray result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::multiplies<std::complex<Real> >(),a));
        return result;
    }

    inline const Disposable<ComplexArray> operator/(const ComplexArray& v1,
                                             const ComplexArray& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be divided");
        ComplexArray result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                       std::divides<std::complex<Real> >());
        return result;
    }

    inline const Disposable<ComplexArray> operator/(const ComplexArray& v1, std::complex<Real>  a) {
        ComplexArray result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::divides<std::complex<Real> >(),a));
        return result;
    }

    inline const Disposable<ComplexArray> operator/(std::complex<Real>  a, const ComplexArray& v2) {
        ComplexArray result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::divides<std::complex<Real> >(),a));
        return result;
    }

    // functions

    /*    inline const Disposable<ComplexArray> Abs(const ComplexArray& v) {
        ComplexArray result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<std::complex<Real> ,std::complex<Real> >(std::fabs));
        return result;
    }

    inline const Disposable<ComplexArray> Sqrt(const ComplexArray& v) {
        ComplexArray result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<std::complex<Real> ,std::complex<Real> >(std::sqrt));
        return result;
    }

    inline const Disposable<ComplexArray> Log(const ComplexArray& v) {
        ComplexArray result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<std::complex<Real> ,std::complex<Real> >(std::log));
        return result;
    }

    inline const Disposable<ComplexArray> Exp(const ComplexArray& v) {
        ComplexArray result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<std::complex<Real> ,std::complex<Real> >(std::exp));
        return result;
    }
    */
    inline void swap(ComplexArray& v, ComplexArray& w) {
        v.swap(w);
    }

    inline std::ostream& operator<<(std::ostream& out, const ComplexArray& a) {
        std::streamsize width = out.width();
        out << "[ ";
        if (!a.empty()) {
            for (Size n=0; n<a.size()-1; ++n)
                out << std::setw(width) << a[n] << "; ";
            out << std::setw(width) << a.back();
        }
        out << " ]";
        return out;
    }

}


#endif
