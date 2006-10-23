/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/Swaption/lfmswaptionengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    void LfmSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementType==Settlement::Physical,
                   "cash-settled swaptions not priced with Lfm engine");

        static const Spread basisPoint = 1.0e-4;
        Time exercise = arguments_.stoppingTimes[0];
        Time length   = arguments_.fixedPayTimes.back()
                             - arguments_.fixedResetTimes[0];
        Option::Type w = arguments_.type==VanillaSwap::Payer ?
                                                Option::Call : Option::Put;
        Volatility vol = model_->getSwaptionVolatilityMatrix()
                                           ->volatility(exercise, length,
                                                        arguments_.fairRate,
                                                        true);
        results_.value = (arguments_.fixedBPS/basisPoint) *
            blackFormula(w, arguments_.fixedRate, arguments_.fairRate,
                         vol*std::sqrt(exercise));
    }

}

