
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
        const boost::shared_ptr<BlackScholesProcess>& stochProc,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : VanillaOption(stochProc, payoff, exercise, engine), 
      foreignRiskFreeTS_(foreignRiskFreeTS),
      exchRateVolTS_(exchRateVolTS), correlation_(correlation) {
        QL_REQUIRE(engine, "null engine or wrong engine type");
        registerWith(foreignRiskFreeTS_);
        registerWith(exchRateVolTS_);
        registerWith(correlation_);
    }

    Real QuantoVanillaOption::qvega() const {
        calculate();
        QL_REQUIRE(qvega_ != Null<Real>(),
                   "exchange rate vega calculation failed");
        return qvega_;
    }

    Real QuantoVanillaOption::qrho() const {
        calculate();
        QL_REQUIRE(qrho_ != Null<Real>(),
                   "foreign interest rate rho calculation failed");
        return qrho_;
    }

    Real QuantoVanillaOption::qlambda() const {
        calculate();
        QL_REQUIRE(qlambda_ != Null<Real>(),
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
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->foreignRiskFreeTS = foreignRiskFreeTS_;

        arguments->exchRateVolTS = exchRateVolTS_;

        QL_REQUIRE(correlation_, "null correlation given");
        arguments->correlation = correlation_->value();

    }

    void QuantoVanillaOption::performCalculations() const {
        Option::performCalculations();

        const VanillaOption::results* vanillaResults =
            dynamic_cast<const VanillaOption::results*>(engine_->results());
        QL_ENSURE(vanillaResults != 0,
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
                  "no quanto results returned from pricing engine");
        qrho_        = quantoResults->qrho;
        qvega_       = quantoResults->qvega;
        qlambda_     = quantoResults->qlambda;
    }

}

