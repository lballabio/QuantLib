/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen
 
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

#include <ql/math/functional.hpp>
#include <ql/pricingengines/basket/kirkengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <utility>

namespace QuantLib {

    KirkEngine::KirkEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
                           ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
                           Real correlation)
    : SpreadBlackScholesVanillaEngine(std::move(process1), std::move(process2), correlation) {
    }

    Real KirkEngine::calculate(
        Real f1, Real f2, Real strike, Option::Type optionType,
        Real variance1, Real variance2, DiscountFactor df) const {
        
        const Real f = f1/(f2 + strike);
        const Real v 
            = std::sqrt(variance1 
                        + variance2*squared(f2/(f2+strike))
                        - 2*rho_*std::sqrt(variance1*variance2)
                            *(f2/(f2+strike)));
        
        BlackCalculator black(
             ext::make_shared<PlainVanillaPayoff>(
                 optionType,1.0),
             f, v, df);
        
        return (f2 + strike)*black.value();
    }
}

