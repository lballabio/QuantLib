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
/*! \file method.hpp
    \brief Abstract optimization method class

    \fullpath
    ql/Optimization/%method.hpp
*/

#ifndef quantlib_optimization_method_h
#define quantlib_optimization_method_h

#include <ql/handle.hpp>
#include <ql/Optimization/constraint.hpp>
#include <ql/Optimization/costfunction.hpp>
#include <ql/Optimization/criteria.hpp>

namespace QuantLib {

    namespace Optimization {

        class Problem;

        //! Optimization Method abstract class for constrained optimization pb
        class Method {
          public:
            explicit Method()
            : iterationNumber_(0), functionEvaluation_(0),
              gradientEvaluation_(0), functionValue_(1), squaredNorm_(1) {}
            virtual ~Method() {}

            //! Set initial value
            void setInitialValue(const Array& initialValue);

            //! Set optimization end criteria
            inline void setEndCriteria(const EndCriteria& endCriteria) {
                endCriteria_ = endCriteria;
            }

            //! current iteration number
            inline int& iterationNumber() {
                return iterationNumber_;
            }

            //! optimization end criteria
            inline EndCriteria& endCriteria() { return endCriteria_; }

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
            virtual void minimize(Problem& P) = 0;
          protected:
            //! initial value of unknowns
            Array initialValue_;
            //! current iteration step in the Optimization process
            int iterationNumber_;
            //! optimization end criteria
            EndCriteria endCriteria_;
            //! number of evaluation of cost function and its gradient
            int functionEvaluation_, gradientEvaluation_;
            //! function and gradient norm values of the last step
            double functionValue_, squaredNorm_;
            //! current values of the local minimum and the search direction
            Array x_, searchDirection_;
        };

        // Inline methods

        inline void Method::setInitialValue(const Array& initialValue) {
            iterationNumber_ = 0;
            initialValue_ = initialValue;
            x_ = initialValue;
            searchDirection_ = Array(x_.size ());
        }
        
    }

}
#endif
