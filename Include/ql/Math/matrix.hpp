
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file matrix.hpp

    \fullpath
    Include/ql/Math/%matrix.hpp
    \brief matrix used in linear algebra.

*/

// $Id$
// $Log$
// Revision 1.12  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.11  2001/08/28 12:30:14  nando
// unsigned int instead of int
//

#ifndef quantlib_matrix_h
#define quantlib_matrix_h

#include "ql/array.hpp"
#include "ql/Utilities/steppingiterator.hpp"

namespace QuantLib {

    //! Mathematical functions and classes
    namespace Math {

        //! %matrix used in linear algebra.
        /*! This class implements the concept of vector as used in linear
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
            Matrix(unsigned int rows, unsigned int columns);
            //! creates the matrix and fills it with <tt>value</tt>
            Matrix(unsigned int rows, unsigned int columns, double value);
            Matrix(const Matrix&);
            ~Matrix();
            Matrix& operator=(const Matrix&);
            //@}

            //! \name Algebraic operators
            /*! \pre all matrices involved in an algebraic expression must have
                the same size.
            */
            //@{
            Matrix& operator+=(const Matrix&);
            Matrix& operator-=(const Matrix&);
            Matrix& operator*=(double);
            Matrix& operator/=(double);
            //@}

            typedef double* iterator;
            typedef const double* const_iterator;
            typedef QL_REVERSE_ITERATOR(iterator,double) reverse_iterator;
            typedef QL_REVERSE_ITERATOR(const_iterator,double)
                const_reverse_iterator;
            typedef double* row_iterator;
            typedef const double* const_row_iterator;
            typedef QL_REVERSE_ITERATOR(row_iterator,double)
                reverse_row_iterator;
            typedef QL_REVERSE_ITERATOR(const_row_iterator,double)
                const_reverse_row_iterator;
            typedef Utilities::stepping_iterator<double*> column_iterator;
            typedef Utilities::stepping_iterator<const double*>
                const_column_iterator;
            typedef QL_REVERSE_ITERATOR(column_iterator,double)
                reverse_column_iterator;
            typedef QL_REVERSE_ITERATOR(const_column_iterator,double)
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
            const_row_iterator row_begin(unsigned int i) const;
            row_iterator row_begin(unsigned int i);
            const_row_iterator row_end(unsigned int i) const;
            row_iterator row_end(unsigned int i);
            const_reverse_row_iterator row_rbegin(unsigned int i) const;
            reverse_row_iterator row_rbegin(unsigned int i);
            const_reverse_row_iterator row_rend(unsigned int i) const;
            reverse_row_iterator row_rend(unsigned int i);
            const_column_iterator column_begin(unsigned int i) const;
            column_iterator column_begin(unsigned int i);
            const_column_iterator column_end(unsigned int i) const;
            column_iterator column_end(unsigned int i);
            const_reverse_column_iterator column_rbegin(unsigned int i) const;
            reverse_column_iterator column_rbegin(unsigned int i);
            const_reverse_column_iterator column_rend(unsigned int i) const;
            reverse_column_iterator column_rend(unsigned int i);
            //@}

            //! \name Element access
            //@{
            const_row_iterator operator[](unsigned int) const;
            row_iterator operator[](unsigned int);
            Array diagonal(void) const;
            //@}

            //! \name Inspectors
            //@{
            unsigned int rows() const;
            unsigned int columns() const;
            //@}

          private:
            void allocate_(unsigned int rows, unsigned int columns);
            void copy_(const Matrix&);
          private:
            double* pointer_;
            unsigned int rows_, columns_;
        };

        // algebraic operators

        /*! \relates Matrix */
        Matrix operator+(const Matrix&, const Matrix&);
        /*! \relates Matrix */
        Matrix operator-(const Matrix&, const Matrix&);
        /*! \relates Matrix */
        Matrix operator*(const Matrix&, double);
        /*! \relates Matrix */
        Matrix operator*(double, const Matrix&);
        /*! \relates Matrix */
        Matrix operator/(const Matrix&, double);


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
        Matrix outerProduct(const Array &v1, const Array &v2);
        //! returns the square root of a real symmetric matrix
        /*! \relates Matrix */
        Matrix matrixSqrt(const Matrix &realSymmetricMatrix);


        // inline definitions

        inline Matrix::Matrix()
        : pointer_(0), rows_(0), columns_(0) {}

        inline Matrix::Matrix(unsigned int rows, unsigned int columns)
        : pointer_(0), rows_(0), columns_(0) {
            if (rows > 0 && columns > 0)
                allocate_(rows,columns);
        }

        inline Matrix::Matrix(unsigned int rows,
                              unsigned int columns,
                              double value)
        : pointer_(0), rows_(0), columns_(0) {
            if (rows > 0 && columns > 0)
                allocate_(rows,columns);
            std::fill(begin(),end(),value);
        }

