
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email
 quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file array.hpp
    \brief 1-D array used in linear algebra.
*/

#ifndef quantlib_array_h
#define quantlib_array_h

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/disposable.hpp>
#include <ql/basicdataformatters.hpp>
#include <functional>
#include <numeric>

namespace QuantLib {

    //! 1-D array used in linear algebra.
    /*! This class implements the concept of vector as used in linear
        algebra.
        As such, it is <b>not</b> meant to be used as a container -
        <tt>std::vector</tt> should be used instead.
    */
    class Array {
      public:
        //! \name Constructors, destructor, and assignment
        //@{
        //! creates the array with the given dimension
        explicit Array(Size size = 0);
        //! creates the array and fills it with <tt>value</tt>
        Array(Size size, double value);
        /*! \brief creates the array and fills it according to
            \f$ a_{0} = value, a_{i}=a_{i-1}+increment \f$
        */
        Array(Size size, double value, double increment);
        Array(const Array&);
        Array(const Disposable<Array>&);
        ~Array();
        Array& operator=(const Array&);
        Array& operator=(const Disposable<Array>&);
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
        const Array& operator+=(const Array&);
        const Array& operator+=(double);
        const Array& operator-=(const Array&);
        const Array& operator-=(double);
        const Array& operator*=(const Array&);
        const Array& operator*=(double);
        const Array& operator/=(const Array&);
        const Array& operator/=(double);
        //@}
        //! \name Element access
        //@{
        //! read-only
        double operator[](Size) const;
        //! read-write
        double& operator[](Size);
        //@}
        //! \name Inspectors
        //@{
        //! dimension of the array
        Size size() const;
        //@}
        typedef double* iterator;
        typedef const double* const_iterator;
        typedef QL_REVERSE_ITERATOR(iterator,double) reverse_iterator;
        typedef QL_REVERSE_ITERATOR(const_iterator,double)
            const_reverse_iterator;
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
        void swap(Array&);
        //@}
      private:
        void allocate(Size size);
        void resize(Size size);
        void copy(const Array&);
        double* pointer_;
        Size n_, bufferSize_;
    };

    /*! \relates Array */
    double DotProduct(const Array&, const Array&);

    // unary operators
    /*! \relates Array */
    const Disposable<Array> operator+(const Array& v);
    /*! \relates Array */
    const Disposable<Array> operator-(const Array& v);

