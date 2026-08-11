/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yassine Idyiahia

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file gaussiancopulaspreadengine.hpp
    \brief Spread option pricing via nested Gauss-Hermite quadrature on a Gaussian copula
*/

#ifndef quantlib_gaussian_copula_spread_engine_hpp
#define quantlib_gaussian_copula_spread_engine_hpp

#include <ql/instruments/basketoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Gaussian copula engine for spread options with smile-implied marginals
    /*! Prices \f$ \max(S_1 - S_2 - K, 0) \f$ via nested Gauss-Hermite
        quadrature over a Gaussian copula with smile-implied marginals.

        \ingroup basketengines

        \test the correctness of the returned value is tested by
              benchmarking against 2D Dupire local-vol PDE.
    */
    class GaussianCopulaSpreadEngine : public BasketOption::engine {
      public:
        GaussianCopulaSpreadEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
            ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
            Real correlation,
            Size nPoints = 64);

        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process1_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> process2_;
        Real rho_;
        Size nPoints_;
    };
}

#endif
