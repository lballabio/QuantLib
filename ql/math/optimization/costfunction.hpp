/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file costfunction.hpp
    \brief Optimization cost function class
*/

#ifndef quantlib_optimization_costfunction_h
#define quantlib_optimization_costfunction_h

#include <ql/math/array.hpp>

namespace QuantLib {

    //!  Cost function abstract class for optimization problem
    class CostFunction {
      public:
        virtual ~CostFunction() {}
        //! method to overload to compute the cost function value in x
        virtual Real value(const Array& x) const = 0;
        //! method to overload to compute the cost function values in x
        virtual Disposable<Array> values(const Array& x) const =0;

        //! method to overload to compute grad_f, the first derivative of
        //  the cost function with respect to x
        virtual void gradient(Array& grad, const Array& x) const {
            Real eps = finiteDifferenceEpsilon(), fp, fm;
            Array xx(x);
            for (Size i=0; i<x.size(); i++) {
                xx[i] += eps;
                fp = value(xx);
                xx[i] -= 2.0*eps;
                fm = value(xx);
                grad[i] = 0.5*(fp - fm)/eps;
                xx[i] = x[i];
            }
        }

        //! method to overload to compute grad_f, the first derivative of
        //  the cost function with respect to x and also the cost function
        virtual Real valueAndGradient(Array& grad,
                                      const Array& x) const {
            gradient(grad, x);
            return value(x);
        }

        //! Default epsilon for finite difference method :
        virtual Real finiteDifferenceEpsilon() const { return 1e-8; }
    };

    class ParametersTransformation {
      public:
        virtual ~ParametersTransformation() {}
        virtual Array direct(const Array& x) const = 0;
        virtual Array inverse(const Array& x) const = 0;
    };
}

#endif
