
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_operators_i
#define quantlib_operators_i

%module Operators

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%include Vectors.i
%include BoundaryConditions.i

%{
#include "tridiagonaloperator.h"
QL_USING(QuantLib::Operators,TridiagonalOperator)
%}

class TridiagonalOperator {
  public:
	// constructors
	TridiagonalOperator(DoubleArray low, DoubleArray mid, DoubleArray high);
	// operator interface
	DoubleArray solveFor(DoubleArray rhs) const;
	DoubleArray applyTo(DoubleArray v) const;
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


#endif
