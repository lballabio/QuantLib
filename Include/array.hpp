
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*
    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file array.hpp
    \brief 1-D array used in linear algebra.
*/

#ifndef quantlib_array_h
#define quantlib_array_h

#include "qldefines.hpp"
#include "qlerrors.hpp"
#if QL_EXPRESSION_TEMPLATES_WORK
#include "expressiontemplates.hpp"
#endif
#include <algorithm>
#include <functional>
#include <numeric>
#include <iterator>

namespace QuantLib {

    //! 1-D array used in linear algebra.
    /*! This class implements the concept of vector as used in linear algebra.
        As such, it is <b>not</b> meant to be used as a container -
        <tt>std::vector</tt> should be used instead.
    */
    class Array {
      public:
        //! \name Constructors, destructor, and assignment
        //@{
        //! creates the array with the given dimension
        explicit Array(int size = 0);
        //! creates the array and fills it with <tt>value</tt>
        Array(int size, double value);
        /*! creates the array and fills it according to \f$ a_{0} = value,
            a_{i}=a_{i-1}+increment \f$
        */
        Array(int size, double value, double increment);
        Array(const Array& from);
        #if QL_EXPRESSION_TEMPLATES_WORK
        template <class Iter> Array(const VectorialExpression<Iter>& e)
        : pointer(0), n(0), bufferSize(0) { allocate(e.size()); copy(e); }
        #endif
        ~Array();
        Array& operator=(const Array& from);
        #if QL_EXPRESSION_TEMPLATES_WORK
        template <class Iter> Array& operator=(
          const VectorialExpression<Iter>& e) {
            resize(e.size()); copy(e); return *this;
        }
        #endif
        //@}
        /*! \name Vector algebra

            <tt>v += x</tt> and similar operation involving a scalar value
            are shortcuts for \f$ \forall i : v_i = v_i + x \f$

            <tt>v *= w</tt> and similar operation involving two vectors are
            shortcuts for \f$ \forall i : v_i = v_i \times w_i \f$

            This implementation was inspired by T. L. Veldhuizen,
            <i>Expression templates</i>, C++ Report, 7(5):26-31, June 1995
            available at http://extreme.indiana.edu/~tveldhui/papers/

            \pre all arrays involved in an algebraic expression must have
            the same size.
        */
        //@{
        Array& operator+=(const Array&);
        Array& operator+=(double);
        Array& operator-=(const Array&);
        Array& operator-=(double);
        Array& operator*=(const Array&);
        Array& operator*=(double);
        Array& operator/=(const Array&);
        Array& operator/=(double);
        #if QL_EXPRESSION_TEMPLATES_WORK
        template <class Iter> Array& operator+=(
          const VectorialExpression<Iter>& e) {
            #ifdef QL_DEBUG
                QL_REQUIRE(size() == e.size(),
                    "adding arrays with different sizes")
            #endif
            iterator i = begin(), j = end();
            while (i != j) { *i += *e; ++i; ++e; }
            return *this;
        }
        template <class Iter> Array& operator-=(
          const VectorialExpression<Iter>& e) {
            #ifdef QL_DEBUG
                QL_REQUIRE(size() == e.size(),
                    "subtracting arrays with different sizes")
            #endif
            iterator i = begin(), j = end();
            while (i != j) { *i -= *e; ++i; ++e; }
            return *this;
        }
        template <class Iter> Array& operator*=(
          const VectorialExpression<Iter>& e) {
            #ifdef QL_DEBUG
                QL_REQUIRE(size() == e.size(),
                    "multiplying arrays with different sizes")
            #endif
            iterator i = begin(), j = end();
            while (i != j) { *i *= *e; ++i; ++e; }
            return *this;
        }
        template <class Iter> Array& operator/=(
          const VectorialExpression<Iter>& e) {
            #ifdef QL_DEBUG
                QL_REQUIRE(size() == e.size(),
                    "dividing arrays with different sizes")
            #endif
            iterator i = begin(), j = end();
            while (i != j) { *i /= *e; ++i; ++e; }
            return *this;
        }
        #endif
        //@}
        //! \name Element access
        //@{
        //! read-only
        double operator[](int) const;
        //! read-write
        double& operator[](int);
        //@}
        //! \name Inspectors
        //@{
        //! dimension of the array
        int size() const;
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
      private:
        void allocate(int size);
        void resize(int size);
        void copy(const Array& from) {
            std::copy(from.begin(),from.end(),begin());
        }
        #if QL_EXPRESSION_TEMPLATES_WORK
        template <class Iter> void copy(
          const VectorialExpression<Iter>& e) {
            iterator i = begin(), j = end();
            while (i != j) {
                *i = *e;
                ++i; ++e;
            }
        }
        #endif
        double* pointer;
        int n, bufferSize;
    };

