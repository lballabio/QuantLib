/*
 Copyright (C) 2001, 2002 Nicolas Di Césaré

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
/*! \file problem.hpp
    \brief Abstract optimization class

    \fullpath
    ql/Optimization/%problem.hpp
*/

#ifndef quantlib_optimization_problem_h
#define quantlib_optimization_problem_h

#include <ql/Optimization/method.hpp>

namespace QuantLib {

    namespace Optimization {

        //! Constrained optimization problem
        class Problem {
          public:
            //! default constructor
            Problem(CostFunction& f,// Function and it gradient vector
                    Constraint& c,  // Constraint
                    Method& meth)   // Optimization method
            : costFunction_(f), constraint_(c), method_(meth) {}
            //! destructor
            ~Problem() {}

            //! call cost function computation and increment evaluation counter
            double value(const Array& x);

            //! call cost function gradient computation and increment
            //  evaluation counter
            void gradient(Array& grad_f, const Array& x);

            //! call cost function computation and it gradient
            double valueAndGradient(Array& grad_f, const Array& x);

            //! Constrained optimization method
            Method& method() { return method_; }

            //! Constraint
            Constraint& constraint() { return constraint_; }

            //! Cost function
            CostFunction& costFunction() { return costFunction_; }

            //! Minimization
            void minimize() { method_.minimize(*this); }

            Array& minimumValue() { return method_.x (); }

          protected:
            //! Unconstrained cost function
            CostFunction& costFunction_;
            //! Constraint
            Constraint& constraint_;
            //! constrained optimization method
            Method& method_;
        };

        // inline definitions

        inline double Problem::value(const Array& x) {
            method_.functionEvaluation()++;
            return costFunction_.value(x);
        }

        inline void Problem::gradient(Array& grad_f, const Array& x) {
            method_.gradientEvaluation()++;
            costFunction_.gradient(grad_f, x);
        }

        inline double Problem::valueAndGradient(Array& grad_f, const Array& x) {
            method_.functionEvaluation()++;
            method_.gradientEvaluation()++;
            return costFunction_.valueAndGradient(grad_f, x);
        }

    }

}
#endif
