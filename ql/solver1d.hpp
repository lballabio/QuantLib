

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file solver1d.hpp
    \brief Abstract 1-D solver class

    \fullpath
    ql/%solver1d.hpp
*/

// $Id$

#ifndef quantlib_solver1d_h
#define quantlib_solver1d_h

#include <ql/null.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    /*! \namespace QuantLib::Solvers1D
        \brief Concrete implementations of the Solver1D interface

        See sect. \ref solvers1d
    */

    #define MAX_FUNCTION_EVALUATIONS 100

    //! Objective function for 1-D solvers
    /*! This is the function whose zeroes must be found.
    */
    class ObjectiveFunction {
      public:
        virtual ~ObjectiveFunction() {}
        //! returns \f$ f(x) \f$
        virtual double operator()(double x) const = 0;
        //! returns \f$ f'(x) \f$
        virtual double derivative(double x) const { return Null<double>(); }
    };

    //! Abstract base class for 1-D solvers
    class Solver1D {
      public:
        Solver1D()
        : maxEvaluations_(MAX_FUNCTION_EVALUATIONS),
          lowBoundEnforced_(false), hiBoundEnforced_(false) {}
        virtual ~Solver1D() {}
        //! \name Modifiers
        //@{
        /*! This method returns the zero of the ObjectiveFunction f,
            determined with the given accuracy (i.e., \f$ x \f$ is considered
            a zero if \f$ |f(x)| < accuracy \f$).
            This method contains a bracketing routine to which an initial
            guess must be supplied as well as a step used to scan the range
            of the possible bracketing values.
        */
        double solve(const ObjectiveFunction& f,
                     double xAccuracy,
                     double guess,
                     double step) const;
        /*! This method returns the zero of the ObjectiveFunction f,
            determined with the given accuracy (i.e., \f$ x \f$ is considered
            a zero if \f$ |f(x)| < accuracy \f$). An initial guess must be
            supplied, as well as two values which must bracket the zero
            (i.e., either \f$ f(x_{min}) > 0 \f$ && \f$ f(x_{max}) < 0 \f$,
            or \f$ f(x_{min}) < 0 \f$ && \f$ f(x_{max}) > 0 \f$ must be
            true).
        */
        double solve(const ObjectiveFunction& f,
                     double xAccuracy,
                     double guess,
                     double xMin,
                     double xMax) const;
        /*! This method sets the maximum number of function evaluations for
            the bracketing routine. An Error is thrown if a bracket is not
            found after this number of evaluations.
        */
        void setMaxEvaluations(int evaluations);
        //! sets the lower bound for the function domain
        void setLowBound(double lowBound) {
            lowBound_ = lowBound;
            lowBoundEnforced_ = true;
        }
        //! sets the upper bound for the function domain
        void setHiBound(double hiBound) {
            hiBound_ = hiBound;
            hiBoundEnforced_ = true;
        }
        //@}
      protected:
        /*! This method must be implemented in derived classes and contains
            the actual code which searches for the zeroes of the
            ObjectiveFunction. It assumes that:
            - <b>xMin_</b> and  <b>xMax_</b> form a valid bracket;
            - <b>fxMin_</b> and <b>fxMax_</b> contain the values of the
              function in <b>xMin_</b> and  <b>xMax_</b>;
            - <b>root_</b> was initialized to a valid initial guess.
        */
        virtual double solve_(const ObjectiveFunction& f,
                              double xAccuracy) const = 0;
        mutable double root_, xMin_, xMax_, fxMin_, fxMax_;
        int maxEvaluations_;
        mutable int evaluationNumber_;
      private:
        double enforceBounds_(double x) const;
        double lowBound_, hiBound_;
        bool lowBoundEnforced_, hiBoundEnforced_;
    };


    // inline definitions

    inline void Solver1D::setMaxEvaluations(int evaluations) {
        QL_REQUIRE(evaluations > 0, "negative or null evaluations number");
        maxEvaluations_ = evaluations;
    }

    inline double Solver1D::enforceBounds_(double x) const {
        if (lowBoundEnforced_ && x < lowBound_)
        return lowBound_;

        if (hiBoundEnforced_ && x > hiBound_)
        return hiBound_;

        return x;
    }

}


#endif
