
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

/*! \file solver1d.h
    \brief Abstract 1-D solver class

    $Source$
    $Name$
    $Log$
    Revision 1.10  2001/01/17 13:53:35  nando
    80 columns enforced
    tabs removed
    private data member now have trailing underscore

    Revision 1.9  2000/12/27 14:05:56  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.8  2000/12/20 17:00:57  enri
    modified to use new macros

    Revision 1.7  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

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
        : maxEvaluations_(MAX_FUNCTION_EVALUATIONS), lowBoundEnforced_(false),
          hiBoundEnforced_(false) {}
        virtual ~Solver1D() {}
        //! \name Modifiers
        //@{
        /*! This method returns the zero of the ObjectiveFunction f,
            determined with the given accuracy (i.e., \f$ x \f$ is considered a
            zero if \f$ |f(x)| < accuracy \f$).
            This method contains a bracketing routine to which an initial guess
            must be supplied as well as a step used to scan the range of the
            possible bracketing values.
        */
        double solve(const ObjectiveFunction& f,
                     double xAccuracy,
                     double guess,
                     double step) const;
        /*! This method returns the zero of the ObjectiveFunction f,
            determined with the given accuracy (i.e., \f$ x \f$ is considered a
            zero if \f$ |f(x)| < accuracy \f$). An initial guess must be
            supplied, as well as two values which must bracket the zero (i.e.,
            either \f$ f(x_{min}) > 0 \f$ && \f$ f(x_{max}) < 0 \f$, or \f$
            f(x_{min}) < 0 \f$ && \f$ f(x_{max}) > 0 \f$ must be true).
        */
        double solve(const ObjectiveFunction& f,
                     double xAccuracy,
                     double guess,
                     double xMin_,
                     double xMax_) const;
        /*! This method sets the maximum number of function evaluations for the
            bracketing routine. An Error is thrown if a bracket is not found
            after this number of evaluations.
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
            - <b>fxMin_</b> and <b>fxMax_</b> contain the values of the function
              in <b>xMin_</b> and  <b>xMax_</b>;
            - <b>root</b> was initialized to a valid initial guess.
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