    /*! \relates Array */
    double DotProduct(const Array&, const Array&);

    // unary operators

    #if QL_EXPRESSION_TEMPLATES_WORK
        // unary plus

        /*! \relates Array */
        VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,Plus> >
        operator+(const Array& v);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,Plus> >
        operator+(const VectorialExpression<Iter1>& e);

        // unary minus
        /*! \relates Array */
        VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,Minus> >
        operator-(const Array& v);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,Minus> >
        operator-(const VectorialExpression<Iter1>& e);
    #else
        /*! \relates Array */
        Array operator+(const Array& v);
        /*! \relates Array */
        Array operator-(const Array& v);
    #endif

    // binary operators
    #if QL_EXPRESSION_TEMPLATES_WORK

        // addition

        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Array::const_iterator,Add> >
        operator+(const Array& v1, const Array& v2);
        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<Array::const_iterator,Scalar,Add> >
        operator+(const Array& v1, double x);
        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<Scalar,Array::const_iterator,Add> >
        operator+(double x, const Array& v2);
        /*! \relates Array */
        template <class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,VectorialExpression<Iter2>,Add> >
        operator+(const Array& v1, const VectorialExpression<Iter2>& e2);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,Array::const_iterator,Add> >
        operator+(const VectorialExpression<Iter1>& e1, const Array& v2);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Add> >
        operator+(const VectorialExpression<Iter1>& e1, double x);
        /*! \relates Array */
        template <class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Add> >
        operator+(double x, const VectorialExpression<Iter2>& e2);
        /*! \relates Array */
        template <class Iter1, class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,VectorialExpression<Iter2>,Add> >
        operator+(const VectorialExpression<Iter1>& e1,
            const VectorialExpression<Iter2>& e2);

        // subtraction

        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Array::const_iterator,Subtract> >
        operator-(const Array& v1, const Array& v2);
        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<Array::const_iterator,Scalar,Subtract> >
        operator-(const Array& v1, double x);
        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<
        Scalar,Array::const_iterator,Subtract> >
        operator-(double x, const Array& v2);
        /*! \relates Array */
        template <class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,VectorialExpression<Iter2>,Subtract> >
        operator-(const Array& v1, const VectorialExpression<Iter2>& e2);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,Array::const_iterator,Subtract> >
        operator-(const VectorialExpression<Iter1>& e1, const Array& v2);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Subtract> >
        operator-(const VectorialExpression<Iter1>& e1, double x);
        /*! \relates Array */
        template <class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Subtract> >
        operator-(double x, const VectorialExpression<Iter2>& e2);
        /*! \relates Array */
        template <class Iter1, class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,VectorialExpression<Iter2>,Subtract> >
        operator-(const VectorialExpression<Iter1>& e1,
            const VectorialExpression<Iter2>& e2);

        // multiplication

        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Array::const_iterator,Multiply> >
        operator*(const Array& v1, const Array& v2);
        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<Array::const_iterator,Scalar,Multiply> >
        operator*(const Array& v1, double x);
        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<Scalar,Array::const_iterator,Multiply> >
        operator*(double x, const Array& v2);
        /*! \relates Array */
        template <class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,VectorialExpression<Iter2>,Multiply> >
        operator*(const Array& v1, const VectorialExpression<Iter2>& e2);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,Array::const_iterator,Multiply> >
        operator*(const VectorialExpression<Iter1>& e1, const Array& v2);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Multiply> >
        operator*(const VectorialExpression<Iter1>& e1, double x);
        /*! \relates Array */
        template <class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Multiply> >
        operator*(double x, const VectorialExpression<Iter2>& e2);
        /*! \relates Array */
        template <class Iter1, class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,VectorialExpression<Iter2>,Multiply> >
        operator*(const VectorialExpression<Iter1>& e1,
            const VectorialExpression<Iter2>& e2);

