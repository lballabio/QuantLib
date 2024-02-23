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

/*! \file operatorsplittingspreadengine.hpp
    \brief Analytic operator splitting approximation by Chi-Fai Lo (2015)
*/

#ifndef quantlib_operator_splitting_spread_engine_hpp
#define quantlib_operator_splitting_spread_engine_hpp

#include <ql/pricingengines/basket/spreadblackscholesvanillaengine.hpp>

namespace QuantLib {

    //! Pricing engine for spread option on two futures
    /*! Chi-Fai Lo,
        Pricing Spread Options by the Operator Splitting Method,
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2429696

        \ingroup basketengines
    */
    class OperatorSplittingSpreadEngine : public SpreadBlackScholesVanillaEngine {
      public:
        OperatorSplittingSpreadEngine(
            ext::shared_ptr<BlackProcess> process1,
            ext::shared_ptr<BlackProcess> process2,
            Real correlation);

      protected:
        Real calculate(
          Real strike, Option::Type optionType,
          Real variance1, Real variance2, DiscountFactor df) const override;
    };
}


#endif
