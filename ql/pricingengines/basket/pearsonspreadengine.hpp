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

/*! \file pearsonspreadengine.hpp
    \brief Pearson (1995) spread option pricing via 1-D numerical integration
*/

#ifndef quantlib_pearson_spread_engine_hpp
#define quantlib_pearson_spread_engine_hpp

#include <ql/pricingengines/basket/spreadblackscholesvanillaengine.hpp>

namespace QuantLib {

    //! Pricing engine for spread options using Pearson (1995)
    /*! References: Neil D. Pearson, "An Efficient Approach for
        Pricing Spread Options", Journal of Derivatives, 3 (1995), 76-91.

        \ingroup basketengines

        \test the correctness of the returned value is tested by
              benchmarking against 2D PDE spread-option pricing.
    */
    class PearsonSpreadEngine : public SpreadBlackScholesVanillaEngine {
      public:
        PearsonSpreadEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
                            ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
                            Real correlation,
                            Real integrationTolerance = 1e-10,
                            Size maxIntegrationIterations = 10000,
                            Real nStd = 8.0);

      protected:
        Real calculate(Real f1, Real f2, Real strike, Option::Type optionType,
                       Real variance1, Real variance2, DiscountFactor df) const override;

      private:
        Real integrationTolerance_;
        Size maxIntegrationIterations_;
        Real nStd_;
    };
}

#endif