    // binary operators
    /*! \relates Array */
    const Disposable<Array> operator+(const Array&, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator+(const Array&, double);
    /*! \relates Array */
    const Disposable<Array> operator+(double, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator-(const Array&, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator-(const Array&, double);
    /*! \relates Array */
    const Disposable<Array> operator-(double, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator*(const Array&, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator*(const Array&, double);
    /*! \relates Array */
    const Disposable<Array> operator*(double, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator/(const Array&, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator/(const Array&, double);
    /*! \relates Array */
    const Disposable<Array> operator/(double, const Array&);

    // math functions
    /*! \relates Array */
    const Disposable<Array> Abs(const Array&);
    /*! \relates Array */
    const Disposable<Array> Sqrt(const Array&);
    /*! \relates Array */
    const Disposable<Array> Log(const Array&);
    /*! \relates Array */
    const Disposable<Array> Exp(const Array&);

    // inline definitions

    inline Array::Array(Size size)
    : pointer_(0), n_(0), bufferSize_(0) {
        if (size > 0)
            allocate(size);
    }

    inline Array::Array(Size size, double value)
    : pointer_(0), n_(0), bufferSize_(0) {
        if (size > 0)
            allocate(size);
        std::fill(begin(),end(),value);
    }

    inline Array::Array(Size size, double value, double increment)
    : pointer_(0), n_(0), bufferSize_(0) {
        if (size > 0)
            allocate(size);
        for (iterator i=begin(); i!=end(); i++,value+=increment)
            *i = value;
    }

    inline Array::Array(const Array& from)
    : pointer_(0), n_(0), bufferSize_(0) {
        allocate(from.size());
        copy(from);
    }

    inline Array::Array(const Disposable<Array>& from)
    : pointer_(0), n_(0), bufferSize_(0) {
        swap(const_cast<Disposable<Array>&>(from));
    }

    inline Array::~Array() {
        if (pointer_ != 0 && bufferSize_ != 0)
            delete[] pointer_;
        pointer_ = 0;
        n_ = bufferSize_ = 0;
    }

    inline Array& Array::operator=(const Array& from) {
        if (this != &from) {
            resize(from.size());
            copy(from);
        }
        return *this;
    }

    inline Array& Array::operator=(const Disposable<Array>& from) {
        swap(const_cast<Disposable<Array>&>(from));
        return *this;
    }

    inline const Array& Array::operator+=(const Array& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes ("
                   + SizeFormatter::toString(n_) +
                   ", "
                   + SizeFormatter::toString(v.n_) +
                   ") cannot be added");
        std::transform(begin(),end(),v.begin(),begin(),
                       std::plus<double>());
        return *this;
    }

    inline const Array& Array::operator+=(double x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::plus<double>(),x));
        return *this;
    }

    inline const Array& Array::operator-=(const Array& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes ("
                   + SizeFormatter::toString(n_) +
                   ", "
                   + SizeFormatter::toString(v.n_) +
                   ") cannot be subtracted");
        std::transform(begin(),end(),v.begin(),begin(),
                       std::minus<double>());
        return *this;
    }

    inline const Array& Array::operator-=(double x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::minus<double>(),x));
        return *this;
    }

    inline const Array& Array::operator*=(const Array& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes ("
                   + SizeFormatter::toString(n_) +
                   ", "
                   + SizeFormatter::toString(v.n_) +
                   ") cannot be multiplied");
        std::transform(begin(),end(),v.begin(),begin(),
                       std::multiplies<double>());
        return *this;
    }

    inline const Array& Array::operator*=(double x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::multiplies<double>(),x));
        return *this;
    }

    inline const Array& Array::operator/=(const Array& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes ("
                   + SizeFormatter::toString(n_) +
                   ", "
                   + SizeFormatter::toString(v.n_) +
                   ") cannot be divided");
        std::transform(begin(),end(),v.begin(),begin(),
                       std::divides<double>());
        return *this;
    }

    inline const Array& Array::operator/=(double x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::divides<double>(),x));
        return *this;
    }

    inline double Array::operator[](Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<n_,
                   "array cannot be accessed out of range");
        #endif
        return pointer_[i];
    }

    inline double& Array::operator[](Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<n_,
                   "array cannot be accessed out of range");
        #endif
        return pointer_[i];
    }

    inline Size Array::size() const {
        return n_;
    }

    inline Array::const_iterator Array::begin() const {
        return pointer_;
    }

    inline Array::iterator Array::begin() {
        return pointer_;
    }

    inline Array::const_iterator Array::end() const {
        return pointer_+n_;
    }

    inline Array::iterator Array::end() {
        return pointer_+n_;
    }

    inline Array::const_reverse_iterator Array::rbegin() const {
        return const_reverse_iterator(end());
    }

    inline Array::reverse_iterator Array::rbegin() {
        return reverse_iterator(end());
    }

    inline Array::const_reverse_iterator Array::rend() const {
        return const_reverse_iterator(begin());
    }

    inline Array::reverse_iterator Array::rend() {
        return reverse_iterator(begin());
    }

    inline void Array::swap(Array& from) {
        std::swap(pointer_,from.pointer_);
        std::swap(n_,from.n_);
        std::swap(bufferSize_,from.bufferSize_);
    }

    inline void Array::allocate(Size size) {
        if (pointer_ != 0 && bufferSize_ != 0)
            delete[] pointer_;
        if (size <= 0) {
            pointer_ = 0;
        } else {
            n_ = size;
            bufferSize_ = size+size/10+10;
            try {
                pointer_ = new double[bufferSize_];
            }
            catch (...) {
                pointer_ = 0;
            }
            if (pointer_ == 0) {
                n_ = bufferSize_ = size;
                try {
                    pointer_ = new double[bufferSize_];
                }
                catch (...) {
                    pointer_ = 0;
                }
                if (pointer_ == 0) {
                    n_ = bufferSize_ = 0;
                    QL_FAIL("out of memory");
                }
            }
        }
    }

    inline void Array::resize(Size size) {
        if (size != n_) {
            if (size <= bufferSize_) {
                n_ = size;
            } else {
                Array temp(size);
                std::copy(begin(),end(),temp.begin());
                allocate(size);
                copy(temp);
            }
        }
    }

    inline void Array::copy(const Array& from) {
        std::copy(from.begin(),from.end(),begin());
    }

    // dot product

    inline double DotProduct(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes ("
                   + SizeFormatter::toString(v1.size()) +
                   ", "
                   + SizeFormatter::toString(v2.size()) +
                   ") cannot be multiplied");
        return std::inner_product(v1.begin(),v1.end(),v2.begin(),0.0);
    }

    // overloaded operators

    // unary

    inline const Disposable<Array> operator+(const Array& v) {
        Array result = v;
        return result;
    }

    inline const Disposable<Array> operator-(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::negate<double>());
        return result;
    }


    // binary operators

    inline const Disposable<Array> operator+(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes ("
                   + SizeFormatter::toString(v1.size()) +
                   ", "
                   + SizeFormatter::toString(v2.size()) +
                   ") cannot be added");
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                       std::plus<double>());
        return result;
    }

    inline const Disposable<Array> operator+(const Array& v1, double a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::plus<double>(),a));
        return result;
    }

    inline const Disposable<Array> operator+(double a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::plus<double>(),a));
        return result;
    }

    inline const Disposable<Array> operator-(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes ("
                   + SizeFormatter::toString(v1.size()) +
                   ", "
                   + SizeFormatter::toString(v2.size()) +
                   ") cannot be subtracted");
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                       std::minus<double>());
        return result;
    }

    inline const Disposable<Array> operator-(const Array& v1, double a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::minus<double>(),a));
        return result;
    }

    inline const Disposable<Array> operator-(double a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::minus<double>(),a));
        return result;
    }

    inline const Disposable<Array> operator*(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes ("
                   + SizeFormatter::toString(v1.size()) +
                   ", "
                   + SizeFormatter::toString(v2.size()) +
                   ") cannot be multiplied");
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                       std::multiplies<double>());
        return result;
    }

    inline const Disposable<Array> operator*(const Array& v1, double a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::multiplies<double>(),a));
        return result;
    }

    inline const Disposable<Array> operator*(double a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::multiplies<double>(),a));
        return result;
    }

    inline const Disposable<Array> operator/(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes ("
                   + SizeFormatter::toString(v1.size()) +
                   ", "
                   + SizeFormatter::toString(v2.size()) +
                   ") cannot be divided");
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                       std::divides<double>());
        return result;
    }

    inline const Disposable<Array> operator/(const Array& v1, double a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::divides<double>(),a));
        return result;
    }

    inline const Disposable<Array> operator/(double a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::divides<double>(),a));
        return result;
    }

    // functions

    inline const Disposable<Array> Abs(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<double,double>(QL_FABS));
        return result;
    }

    inline const Disposable<Array> Sqrt(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<double,double>(QL_SQRT));
        return result;
    }

    inline const Disposable<Array> Log(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<double,double>(QL_LOG));
        return result;
    }

    inline const Disposable<Array> Exp(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<double,double>(QL_EXP));
        return result;
    }

}


#endif
