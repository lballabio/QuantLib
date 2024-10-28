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

/*! \file choibasketengine.hpp
    \brief Jaehyuk Choi: Sum of all Black-Scholes-Merton Models
*/

#ifndef quantlib_choi_basket_engine_hpp
#define quantlib_choi_basket_engine_hpp

#include <ql/instruments/basketoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for basket option on multiple underlyings
    /*! This class implements the pricing formula from
        "Sum of all Black-Scholes-Merton Models: An efficient Pricing Method for
        Spread, Basket and Asian Options", Jaehyuk Choi, 2018
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2913048

        Python implementation from the author of the paper is also available
        https://github.com/PyFE/PyFENG

        \ingroup basketengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class ChoiBasketEngine : public BasketOption::engine {
      public:
        // lambda controls the precision,
        // fast: 4, accurate: 8, high precision: 20
        ChoiBasketEngine(
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
            Matrix rho,
            Real lambda = 10.0,
            Size maxNrIntegrationSteps = std::numeric_limits<Size>::max(),
            bool calcfwdDelta = false,
            bool controlVariate = false);

        void calculate() const override;

      private:
        const Size n_;
        const std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes_;
        const Matrix rho_;
        const Real lambda_;
        const Size maxNrIntegrationSteps_;
        const bool calcFwdDelta_, controlVariate_;
    };
}

#endif
