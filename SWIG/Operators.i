
/*
 * Copyright (C) 2000
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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/* $Source$
   $Log$
   Revision 1.15  2001/03/27 16:16:13  marmar
   Functions SymmetricEigenvalues and SymmetricEigenvectors added

   Revision 1.14  2001/03/21 10:48:31  marmar
   valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
   are no longer methods of BSMNumericalOption but separate
   functions

   Revision 1.13  2001/03/14 16:20:49  lballabio
   Added augmented assignment operators for Python 2.0

   Revision 1.12  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_operators_i
#define quantlib_operators_i

%include QLArray.i
%include BoundaryConditions.i

%{
using QuantLib::FiniteDifferences::TridiagonalOperator;
%}

class TridiagonalOperator {
  public:
    // constructors
    TridiagonalOperator(Array low, Array mid, Array high);
    ~TridiagonalOperator();
    // operator interface
    Array solveFor(const Array& rhs) const;
    Array applyTo(const Array& v) const;
    // inspectors
    int size() const;
    // modifiers
    void setLowerBC(BoundaryCondition bc);
    void setHigherBC(BoundaryCondition bc);
    void setFirstRow(double, double);
    void setMidRow(int, double, double, double);
    void setMidRows(double, double, double);
    void setLastRow(double, double);
};

%addmethods TridiagonalOperator {
    #if defined(SWIGPYTHON)
    TridiagonalOperator __add__(const TridiagonalOperator& O) {
        return *self+O;
    }
    TridiagonalOperator __iadd__(const TridiagonalOperator& O) {
        return *self+O;
    }
    TridiagonalOperator __sub__(const TridiagonalOperator& O) {
        return *self-O;
    }
    TridiagonalOperator __isub__(const TridiagonalOperator& O) {
        return *self-O;
    }
    TridiagonalOperator __mul__(double a) {
        return *self*a;
    }
    TridiagonalOperator __imul__(double a) {
        return *self*a;
    }
    TridiagonalOperator __rmul__(double a) {
        return *self*a;
    }
    TridiagonalOperator __div__(double a) {
        return *self/a;
    }
    TridiagonalOperator __idiv__(double a) {
        return *self/a;
    }
    #endif
};

%{
TridiagonalOperator TridiagonalIdentity(int gridPoints) {
    TridiagonalOperator I(gridPoints);
    I.setFirstRow(1.0,0.0);
    I.setMidRows(0.0,1.0,0.0);
    I.setLastRow(0.0,1.0);
    return I;
}

using QuantLib::FiniteDifferences::DPlus;
using QuantLib::FiniteDifferences::DMinus;
using QuantLib::FiniteDifferences::DZero;
using QuantLib::FiniteDifferences::DPlusDMinus;
%}

%name(Identity) TridiagonalOperator TridiagonalIdentity(int gridPoints);

class DPlus : public TridiagonalOperator {
  public:
    DPlus(int gridPoints, double h);
    ~DPlus();
};

class DMinus : public TridiagonalOperator {
  public:
    DMinus(int gridPoints, double h);
    ~DMinus();
};

class DZero : public TridiagonalOperator {
  public:
    DZero(int gridPoints, double h);
    ~DZero();
};

class DPlusDMinus : public TridiagonalOperator {
  public:
    DPlusDMinus(int gridPoints, double h);
    ~DPlusDMinus();
};


%{
using QuantLib::FiniteDifferences::valueAtCenter;
using QuantLib::FiniteDifferences::firstDerivativeAtCenter;
using QuantLib::FiniteDifferences::secondDerivativeAtCenter;
%}

double valueAtCenter(const Array& a);

double firstDerivativeAtCenter(const Array& a, const Array& g);

double secondDerivativeAtCenter(const Array& a, const Array& g);        

%{
using QuantLib::Math::SymmetricEigenvalues;
using QuantLib::Math::SymmetricEigenvectors;
%}

Array SymmetricEigenvalues(Matrix &s);

Matrix SymmetricEigenvectors(Matrix &s);

#endif

