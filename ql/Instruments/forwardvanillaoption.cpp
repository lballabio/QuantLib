
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file forwardvanillaoption.cpp
    \brief Forward version of a vanilla option

    \fullpath
    ql/Instruments/%forwardvanillaoption.cpp
*/

// $Id$

#include <ql/Instruments/forwardvanillaoption.hpp>
#include <ql/PricingEngines/forwardengines.hpp>

namespace QuantLib {

    using PricingEngines::VanillaOptionArguments;
    using PricingEngines::VanillaOptionResults;
    using PricingEngines::ForwardOptionArguments;

    namespace Instruments {

        ForwardVanillaOption::ForwardVanillaOption(Option::Type type,
            const RelinkableHandle<MarketElement>& underlying,
            const RelinkableHandle<TermStructure>& dividendYield,
            const RelinkableHandle<TermStructure>& riskFreeRate,
            const Date& exerciseDate,
            const RelinkableHandle<MarketElement>& volatility,
//            const Handle<PricingEngines::ForwardEngine>& engine,
            const Handle<PricingEngine>& engine,
            double moneyness,
            Time resetTime,
            const std::string& isinCode,
            const std::string& description)
        : VanillaOption(type, underlying, 0.0, dividendYield, riskFreeRate,
          exerciseDate, volatility, engine, isinCode, description),
          moneyness_(moneyness), resetTime_(resetTime) {}

        void ForwardVanillaOption::setupEngine() const {
            VanillaOption::setupEngine();
            ForwardOptionArguments<VanillaOptionArguments>* arguments =
                dynamic_cast
                <ForwardOptionArguments<VanillaOptionArguments> *>(
                    engine_->arguments());
            QL_REQUIRE(arguments != 0,
                       "pricing engine does not supply needed arguments");

            arguments->moneyness = moneyness_;
            arguments->resetTime = resetTime_;

        }

        void ForwardVanillaOption::performCalculations() const {
            if (exerciseDate_ <= riskFreeRate_->referenceDate()) {
                isExpired_ = true;
                NPV_ = delta_ = gamma_ =       theta_ =
                        vega_ =   rho_ = dividendRho_ = 0.0;
            } else {
                isExpired_ = false;
                Option::performCalculations();

                const VanillaOptionResults* vanillaResults =
                    dynamic_cast<const VanillaOptionResults*>(
                    engine_->results());
                QL_ENSURE(vanillaResults != 0,
                          "no vanilla results returned from pricing engine");
                delta_       = vanillaResults->delta;
                gamma_       = vanillaResults->gamma;
                theta_       = vanillaResults->theta;
                vega_        = vanillaResults->vega;
                rho_         = vanillaResults->rho;
                dividendRho_ = vanillaResults->dividendRho;

            }
            QL_ENSURE(isExpired_ || NPV_ != Null<double>(),
                      "null value returned from option pricer");
        }

    }

}

