/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Klaus Spanderen

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

/*! \file qrfpamericanengine.hpp
*/

#ifndef quantlib_qr_fp_american_engine_hpp
#define quantlib_qr_fp_american_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {
    class Integrator;

    /*! High performance/precision American engine based on
        fixed point iteration for the exercise boundary
     */

    /*! References:
        Leif Andersen, Mark Lake and Dimitri Offengenden (2015)
        "High Performance American Option Pricing",
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2547027

        Leif Andersen, Mark Lake (2021)
        "Fast American Option Pricing: The Double-Boundary Case"

        https://onlinelibrary.wiley.com/doi/abs/10.1002/wilm.10969
    */

    /* Gauss-Legendre (l,m,n)-p Scheme
         l: order of Gauss-Legendre integration in every fixed point iterations
            step.
         m: fixed point iteration steps, first step is a partial Jacobi-Newton,
            the rest are naive Richardson fixed point iterations
         n: number of Chebyshev nodes to interpolate the exercise boundary
         p: order of Gauss-Legendre integration in final conversion of the
            exercise boundary into option prices
    */

    class QdFixedPointInterationScheme {
      public:
        virtual Size getNumberOfChebyshevNodes() const = 0;
        virtual Size getNumberOfNaiveFixedPointSteps() const = 0;
        virtual Size getNumberOfJacobiNewtonFixedPointSteps() const = 0;

        virtual ext::shared_ptr<Integrator>
            getFixedPointInteration() const = 0;
        virtual ext::shared_ptr<Integrator>
            getExerciseBoundaryToPriceIntegration() const = 0;
    };

    class QdFpAmericanEngine : public VanillaOption::engine {
      public:
        explicit QdFpAmericanEngine(
          ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess);

        void calculate() const override;
      private:
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess_;
    };
}

#endif
