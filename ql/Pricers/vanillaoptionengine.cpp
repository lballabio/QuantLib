

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file vanillaoptionengine.cpp
    \brief Vanilla (no dividends, no barriers) option on a single asset

    \fullpath
    ql/Instruments/%vanillaoptionengine.cpp
*/

// $Id$

#include <ql/Pricers/vanillaoptionengine.hpp>

namespace QuantLib {

    namespace Pricers {

        Arguments* VanillaOptionEngine::parameters() {
            return &parameters_;
        }

        void VanillaOptionEngine::validateParameters() const {
            QL_REQUIRE(parameters_.type != Option::Type(-1),
                       "no option type given");
            QL_REQUIRE(parameters_.underlying != Null<double>(),
                       "null underlying given");
            QL_REQUIRE(parameters_.underlying > 0.0,
                       "negative or zero underlying given");
            QL_REQUIRE(parameters_.strike != Null<double>(),
                       "null strike given");
            QL_REQUIRE(parameters_.strike > 0.0,
                       "negative or zero strike given");
            QL_REQUIRE(parameters_.dividendYield != Null<double>(),
                       "null dividend yield given");
            QL_REQUIRE(parameters_.riskFreeRate != Null<double>(),
                       "null risk free rate given");
            QL_REQUIRE(parameters_.residualTime != Null<double>(),
                       "null residual time given");
            QL_REQUIRE(parameters_.residualTime > 0.0,
                       "negative or zero residual time given");
            QL_REQUIRE(parameters_.volatility != Null<double>(),
                       "null volatility given");
            QL_REQUIRE(parameters_.volatility > 0.0,
                       "negative or zero volatility given");
        }

        const Results* VanillaOptionEngine::results() const {
            return &results_;
        }

    }

}

