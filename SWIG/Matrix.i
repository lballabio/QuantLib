
/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef quantlib_matrix_i
#define quantlib_matrix_i

%module Matrix

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: Matrix is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%include String.i
%include QLArray.i

%{
using QuantLib::Math::Matrix;
typedef QuantLib::Math::Matrix::row_iterator MatrixRow;
using QuantLib::Math::outerProduct;
using QuantLib::Math::transpose;
using QuantLib::Math::matrixSqrt;
%}

class Matrix {
  public:
    Matrix(int rows, int columns, double value = 0.0);
    ~Matrix();
    int rows() const;
    int columns() const;
};

class MatrixRow {
  public:
    ~MatrixRow();
};

%addmethods Matrix {
    MatrixRow __getitem__(int i) {
        return (*self)[i];
    }
    Matrix __add__(const Matrix& m) {
        return *self+m;
    }
    Matrix __sub__(const Matrix& m) {
        return *self-m;
    }
    Matrix __mul__(double x) {
        return *self*x;
    }
    Matrix __div__(double x) {
        return *self/x;
    }
    String __str__() {
        String s;
        for (int j=0; j<self->rows(); j++) {
    	    s += "\n";
            for (int i=0; i<self->columns(); i++) {
                if (i != 0)
                    s += ",";
                s += QuantLib::DoubleFormatter::toString((*self)[j][i]);
            }
        }
        s += "\n";
        return s;
    }
};

%addmethods MatrixRow {
    double __getitem__(int i) {
        return (*self)[i];
    }
    void __setitem__(int i, double x) {
        (*self)[i] = x;
    }
};

// functions

Matrix transpose(const Matrix& m);
Matrix outerProduct(const Array& v1, const Array& v2);
%inline %{
    Matrix matrixProduct(const Matrix& m1, const Matrix& m2) {
        return m1*m2;
    }
%}
Matrix matrixSqrt(const Matrix& m);



#endif
