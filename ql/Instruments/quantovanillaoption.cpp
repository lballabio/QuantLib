
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

#include <ql/Instruments/quantovanillaoption.hpp>

namespace QuantLib {

    QuantoVanillaOption::QuantoVanillaOption(
        const RelinkableHandle<TermStructure>& foreignRiskFreeTS,
        const RelinkableHandle<BlackVolTermStructure>& exchRateVolTS,
        const RelinkableHandle<Quote>& correlation,
        const Handle<BlackScholesStochasticProcess>& stochProc,
        const Handle<StrikedTypePayoff>& payoff,
        const Handle<Exercise>& exercise,
        const Handle<PricingEngine>& engine)
    : VanillaOption(stochProc, payoff, exercise, engine), 
      foreignRiskFreeTS_(foreignRiskFreeTS),
      exchRateVolTS_(exchRateVolTS), correlation_(correlation) {
        QL_REQUIRE(!IsNull(engine),
                   "QuantoVanillaOption::QuantoVanillaOption : "
                   "null engine or wrong engine type");
        registerWith(foreignRiskFreeTS_);
        registerWith(exchRateVolTS_);
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

    void QuantoVanillaOption::setupExpired() const {
        VanillaOption::setupExpired();
        qvega_ = qrho_ = qlambda_ = 0.0;
    }

    void QuantoVanillaOption::setupArguments(Arguments* args) const {
        VanillaOption::setupArguments(args);
        QuantoVanillaOption::arguments* arguments =
            dynamic_cast<QuantoVanillaOption::arguments*>(args);
        QL_REQUIRE(arguments != 0,
                   "QuantoVanillaOption::setupArguments() : "
                   "wrong argument type");

        arguments->foreignRiskFreeTS = foreignRiskFreeTS_;

        arguments->exchRateVolTS = exchRateVolTS_;

        QL_REQUIRE(!IsNull(correlation_),
                   "QuantoVanillaOption::setupArguments() : "
                   "null correlation given");
        arguments->correlation =
            correlation_->value();

    }

    void QuantoVanillaOption::performCalculations() const {
        Option::performCalculations();

        const VanillaOption::results* vanillaResults =
            dynamic_cast<const VanillaOption::results*>(engine_->results());
        QL_ENSURE(vanillaResults != 0,
                  "QuantoVanillaOption::performCalculations() : "
                  "no vanilla results returned from pricing engine");
        delta_       = vanillaResults->delta;
        gamma_       = vanillaResults->gamma;
        theta_       = vanillaResults->theta;
        vega_        = vanillaResults->vega;
        rho_         = vanillaResults->rho;
        dividendRho_ = vanillaResults->dividendRho;

        const QuantoVanillaOption::results* quantoResults =
            dynamic_cast<const QuantoVanillaOption::results*>(
                                                          engine_->results());
        QL_ENSURE(quantoResults != 0,
                  "QuantoVanillaOption::performCalculations() : "
                  "no quanto results returned from pricing engine");
        qrho_        = quantoResults->qrho;
        qvega_       = quantoResults->qvega;
        qlambda_     = quantoResults->qlambda;

        QL_ENSURE(NPV_ != Null<double>(),
                  "QuantoVanillaOption::performCalculations() : "
                  "null value returned from option pricer");
    }

}

