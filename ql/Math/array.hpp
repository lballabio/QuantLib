
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
        Array(Size size, Real value);
        /*! \brief creates the array and fills it according to
            \f$ a_{0} = value, a_{i}=a_{i-1}+increment \f$
        */
        Array(Size size, Real value, Real increment);
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
        const Array& operator+=(Real);
        const Array& operator-=(const Array&);
        const Array& operator-=(Real);
        const Array& operator*=(const Array&);
        const Array& operator*=(Real);
        const Array& operator/=(const Array&);
        const Array& operator/=(Real);
        //@}
        //! \name Element access
        //@{
        //! read-only
        Real operator[](Size) const;
        //! read-write
        Real& operator[](Size);
        //@}
        //! \name Inspectors
        //@{
        //! dimension of the array
        Size size() const;
        //@}
        typedef Real* iterator;
        typedef const Real* const_iterator;
        typedef QL_REVERSE_ITERATOR(iterator,Real) reverse_iterator;
        typedef QL_REVERSE_ITERATOR(const_iterator,Real)
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
        Real* pointer_;
        Size n_, bufferSize_;
    };

    /*! \relates Array */
    Real DotProduct(const Array&, const Array&);

    // unary operators
    /*! \relates Array */
    const Disposable<Array> operator+(const Array& v);
    /*! \relates Array */
    const Disposable<Array> operator-(const Array& v);

    // binary operators
    /*! \relates Array */
    const Disposable<Array> operator+(const Array&, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator+(const Array&, Real);
    /*! \relates Array */
    const Disposable<Array> operator+(Real, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator-(const Array&, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator-(const Array&, Real);
    /*! \relates Array */
    const Disposable<Array> operator-(Real, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator*(const Array&, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator*(const Array&, Real);
    /*! \relates Array */
    const Disposable<Array> operator*(Real, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator/(const Array&, const Array&);
    /*! \relates Array */
    const Disposable<Array> operator/(const Array&, Real);
    /*! \relates Array */
    const Disposable<Array> operator/(Real, const Array&);

    // math functions
    /*! \relates Array */
    const Disposable<Array> Abs(const Array&);
    /*! \relates Array */
    const Disposable<Array> Sqrt(const Array&);
    /*! \relates Array */
    const Disposable<Array> Log(const Array&);
    /*! \relates Array */
    const Disposable<Array> Exp(const Array&);


    //! format arrays for output
    class ArrayFormatter {
      public:
        static std::string toString(const Array& a,
                                    Integer precision = 6,
                                    Integer digits = 0,
                                    Size elementsPerRow = QL_MAX_INTEGER) {
            #ifndef QL_PATCH_MSVC6
            return SequenceFormatter::toString(a.begin(),a.end(),
                                               precision,digits,
                                               elementsPerRow);
            #else
            std::string s = "[ ";
            for (Size n=0; n<a.size(); ++n) {
                if (n == elementsPerRow) {
                    s += ";\n  ";
                    n = 0;
                }
                if (n!=0)
                    s += " ; ";
                s += DecimalFormatter::toString(a[n], precision, digits);
            }
            s += " ]";
            return s;
            #endif
        }
    };


    // inline definitions

    inline Array::Array(Size size)
    : pointer_(0), n_(0), bufferSize_(0) {
        if (size > 0)
            allocate(size);
    }

    inline Array::Array(Size size, Real value)
    : pointer_(0), n_(0), bufferSize_(0) {
        if (size > 0)
            allocate(size);
        std::fill(begin(),end(),value);
    }

    inline Array::Array(Size size, Real value, Real increment)
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
                       std::plus<Real>());
        return *this;
    }

    inline const Array& Array::operator+=(Real x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::plus<Real>(),x));
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
                       std::minus<Real>());
        return *this;
    }

    inline const Array& Array::operator-=(Real x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::minus<Real>(),x));
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
                       std::multiplies<Real>());
        return *this;
    }

    inline const Array& Array::operator*=(Real x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::multiplies<Real>(),x));
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
                       std::divides<Real>());
        return *this;
    }

    inline const Array& Array::operator/=(Real x) {
        std::transform(begin(),end(),begin(),
                       std::bind2nd(std::divides<Real>(),x));
        return *this;
    }

    inline Real Array::operator[](Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<n_,
                   "array cannot be accessed out of range");
        #endif
        return pointer_[i];
    }

    inline Real& Array::operator[](Size i) {
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
                pointer_ = new Real[bufferSize_];
            }
            catch (...) {
                pointer_ = 0;
            }
            if (pointer_ == 0) {
                n_ = bufferSize_ = size;
                try {
                    pointer_ = new Real[bufferSize_];
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

    inline Real DotProduct(const Array& v1, const Array& v2) {
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
                       std::negate<Real>());
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
                       std::plus<Real>());
        return result;
    }

    inline const Disposable<Array> operator+(const Array& v1, Real a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::plus<Real>(),a));
        return result;
    }

    inline const Disposable<Array> operator+(Real a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::plus<Real>(),a));
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
                       std::minus<Real>());
        return result;
    }

    inline const Disposable<Array> operator-(const Array& v1, Real a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::minus<Real>(),a));
        return result;
    }

    inline const Disposable<Array> operator-(Real a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::minus<Real>(),a));
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
                       std::multiplies<Real>());
        return result;
    }

    inline const Disposable<Array> operator*(const Array& v1, Real a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::multiplies<Real>(),a));
        return result;
    }

    inline const Disposable<Array> operator*(Real a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::multiplies<Real>(),a));
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
                       std::divides<Real>());
        return result;
    }

    inline const Disposable<Array> operator/(const Array& v1, Real a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(),
                       std::bind2nd(std::divides<Real>(),a));
        return result;
    }

    inline const Disposable<Array> operator/(Real a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(),
                       std::bind1st(std::divides<Real>(),a));
        return result;
    }

    // functions

    inline const Disposable<Array> Abs(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<Real,Real>(QL_FABS));
        return result;
    }

    inline const Disposable<Array> Sqrt(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<Real,Real>(QL_SQRT));
        return result;
    }

    inline const Disposable<Array> Log(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<Real,Real>(QL_LOG));
        return result;
    }

    inline const Disposable<Array> Exp(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       std::ptr_fun<Real,Real>(QL_EXP));
        return result;
    }

}


#endif
