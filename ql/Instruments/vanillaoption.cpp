
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

/*! \file vanillaoption.cpp
    \brief Vanilla option on a single asset
*/

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    VanillaOption::VanillaOption(
                         Handle<StrikedTypePayoff> payoff,
                         const RelinkableHandle<Quote>& underlying,
                         const RelinkableHandle<TermStructure>& dividendTS,
                         const RelinkableHandle<TermStructure>& riskFreeTS,
                         const Exercise& exercise,
                         const RelinkableHandle<BlackVolTermStructure>& volTS,
                         const Handle<PricingEngine>& engine,
                         const std::string& isinCode,
                         const std::string& description)
    : OneAssetStrikedOption(payoff, underlying, dividendTS, riskFreeTS,
      exercise, volTS, engine, isinCode, description) {}

    void VanillaOption::performCalculations() const {
        // enforce in this class any check on engine/payoff
        OneAssetStrikedOption::performCalculations();
    }

}

