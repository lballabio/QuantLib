
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef quantlib_boundary_conditions_i
#define quantlib_boundary_conditions_i

%module BoundaryConditions

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%{
using QuantLib::FiniteDifferences::BoundaryCondition;
typedef BoundaryCondition::Type BoundaryConditionType;
using QuantLib::StringFormatter;
%}

// typemap boundary condition type to the corresponding strings

%typemap(python,in) BoundaryConditionType, const BoundaryConditionType & {
	if (PyString_Check($source)) {
		std::string s(PyString_AsString($source));
		s = StringFormatter::toLowercase(s);
		if (s == "" || s == "none")
		    $target = new BoundaryConditionType(BoundaryCondition::None);
		else if (s == "neumann")
		    $target = new BoundaryConditionType(BoundaryCondition::Neumann);
		else if (s == "dirichlet")
		    $target = new BoundaryConditionType(BoundaryCondition::Dirichlet);
		else {
			PyErr_SetString(PyExc_TypeError,"not a boundary condition type");
			return NULL;
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a boundary condition type");
		return NULL;
	}
};

%typemap(python,freearg) BoundaryConditionType, const BoundaryConditionType & {
	delete $source;
};

%typemap(python,out) BoundaryConditionType, const BoundaryConditionType & {
	switch (*$source) {
	  case BoundaryCondition::None:
	    $target = PyString_FromString("None");
	    break;
	  case BoundaryCondition::Neumann:
	    $target = PyString_FromString("Neumann");
	    break;
	  case BoundaryCondition::Dirichlet:
	    $target = PyString_FromString("Dirichlet");
	    break;
	}
};

%typemap(python,ret) BoundaryConditionType {
	delete $source;
};

// Boundary condition

class BoundaryCondition {
  public:
    // constructors
	BoundaryCondition(BoundaryConditionType type, double value);
	~BoundaryCondition();
	// inspectors
	BoundaryConditionType type() const;
	double value() const;
};


#endif
