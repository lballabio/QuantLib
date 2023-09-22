/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

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

#include <ql/math/matrix.hpp>
#include <iomanip>

#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4180)
#pragma warning(disable:4127)
#endif

#if BOOST_VERSION == 106400
#include <boost/serialization/array_wrapper.hpp>
#endif
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

namespace QuantLib {

    Matrix inverse(const Matrix& m) {
        QL_REQUIRE(m.rows() == m.columns(), "matrix is not square");

        boost::numeric::ublas::matrix<Real> a(m.rows(), m.columns());

        std::copy(m.begin(), m.end(), a.data().begin());

        boost::numeric::ublas::permutation_matrix<Size> pert(m.rows());

        // lu decomposition
        Size singular = 1;
        try {
            singular = lu_factorize(a, pert);
        } catch (const boost::numeric::ublas::internal_logic& e) {
            QL_FAIL("lu_factorize error: " << e.what());
        } catch (const boost::numeric::ublas::external_logic& e) {
            QL_FAIL("lu_factorize error: " << e.what());
        }
        QL_REQUIRE(singular == 0, "singular matrix given");

        boost::numeric::ublas::matrix<Real>
            inverse = boost::numeric::ublas::identity_matrix<Real>(m.rows());

        // backsubstitution
        try {
            boost::numeric::ublas::lu_substitute(a, pert, inverse);
        } catch (const boost::numeric::ublas::internal_logic& e) {
            QL_FAIL("lu_substitute error: " << e.what());
        }

        Matrix retVal(m.rows(), m.columns());
        std::copy(inverse.data().begin(), inverse.data().end(),
                  retVal.begin());

        return retVal;
    }

    Real determinant(const Matrix& m) {
        QL_REQUIRE(m.rows() == m.columns(), "matrix is not square");

        boost::numeric::ublas::matrix<Real> a(m.rows(), m.columns());
        std::copy(m.begin(), m.end(), a.data().begin());


        // lu decomposition
        boost::numeric::ublas::permutation_matrix<Size> pert(m.rows());
        /* const Size singular = */ lu_factorize(a, pert);

        Real retVal = 1.0;

        for (Size i=0; i < m.rows(); ++i) {
            if (pert[i] != i)
                retVal *= -a(i,i);
            else
                retVal *=  a(i,i);
        }
        return retVal;
    }

    // definitions

    Matrix::Matrix() : data_((Real*)nullptr) {}

    Matrix::Matrix(Size rows, Size columns)
    : data_(rows * columns > 0 ? new Real[rows * columns] : (Real*)nullptr), rows_(rows),
      columns_(columns) {}

    Matrix::Matrix(Size rows, Size columns, Real value)
    : data_(rows * columns > 0 ? new Real[rows * columns] : (Real*)nullptr), rows_(rows),
      columns_(columns) {
        std::fill(begin(),end(),value);
    }

    Matrix::Matrix(const Matrix& from)
    : data_(!from.empty() ? new Real[from.rows_ * from.columns_] : (Real*)nullptr),
      rows_(from.rows_), columns_(from.columns_) {
        #if defined(QL_PATCH_MSVC) && defined(QL_DEBUG)
        if (!from.empty())
        #endif
        std::copy(from.begin(),from.end(),begin());
    }

    Matrix::Matrix(Matrix&& from) noexcept
    : data_((Real*)nullptr) {
        swap(from);
    }

    Matrix::Matrix(std::initializer_list<std::initializer_list<Real>> data)
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

    Matrix& Matrix::operator=(const Matrix& from) {
        // strong guarantee
        Matrix temp(from);
        swap(temp);
        return *this;
    }

    Matrix& Matrix::operator=(Matrix&& from) noexcept {
        swap(from);
        return *this;
    }

    bool Matrix::operator==(const Matrix& to) const {
        return rows_ == to.rows_ && columns_ == to.columns_ &&
               std::equal(begin(), end(), to.begin());
    }

