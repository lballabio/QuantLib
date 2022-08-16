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

/*! \file qdfpamericanengine.hpp
*/

#ifndef quantlib_qd_fp_american_engine_hpp
#define quantlib_qd_fp_american_engine_hpp

#include <ql/pricingengines/vanilla/qdplusamericanengine.hpp>

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

    class QdFpIterationScheme {
      public:
        virtual Size getNumberOfChebyshevInterpolationNodes() const = 0;
        virtual Size getNumberOfNaiveFixedPointSteps() const = 0;
        virtual Size getNumberOfJacobiNewtonFixedPointSteps() const = 0;

        virtual ext::shared_ptr<Integrator>
            getFixedPointIntegrator() const = 0;
        virtual ext::shared_ptr<Integrator>
            getExerciseBoundaryToPriceIntegrator() const = 0;

        virtual ~QdFpIterationScheme() {}
    };

    /* Gauss-Legendre (l,m,n)-p Scheme
         l: order of Gauss-Legendre integration within every fixed point iterations
            step.
         m: fixed point iteration steps, first step is a partial Jacobi-Newton,
            the rest are naive Richardson fixed point iterations
         n: number of Chebyshev nodes to interpolate the exercise boundary
         p: order of Gauss-Legendre integration in final conversion of the
            exercise boundary into option prices
    */
    class QdFpLegendreIterationScheme: public QdFpIterationScheme {
      public:
        QdFpLegendreIterationScheme(Size l, Size m, Size n, Size p);

        Size getNumberOfChebyshevInterpolationNodes() const override;
        Size getNumberOfNaiveFixedPointSteps() const override;
        Size getNumberOfJacobiNewtonFixedPointSteps() const override;

        ext::shared_ptr<Integrator>
            getFixedPointIntegrator() const override;
        ext::shared_ptr<Integrator>
            getExerciseBoundaryToPriceIntegrator() const override;

      private:
        const Size m_, n_;
        const ext::shared_ptr<Integrator> fpIntegrator_;
        const ext::shared_ptr<Integrator> exerciseBoundaryIntegrator_;
    };

    class QdFpIterationSchemeStdFactory {
      public:
        static ext::shared_ptr<QdFpIterationScheme> fastScheme();
        static ext::shared_ptr<QdFpIterationScheme> accurateScheme();
        static ext::shared_ptr<QdFpIterationScheme> highPrecisionScheme();
        static ext::shared_ptr<QdFpIterationScheme> tanhSinhScheme();

      private:
        static ext::shared_ptr<QdFpIterationScheme>
            fastScheme_, accurateScheme_, highPrecisionScheme_, tanhSinhScheme_;
    };

    class QdFpAmericanEngine : public detail::QdPutCallParityEngine {
      public:
        explicit QdFpAmericanEngine(
          ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess,
          ext::shared_ptr<QdFpIterationScheme> iterationScheme =
              QdFpIterationSchemeStdFactory::fastScheme());

      protected:
        Real calculatePut(
            Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const override;

      private:
        const ext::shared_ptr<QdFpIterationScheme> iterationScheme_;
    };
}

#endif
