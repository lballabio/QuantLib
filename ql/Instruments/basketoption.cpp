
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file basketoption.cpp
    \brief Basket option on a number of assets
*/

#include <ql/Instruments/basketoption.hpp>

namespace QuantLib {

    BasketOption::BasketOption(       
        const BasketType basketType,
        const std::vector<Handle<BlackScholesStochasticProcess> >& stochProcs,
        const Handle<PlainVanillaPayoff>& payoff,
        const Handle<Exercise>& exercise,        
        const Matrix& correlation,
        const Handle<PricingEngine>& engine,
        const std::string& isinCode,
        const std::string& description)
    : MultiAssetOption(stochProcs, payoff, exercise, correlation, engine, isinCode,
      description), basketType_(basketType) {}

    void BasketOption::performCalculations() const {
        // enforce in this class any check on engine/payoff
        MultiAssetOption::performCalculations();
    }

    void BasketOption::setupArguments(Arguments* args) const {
        BasketOption::arguments* arguments =
            dynamic_cast<BasketOption::arguments*>(args);
        QL_REQUIRE(arguments != 0,
                   "BasketOption::setupArguments : "
                   "wrong argument type");

        arguments->basketType = basketType_;

        MultiAssetOption::setupArguments(args);
    }

    void BasketOption::arguments::validate() const {
        #if defined(QL_PATCH_MICROSOFT)
        MultiAssetOption::arguments copy = *this;
        copy.validate();
        #else
        MultiAssetOption::arguments::validate();
        #endif

    }
}

