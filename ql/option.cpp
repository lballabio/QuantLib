
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file option.cpp
    \brief Base option class

    \fullpath
    ql/%option.cpp
*/

// $Id$

#include <ql/option.hpp>

namespace QuantLib {

    Option::Option(const Handle<OptionPricingEngine>& engine,
        const std::string& isinCode, const std::string& description)
    : Instrument(isinCode, description), engine_(engine) {
        QL_REQUIRE(!engine_.isNull(), "null pricing engine");
    }

    Option::~Option() {}

    void Option::setPricingEngine(const Handle<OptionPricingEngine>& engine) {
        QL_REQUIRE(!engine.isNull(), "null pricing engine");
        engine_ = engine;
        // this will trigger recalculation and notify observers
        update();
        setupEngine();
    }

    void Option::performCalculations() const {
        setupEngine();
        engine_->validateParameters();
        engine_->calculate();
        const OptionValue* results =
            dynamic_cast<const OptionValue*>(engine_->results());
        QL_ENSURE(results != 0, "no results returned from option pricer");
        NPV_ = results->value;
    }

}

