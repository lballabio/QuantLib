
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

/* $Source$
   $Log$
   Revision 1.9  2001/04/06 18:46:21  nando
   changed Authors, Contributors, Licence and copyright header

   Revision 1.8  2001/03/14 16:20:49  lballabio
   Added augmented assignment operators for Python 2.0

   Revision 1.7  2001/03/12 12:59:01  marmar
   __str__ now represents the object while __repr__ is unchanged

   Revision 1.6  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_matrix_i
#define quantlib_matrix_i

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
    #if defined(SWIGPYTHON)
    MatrixRow __getitem__(int i) {
        return (*self)[i];
    }
    Matrix __add__(const Matrix& m) {
        return *self+m;
    }
    Matrix __iadd__(const Matrix& m) {
        return *self+m;
    }
    Matrix __sub__(const Matrix& m) {
        return *self-m;
    }
    Matrix __isub__(const Matrix& m) {
        return *self-m;
    }
    Matrix __mul__(double x) {
        return *self*x;
    }
    Matrix __imul__(double x) {
        return *self*x;
    }
    Matrix __rmul__(double x) {
        return *self*x;
    }
    Matrix __div__(double x) {
        return *self/x;
    }
    Matrix __idiv__(double x) {
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
    #endif
};

%addmethods MatrixRow {
    #if defined(SWIGPYTHON)
    double __getitem__(int i) {
        return (*self)[i];
    }
    void __setitem__(int i, double x) {
        (*self)[i] = x;
    }
    #endif
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
