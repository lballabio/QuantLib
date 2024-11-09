/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file singlefactorbsmbasketengine.hpp
    \brief Basket engine where all underlyings are driven by one stochastic factor
*/

#ifndef quantlib_single_factor_bsm_basket_engine_hpp
#define quantlib_single_factor_bsm_basket_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for baskets where all underlyings are driven by one stochastic factor
    /*! Jaehyuk Choi,
        Sum of all Black-Scholes-Merton Models:
        An efficient Pricing Method for Spread, Basket and Asian Options,
        https://arxiv.org/pdf/1805.03172

        \ingroup basketengines
    */

    class SumExponentialsRootSolver {
      public:
        enum Strategy {Ridder, Newton, Brent, Halley};

        SumExponentialsRootSolver(Array a, Array sig, Real K);

        Real operator()(Real x) const;
        Real derivative(Real x) const;
        Real secondDerivative(Real x) const;

        Real getRoot(Real xTol = 1e6*QL_EPSILON, Strategy strategy = Brent) const;

        Size getFCtr() const;
        Size getDerivativeCtr() const;
        Size getSecondDerivativeCtr() const;

      private:
        const Array a_, sig_;
        const Real K_;
        mutable Size fCtr_, fPrimeCtr_, fDoublePrimeCtr_;
    };

    class SingleFactorBsmBasketEngine : public BasketOption::engine {
      public:
        SingleFactorBsmBasketEngine(
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > p,
            Real xTol = 1e4*QL_EPSILON);

        void calculate() const override;

      private:
        const Real xTol_;
        const Size n_;
        const std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes_;
    };
}


#endif
