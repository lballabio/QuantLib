
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

#ifndef quantlib_solver1d_i
#define quantlib_solver1d_i

%module Solvers1D

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: Solvers1D is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

// Function to find root of

%{
using QuantLib::ObjectiveFunction;
%}

class ObjectiveFunction {
  public:
	virtual double value(double x) const = 0;
	virtual double derivative(double x) const;
};

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
	double value(double x) const {
		PyObject* pyResult = PyObject_CallMethod(thePyFunction,"value","d",x);
		QL_ENSURE(pyResult != NULL, "failed to call value() on Python object");
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

// 1D Solver interface

%{
using QuantLib::Solver1D;
%}

class Solver1D {
  public:
	virtual double solve(const ObjectiveFunction& f, double xAccuracy, 
	  double guess, double step) const;
	%name(bracketedSolve) virtual double solve(const ObjectiveFunction& f,
	  double xAccuracy, double guess, double xMin, double xMax) const;
	void setMaxEvaluations(int evaluations);
};

#if defined(SWIGPYTHON)
%addmethods Solver1D {
	double pySolve(PyObject *pyFunction, double xAccuracy, double guess, 
	  double step) {
		PyObjectiveFunction f(pyFunction);
		return self->solve(f, xAccuracy, guess, step);
	}
	double pyBracketedSolve(PyObject *pyFunction, double xAccuracy, 
	  double guess, double xMin, double xMax) {
		PyObjectiveFunction f(pyFunction);
		return self->solve(f, xAccuracy, guess, xMin, xMax);
	}
}
#endif

// Actual solvers

%{
using QuantLib::Solvers1D::Bisection;
using QuantLib::Solvers1D::Brent;
using QuantLib::Solvers1D::FalsePosition;
using QuantLib::Solvers1D::Newton;
using QuantLib::Solvers1D::NewtonSafe;
using QuantLib::Solvers1D::Ridder;
using QuantLib::Solvers1D::Secant;
%}

class Bisection : public Solver1D {
  public:
	Bisection();
	~Bisection();
};

class Brent : public Solver1D {
  public:
	Brent();
	~Brent();
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