    bool Matrix::operator!=(const Matrix& to) const { 
        return !this->operator==(to); 
    }

    void Matrix::swap(Matrix& from) noexcept {
        data_.swap(from.data_);
        std::swap(rows_, from.rows_);
        std::swap(columns_, from.columns_);
    }

    const Matrix& Matrix::operator+=(const Matrix& m) {
        QL_REQUIRE(rows_ == m.rows_ && columns_ == m.columns_,
                   "matrices with different sizes (" <<
                   m.rows_ << "x" << m.columns_ << ", " <<
                   rows_ << "x" << columns_ << ") cannot be "
                   "added");
        std::transform(begin(), end(), m.begin(), begin(), std::plus<>());
        return *this;
    }

    const Matrix& Matrix::operator-=(const Matrix& m) {
        QL_REQUIRE(rows_ == m.rows_ && columns_ == m.columns_,
                   "matrices with different sizes (" <<
                   m.rows_ << "x" << m.columns_ << ", " <<
                   rows_ << "x" << columns_ << ") cannot be "
                   "subtracted");
        std::transform(begin(), end(), m.begin(), begin(), std::minus<>());
        return *this;
    }

    const Matrix& Matrix::operator*=(Real x) {
        std::transform(begin(), end(), begin(), [=](Real y) -> Real { return y * x; });
        return *this;
    }

    const Matrix& Matrix::operator/=(Real x) {
        std::transform(begin(),end(),begin(), [=](Real y) -> Real { return y / x; });
        return *this;
    }

    Matrix::const_iterator Matrix::begin() const {
        return data_.get();
    }

    Matrix::iterator Matrix::begin() {
        return data_.get();
    }

    Matrix::const_iterator Matrix::end() const {
        return data_.get()+rows_*columns_;
    }

    Matrix::iterator Matrix::end() {
        return data_.get()+rows_*columns_;
    }

    Matrix::const_reverse_iterator Matrix::rbegin() const {
        return const_reverse_iterator(end());
    }

    Matrix::reverse_iterator Matrix::rbegin() {
        return reverse_iterator(end());
    }

    Matrix::const_reverse_iterator Matrix::rend() const {
        return const_reverse_iterator(begin());
    }

    Matrix::reverse_iterator Matrix::rend() {
        return reverse_iterator(begin());
    }

    Matrix::const_row_iterator
    Matrix::row_begin(Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<rows_,
                   "row index (" << i << ") must be less than " << rows_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return data_.get()+columns_*i;
    }

    Matrix::row_iterator Matrix::row_begin(Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<rows_,
                   "row index (" << i << ") must be less than " << rows_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return data_.get()+columns_*i;
    }

    Matrix::const_row_iterator Matrix::row_end(Size i) const{
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<rows_,
                   "row index (" << i << ") must be less than " << rows_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return data_.get()+columns_*(i+1);
    }

    Matrix::row_iterator Matrix::row_end(Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<rows_,
                   "row index (" << i << ") must be less than " << rows_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return data_.get()+columns_*(i+1);
    }

    Matrix::const_reverse_row_iterator
    Matrix::row_rbegin(Size i) const {
        return const_reverse_row_iterator(row_end(i));
    }

    Matrix::reverse_row_iterator Matrix::row_rbegin(Size i) {
        return reverse_row_iterator(row_end(i));
    }

    Matrix::const_reverse_row_iterator
    Matrix::row_rend(Size i) const {
        return const_reverse_row_iterator(row_begin(i));
    }

    Matrix::reverse_row_iterator Matrix::row_rend(Size i) {
        return reverse_row_iterator(row_begin(i));
    }

    Matrix::const_column_iterator
    Matrix::column_begin(Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<columns_,
                   "column index (" << i << ") must be less than " << columns_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return const_column_iterator(data_.get()+i,columns_);
    }

    Matrix::column_iterator Matrix::column_begin(Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<columns_,
                   "column index (" << i << ") must be less than " << columns_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return column_iterator(data_.get()+i,columns_);
    }

