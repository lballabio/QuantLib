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

        Matrix& operator=(const Matrix&);
        Matrix& operator=(Matrix&&) noexcept;
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
        typedef boost::reverse_iterator<iterator> reverse_iterator;
        typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef Real* row_iterator;
        typedef const Real* const_row_iterator;
        typedef boost::reverse_iterator<row_iterator> reverse_row_iterator;
        typedef boost::reverse_iterator<const_row_iterator>
                                                const_reverse_row_iterator;
        typedef step_iterator<iterator> column_iterator;
        typedef step_iterator<const_iterator> const_column_iterator;
        typedef boost::reverse_iterator<column_iterator>
                                                   reverse_column_iterator;
        typedef boost::reverse_iterator<const_column_iterator>
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
        void swap(Matrix&);
        //@}
      private:
        std::unique_ptr<Real[]> data_;
        Size rows_ = 0, columns_ = 0;
    };

    // algebraic operators

    /*! \relates Matrix */
    Matrix operator+(const Matrix&, const Matrix&);
    /*! \relates Matrix */
    Matrix operator-(const Matrix&, const Matrix&);
    /*! \relates Matrix */
    Matrix operator*(const Matrix&, Real);
    /*! \relates Matrix */
    Matrix operator*(Real, const Matrix&);
    /*! \relates Matrix */
    Matrix operator/(const Matrix&, Real);


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
    void swap(Matrix&, Matrix&);

    /*! \relates Matrix */
    std::ostream& operator<<(std::ostream&, const Matrix&);

    /*! \relates Matrix */
    Matrix inverse(const Matrix& m);

    /*! \relates Matrix */
    Real determinant(const Matrix& m);

    // inline definitions

    inline Matrix::Matrix() : data_((Real*)nullptr) {}

    inline Matrix::Matrix(Size rows, Size columns)
    : data_(rows * columns > 0 ? new Real[rows * columns] : (Real*)nullptr), rows_(rows),
      columns_(columns) {}

    inline Matrix::Matrix(Size rows, Size columns, Real value)
    : data_(rows * columns > 0 ? new Real[rows * columns] : (Real*)nullptr), rows_(rows),
      columns_(columns) {
        std::fill(begin(),end(),value);
    }

    template <class Iterator>
    inline Matrix::Matrix(Size rows, Size columns, Iterator begin, Iterator end)
    : data_(rows * columns > 0 ? new Real[rows * columns] : (Real*)nullptr), rows_(rows),
      columns_(columns) {
        std::copy(begin, end, this->begin());
    }

    inline Matrix::Matrix(const Matrix& from)
    : data_(!from.empty() ? new Real[from.rows_ * from.columns_] : (Real*)nullptr),
      rows_(from.rows_), columns_(from.columns_) {
        #if defined(QL_PATCH_MSVC) && defined(QL_DEBUG)
        if (!from.empty())
        #endif
        std::copy(from.begin(),from.end(),begin());
    }

    inline Matrix::Matrix(Matrix&& from) noexcept
    : data_((Real*)nullptr) {
        swap(from);
    }

    inline Matrix::Matrix(std::initializer_list<std::initializer_list<Real>> data)
    : data_(data.size() == 0 || data.begin()->size() == 0 ?
            (Real*)nullptr : new Real[data.size() * data.begin()->size()]),
      rows_(data.size()), columns_(data.size() == 0 ? 0 : data.begin()->size()) {
        Size i=0;
        for (const auto& row : data) {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(row.size() == columns_,
                       "a matrix needs the same number of elements for each row");
            #endif
            std::copy(row.begin(), row.end(), row_begin(i));
            ++i;
        }
    }

    inline Matrix& Matrix::operator=(const Matrix& from) {
        // strong guarantee
        Matrix temp(from);
        swap(temp);
        return *this;
    }

    inline Matrix& Matrix::operator=(Matrix&& from) noexcept {
        swap(from);
        return *this;
    }

    inline void Matrix::swap(Matrix& from) {
        using std::swap;
        data_.swap(from.data_);
        swap(rows_,from.rows_);
        swap(columns_,from.columns_);
    }

    inline const Matrix& Matrix::operator+=(const Matrix& m) {
        QL_REQUIRE(rows_ == m.rows_ && columns_ == m.columns_,
                   "matrices with different sizes (" <<
                   m.rows_ << "x" << m.columns_ << ", " <<
                   rows_ << "x" << columns_ << ") cannot be "
                   "added");
        std::transform(begin(),end(),m.begin(),
                       begin(),std::plus<Real>());
        return *this;
    }

    inline const Matrix& Matrix::operator-=(const Matrix& m) {
        QL_REQUIRE(rows_ == m.rows_ && columns_ == m.columns_,
                   "matrices with different sizes (" <<
                   m.rows_ << "x" << m.columns_ << ", " <<
                   rows_ << "x" << columns_ << ") cannot be "
                   "subtracted");
        std::transform(begin(),end(),m.begin(),begin(),
                       std::minus<Real>());
        return *this;
    }

    inline const Matrix& Matrix::operator*=(Real x) {
        std::transform(begin(), end(), begin(),
                       [=](Real y) { return y*x; });
        return *this;
    }

    inline const Matrix& Matrix::operator/=(Real x) {
        std::transform(begin(),end(),begin(),
                       [=](Real y) { return y/x; });
        return *this;
    }

    inline Matrix::const_iterator Matrix::begin() const {
        return data_.get();
    }

    inline Matrix::iterator Matrix::begin() {
        return data_.get();
    }

    inline Matrix::const_iterator Matrix::end() const {
        return data_.get()+rows_*columns_;
    }

    inline Matrix::iterator Matrix::end() {
        return data_.get()+rows_*columns_;
    }

    inline Matrix::const_reverse_iterator Matrix::rbegin() const {
        return const_reverse_iterator(end());
    }

    inline Matrix::reverse_iterator Matrix::rbegin() {
        return reverse_iterator(end());
    }

    inline Matrix::const_reverse_iterator Matrix::rend() const {
        return const_reverse_iterator(begin());
    }

    inline Matrix::reverse_iterator Matrix::rend() {
        return reverse_iterator(begin());
    }

    inline Matrix::const_row_iterator
    Matrix::row_begin(Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<rows_,
                   "row index (" << i << ") must be less than " << rows_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return data_.get()+columns_*i;
    }

    inline Matrix::row_iterator Matrix::row_begin(Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<rows_,
                   "row index (" << i << ") must be less than " << rows_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return data_.get()+columns_*i;
    }

    inline Matrix::const_row_iterator Matrix::row_end(Size i) const{
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<rows_,
                   "row index (" << i << ") must be less than " << rows_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return data_.get()+columns_*(i+1);
    }

    inline Matrix::row_iterator Matrix::row_end(Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<rows_,
                   "row index (" << i << ") must be less than " << rows_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return data_.get()+columns_*(i+1);
    }

    inline Matrix::const_reverse_row_iterator
    Matrix::row_rbegin(Size i) const {
        return const_reverse_row_iterator(row_end(i));
    }

    inline Matrix::reverse_row_iterator Matrix::row_rbegin(Size i) {
        return reverse_row_iterator(row_end(i));
    }

    inline Matrix::const_reverse_row_iterator
    Matrix::row_rend(Size i) const {
        return const_reverse_row_iterator(row_begin(i));
    }

    inline Matrix::reverse_row_iterator Matrix::row_rend(Size i) {
        return reverse_row_iterator(row_begin(i));
    }

    inline Matrix::const_column_iterator
    Matrix::column_begin(Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<columns_,
                   "column index (" << i << ") must be less than " << columns_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return const_column_iterator(data_.get()+i,columns_);
    }

    inline Matrix::column_iterator Matrix::column_begin(Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<columns_,
                   "column index (" << i << ") must be less than " << columns_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return column_iterator(data_.get()+i,columns_);
    }

    inline Matrix::const_column_iterator
    Matrix::column_end(Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<columns_,
                   "column index (" << i << ") must be less than " << columns_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return const_column_iterator(data_.get()+i+rows_*columns_,columns_);
    }

    inline Matrix::column_iterator Matrix::column_end(Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<columns_,
                   "column index (" << i << ") must be less than " << columns_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return column_iterator(data_.get()+i+rows_*columns_,columns_);
    }

    inline Matrix::const_reverse_column_iterator
    Matrix::column_rbegin(Size i) const {
        return const_reverse_column_iterator(column_end(i));
    }

    inline Matrix::reverse_column_iterator
    Matrix::column_rbegin(Size i) {
        return reverse_column_iterator(column_end(i));
    }

    inline Matrix::const_reverse_column_iterator
    Matrix::column_rend(Size i) const {
        return const_reverse_column_iterator(column_begin(i));
    }

    inline Matrix::reverse_column_iterator
    Matrix::column_rend(Size i) {
        return reverse_column_iterator(column_begin(i));
    }

    inline Matrix::const_row_iterator
    Matrix::operator[](Size i) const {
        return row_begin(i);
    }

    inline Matrix::const_row_iterator
    Matrix::at(Size i) const {
        QL_REQUIRE(i < rows_, "matrix access out of range");
        return row_begin(i);
    }

    inline Matrix::row_iterator Matrix::operator[](Size i) {
        return row_begin(i);
    }

    inline Matrix::row_iterator Matrix::at(Size i) {
        QL_REQUIRE(i < rows_, "matrix access out of range");
        return row_begin(i);
    }

    inline Array Matrix::diagonal() const {
        Size arraySize = std::min<Size>(rows(), columns());
        Array tmp(arraySize);
        for(Size i = 0; i < arraySize; i++)
            tmp[i] = (*this)[i][i];
        return tmp;
    }

    inline Real &Matrix::operator()(Size i, Size j) const {
        return data_[i*columns()+j];
    }

    inline Size Matrix::rows() const {
        return rows_;
    }

    inline Size Matrix::columns() const {
        return columns_;
    }

    inline Size Matrix::size1() const {
        return rows();
    }

    inline Size Matrix::size2() const {
        return columns();
    }

    inline bool Matrix::empty() const {
        return rows_ == 0 || columns_ == 0;
    }

    inline Matrix operator+(const Matrix& m1, const Matrix& m2) {
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "added");
        Matrix temp(m1.rows(),m1.columns());
        std::transform(m1.begin(),m1.end(),m2.begin(),temp.begin(),
                       std::plus<Real>());
        return temp;
    }

    inline Matrix operator-(const Matrix& m1, const Matrix& m2) {
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "subtracted");
        Matrix temp(m1.rows(),m1.columns());
        std::transform(m1.begin(),m1.end(),m2.begin(),temp.begin(),
                       std::minus<Real>());
        return temp;
    }

    inline Matrix operator*(const Matrix& m, Real x) {
        Matrix temp(m.rows(),m.columns());
        std::transform(m.begin(), m.end(), temp.begin(),
                       [=](Real y) { return y*x; });
        return temp;
    }

    inline Matrix operator*(Real x, const Matrix& m) {
        Matrix temp(m.rows(),m.columns());
        std::transform(m.begin(), m.end(), temp.begin(),
                       [=](Real y) { return x*y; });
        return temp;
    }

    inline Matrix operator/(const Matrix& m, Real x) {
        Matrix temp(m.rows(),m.columns());
        std::transform(m.begin(), m.end(), temp.begin(),
                       [=](Real y) { return y/x; });
        return temp;
    }

    inline Array operator*(const Array& v, const Matrix& m) {
        QL_REQUIRE(v.size() == m.rows(),
                   "vectors and matrices with different sizes ("
                   << v.size() << ", " << m.rows() << "x" << m.columns() <<
                   ") cannot be multiplied");
        Array result(m.columns());
        for (Size i=0; i<result.size(); i++)
            result[i] =
                std::inner_product(v.begin(),v.end(),
                                   m.column_begin(i),0.0);
        return result;
    }

    inline Array operator*(const Matrix& m, const Array& v) {
        QL_REQUIRE(v.size() == m.columns(),
                   "vectors and matrices with different sizes ("
                   << v.size() << ", " << m.rows() << "x" << m.columns() <<
                   ") cannot be multiplied");
        Array result(m.rows());
        for (Size i=0; i<result.size(); i++)
            result[i] =
                std::inner_product(v.begin(),v.end(),m.row_begin(i),0.0);
        return result;
    }

    inline Matrix operator*(const Matrix& m1, const Matrix& m2) {
        QL_REQUIRE(m1.columns() == m2.rows(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "multiplied");
        Matrix result(m1.rows(),m2.columns(),0.0);
        for (Size i=0; i<result.rows(); ++i) {
            for (Size k=0; k<m1.columns(); ++k) {
                for (Size j=0; j<result.columns(); ++j) {
                    result[i][j] += m1[i][k]*m2[k][j];
                }
            }
        }
        return result;
    }

    inline Matrix transpose(const Matrix& m) {
        Matrix result(m.columns(),m.rows());
        #if defined(QL_PATCH_MSVC) && defined(QL_DEBUG)
        if (!m.empty())
        #endif
        for (Size i=0; i<m.rows(); i++)
            std::copy(m.row_begin(i),m.row_end(i),result.column_begin(i));
        return result;
    }

    inline Matrix outerProduct(const Array& v1, const Array& v2) {
        return outerProduct(v1.begin(), v1.end(), v2.begin(), v2.end());
    }

    template <class Iterator1, class Iterator2>
    inline Matrix outerProduct(Iterator1 v1begin, Iterator1 v1end, Iterator2 v2begin, Iterator2 v2end) {

        Size size1 = std::distance(v1begin, v1end);
        QL_REQUIRE(size1>0, "null first vector");

        Size size2 = std::distance(v2begin, v2end);
        QL_REQUIRE(size2>0, "null second vector");

        Matrix result(size1, size2);

        for (Size i=0; v1begin!=v1end; i++, v1begin++)
            std::transform(v2begin, v2end, result.row_begin(i),
                           [=](Real y) { return y * (*v1begin); });

        return result;
    }

    inline void swap(Matrix& m1, Matrix& m2) {
        m1.swap(m2);
    }

    inline std::ostream& operator<<(std::ostream& out, const Matrix& m) {
        std::streamsize width = out.width();
        for (Size i=0; i<m.rows(); i++) {
            out << "| ";
            for (Size j=0; j<m.columns(); j++)
                out << std::setw(int(width)) << m[i][j] << " ";
            out << "|\n";
        }
        return out;
    }

}


#endif
