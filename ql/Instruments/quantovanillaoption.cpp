
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

    using PricingEngines::VanillaOptionArguments;
    using PricingEngines::VanillaOptionResults;
    using PricingEngines::QuantoOptionArguments;
    using PricingEngines::QuantoOptionResults;

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
            QL_REQUIRE(!engine.isNull(),
                "QuantoVanillaOption::QuantoVanillaOption : "
                "null engine or wrong engine type");
            registerWith(foreignRiskFreeRate_);
            registerWith(exchangeRateVolatility_);
            registerWith(correlation_);
        }

        double QuantoVanillaOption::qvega() const {
            calculate();
            QL_REQUIRE(qvega_ != Null<double>(),
               "QuantoVanillaOption::qvega() : "
               "exchange rate vega calculation failed");
            return qvega_;
        }

        double QuantoVanillaOption::qrho() const {
            calculate();
            QL_REQUIRE(qrho_ != Null<double>(),
               "QuantoVanillaOption::qrho() : "
               "foreign interest rate rho calculation failed");
            return qrho_;
        }

        double QuantoVanillaOption::qlambda() const {
            calculate();
            QL_REQUIRE(qlambda_ != Null<double>(),
               "QuantoVanillaOption::qlambda() : "
               "quanto correlation sensitivity calculation failed");
            return qlambda_;
        }

        void QuantoVanillaOption::setupEngine() const {
            VanillaOption::setupEngine();
            QuantoOptionArguments<VanillaOptionArguments>* arguments =
                dynamic_cast
                <QuantoOptionArguments<VanillaOptionArguments>*>(
                    engine_->arguments());
            QL_REQUIRE(arguments != 0,
               "QuantoVanillaOption::setupEngine() : "
               "pricing engine does not supply needed arguments");

            if (foreignRiskFreeRate_.isNull())
                arguments->foreignRiskFreeRate = 0.0;
            else
                arguments->foreignRiskFreeRate =
                foreignRiskFreeRate_->zeroYield(exerciseDate_);

            QL_REQUIRE(!exchangeRateVolatility_.isNull(),
                "QuantoVanillaOption::setupEngine() : "
                "null exchange rate volatility given");
            arguments->exchangeRateVolatility =
                exchangeRateVolatility_->value();

            QL_REQUIRE(!correlation_.isNull(),
                "QuantoVanillaOption::setupEngine() : "
                "null correlation given");
            arguments->correlation =
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

                const VanillaOptionResults* vanillaResults =
                    dynamic_cast<const VanillaOptionResults*>(
                        engine_->results());
                QL_ENSURE(vanillaResults != 0,
                    "QuantoVanillaOption::performCalculations() : "
                    "no vanilla results returned from pricing engine");
                delta_       = vanillaResults->delta;
                gamma_       = vanillaResults->gamma;
                theta_       = vanillaResults->theta;
                vega_        = vanillaResults->vega;
                rho_         = vanillaResults->rho;
                dividendRho_ = vanillaResults->dividendRho;

                const QuantoOptionResults<VanillaOptionResults>* 
                    quantoResults =
                    dynamic_cast
                    <const QuantoOptionResults<VanillaOptionResults>*>(
                        engine_->results());
                QL_ENSURE(quantoResults != 0,
                    "QuantoVanillaOption::performCalculations() : "
                    "no quanto results returned from pricing engine");
                qrho_        = quantoResults->qrho;
                qvega_       = quantoResults->qvega;
                qlambda_     = quantoResults->qlambda;

            }
            QL_ENSURE(isExpired_ || NPV_ != Null<double>(),
                "QuantoVanillaOption::performCalculations() : "
                "null value returned from option pricer");
        }

    }

}

