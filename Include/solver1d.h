
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

// $Source$

// $Log$
// Revision 1.6  2000/12/13 18:09:52  nando
// CVS keyword added
//

/*! \file solver1d.h
	\brief Abstract 1-D solver class
*/

/*! \namespace QuantLib::Solvers1D
	\brief Concrete implementations of the Solver1D interface
*/

#ifndef quantlib_solver1d_h
#define quantlib_solver1d_h

#include "qldefines.h"
#include "null.h"
#include "qlerrors.h"
#include "dataformatters.h"
#include <cmath>
#include <limits>

namespace QuantLib {

	#define MAX_FUNCTION_EVALUATIONS 100

	//! Objective function for 1-D solvers
	/*! This is the function whose zeroes must be found.
	*/
	class ObjectiveFunction {
	  public:
		virtual ~ObjectiveFunction() {}
		//! returns \f$ f(x) \f$
		virtual double value(double x) const = 0;
		//! returns \f$ f'(x) \f$
		virtual double derivative(double x) const { return Null<double>(); }
	};

	//! Abstract base class for 1-D solvers
	class Solver1D {
	  public:
		Solver1D()
		: maxEvaluations(MAX_FUNCTION_EVALUATIONS), lowBoundEnforced(false), hiBoundEnforced(false) {}
		virtual ~Solver1D() {}
		//! \name Modifiers
		//@{
		/*! This method returns the zero of the ObjectiveFunction f, determined with the given accuracy
			(i.e., \f$ x \f$ is considered a zero if \f$ |f(x)| < accuracy \f$). This method contains
			a bracketing routine to which an initial guess must be supplied as well as a step used to scan
			the range of the possible bracketing values.
		*/
		double solve(const ObjectiveFunction& f, double xAccuracy, double guess, double step) const;
		/*! This method returns the zero of the ObjectiveFunction f, determined with the given accuracy
			(i.e., \f$ x \f$ is considered a zero if \f$ |f(x)| < accuracy \f$). An initial guess must be 
			supplied, as well as two values which must bracket the zero (i.e., either \f$ f(x_{min}) > 0 \f$
			&& \f$ f(x_{max}) < 0 \f$, or \f$ f(x_{min}) < 0 \f$ && \f$ f(x_{max}) > 0 \f$ must be true).
		*/
		double solve(const ObjectiveFunction& f, double xAccuracy, double guess, double xMin, double xMax) const;
		/*! This method sets the maximum number of function evaluations for the bracketing routine. An Error
			is thrown if a bracket is not found after this number of evaluations.
		*/
		void setMaxEvaluations(int evaluations);
		//! sets the lower bound for the function domain
		void setLowBound(double lowBound) {theLowBound = lowBound; lowBoundEnforced = true; }
		//! sets the upper bound for the function domain
		void setHiBound(double hiBound) {theHiBound = hiBound;  hiBoundEnforced = true; }
		//@}
	  protected:
		/*! This method must be implemented in derived classes and contains the actual
			code which searches for the zeroes of the ObjectiveFunction. It assumes that:
			- <b>xMin</b> and  <b>xMax</b> form a valid bracket;
			- <b>fxMin</b> and <b>fxMax</b> contain the values of the function in <b>xMin</b> and  <b>xMax</b>;
			- <b>root</b> was initialized to a valid initial guess.
		*/
		virtual double _solve(const ObjectiveFunction& f, double xAccuracy) const = 0;
		mutable double root, xMin, xMax, fxMin, fxMax;
		int maxEvaluations;
		mutable int evaluationNumber;
	  private:
		double enforceBounds(double x) const;
		double theLowBound, theHiBound;
		bool lowBoundEnforced, hiBoundEnforced;
	};
	
	
	// inline definitions
	
	inline void Solver1D::setMaxEvaluations(int evaluations) {
		Require(evaluations > 0, "negative or null evaluations number");
		maxEvaluations = evaluations;
	}
	
	inline double Solver1D::enforceBounds(double x) const {
		if (lowBoundEnforced && x < theLowBound)
		return theLowBound;
	
		if (hiBoundEnforced && x > theHiBound)
		return theHiBound;
	
		return x;
	}

}


#endif
