
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.13  2001/04/23 12:29:29  lballabio
    Fixed linking in setup.py (and some tweakings in SWIG interfaces)

    Revision 1.12  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_boundary_conditions_i
#define quantlib_boundary_conditions_i

%{
using QuantLib::FiniteDifferences::BoundaryCondition;
typedef BoundaryCondition::Type BoundaryConditionType;
using QuantLib::StringFormatter;
%}

// typemap boundary condition type to the corresponding strings

%typemap(python,in) BoundaryConditionType (BoundaryConditionType temp), 
  const BoundaryConditionType & (BoundaryConditionType temp) {
    if (PyString_Check($source)) {
        std::string s(PyString_AsString($source));
        s = StringFormatter::toLowercase(s);
        if (s == "" || s == "none")
            temp = BoundaryCondition::None;
        else if (s == "neumann")
            temp = BoundaryCondition::Neumann;
        else if (s == "dirichlet")
            temp = BoundaryCondition::Dirichlet;
        else {
            PyErr_SetString(PyExc_TypeError,"not a boundary condition type");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a boundary condition type");
        return NULL;
    }
    $target = &temp;
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

%typemap(ruby,in) BoundaryConditionType (BoundaryConditionType temp), 
  const BoundaryConditionType & (BoundaryConditionType temp) {
    if (TYPE($source) == T_STRING) {
        std::string s(STR2CSTR($source));
        s = StringFormatter::toLowercase(s);
        if (s == "" || s == "none")
            temp = BoundaryCondition::None;
        else if (s == "neumann")
            temp = BoundaryCondition::Neumann;
        else if (s == "dirichlet")
            temp = BoundaryCondition::Dirichlet;
        else {
            rb_raise(rb_eTypeError,"not a boundary condition type");
        }
    } else {
        rb_raise(rb_eTypeError,"not a boundary condition type");
    }
    $target = &temp;
};

%typemap(ruby,out) BoundaryConditionType, const BoundaryConditionType & {
    switch (*$source) {
      case BoundaryCondition::None:
        $target = rb_str_new2("None");
        break;
      case BoundaryCondition::Neumann:
        $target = rb_str_new2("Neumann");
        break;
      case BoundaryCondition::Dirichlet:
        $target = rb_str_new2("Dirichlet");
        break;
    }
};

%typemap(ruby,ret) BoundaryConditionType {
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

