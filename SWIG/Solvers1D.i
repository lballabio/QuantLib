
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
   Revision 1.13  2001/04/02 10:59:48  lballabio
   Changed ObjectiveFunction::value to ObjectiveFunction::operator() - also in Python module

   Revision 1.12  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

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

// Actual solvers

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
