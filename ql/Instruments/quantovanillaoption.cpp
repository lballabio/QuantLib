
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file quantovanillaoption.cpp
    \brief Quanto version of a vanilla option

    \fullpath
    ql/Instruments/%quantovanillaoption.cpp
*/

// $Id$

#include <ql/Instruments/quantovanillaoption.hpp>

namespace QuantLib {

    namespace Instruments {

        QuantoVanillaOption::QuantoVanillaOption(Option::Type type,
            const RelinkableHandle<MarketElement>& underlying,
            double strike,
            const RelinkableHandle<TermStructure>& dividendYield,
            const RelinkableHandle<TermStructure>& riskFreeRate,
            const Date& exerciseDate,
            const RelinkableHandle<MarketElement>& volatility,
//            const Handle<PricingEngines::QuantoVanillaEngine>& engine,
            const Handle<PricingEngine>& engine,
            const RelinkableHandle<TermStructure>& foreignRiskFreeRate,
            const RelinkableHandle<MarketElement>& exchangeRateVolatility,
            const RelinkableHandle<MarketElement>& correlation,
            const std::string& isinCode,
            const std::string& description)
        : VanillaOption(type, underlying, strike, dividendYield, riskFreeRate,
          exerciseDate, volatility, engine, isinCode, description),
          foreignRiskFreeRate_(foreignRiskFreeRate),
          exchangeRateVolatility_(exchangeRateVolatility),
          correlation_(correlation) {
            registerWith(foreignRiskFreeRate_);
            registerWith(exchangeRateVolatility_);
            registerWith(correlation_);
        }

        double QuantoVanillaOption::qvega() const {
            calculate();
            QL_REQUIRE(qvega_ != Null<double>(),
                       "vega calculation failed");
            return qvega_;
        }

        double QuantoVanillaOption::qrho() const {
            calculate();
            QL_REQUIRE(qrho_ != Null<double>(),
                       "quanto rho calculation failed");
            return qrho_;
        }

        double QuantoVanillaOption::qlambda() const {
            calculate();
            QL_REQUIRE(qlambda_ != Null<double>(),
                       "quanto correlation sensitivity calculation failed");
            return qlambda_;
        }

        void QuantoVanillaOption::setupEngine() const {
            VanillaOption::setupEngine();
            PricingEngines::QuantoOptionParameters* parameters =
                dynamic_cast<PricingEngines::QuantoOptionParameters*>(
                    engine_->parameters());
            QL_REQUIRE(parameters != 0,
                       "pricing engine does not supply needed parameters");

            if (foreignRiskFreeRate_.isNull())
                parameters->foreignRiskFreeRate = 0.0;
            else
                parameters->foreignRiskFreeRate =
                foreignRiskFreeRate_->zeroYield(exerciseDate_);

            QL_REQUIRE(!exchangeRateVolatility_.isNull(),
                "null exchange rate volatility given");
            parameters->exchangeRateVolatility =
                exchangeRateVolatility_->value();

            QL_REQUIRE(!correlation_.isNull(),
                "null correlation given");
            parameters->correlation =
                correlation_->value();

        }

        void QuantoVanillaOption::performCalculations() const {
            if (exerciseDate_ <= riskFreeRate_->settlementDate()) {
                isExpired_ = true;
                NPV_ = delta_ = gamma_ = theta_ =
                    vega_ = rho_ = dividendRho_ =
                    qvega_ = qrho_ = qlambda_ = 0.0;
            } else {
                isExpired_ = false;
                Option::performCalculations();

                const PricingEngines::VanillaOptionResults* vanillaResults =
                    dynamic_cast<const PricingEngines::VanillaOptionResults*>(
                    engine_->results());
                QL_ENSURE(vanillaResults != 0,
                          "no vanilla results returned from pricing engine");
                delta_       = vanillaResults->delta;
                gamma_       = vanillaResults->gamma;
                theta_       = vanillaResults->theta;
                vega_        = vanillaResults->vega;
                rho_         = vanillaResults->rho;
                dividendRho_ = vanillaResults->dividendRho;

                const PricingEngines::QuantoOptionResults* quantoResults =
                    dynamic_cast<const PricingEngines::QuantoOptionResults*>(
                    engine_->results());
                QL_ENSURE(quantoResults != 0,
                          "no quanto results returned from pricing engine");
                qrho_        = quantoResults->qrho;
                qvega_       = quantoResults->qvega;
                qlambda_     = quantoResults->qlambda;

            }
            QL_ENSURE(isExpired_ || NPV_ != Null<double>(),
                      "null value returned from option pricer");
        }

    }

}

