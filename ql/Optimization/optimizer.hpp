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
/*! \file optimizer.hpp
    \brief Abstract optimization class

    \fullpath
    ql/Optimization/%optimizer.hpp
*/

#ifndef quantlib_optimization_optimizer_h
#define quantlib_optimization_optimizer_h

#include "ql/array.hpp"
#include "ql/handle.hpp"

#include "ql/Optimization/constraint.hpp"
#include "ql/Optimization/costfunction.hpp"
#include "ql/Optimization/criteria.hpp"

namespace QuantLib {

    namespace Optimization {

        class OptimizationMethod;
        class OptimizationProblem;
        /*!
          Optimization Method abstract class for unconstrained optimization pb
        */
        class OptimizationMethod {
          public:
            explicit OptimizationMethod()
            : iterationNumber_(0), functionEvaluation_(0),
              gradientEvaluation_(0), functionValue_(1), squaredNorm_(1) {}
            virtual ~OptimizationMethod() {}

            //! Set initial value
            inline void setInitialValue(const Array& initialValue) {
                iterationNumber_ = 0;
                initialValue_ = initialValue;
                x_ = initialValue;
                searchDirection_ = Array(x_.size ());
            }
            //! Set optimization end criteria
            inline void setEndCriteria(
                const OptimizationEndCriteria& endCriteria) {
                endCriteria_ = endCriteria;
            }

            //! current iteration number
            inline int& iterationNumber() {
                return iterationNumber_;
            }

            //! optimization end criteria
            inline OptimizationEndCriteria& endCriteria() {
                return endCriteria_;
            }

            //! number of evaluation of cost function
            inline int& functionEvaluation() { return functionEvaluation_; }
            //! number of evaluation of cost function gradient
            inline int& gradientEvaluation() { return gradientEvaluation_; }
            //! value of cost function
            inline double& functionValue() { return functionValue_; }
            //! value of cost function gradient norm
            inline double& gradientNormValue() { return squaredNorm_; }
            //! current value of the local minimum
            Array& x() { return x_; }
            //! current value of the search direction
            Array& searchDirection() { return searchDirection_; }

            //! minimize the optimization problem P
            virtual void minimize(OptimizationProblem& P) = 0;
          protected:
            //! initial value of unknowns
            Array initialValue_;
            //! current iteration step in the Optimization process
            int iterationNumber_;
            //! optimization end criteria
            OptimizationEndCriteria endCriteria_;
            //! number of evaluation of cost function and its gradient
            int functionEvaluation_, gradientEvaluation_;
            //! function and gradient norm values of the last step
            double functionValue_, squaredNorm_;
            //! current values of the local minimum and the search direction
            Array x_, searchDirection_;
        };

        /*!
          Unconstrained optimization pb
        */
        class OptimizationProblem {
          public:
            //! default constructor
            OptimizationProblem(
                CostFunction& f,          // Function and it gradient vector
                Constraint& c,            // Constraint
                OptimizationMethod& meth) // Optimization method
            : costFunction_(f), constraint_(c), method_(meth) {}
            //! destructor
            ~OptimizationProblem() {}

            //! call cost function computation and increment evaluation counter
            double value(const Array& x) {
                method_.functionEvaluation()++;
                return costFunction_.value(x);
            }

            //! call cost function gradient computation and increment
            //  evaluation counter
            void gradient(Array& grad_f, const Array& x) {
                method_.gradientEvaluation()++;
                costFunction_.gradient(grad_f, x);
            }

            //! call cost function computation and it gradient
            double valueAndGradient (Array& grad_f, const Array& x) {
                method_.functionEvaluation()++;
                method_.gradientEvaluation()++;
                return costFunction_.valueAndGradient(grad_f, x);
            }

            //! Unconstrained optimization method
            OptimizationMethod &optimisationMethod () {
                return method_;
            }

            //! constraint
            Constraint& constraint() {
                return constraint_;
            }

            CostFunction& costFunction() {
                return costFunction_;
            }

            //! Minimization
            void minimize() { method_.minimize(*this); }

            Array& minimumValue() { return method_.x (); }

          protected:
            //! Unconstrained cost function
            CostFunction& costFunction_;
            //! Constraint
            Constraint& constraint_;
            //! Unconstrained optimization method
            OptimizationMethod& method_;
        };

    }

}
#endif
