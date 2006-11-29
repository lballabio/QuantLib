/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file method.hpp
    \brief Abstract optimization method class
*/

#ifndef quantlib_optimization_method_h
#define quantlib_optimization_method_h

#include <ql/Utilities/null.hpp>
#include <ql/Optimization/constraint.hpp>
#include <ql/Optimization/costfunction.hpp>
#include <ql/Optimization/criteria.hpp>

namespace QuantLib {

    class Problem;

    //! Abstract class for constrained optimization method
    class OptimizationMethod {
      public:
        OptimizationMethod(const Array& initialValue = Array(),
                           const EndCriteria& endCriteria = EndCriteria());
        virtual ~OptimizationMethod() {}

        //! Set initial value
        virtual void setInitialValue(const Array& initialValue);

        //! Set optimization end criteria
        void setEndCriteria(const EndCriteria& endCriteria);

        //! minimize the optimization problem P
        virtual void minimize(const Problem& P) = 0;

        //! current value of the local minimum
        const Array& x() const { return x_; }

        //! current iteration number
        Integer iterationNumber() const { return iterationNumber_; }

        //! optimization end criteria
        const EndCriteria& endCriteria() const { return endCriteria_; }

        //! number of evaluation of cost function
        Integer& functionEvaluation() { return functionEvaluation_; }
        Integer functionEvaluation() const { return functionEvaluation_; }

        //! number of evaluation of cost function gradient
        Integer& gradientEvaluation() { return gradientEvaluation_; }
        Integer gradientEvaluation() const { return gradientEvaluation_; }

        //! value of cost function
        Real& functionValue() { return functionValue_; }
        Real functionValue() const { return functionValue_; }

        //! value of cost function gradient norm
        Real& gradientNormValue() { return squaredNorm_; }
        Real gradientNormValue() const { return squaredNorm_; }

      protected:
        void reset();
        //! current iteration step in the Optimization process
        Integer iterationNumber_;
        //! number of evaluation of cost function and its gradient
        Integer functionEvaluation_, gradientEvaluation_;
        //! function and gradient norm values of the last step
        Real functionValue_, squaredNorm_;
        //! optimization end criteria
        EndCriteria endCriteria_;
        //! current values of the local minimum
        Array x_;
    };

}

#endif