        // division

        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Array::const_iterator,Divide> >
        operator/(const Array& v1, const Array& v2);
        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Scalar,Divide> >
        operator/(const Array& v1, double x);
        /*! \relates Array */
        VectorialExpression<
        BinaryVectorialExpression<Scalar,Array::const_iterator,Divide> >
        operator/(double x, const Array& v2);
        /*! \relates Array */
        template <class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,VectorialExpression<Iter2>,Divide> >
        operator/(const Array& v1, const VectorialExpression<Iter2>& e2);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,Array::const_iterator,Divide> >
        operator/(const VectorialExpression<Iter1>& e1, const Array& v2);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Divide> >
        operator/(const VectorialExpression<Iter1>& e1, double x);
        /*! \relates Array */
        template <class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Divide> >
        operator/(double x, const VectorialExpression<Iter2>& e2);
        /*! \relates Array */
        template <class Iter1, class Iter2>
        VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,VectorialExpression<Iter2>,Divide> >
        operator/(const VectorialExpression<Iter1>& e1,
            const VectorialExpression<Iter2>& e2);
    #else
        /*! \relates Array */
        Array operator+(const Array&, const Array&);
        /*! \relates Array */
        Array operator+(const Array&, double);
        /*! \relates Array */
        Array operator+(double, const Array&);
        /*! \relates Array */
        Array operator-(const Array&, const Array&);
        /*! \relates Array */
        Array operator-(const Array&, double);
        /*! \relates Array */
        Array operator-(double, const Array&);
        /*! \relates Array */
        Array operator*(const Array&, const Array&);
        /*! \relates Array */
        Array operator*(const Array&, double);
        /*! \relates Array */
        Array operator*(double, const Array&);
        /*! \relates Array */
        Array operator/(const Array&, const Array&);
        /*! \relates Array */
        Array operator/(const Array&, double);
        /*! \relates Array */
        Array operator/(double, const Array&);
    #endif

    // math functions

    #if QL_EXPRESSION_TEMPLATES_WORK
        /*! \relates Array */
        VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,AbsoluteValue> >
        Abs(const Array& v);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,AbsoluteValue> >
        Abs(const VectorialExpression<Iter1>& e);
        /*! \relates Array */
        VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,SquareRoot> >
        Sqrt(const Array& v);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,SquareRoot> >
        Sqrt(const VectorialExpression<Iter1>& e);
        /*! \relates Array */
        VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,Logarithm> >
        Log(const Array& v);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,Logarithm> >
        Log(const VectorialExpression<Iter1>& e);
        /*! \relates Array */
        VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,Exponential> >
        Exp(const Array& v);
        /*! \relates Array */
        template <class Iter1>
        VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,Exponential> >
        Exp(const VectorialExpression<Iter1>& e);
    #else
        /*! \relates Array */
        Array Abs(const Array&);
        /*! \relates Array */
        Array Sqrt(const Array&);
        /*! \relates Array */
        Array Log(const Array&);
        /*! \relates Array */
        Array Exp(const Array&);
    #endif

    // inline definitions

    inline Array::Array(int size)
    : pointer(0), n(0), bufferSize(0) {
        if (size > 0)
            allocate(size);
    }

    inline Array::Array(int size, double value)
    : pointer(0), n(0), bufferSize(0) {
        if (size > 0)
            allocate(size);
        std::fill(begin(),end(),value);
    }

    inline Array::Array(int size, double value, double increment)
    : pointer(0), n(0), bufferSize(0) {
        if (size > 0)
            allocate(size);
        for (iterator i=begin(); i!=end(); i++,value+=increment)
            *i = value;
    }

    inline Array::Array(const Array& from)
    : pointer(0), n(0), bufferSize(0) {
        allocate(from.size());
        copy(from);
    }