        inline Matrix::Matrix(const Matrix& from)
        : pointer_(0), rows_(0), columns_(0) {
            allocate_(from.rows(), from.columns());
            copy_(from);
        }

        inline Matrix::~Matrix() {
            if (pointer_ != 0 && rows_ != 0 && columns_ != 0)
                delete[] pointer_;
            pointer_ = 0;
            rows_ = columns_ = 0;
        }

        inline Matrix& Matrix::operator=(const Matrix& from) {
            if (this != &from) {
                allocate_(from.rows(),from.columns());
                copy_(from);
            }
            return *this;
        }

        inline void Matrix::allocate_(unsigned int rows, unsigned int columns) {
            if (rows_ == rows && columns_ == columns)
                return;
            if (pointer_ != 0 && rows_ != 0 && columns_ != 0)
                delete[] pointer_;
            if (rows == 0 || columns == 0) {
                pointer_ = 0;
                rows_ = columns_ = 0;
            } else {
                pointer_ = new double[rows*columns];
                rows_ = rows;
                columns_ = columns;
            }
        }

        inline void Matrix::copy_(const Matrix& from) {
            std::copy(from.begin(),from.end(),begin());
        }

        inline Matrix& Matrix::operator+=(const Matrix& m) {
            #ifdef QL_DEBUG
                QL_REQUIRE(rows_ == m.rows_ && columns_ == m.columns_,
                    "matrices with different sizes cannot be added");
            #endif
            std::transform(begin(),end(),m.begin(),begin(),std::plus<double>());
            return *this;
        }

        inline Matrix& Matrix::operator-=(const Matrix& m) {
            #ifdef QL_DEBUG
                QL_REQUIRE(rows_ == m.rows_ && columns_ == m.columns_,
                    "matrices with different sizes cannot be subtracted");
            #endif
            std::transform(begin(),end(),m.begin(),begin(),
                std::minus<double>());
            return *this;
        }

        inline Matrix& Matrix::operator*=(double x) {
            std::transform(begin(),end(),begin(),
                std::bind2nd(std::multiplies<double>(),x));
            return *this;
        }

        inline Matrix& Matrix::operator/=(double x) {
            std::transform(begin(),end(),begin(),
                std::bind2nd(std::divides<double>(),x));
            return *this;
        }

        inline Matrix::const_iterator Matrix::begin() const {
            return pointer_;
        }

        inline Matrix::iterator Matrix::begin() {
            return pointer_;
        }

        inline Matrix::const_iterator Matrix::end() const {
            return pointer_+rows_*columns_;
        }