    Matrix::const_column_iterator
    Matrix::column_end(Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<columns_,
                   "column index (" << i << ") must be less than " << columns_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return const_column_iterator(data_.get()+i+rows_*columns_,columns_);
    }

    Matrix::column_iterator Matrix::column_end(Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<columns_,
                   "column index (" << i << ") must be less than " << columns_ <<
                   ": matrix cannot be accessed out of range");
        #endif
        return column_iterator(data_.get()+i+rows_*columns_,columns_);
    }

    Matrix::const_reverse_column_iterator
    Matrix::column_rbegin(Size i) const {
        return const_reverse_column_iterator(column_end(i));
    }

    Matrix::reverse_column_iterator
    Matrix::column_rbegin(Size i) {
        return reverse_column_iterator(column_end(i));
    }

    Matrix::const_reverse_column_iterator
    Matrix::column_rend(Size i) const {
        return const_reverse_column_iterator(column_begin(i));
    }

    Matrix::reverse_column_iterator
    Matrix::column_rend(Size i) {
        return reverse_column_iterator(column_begin(i));
    }

    Matrix::const_row_iterator
    Matrix::operator[](Size i) const {
        return row_begin(i);
    }

    Matrix::const_row_iterator
    Matrix::at(Size i) const {
        QL_REQUIRE(i < rows_, "matrix access out of range");
        return row_begin(i);
    }

    Matrix::row_iterator Matrix::operator[](Size i) {
        return row_begin(i);
    }

    Matrix::row_iterator Matrix::at(Size i) {
        QL_REQUIRE(i < rows_, "matrix access out of range");
        return row_begin(i);
    }

    Array Matrix::diagonal() const {
        Size arraySize = std::min<Size>(rows(), columns());
        Array tmp(arraySize);
        for(Size i = 0; i < arraySize; i++)
            tmp[i] = (*this)[i][i];
        return tmp;
    }

    Real &Matrix::operator()(Size i, Size j) const {
        return data_[i*columns()+j];
    }

    Size Matrix::rows() const {
        return rows_;
    }

    Size Matrix::columns() const {
        return columns_;
    }

    Size Matrix::size1() const {
        return rows();
    }

    Size Matrix::size2() const {
        return columns();
    }

    bool Matrix::empty() const {
        return rows_ == 0 || columns_ == 0;
    }

    Matrix operator+(const Matrix& m1, const Matrix& m2) {
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "added");
        Matrix temp(m1.rows(),m1.columns());
        std::transform(m1.begin(), m1.end(), m2.begin(), temp.begin(), std::plus<>());
        return temp;
    }

    Matrix operator+(const Matrix& m1, Matrix&& m2) {
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "added");
        std::transform(m1.begin(), m1.end(), m2.begin(), m2.begin(), std::plus<>());
        return std::move(m2);
    }

    Matrix operator+(Matrix&& m1, const Matrix& m2) {
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "added");
        std::transform(m1.begin(), m1.end(), m2.begin(), m1.begin(), std::plus<>());
        return std::move(m1);
    }

    Matrix operator+(Matrix&& m1, Matrix&& m2) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "added");
        std::transform(m1.begin(), m1.end(), m2.begin(), m1.begin(), std::plus<>());
        return std::move(m1);
    }

    Matrix operator-(const Matrix& m1) {
        Matrix temp(m1.rows(), m1.columns());
        std::transform(m1.begin(), m1.end(), temp.begin(), std::negate<>());
        return temp;
    }

    Matrix operator-(Matrix&& m1) {
        std::transform(m1.begin(), m1.end(), m1.begin(), std::negate<>());
        return std::move(m1);
    }

    Matrix operator-(const Matrix& m1, const Matrix& m2) {
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "subtracted");
        Matrix temp(m1.rows(),m1.columns());
        std::transform(m1.begin(), m1.end(), m2.begin(), temp.begin(), std::minus<>());
        return temp;
    }

    Matrix operator-(const Matrix& m1, Matrix&& m2) {
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "subtracted");
        std::transform(m1.begin(), m1.end(), m2.begin(), m2.begin(), std::minus<>());
        return std::move(m2);
    }

    Matrix operator-(Matrix&& m1, const Matrix& m2) {
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "subtracted");
        std::transform(m1.begin(), m1.end(), m2.begin(), m1.begin(), std::minus<>());
        return std::move(m1);
    }

    Matrix operator-(Matrix&& m1, Matrix&& m2) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        QL_REQUIRE(m1.rows() == m2.rows() &&
                   m1.columns() == m2.columns(),
                   "matrices with different sizes (" <<
                   m1.rows() << "x" << m1.columns() << ", " <<
                   m2.rows() << "x" << m2.columns() << ") cannot be "
                   "subtracted");
        std::transform(m1.begin(), m1.end(), m2.begin(), m1.begin(), std::minus<>());
        return std::move(m1);
    }

    Matrix operator*(const Matrix& m, Real x) {
        Matrix temp(m.rows(),m.columns());
        std::transform(m.begin(), m.end(), temp.begin(), [=](Real y) -> Real { return y * x; });
        return temp;
    }

    Matrix operator*(Matrix&& m, Real x) {
        std::transform(m.begin(), m.end(), m.begin(), [=](Real y) -> Real { return y * x; });
        return std::move(m);
    }

    Matrix operator*(Real x, const Matrix& m) {
        Matrix temp(m.rows(),m.columns());
        std::transform(m.begin(), m.end(), temp.begin(), [=](Real y) -> Real { return x * y; });
        return temp;
    }

    Matrix operator*(Real x, Matrix&& m) {
        std::transform(m.begin(), m.end(), m.begin(), [=](Real y) -> Real { return x * y; });
        return std::move(m);
    }

    Matrix operator/(const Matrix& m, Real x) {
        Matrix temp(m.rows(),m.columns());
        std::transform(m.begin(), m.end(), temp.begin(), [=](Real y) -> Real { return y / x; });
        return temp;
    }

    Matrix operator/(Matrix&& m, Real x) {
        std::transform(m.begin(), m.end(), m.begin(), [=](Real y) -> Real { return y / x; });
        return std::move(m);
    }

    Array operator*(const Array& v, const Matrix& m) {
        QL_REQUIRE(v.size() == m.rows(),
                   "vectors and matrices with different sizes ("
                   << v.size() << ", " << m.rows() << "x" << m.columns() <<
                   ") cannot be multiplied");
        Array result(m.columns());
        for (Size i=0; i<result.size(); i++)
            result[i] =
                std::inner_product(v.begin(),v.end(),
                                   m.column_begin(i),Real(0.0));
        return result;
    }

    Array operator*(const Matrix& m, const Array& v) {
        QL_REQUIRE(v.size() == m.columns(),
                   "vectors and matrices with different sizes ("
                   << v.size() << ", " << m.rows() << "x" << m.columns() <<
                   ") cannot be multiplied");
        Array result(m.rows());
        for (Size i=0; i<result.size(); i++)
            result[i] =
                std::inner_product(v.begin(),v.end(),m.row_begin(i),Real(0.0));
        return result;
    }

    Matrix operator*(const Matrix& m1, const Matrix& m2) {
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

    Matrix transpose(const Matrix& m) {
        Matrix result(m.columns(),m.rows());
        #if defined(QL_PATCH_MSVC) && defined(QL_DEBUG)
        if (!m.empty())
        #endif
        for (Size i=0; i<m.rows(); i++)
            std::copy(m.row_begin(i),m.row_end(i),result.column_begin(i));
        return result;
    }

    Matrix outerProduct(const Array& v1, const Array& v2) {
        return outerProduct(v1.begin(), v1.end(), v2.begin(), v2.end());
    }

    void swap(Matrix& m1, Matrix& m2) noexcept {
        m1.swap(m2);
    }

    std::ostream& operator<<(std::ostream& out, const Matrix& m) {
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
