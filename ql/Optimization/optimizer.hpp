
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
            virtual void Minimize(OptimizationProblem& P) = 0;
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
                OptimizationMethod& meth) // Optimization method
            : costFunction_(f), method_(meth) {}
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

            //! Minimization
            void Minimize() { method_.Minimize(*this); }

            Array& minimumValue() { return method_.x (); }

          protected:
            //! Unconstrained cost function
            CostFunction& costFunction_;
            //! Unconstrained optimization method
            OptimizationMethod& method_;
 
        };

    }

}
#endif
