
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
        QL_REQUIRE(!engine_.isNull(), "null pricing engine passed");
    }

    Option::~Option() {}

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

