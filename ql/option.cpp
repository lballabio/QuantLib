
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file option.cpp
    \brief Base option class
*/

// $Id$

#include <ql/option.hpp>

namespace QuantLib {

    Option::Option(const Handle<PricingEngine>& engine,
        const std::string& isinCode, const std::string& description)
    : Instrument(isinCode, description), errorEstimate_(Null<double>()),
      engine_(engine) {}

    Option::~Option() {}

    double Option::errorEstimate() const {
        calculate();
        QL_REQUIRE(errorEstimate_ != Null<double>(),
                   "error estimate not provided");
        return errorEstimate_;
    }

    void Option::setPricingEngine(const Handle<PricingEngine>& engine) {
        QL_REQUIRE(!engine.isNull(), 
                   "Option::setPricingEngine : "
                   "null pricing engine not allowed");
        engine_ = engine;
        // this will trigger recalculation and notify observers
        update();
        setupEngine();
    }

    void Option::performCalculations() const {
        engine_->reset();
        setupEngine();
        engine_->arguments()->validate();
        engine_->calculate();
        const OptionValue* results =
            dynamic_cast<const OptionValue*>(engine_->results());
        QL_ENSURE(results != 0,
            "Option::performCalculations : "
            "no results returned from option pricer");
        NPV_ = results->value;
        errorEstimate_ = results->errorEstimate;
    }

}

