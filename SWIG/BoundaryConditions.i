
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_boundary_conditions_i
#define quantlib_boundary_conditions_i

%module BoundaryConditions

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%{
#include "boundarycondition.h"
using QuantLib::PDE::BoundaryCondition;
typedef BoundaryCondition::Type BoundaryConditionType;

#include "dataformatters.h"
using QuantLib::StringFormatter;
%}

// typemap boundary condition type to the corresponding strings

%typemap(python,in) BoundaryConditionType, BoundaryConditionType * {
	if (PyString_Check($source)) {
		std::string s(PyString_AsString($source));
		s = StringFormatter::toLowercase(s);
		if (s == "" || s == "none")	$target = new BoundaryConditionType(BoundaryCondition::None);
		else if (s == "neumann")	$target = new BoundaryConditionType(BoundaryCondition::Neumann);
		else if (s == "dirichlet")	$target = new BoundaryConditionType(BoundaryCondition::Dirichlet);
		else {
			PyErr_SetString(PyExc_TypeError,"not a boundary condition type");
			return NULL;
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a boundary condition type");
		return NULL;
	}
};

%typemap(python,freearg) BoundaryConditionType, BoundaryConditionType * {
	delete $source;
};

%typemap(python,out) BoundaryConditionType, BoundaryConditionType * {
	switch (*$source) {
	  case BoundaryCondition::None:			$target = PyString_FromString("None");		break;
	  case BoundaryCondition::Neumann:		$target = PyString_FromString("Neumann");	break;
	  case BoundaryCondition::Dirichlet:	$target = PyString_FromString("Dirichlet");	break;
	}
};

%typemap(python,ret) BoundaryConditionType {
	delete $source;
};

// Boundary condition

class BoundaryCondition {
  public:
	BoundaryCondition(BoundaryConditionType type, double value);
	BoundaryConditionType type() const;
	double value() const;
};


#endif
