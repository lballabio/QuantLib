
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
    Revision 1.16  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_solver1d_i
#define quantlib_solver1d_i

// Function to find root of

%{
using QuantLib::ObjectiveFunction;
%}

#if defined(SWIGPYTHON)

// Python function to solve for
%typemap(python,in) PyObject *pyFunction {
	$target = $source;
}

%{
// its C++ container
class PyObjectiveFunction : public ObjectiveFunction {
  public:
	PyObjectiveFunction(PyObject *pyFunction) : thePyFunction(pyFunction) {}
	double operator()(double x) const {
		PyObject* pyResult = PyObject_CallFunction(thePyFunction,"d",x);
		QL_ENSURE(pyResult != NULL, "failed to call Python function");
		double result = PyFloat_AsDouble(pyResult);
		Py_XDECREF(pyResult);
		return result;
	}
	double derivative(double x) const {
		PyObject* pyResult =
		  PyObject_CallMethod(thePyFunction,"derivative","d",x);
		QL_ENSURE(pyResult != NULL,
		  "failed to call derivative() on Python object");
		double result = PyFloat_AsDouble(pyResult);
		Py_XDECREF(pyResult);
		return result;
	}
  private:
	PyObject* thePyFunction;
};
%}
#endif

#if defined(SWIGRUBY)
%{
#include <iostream>
// C++ container for Ruby block
class RubyObjectiveFunction : public ObjectiveFunction {
  public:
    double operator()(double x) const {
        return NUM2DBL(rb_yield(rb_float_new(x)));
    }
};
%}
#endif

// 1D Solver interface

%{
using QuantLib::Solver1D;
using QuantLib::Solvers1D::Bisection;
using QuantLib::Solvers1D::Brent;
using QuantLib::Solvers1D::FalsePosition;
using QuantLib::Solvers1D::Newton;
using QuantLib::Solvers1D::NewtonSafe;
using QuantLib::Solvers1D::Ridder;
using QuantLib::Solvers1D::Secant;
%}

class Solver1D {
  public:
    void setMaxEvaluations(int evaluations);
    void setLowBound(double lowBound);
    %pragma(ruby) alias = "lowBound= setLowBound";
    void setHiBound(double hiBound);
    %pragma(ruby) alias = "hiBound= setHiBound";
};

#if defined(SWIGPYTHON)
%addmethods Solver1D {
    double solve(PyObject *pyFunction, double xAccuracy, double guess,
        double step) {
            PyObjectiveFunction f(pyFunction);
            return self->solve(f, xAccuracy, guess, step);
    }
    double bracketedSolve(PyObject *pyFunction, double xAccuracy,
        double guess, double xMin, double xMax) {
            PyObjectiveFunction f(pyFunction);
            return self->solve(f, xAccuracy, guess, xMin, xMax);
    }
}
#endif

#if defined(SWIGRUBY)
%addmethods Solver1D {
    void crash() {}
    double solve(double xAccuracy, double guess, double step) {
        RubyObjectiveFunction f;
        return self->solve(f, xAccuracy, guess, step);
    }
    double bracketedSolve(double xAccuracy, double guess,
      double xMin, double xMax) {
        RubyObjectiveFunction f;
        return self->solve(f, xAccuracy, guess, xMin, xMax);
    }
}
#endif

// Actual solvers

#if defined(SWIGPYTHON) || defined(SWIGRUBY)
class Brent : public Solver1D {
  public:
    Brent();
    ~Brent();
};
#endif

#if defined(SWIGPYTHON)
class Bisection : public Solver1D {
  public:
    Bisection();
    ~Bisection();
};

class FalsePosition : public Solver1D {
  public:
    FalsePosition();
    ~FalsePosition();
};

class Newton : public Solver1D {
  public:
    Newton();
    ~Newton();
};

class NewtonSafe : public Solver1D {
  public:
    NewtonSafe();
    ~NewtonSafe();
};

class Ridder : public Solver1D {
  public:
    Ridder();
    ~Ridder();
};

class Secant : public Solver1D {
  public:
    Secant();
    ~Secant();
};
#endif


#endif