    inline Array::~Array() {
        if (pointer != 0 && bufferSize != 0)
            delete[] pointer;
        pointer = 0;
        n = bufferSize = 0;
    }

    inline Array& Array::operator=(const Array& from) {
        if (this != &from) {
            resize(from.size());
            copy(from);
        }
        return *this;
    }

    inline Array& Array::operator+=(const Array& v) {
        #ifdef QL_DEBUG
            QL_REQUIRE(n == v.n,
                "arrays with different sizes cannot be added");
        #endif
        std::transform(begin(),end(),v.begin(),begin(),std::plus<double>());
        return *this;
    }

    inline Array& Array::operator+=(double x) {
        std::transform(begin(),end(),begin(),
            std::bind2nd(std::plus<double>(),x));
        return *this;
    }

    inline Array& Array::operator-=(const Array& v) {
        #ifdef QL_DEBUG
            QL_REQUIRE(n == v.n,
                "arrays with different sizes cannot be subtracted");
        #endif
        std::transform(begin(),end(),v.begin(),begin(),
            std::minus<double>());
        return *this;
    }

    inline Array& Array::operator-=(double x) {
        std::transform(begin(),end(),begin(),
            std::bind2nd(std::minus<double>(),x));
        return *this;
    }

    inline Array& Array::operator*=(const Array& v) {
        #ifdef QL_DEBUG
            QL_REQUIRE(n == v.n,
                "arrays with different sizes cannot be multiplied");
        #endif
        std::transform(begin(),end(),v.begin(),begin(),
            std::multiplies<double>());
        return *this;
    }

    inline Array& Array::operator*=(double x) {
        std::transform(begin(),end(),begin(),
            std::bind2nd(std::multiplies<double>(),x));
        return *this;
    }

    inline Array& Array::operator/=(const Array& v) {
        #ifdef QL_DEBUG
            QL_REQUIRE(n == v.n,
                "arrays with different sizes cannot be divided");
        #endif
        std::transform(begin(),end(),v.begin(),begin(),
            std::divides<double>());
        return *this;
    }

    inline Array& Array::operator/=(double x) {
        std::transform(begin(),end(),begin(),
            std::bind2nd(std::divides<double>(),x));
        return *this;
    }

    inline double Array::operator[](int i) const {
        #ifdef QL_DEBUG
            QL_REQUIRE(i>=0 && i<n,
                "array cannot be accessed out of range");
        #endif
        return pointer[i];
    }

    inline double& Array::operator[](int i) {
        #ifdef QL_DEBUG
            QL_REQUIRE(i>=0 && i<n,
                "array cannot be accessed out of range");
        #endif
        return pointer[i];
    }

    inline int Array::size() const {
        return n;
    }

    inline void Array::resize(int size) {
        if (size != n) {
            if (size <= bufferSize) {
                n = size;
            } else {
                Array temp(size);
                std::copy(begin(),end(),temp.begin());
                allocate(size);
                copy(temp);
            }
        }
    }

    inline Array::const_iterator Array::begin() const {
        return pointer;
    }

    inline Array::iterator Array::begin() {
        return pointer;
    }

    inline Array::const_iterator Array::end() const {
        return pointer+n;
    }

