
#ifndef quantlib_solver1d_i
#define quantlib_solver1d_i

%module Solvers1D

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%{
#include "bisection.h"
#include "brent.h"
#include "falseposition.h"
#include "newton.h"
#include "newtonsafe.h"
#include "ridder.h"
#include "secant.h"
%}

// Function to find root of

%{
QL_USING(QuantLib,Function)
%}

class Function {
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
QL_USING(QuantLib,Ensure)

// its C++ container
class PyFunction : public Function {
  public:
	PyFunction(PyObject *pyFunction) : thePyFunction(pyFunction) {}
	double value(double x) const {
		PyObject* pyResult = PyObject_CallMethod(thePyFunction,"value","d",x);
		Ensure(pyResult != NULL, "failed to call value() on Python object");
		double result = PyFloat_AsDouble(pyResult);
		Py_XDECREF(pyResult);
		return result;
	}
	double derivative(double x) const {
		PyObject* pyResult = PyObject_CallMethod(thePyFunction,"derivative","d",x);
		Ensure(pyResult != NULL, "failed to call derivative() on Python object");
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
QL_USING(QuantLib,Solver1D)
%}

class Solver1D {
  public:
	virtual double solve(const Function& f, double xAccuracy, double guess, double step) const;
	%name(bracketedSolve) virtual double solve(const Function& f, double xAccuracy, double guess, double xMin, double xMax) const;
	void setMaxEvaluations(int evaluations);
};

#if defined(SWIGPYTHON)
%addmethods Solver1D {
	double pySolve(PyObject *pyFunction, double xAccuracy, double guess, double step) {
		PyFunction f(pyFunction);
		return self->solve(f, xAccuracy, guess, step);
	}
	double pyBracketedSolve(PyObject *pyFunction, double xAccuracy, double guess, double xMin, double xMax) {
		PyFunction f(pyFunction);
		return self->solve(f, xAccuracy, guess, xMin, xMax);
	}
}
#endif

// Actual solvers

%{
QL_USING(QuantLib::Solvers1D,Bisection)
QL_USING(QuantLib::Solvers1D,Brent)
QL_USING(QuantLib::Solvers1D,FalsePosition)
QL_USING(QuantLib::Solvers1D,Newton)
QL_USING(QuantLib::Solvers1D,NewtonSafe)
QL_USING(QuantLib::Solvers1D,Ridder)
QL_USING(QuantLib::Solvers1D,Secant)
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
