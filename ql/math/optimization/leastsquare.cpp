/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/math/optimization/conjugategradient.hpp>
#include <ql/math/optimization/leastsquare.hpp>
#include <ql/math/optimization/problem.hpp>
#include <utility>

namespace QuantLib {

    Real LeastSquareFunction::value(const Array & x) const {
        // size of target and function to fit vectors
        Array target(lsp_.size()), fct2fit(lsp_.size());
        // compute its values
        lsp_.targetAndValue(x, target, fct2fit);
        // do the difference
        Array diff = target - fct2fit;
        // and compute the scalar product (square of the norm)
        return DotProduct(diff, diff);
    }

    Disposable<Array> LeastSquareFunction::values(const Array& x) const {
        // size of target and function to fit vectors
        Array target(lsp_.size()), fct2fit(lsp_.size());
        // compute its values
        lsp_.targetAndValue(x, target, fct2fit);
        // do the difference
        Array diff = target - fct2fit;
        return diff*diff;
    }

    void LeastSquareFunction::gradient(Array& grad_f,
                                       const Array& x) const {
        // size of target and function to fit vectors
        Array target (lsp_.size ()), fct2fit (lsp_.size ());
        // size of gradient matrix
        Matrix grad_fct2fit (lsp_.size (), x.size ());
        // compute its values
        lsp_.targetValueAndGradient(x, grad_fct2fit, target, fct2fit);
        // do the difference
        Array diff = target - fct2fit;
        // compute derivative
        grad_f = -2.0*(transpose(grad_fct2fit)*diff);
    }

    Real LeastSquareFunction::valueAndGradient(Array& grad_f,
                                               const Array& x) const {
        // size of target and function to fit vectors
        Array target(lsp_.size()), fct2fit(lsp_.size());
        // size of gradient matrix
        Matrix grad_fct2fit(lsp_.size(), x.size());
        // compute its values
        lsp_.targetValueAndGradient(x, grad_fct2fit, target, fct2fit);
        // do the difference
        Array diff = target - fct2fit;
        // compute derivative
        grad_f = -2.0*(transpose(grad_fct2fit)*diff);
        // and compute the scalar product (square of the norm)
        return DotProduct(diff, diff);
    }

    NonLinearLeastSquare::NonLinearLeastSquare(Constraint& c,
                                               Real accuracy,
                                               Size maxiter)
    : exitFlag_(-1), accuracy_ (accuracy), maxIterations_ (maxiter),
      om_ (ext::shared_ptr<OptimizationMethod>(new ConjugateGradient())),
      c_(c)
    {}

    NonLinearLeastSquare::NonLinearLeastSquare(Constraint& c,
                                               Real accuracy,
                                               Size maxiter,
                                               ext::shared_ptr<OptimizationMethod> om)
    : exitFlag_(-1), accuracy_(accuracy), maxIterations_(maxiter), om_(std::move(om)), c_(c) {}

    Array& NonLinearLeastSquare::perform(LeastSquareProblem& lsProblem) {
        Real eps = accuracy_;

        // wrap the least square problem in an optimization function
        LeastSquareFunction lsf(lsProblem);

        // define optimization problem
        Problem P(lsf, c_, initialValue_);

        // minimize
        EndCriteria ec(maxIterations_,
            std::min(static_cast<Size>(maxIterations_/2), static_cast<Size>(100)),
            eps, eps, eps);
        exitFlag_ = om_->minimize(P, ec);

        // summarize results of minimization
        //        nbIterations_ = om_->iterationNumber();

        results_ = P.currentValue();
        resnorm_ = P.functionValue();
        bestAccuracy_ = P.functionValue();

        return results_;
    }

}