        inline Matrix::iterator Matrix::end() {
            return pointer_+rows_*columns_;
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
        Matrix::row_begin(unsigned int i) const {
             return pointer_+columns_*i;
        }

        inline Matrix::row_iterator Matrix::row_begin(unsigned int i) {
            return pointer_+columns_*i;
        }

        inline Matrix::const_row_iterator Matrix::row_end(unsigned int i) const{
            return pointer_+columns_*(i+1);
        }

        inline Matrix::row_iterator Matrix::row_end(unsigned int i) {
            return pointer_+columns_*(i+1);
        }

        inline Matrix::const_reverse_row_iterator
        Matrix::row_rbegin(unsigned int i) const {
            return const_reverse_row_iterator(row_end(i));
        }

        inline Matrix::reverse_row_iterator Matrix::row_rbegin(unsigned int i) {
            return reverse_row_iterator(row_end(i));
        }

        inline Matrix::const_reverse_row_iterator
        Matrix::row_rend(unsigned int i) const {
            return const_reverse_row_iterator(row_begin(i));
        }

        inline Matrix::reverse_row_iterator Matrix::row_rend(unsigned int i) {
            return reverse_row_iterator(row_begin(i));
        }

        inline Matrix::const_column_iterator
        Matrix::column_begin(unsigned int i) const {
            return const_column_iterator(pointer_+i,columns_);
        }

        inline Matrix::column_iterator Matrix::column_begin(unsigned int i) {
            return column_iterator(pointer_+i,columns_);
        }

        inline Matrix::const_column_iterator
        Matrix::column_end(unsigned int i) const {
            return column_begin(i)+rows_;
        }

        inline Matrix::column_iterator Matrix::column_end(unsigned int i) {
            return column_begin(i)+rows_;
        }

        inline Matrix::const_reverse_column_iterator
        Matrix::column_rbegin(unsigned int i) const {
            return const_reverse_column_iterator(column_end(i));
        }

        inline Matrix::reverse_column_iterator
        Matrix::column_rbegin(unsigned int i) {
            return reverse_column_iterator(column_end(i));
        }

        inline Matrix::const_reverse_column_iterator
        Matrix::column_rend(unsigned int i) const {
            return const_reverse_column_iterator(column_begin(i));
        }

        inline Matrix::reverse_column_iterator
        Matrix::column_rend(unsigned int i) {
            return reverse_column_iterator(column_begin(i));
        }

        inline Matrix::const_row_iterator
        Matrix::operator[](unsigned int i) const {
            return row_begin(i);
        }

        inline Matrix::row_iterator Matrix::operator[](unsigned int i) {
            return row_begin(i);
        }

        inline Array Matrix::diagonal(void) const{
            unsigned int arraySize = QL_MIN(rows(),columns());
            Array tmp(arraySize);
            for(unsigned int i = 0; i < arraySize; i++)
                tmp[i] = (*this)[i][i];
            return tmp;
        }

        inline unsigned int Matrix::rows() const {
            return rows_;
        }

        inline unsigned int Matrix::columns() const {
            return columns_;
        }

        inline Matrix operator+(const Matrix& m1, const Matrix& m2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(m1.rows() == m2.rows() &&
                    m1.columns() == m2.columns(),
                    "matrices with different sizes cannot be added");
            #endif
            Matrix temp(m1.rows(),m1.columns());
            std::transform(m1.begin(),m1.end(),m2.begin(),temp.begin(),
                std::plus<double>());
            return temp;
        }

        inline Matrix operator-(const Matrix& m1, const Matrix& m2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(m1.rows() == m2.rows() &&
                    m1.columns() == m2.columns(),
                    "matrices with different sizes cannot be subtracted");
            #endif
            Matrix temp(m1.rows(),m1.columns());
            std::transform(m1.begin(),m1.end(),m2.begin(),temp.begin(),
                std::minus<double>());
            return temp;
        }

        inline Matrix operator*(const Matrix& m, double x) {
            Matrix temp(m.rows(),m.columns());
            std::transform(m.begin(),m.end(),temp.begin(),
                std::bind2nd(std::multiplies<double>(),x));
            return temp;
        }

        inline Matrix operator*(double x, const Matrix& m) {
            Matrix temp(m.rows(),m.columns());
            std::transform(m.begin(),m.end(),temp.begin(),
                std::bind2nd(std::multiplies<double>(),x));
            return temp;
        }

        inline Matrix operator/(const Matrix& m, double x) {
            Matrix temp(m.rows(),m.columns());
            std::transform(m.begin(),m.end(),temp.begin(),
                std::bind2nd(std::divides<double>(),x));
            return temp;
        }

        inline Array operator*(const Array& v, const Matrix& m) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v.size() == m.rows(),
                    "vectors and matrices with different sizes "
                    "cannot be multiplied");
            #endif
            Array result(m.columns());
            for (unsigned int i=0; i<result.size(); i++)
                result[i] =
                    std::inner_product(v.begin(),v.end(),m.column_begin(i),0.0);
            return result;
        }

        inline Array operator*(const Matrix& m, const Array& v) {
            #ifdef QL_DEBUG
                QL_REQUIRE(v.size() == m.columns(),
                    "vectors and matrices with different sizes "
                    "cannot be multiplied");
            #endif
            Array result(m.rows());
            for (unsigned int i=0; i<result.size(); i++)
                result[i] =
                    std::inner_product(v.begin(),v.end(),m.row_begin(i),0.0);
            return result;
        }

        inline Matrix operator*(const Matrix& m1, const Matrix& m2) {
            #ifdef QL_DEBUG
                QL_REQUIRE(m1.columns() == m2.rows(),
                    "matrices with different sizes cannot be multiplied");
            #endif
            Matrix result(m1.rows(),m2.columns());
            for (unsigned int i=0; i<result.rows(); i++)
                for (unsigned int j=0; j<result.columns(); j++)
                    result[i][j] =
                        std::inner_product(m1.row_begin(i), m1.row_end(i),
                            m2.column_begin(j), 0.0);
            return result;
        }

        inline Matrix transpose(const Matrix& m) {
            Matrix result(m.columns(),m.rows());
            for (unsigned int i=0; i<m.rows(); i++)
                std::copy(m.row_begin(i),m.row_end(i),result.column_begin(i));
            return result;
        }

        inline Matrix outerProduct(const Array &v1, const Array &v2){
            QL_REQUIRE(v1.size() > 0 && v2.size() > 0,
                    "outerProduct: vectors must have non-null dimension");
            Matrix result(v1.size(),v2.size());
            for(unsigned int i = 0; i < v1.size(); i++)
                std::transform(v2.begin(),v2.end(),result.row_begin(i),
                    std::bind1st(std::multiplies<double>(),v1[i]));
            return result;
        }

    }

}


#endif
