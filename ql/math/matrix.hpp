/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2015 Michael von den Driesch
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

/*! \file matrix.hpp
    \brief matrix used in linear algebra.
*/

#ifndef quantlib_matrix_hpp
#define quantlib_matrix_hpp

#include <ql/math/array.hpp>
#include <ql/utilities/steppingiterator.hpp>
#include <initializer_list>
#include <iterator>

namespace QuantLib {

    //! %Matrix used in linear algebra.
    /*! This class implements the concept of Matrix as used in linear
        algebra. As such, it is <b>not</b> meant to be used as a
        container.
    */
    class Matrix {
      public:
        //! \name Constructors, destructor, and assignment
        //@{
        //! creates a null matrix
        Matrix();
        //! creates a matrix with the given dimensions
        Matrix(Size rows, Size columns);
        //! creates the matrix and fills it with <tt>value</tt>
        Matrix(Size rows, Size columns, Real value);
        //! creates the matrix and fills it with data from a range.
        /*! \warning if the range defined by [begin, end) is larger
            than the size of the matrix, a memory access violation
            might occur.  It is up to the user to avoid this.
        */
        template <class Iterator>
        Matrix(Size rows, Size columns, Iterator begin, Iterator end);
        Matrix(const Matrix&);
        Matrix(Matrix&&) noexcept;
        Matrix(std::initializer_list<std::initializer_list<Real>>);
        ~Matrix() = default;

        Matrix& operator=(const Matrix&);
        Matrix& operator=(Matrix&&) noexcept;

        bool operator==(const Matrix&) const;
        bool operator!=(const Matrix&) const;
        //@}

        //! \name Algebraic operators
        /*! \pre all matrices involved in an algebraic expression must have
                 the same size.
        */
        //@{
        const Matrix& operator+=(const Matrix&);
        const Matrix& operator-=(const Matrix&);
        const Matrix& operator*=(Real);
        const Matrix& operator/=(Real);
        //@}

        typedef Real* iterator;
        typedef const Real* const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef Real* row_iterator;
        typedef const Real* const_row_iterator;
        typedef std::reverse_iterator<row_iterator> reverse_row_iterator;
        typedef std::reverse_iterator<const_row_iterator>
                                                const_reverse_row_iterator;
        typedef step_iterator<iterator> column_iterator;
        typedef step_iterator<const_iterator> const_column_iterator;
        typedef std::reverse_iterator<column_iterator>
                                                   reverse_column_iterator;
        typedef std::reverse_iterator<const_column_iterator>
                                             const_reverse_column_iterator;
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
        const_row_iterator row_begin(Size i) const;
        row_iterator row_begin(Size i);
        const_row_iterator row_end(Size i) const;
        row_iterator row_end(Size i);
        const_reverse_row_iterator row_rbegin(Size i) const;
        reverse_row_iterator row_rbegin(Size i);
        const_reverse_row_iterator row_rend(Size i) const;
        reverse_row_iterator row_rend(Size i);
        const_column_iterator column_begin(Size i) const;
        column_iterator column_begin(Size i);
        const_column_iterator column_end(Size i) const;
        column_iterator column_end(Size i);
        const_reverse_column_iterator column_rbegin(Size i) const;
        reverse_column_iterator column_rbegin(Size i);
        const_reverse_column_iterator column_rend(Size i) const;
        reverse_column_iterator column_rend(Size i);
        //@}

        //! \name Element access
        //@{
        const_row_iterator operator[](Size) const;
        const_row_iterator at(Size) const;
        row_iterator operator[](Size);
        row_iterator at(Size);
        Array diagonal() const;
        Real& operator()(Size i, Size j) const;
        //@}

        //! \name Inspectors
        //@{
        Size rows() const;
        Size columns() const;
        bool empty() const;
        Size size1() const;
        Size size2() const;
        //@}

        //! \name Utilities
        //@{
        void swap(Matrix&) noexcept;
        //@}
      private:
        std::unique_ptr<Real[]> data_;
        Size rows_ = 0, columns_ = 0;
    };

    // algebraic operators

    /*! \relates Matrix */
    Matrix operator+(const Matrix&, const Matrix&);
    /*! \relates Matrix */
    Matrix operator+(const Matrix&, Matrix&&);
    /*! \relates Matrix */
    Matrix operator+(Matrix&&, const Matrix&);
    /*! \relates Matrix */
    Matrix operator+(Matrix&&, Matrix&&);
    /*! \relates Matrix */
    Matrix operator-(const Matrix&);
    /*! \relates Matrix */
    Matrix operator-(Matrix&&);
    /*! \relates Matrix */
    Matrix operator-(const Matrix&, const Matrix&);
    /*! \relates Matrix */
    Matrix operator-(const Matrix&, Matrix&&);
    /*! \relates Matrix */
    Matrix operator-(Matrix&&, const Matrix&);
    /*! \relates Matrix */
    Matrix operator-(Matrix&&, Matrix&&);
    /*! \relates Matrix */
    Matrix operator*(const Matrix&, Real);
    /*! \relates Matrix */
    Matrix operator*(Matrix&&, Real);
    /*! \relates Matrix */
    Matrix operator*(Real, const Matrix&);
    /*! \relates Matrix */
    Matrix operator*(Real, Matrix&&);
    /*! \relates Matrix */
    Matrix operator/(const Matrix&, Real);
    /*! \relates Matrix */
    Matrix operator/(Matrix&&, Real);

    // vectorial products

    /*! \relates Matrix */
    Array operator*(const Array&, const Matrix&);
    /*! \relates Matrix */
    Array operator*(const Matrix&, const Array&);
    /*! \relates Matrix */
    Matrix operator*(const Matrix&, const Matrix&);

    // misc. operations

    /*! \relates Matrix */
    Matrix transpose(const Matrix&);

    /*! \relates Matrix */
    Matrix outerProduct(const Array& v1, const Array& v2);

    /*! \relates Matrix */
    template <class Iterator1, class Iterator2>
    Matrix outerProduct(Iterator1 v1begin, Iterator1 v1end, Iterator2 v2begin, Iterator2 v2end);

    /*! \relates Matrix */
    void swap(Matrix&, Matrix&) noexcept;

    /*! \relates Matrix */
    std::ostream& operator<<(std::ostream&, const Matrix&);

    /*! \relates Matrix */
    Matrix inverse(const Matrix& m);

    /*! \relates Matrix */
    Real determinant(const Matrix& m);

    /*! \relates Matrix */
    template <class Iterator1, class Iterator2>
    Matrix outerProduct(Iterator1 v1begin, Iterator1 v1end, Iterator2 v2begin, Iterator2 v2end) {

        Size size1 = std::distance(v1begin, v1end);
        QL_REQUIRE(size1>0, "null first vector");

        Size size2 = std::distance(v2begin, v2end);
        QL_REQUIRE(size2>0, "null second vector");

        Matrix result(size1, size2);

        for (Size i=0; v1begin!=v1end; i++, v1begin++)
            std::transform(v2begin, v2end, result.row_begin(i),
                           [=](Real y) -> Real { return y * (*v1begin); });

        return result;
    }

    /*! \relates Matrix */
    template <class Iterator>
    Matrix::Matrix(Size rows, Size columns, Iterator begin, Iterator end)
    : data_(rows * columns > 0 ? new Real[rows * columns] : (Real*)nullptr), rows_(rows),
      columns_(columns) {
        std::copy(begin, end, this->begin());
    }

}


#endif
