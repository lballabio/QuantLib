/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2009 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file array.hpp
    \brief 1-D array used in linear algebra.
*/

#ifndef quantlib_array_hpp
#define quantlib_array_hpp

#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <iterator>
#include <functional>
#include <algorithm>
#include <numeric>
#include <vector>
#include <initializer_list>
#include <memory>
#include <type_traits>

namespace QuantLib {

    //! 1-D array used in linear algebra.
    /*! This class implements the concept of vector as used in linear
        algebra.
        As such, it is <b>not</b> meant to be used as a container -
        <tt>std::vector</tt> should be used instead.

        \test construction of arrays is checked in a number of cases
    */
    class Array {
      public:
        //! \name Constructors, destructor, and assignment
        //@{
        //! creates the array with size 0
        Array() : Array(static_cast<Size>(0)) {}
        //! creates the array with the given dimension
        explicit Array(Size size);
        //! creates the array and fills it with <tt>value</tt>
        Array(Size size, Real value);
        /*! \brief creates the array and fills it according to
            \f$ a_{0} = value, a_{i}=a_{i-1}+increment \f$
        */
        Array(Size size, Real value, Real increment);
        Array(const Array&);
        Array(Array&&) noexcept;
        Array(std::initializer_list<Real>);
        //! creates the array from an iterable sequence
        template <class ForwardIterator>
        Array(ForwardIterator begin, ForwardIterator end);
        ~Array() = default;

        Array& operator=(const Array&);
        Array& operator=(Array&&) noexcept;

        bool operator==(const Array&) const;
        bool operator!=(const Array&) const;
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
        Real at(Size) const;
        Real front() const;
        Real back() const;
        //! read-write
        Real& operator[](Size);
        Real& at(Size);
        Real& front();
        Real& back();
        //@}
        //! \name Inspectors
        //@{
        //! dimension of the array
        Size size() const;
        //! whether the array is empty
        bool empty() const;
        //@}
        typedef Size size_type;
        typedef Real value_type;
        typedef Real* iterator;
        typedef const Real* const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
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
        void resize(Size n);
        void swap(Array&) noexcept;
        //@}

      private:
        std::unique_ptr<Real[]> data_;
        Size n_;
    };

    #ifdef QL_NULL_AS_FUNCTIONS

    //! specialization of null template for this class
    template <>
    inline Array Null<Array>() {
        return {};
    }

    #else

    //! specialization of null template for this class
    template <>
    class Null<Array> {
      public:
        Null() = default;
        operator Array() const { return Array(); }
    };

    #endif

    /*! \relates Array */
    Real DotProduct(const Array&, const Array&);

    /*! \relates Array */
    Real Norm2(const Array&);

    // unary operators
    /*! \relates Array */
    Array operator+(const Array& v);
    /*! \relates Array */
    Array operator+(Array&& v);
    /*! \relates Array */
    Array operator-(const Array& v);
    /*! \relates Array */
    Array operator-(Array&& v);

    // binary operators
    /*! \relates Array */
    Array operator+(const Array&, const Array&);
    /*! \relates Array */
    Array operator+(const Array&, Array&&);
    /*! \relates Array */
    Array operator+(Array&&, const Array&);
    /*! \relates Array */
    Array operator+(Array&&, Array&&);
    /*! \relates Array */
    Array operator+(const Array&, Real);
    /*! \relates Array */
    Array operator+(Array&&, Real);
    /*! \relates Array */
    Array operator+(Real, const Array&);
    /*! \relates Array */
    Array operator+(Real, Array&&);
    /*! \relates Array */
    Array operator-(const Array&, const Array&);
    /*! \relates Array */
    Array operator-(const Array&, Array&&);
    /*! \relates Array */
    Array operator-(Array&&, const Array&);
    /*! \relates Array */
    Array operator-(Array&&, Array&&);
    /*! \relates Array */
    Array operator-(const Array&, Real);
    /*! \relates Array */
    Array operator-(Real, const Array&);
    /*! \relates Array */
    Array operator-(Array&&, Real);
    /*! \relates Array */
    Array operator-(Real, Array&&);
    /*! \relates Array */
    Array operator*(const Array&, const Array&);
    /*! \relates Array */
    Array operator*(const Array&, Array&&);
    /*! \relates Array */
    Array operator*(Array&&, const Array&);
    /*! \relates Array */
    Array operator*(Array&&, Array&&);
    /*! \relates Array */
    Array operator*(const Array&, Real);
    /*! \relates Array */
    Array operator*(Real, const Array&);
    /*! \relates Array */
    Array operator*(Array&&, Real);
    /*! \relates Array */
    Array operator*(Real, Array&&);
    /*! \relates Array */
    Array operator/(const Array&, const Array&);
    /*! \relates Array */
    Array operator/(const Array&, Array&&);
    /*! \relates Array */
    Array operator/(Array&&, const Array&);
    /*! \relates Array */
    Array operator/(Array&&, Array&&);
    /*! \relates Array */
    Array operator/(const Array&, Real);
    /*! \relates Array */
    Array operator/(Real, const Array&);
    /*! \relates Array */
    Array operator/(Array&&, Real);
    /*! \relates Array */
    Array operator/(Real, Array&&);

    // math functions
    /*! \relates Array */
    Array Abs(const Array&);
    /*! \relates Array */
    Array Abs(Array&&);
    /*! \relates Array */
    Array Sqrt(const Array&);
    /*! \relates Array */
    Array Sqrt(Array&&);
    /*! \relates Array */
    Array Log(const Array&);
    /*! \relates Array */
    Array Log(Array&&);
    /*! \relates Array */
    Array Exp(const Array&);
    /*! \relates Array */
    Array Exp(Array&&);
    /*! \relates Array */
    Array Pow(const Array&, Real);
    /*! \relates Array */
    Array Pow(Array&&, Real);

    // utilities
    /*! \relates Array */
    void swap(Array&, Array&) noexcept;

    // format
    /*! \relates Array */
    std::ostream& operator<<(std::ostream&, const Array&);

    namespace detail {

        template <class I>
        void _fill_array_(Array& a,
                                 std::unique_ptr<Real[]>& data_,
                                 Size& n_,
                                 I begin, I end,
                                 const std::true_type&) {
            // we got redirected here from a call like Array(3, 4)
            // because it matched the constructor below exactly with
            // ForwardIterator = int.  What we wanted was fill an
            // Array with a given value, which we do here.
            Size n = begin;
            Real value = end;
            data_.reset(n ? new Real[n] : (Real*)nullptr);
            n_ = n;
            std::fill(a.begin(),a.end(),value);
        }

        template <class I>
        void _fill_array_(Array& a,
                                 std::unique_ptr<Real[]>& data_,
                                 Size& n_,
                                 I begin, I end,
                                 const std::false_type&) {
            // true iterators
            Size n = std::distance(begin, end);
            data_.reset(n ? new Real[n] : (Real*)nullptr);
            n_ = n;
            #if defined(QL_PATCH_MSVC) && defined(QL_DEBUG)
            if (n_)
            #endif
            std::copy(begin, end, a.begin());
        }

    }

    /*! \relates Array */
    template <class ForwardIterator>
    Array::Array(ForwardIterator begin, ForwardIterator end) {
        // Unfortunately, calls such as Array(3, 4) match this constructor.
        // We have to detect integral types and dispatch.
        detail::_fill_array_(*this, data_, n_, begin, end,
                             std::is_integral<ForwardIterator>());
    }

}


#endif