    inline Array::iterator Array::end() {
        return pointer+n;
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

    inline void Array::allocate(int size) {
        if (pointer != 0 && bufferSize != 0)
            delete[] pointer;
        if (size <= 0) {
            pointer = 0;
        } else {
            n = size;
            bufferSize = size+size/10+10;
            try {
                pointer = new double[bufferSize];
            }
            catch (...) {
                pointer = 0;
            }
            if (pointer == 0) {
                n = bufferSize = size;
                try {
                    pointer = new double[bufferSize];
                }
                catch (...) {
                    pointer = 0;
                }
                if (pointer == 0) {
                    n = bufferSize = 0;
                    throw OutOfMemoryError("Array");
                }
            }
        }
    }

    // dot product

    inline double DotProduct(const Array& v1, const Array& v2) {
        #ifdef QL_DEBUG
            QL_REQUIRE(v1.size() == v2.size(),
                "arrays with different sizes cannot be multiplied");
        #endif
        return std::inner_product(v1.begin(),v1.end(),v2.begin(),0.0);
    }

    // overloaded operators

    // unary

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,Plus> >
        operator+(const Array& v) {
            typedef UnaryVectorialExpression<
                Array::const_iterator,Plus> Iter;
            return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,Plus> >
        operator+(const VectorialExpression<Iter1>& e) {
            typedef UnaryVectorialExpression<
                VectorialExpression<Iter1>,Plus> Iter;
            return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
        }

        inline VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,Minus> >
        operator-(const Array& v) {
            typedef UnaryVectorialExpression<
                Array::const_iterator,Minus> Iter;
            return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,Minus> >
        operator-(const VectorialExpression<Iter1>& e) {
            typedef UnaryVectorialExpression<
                VectorialExpression<Iter1>,Minus> Iter;
            return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
        }

    #else

        inline Array operator+(const Array& v) {
            return v;
        }

        inline Array operator-(const Array& v) {
            Array result(v.size());
            std::transform(v.begin(),v.end(),result.begin(),
                std::negate<double>());
            return result;
        }

    #endif

    // binary operators

    // addition

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Array::const_iterator,Add> >
        operator+(const Array& v1, const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == v2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                Array::const_iterator,Array::const_iterator,Add>
                Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
        }

        inline VectorialExpression<
        BinaryVectorialExpression<Array::const_iterator,Scalar,Add> >
        operator+(const Array& v1, double x) {
            typedef BinaryVectorialExpression<
                Array::const_iterator,Scalar,Add> Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),Scalar(x),v1.size()),v1.size());
        }

        inline VectorialExpression<
        BinaryVectorialExpression<Scalar,Array::const_iterator,Add> >
        operator+(double x, const Array& v2) {
            typedef BinaryVectorialExpression<
                Scalar,Array::const_iterator,Add> Iter;
            return VectorialExpression<Iter>(
                Iter(Scalar(x),v2.begin(),v2.size()),v2.size());
        }

