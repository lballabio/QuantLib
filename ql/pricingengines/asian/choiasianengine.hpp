/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Klaus Spanderen

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

/*! \file choiasianengine.hpp
    \brief Black Scholes arithmetic Asian option engine
*/

#ifndef quantlib_choi_asian_engine_hpp
#define quantlib_choi_asian_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {
    //! Pricing engine for arithmetic Asian options
    /*! This class replicates an arithmetic Asian option using a basket option.
        The pricing of an arithmetic Asian option is substituted with the pricing
        of a basket option.

        \ingroup asianengines
    */
    class ChoiBasketEngine;

    class ChoiAsianEngine
            : public GenericEngine<DiscreteAveragingAsianOption::arguments,
                                   DiscreteAveragingAsianOption::results> {
      public:
        explicit ChoiAsianEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> p,
            Real lambda = 15,
            Size maxNrIntegrationSteps = 2 << 21);

        void calculate() const override;

      private:
        const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        const Real lambda_;
        const Size maxNrIntegrationSteps_;
    };
}

#endif