        template <class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,VectorialExpression<Iter2>,Add> >
        operator+(const Array& v1, const VectorialExpression<Iter2>& e2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == e2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                Array::const_iterator,VectorialExpression<Iter2>,Add>
                Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),e2,v1.size()),v1.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,Array::const_iterator,Add> >
        operator+(const VectorialExpression<Iter1>& e1, const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(e1.size() == v2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,Array::const_iterator,Add>
                Iter;
            return VectorialExpression<Iter>(
                Iter(e1,v2.begin(),v2.size()),v2.size());
        }

        template <class Iter1, class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,VectorialExpression<Iter2>,Add> >
        operator+(const VectorialExpression<Iter1>& e1,
            const VectorialExpression<Iter2>& e2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(e1.size() == e2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,VectorialExpression<Iter2>,Add> Iter;
            return VectorialExpression<Iter>(Iter(e1,e2,e1.size()),e1.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Add> >
        operator+(const VectorialExpression<Iter1>& e1, double x) {
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,Scalar,Add> Iter;
            return VectorialExpression<Iter>(
                Iter(e1,Scalar(x),e1.size()),e1.size());
        }

        template <class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Add> >
        operator+(double x, const VectorialExpression<Iter2>& e2) {
            typedef BinaryVectorialExpression<
                Scalar,VectorialExpression<Iter2>,Add> Iter;
            return VectorialExpression<Iter>(
                Iter(Scalar(x),e2,e2.size()),e2.size());
        }

    #else

        inline Array operator+(const Array& v1,
          const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == v2.size(),
                    "adding arrays with different sizes");
            #endif
            Array result(v1.size());
            std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                std::plus<double>());
            return result;
        }

        inline Array operator+(const Array& v1, double a) {
            Array result(v1.size());
            std::transform(v1.begin(),v1.end(),result.begin(),
                std::bind2nd(std::plus<double>(),a));
            return result;
        }

        inline Array operator+(double a, const Array& v2) {
            Array result(v2.size());
            std::transform(v2.begin(),v2.end(),result.begin(),
                std::bind1st(std::plus<double>(),a));
            return result;
        }

    #endif

    // subtraction

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Array::const_iterator,Subtract> >
        operator-(const Array& v1, const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == v2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
            Array::const_iterator,Array::const_iterator,Subtract>
            Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
        }

        inline VectorialExpression<
        BinaryVectorialExpression<Array::const_iterator,Scalar,Subtract> >
        operator-(const Array& v1, double x) {
            typedef BinaryVectorialExpression<
                Array::const_iterator,Scalar,Subtract> Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),Scalar(x),v1.size()),v1.size());
        }

        inline VectorialExpression<
        BinaryVectorialExpression<Scalar,Array::const_iterator,Subtract> >
        operator-(double x, const Array& v2) {
            typedef BinaryVectorialExpression<
                Scalar,Array::const_iterator,Subtract> Iter;
            return VectorialExpression<Iter>(
                Iter(Scalar(x),v2.begin(),v2.size()),v2.size());
        }

        template <class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,VectorialExpression<Iter2>,Subtract> >
        operator-(const Array& v1, const VectorialExpression<Iter2>& e2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == e2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                Array::const_iterator,VectorialExpression<Iter2>,Subtract>
                Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),e2,v1.size()),v1.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,Array::const_iterator,Subtract> >
        operator-(const VectorialExpression<Iter1>& e1, const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(e1.size() == v2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,Array::const_iterator,Subtract>
                Iter;
            return VectorialExpression<Iter>(
                Iter(e1,v2.begin(),v2.size()),v2.size());
        }

        template <class Iter1, class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,VectorialExpression<Iter2>,Subtract> >
        operator-(const VectorialExpression<Iter1>& e1,
            const VectorialExpression<Iter2>& e2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(e1.size() == e2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,VectorialExpression<Iter2>,Subtract>
                Iter;
            return VectorialExpression<Iter>(Iter(e1,e2,e1.size()),e1.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Subtract> >
        operator-(const VectorialExpression<Iter1>& e1, double x) {
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,Scalar,Subtract> Iter;
            return VectorialExpression<Iter>(
                Iter(e1,Scalar(x),e1.size()),e1.size());
        }

        template <class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Subtract> >
        operator-(double x, const VectorialExpression<Iter2>& e2) {
            typedef BinaryVectorialExpression<
                Scalar,VectorialExpression<Iter2>,Subtract> Iter;
            return VectorialExpression<Iter>(
                Iter(Scalar(x),e2,e2.size()),e2.size());
        }

    #else

        inline Array operator-(const Array& v1,
          const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == v2.size(),
                    "subtracting arrays with different sizes");
            #endif
            Array result(v1.size());
            std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                std::minus<double>());
            return result;
        }

        inline Array operator-(const Array& v1, double a) {
            Array result(v1.size());
            std::transform(v1.begin(),v1.end(),result.begin(),
                std::bind2nd(std::minus<double>(),a));
            return result;
        }

        inline Array operator-(double a, const Array& v2) {
            Array result(v2.size());
            std::transform(v2.begin(),v2.end(),result.begin(),
                std::bind1st(std::minus<double>(),a));
            return result;
        }

    #endif

    // multiplication

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Array::const_iterator,Multiply> >
        operator*(const Array& v1, const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == v2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
            Array::const_iterator,Array::const_iterator,Multiply>
            Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
        }

        inline VectorialExpression<
        BinaryVectorialExpression<Array::const_iterator,Scalar,Multiply> >
        operator*(const Array& v1, double x) {
            typedef BinaryVectorialExpression<
                Array::const_iterator,Scalar,Multiply> Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),Scalar(x),v1.size()),v1.size());
        }

        inline VectorialExpression<
        BinaryVectorialExpression<Scalar,Array::const_iterator,Multiply> >
        operator*(double x, const Array& v2) {
            typedef BinaryVectorialExpression<
                Scalar,Array::const_iterator,Multiply> Iter;
            return VectorialExpression<Iter>(
                Iter(Scalar(x),v2.begin(),v2.size()),v2.size());
        }

        template <class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,VectorialExpression<Iter2>,Multiply> >
        operator*(const Array& v1, const VectorialExpression<Iter2>& e2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == e2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                Array::const_iterator,VectorialExpression<Iter2>,Multiply>
                Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),e2,v1.size()),v1.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,Array::const_iterator,Multiply> >
        operator*(const VectorialExpression<Iter1>& e1, const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(e1.size() == v2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,Array::const_iterator,Multiply>
                Iter;
            return VectorialExpression<Iter>(
                Iter(e1,v2.begin(),v2.size()),v2.size());
        }

        template <class Iter1, class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,VectorialExpression<Iter2>,Multiply> >
        operator*(const VectorialExpression<Iter1>& e1,
            const VectorialExpression<Iter2>& e2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(e1.size() == e2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,VectorialExpression<Iter2>,Multiply>
                Iter;
            return VectorialExpression<Iter>(Iter(e1,e2,e1.size()),e1.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Multiply> >
        operator*(const VectorialExpression<Iter1>& e1, double x) {
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,Scalar,Multiply> Iter;
            return VectorialExpression<Iter>(
                Iter(e1,Scalar(x),e1.size()),e1.size());
        }

        template <class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Multiply> >
        operator*(double x, const VectorialExpression<Iter2>& e2) {
            typedef BinaryVectorialExpression<
                Scalar,VectorialExpression<Iter2>,Multiply> Iter;
            return VectorialExpression<Iter>(
                Iter(Scalar(x),e2,e2.size()),e2.size());
        }

    #else

        inline Array operator*(const Array& v1,
          const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == v2.size(),
                    "multiplying arrays with different sizes");
            #endif
            Array result(v1.size());
            std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                std::multiplies<double>());
            return result;
        }

        inline Array operator*(const Array& v1, double a) {
            Array result(v1.size());
            std::transform(v1.begin(),v1.end(),result.begin(),
                std::bind2nd(std::multiplies<double>(),a));
            return result;
        }

        inline Array operator*(double a, const Array& v2) {
            Array result(v2.size());
            std::transform(v2.begin(),v2.end(),result.begin(),
                std::bind1st(std::multiplies<double>(),a));
            return result;
        }

    #endif

    // division

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,Array::const_iterator,Divide> >
        operator/(const Array& v1, const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == v2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                Array::const_iterator,Array::const_iterator,Divide>
                Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
        }

        inline VectorialExpression<
        BinaryVectorialExpression<Array::const_iterator,Scalar,Divide> >
        operator/(const Array& v1, double x) {
            typedef BinaryVectorialExpression<
                Array::const_iterator,Scalar,Divide> Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),Scalar(x),v1.size()),v1.size());
        }

        inline VectorialExpression<
        BinaryVectorialExpression<Scalar,Array::const_iterator,Divide> >
        operator/(double x, const Array& v2) {
            typedef BinaryVectorialExpression<
                Scalar,Array::const_iterator,Divide> Iter;
            return VectorialExpression<Iter>(
                Iter(Scalar(x),v2.begin(),v2.size()),v2.size());
        }

        template <class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<
        Array::const_iterator,VectorialExpression<Iter2>,Divide> >
        operator/(const Array& v1, const VectorialExpression<Iter2>& e2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == e2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                Array::const_iterator,VectorialExpression<Iter2>,Divide>
                Iter;
            return VectorialExpression<Iter>(
                Iter(v1.begin(),e2,v1.size()),v1.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,Array::const_iterator,Divide> >
        operator/(const VectorialExpression<Iter1>& e1, const Array& v2) {
            #ifdef QL_DEBUG
                    QL_REQUIRE(e1.size() == v2.size(),
                        "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,Array::const_iterator,Divide>
                Iter;
            return VectorialExpression<Iter>(
                Iter(e1,v2.begin(),v2.size()),v2.size());
        }

        template <class Iter1, class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<
        VectorialExpression<Iter1>,VectorialExpression<Iter2>,Divide> >
        operator/(const VectorialExpression<Iter1>& e1,
            const VectorialExpression<Iter2>& e2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(e1.size() == e2.size(),
                    "adding arrays with different sizes");
            #endif
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,VectorialExpression<Iter2>,Divide>
                Iter;
            return VectorialExpression<Iter>(Iter(e1,e2,e1.size()),e1.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Divide> >
        operator/(const VectorialExpression<Iter1>& e1, double x) {
            typedef BinaryVectorialExpression<
                VectorialExpression<Iter1>,Scalar,Divide> Iter;
            return VectorialExpression<Iter>(
                Iter(e1,Scalar(x),e1.size()),e1.size());
        }

        template <class Iter2>
        inline VectorialExpression<
        BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Divide> >
        operator/(double x, const VectorialExpression<Iter2>& e2) {
            typedef BinaryVectorialExpression<
                Scalar,VectorialExpression<Iter2>,Divide> Iter;
            return VectorialExpression<Iter>(
                Iter(Scalar(x),e2,e2.size()),e2.size());
        }

    #else

        inline Array operator/(const Array& v1,
          const Array& v2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v1.size() == v2.size(),
                    "dividing arrays with different sizes");
            #endif
            Array result(v1.size());
            std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(),
                std::divides<double>());
            return result;
        }

        inline Array operator/(const Array& v1, double a) {
            Array result(v1.size());
            std::transform(v1.begin(),v1.end(),result.begin(),
                std::bind2nd(std::divides<double>(),a));
            return result;
        }

        inline Array operator/(double a, const Array& v2) {
            Array result(v2.size());
            std::transform(v2.begin(),v2.end(),result.begin(),
                std::bind1st(std::divides<double>(),a));
            return result;
        }

    #endif

    // functions

    // Abs()

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,AbsoluteValue> >
        Abs(const Array& v) {
            typedef UnaryVectorialExpression<
                Array::const_iterator,AbsoluteValue> Iter;
            return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,AbsoluteValue> >
        Abs(const VectorialExpression<Iter1>& e) {
            typedef UnaryVectorialExpression<
                VectorialExpression<Iter1>,AbsoluteValue> Iter;
            return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
        }

    #else

        inline Array Abs(const Array& v) {
            Array result(v.size());
            std::transform(v.begin(),v.end(),result.begin(),
                std::ptr_fun(QL_FABS));
            return result;
        }

    #endif

    // Sqrt()

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,SquareRoot> >
        Sqrt(const Array& v) {
            typedef UnaryVectorialExpression<
                Array::const_iterator,SquareRoot> Iter;
            return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,SquareRoot> >
        Sqrt(const VectorialExpression<Iter1>& e) {
            typedef UnaryVectorialExpression<
                VectorialExpression<Iter1>,SquareRoot> Iter;
            return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
        }

    #else

        inline Array Sqrt(const Array& v) {
            Array result(v.size());
            std::transform(v.begin(),v.end(),result.begin(),
                std::ptr_fun(QL_SQRT));
            return result;
        }

    #endif

    // Log()

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,Logarithm> >
        Log(const Array& v) {
            typedef UnaryVectorialExpression<
                Array::const_iterator,Logarithm> Iter;
            return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,Logarithm> >
        Log(const VectorialExpression<Iter1>& e) {
            typedef UnaryVectorialExpression<
                VectorialExpression<Iter1>,Logarithm> Iter;
            return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
        }

    #else

        inline Array Log(const Array& v) {
            Array result(v.size());
            std::transform(v.begin(),v.end(),result.begin(),
                std::ptr_fun(QL_LOG));
            return result;
        }

    #endif

    // Exp()

    #if QL_EXPRESSION_TEMPLATES_WORK

        inline VectorialExpression<
        UnaryVectorialExpression<Array::const_iterator,Exponential> >
        Exp(const Array& v) {
            typedef UnaryVectorialExpression<
                Array::const_iterator,Exponential> Iter;
            return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
        }

        template <class Iter1>
        inline VectorialExpression<
        UnaryVectorialExpression<VectorialExpression<Iter1>,Exponential> >
        Exp(const VectorialExpression<Iter1>& e) {
            typedef UnaryVectorialExpression<
                VectorialExpression<Iter1>,Exponential> Iter;
            return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
        }

    #else

        inline Array Exp(const Array& v) {
            Array result(v.size());
            std::transform(v.begin(),v.end(),result.begin(),
                std::ptr_fun(QL_EXP));
            return result;
        }

    #endif

}


#endif
