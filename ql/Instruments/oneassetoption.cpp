
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

/*! \file oneassetoption.cpp
    \brief Option on a single asset
*/

#include <ql/Instruments/oneassetoption.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    OneAssetOption::OneAssetOption(
                         const RelinkableHandle<Quote>& underlying,
                         const RelinkableHandle<TermStructure>& dividendTS,
                         const RelinkableHandle<TermStructure>& riskFreeTS,
                         const Handle<Exercise>& exercise,
                         const RelinkableHandle<BlackVolTermStructure>& volTS,
                         const Handle<PricingEngine>& engine,
                         const std::string& isinCode,
                         const std::string& description)
    : Option(engine, isinCode, description), underlying_(underlying),
      exercise_(exercise), riskFreeTS_(riskFreeTS), dividendTS_(dividendTS),
      volTS_(volTS) {
        registerWith(underlying_);
        registerWith(dividendTS_);
        registerWith(riskFreeTS_);
        registerWith(volTS_);
    }

    bool OneAssetOption::isExpired() const {
        return exercise_->lastDate() < riskFreeTS_->referenceDate();
    }

    double OneAssetOption::delta() const {
        calculate();
        QL_REQUIRE(delta_ != Null<double>(),
                   "OneAssetOption: delta not provided");
        return delta_;
    }

    double OneAssetOption::gamma() const {
        calculate();
        QL_REQUIRE(gamma_ != Null<double>(),
                   "OneAssetOption: gamma not provided");
        return gamma_;
    }

    double OneAssetOption::theta() const {
        calculate();
        QL_REQUIRE(theta_ != Null<double>(),
                   "OneAssetOption: theta not provided");
        return theta_;
    }

    double OneAssetOption::vega() const {
        calculate();
        QL_REQUIRE(vega_ != Null<double>(),
                   "OneAssetOption: vega not provided");
        return vega_;
    }

    double OneAssetOption::rho() const {
        calculate();
        QL_REQUIRE(rho_ != Null<double>(),
                   "OneAssetOption: rho not provided");
        return rho_;
    }

    double OneAssetOption::dividendRho() const {
        calculate();
        QL_REQUIRE(dividendRho_ != Null<double>(),
                   "OneAssetOption: dividend rho not provided");
        return dividendRho_;
    }

    double OneAssetOption::impliedVolatility(double targetValue,
                                            double accuracy,
                                            Size maxEvaluations,
                                            double minVol,
                                            double maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(),
                   "OneAssetOption::impliedVolatility : "
                   "option expired");

        double guess = volTS_->blackVol(exercise_->lastDate(),
                                        underlying_->value());

        ImpliedVolHelper f(engine_,targetValue);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }

    void OneAssetOption::setupExpired() const {
        NPV_ = delta_ = gamma_ = theta_ =
            vega_ = rho_ = dividendRho_ = 0.0;
    }

    void OneAssetOption::setupArguments(Arguments* args) const {
        OneAssetOption::arguments* arguments =
            dynamic_cast<OneAssetOption::arguments*>(args);
        QL_REQUIRE(arguments != 0,
                   "OneAssetOption::setupArguments : "
                   "wrong argument type");

        QL_REQUIRE(!IsNull(underlying_),
                   "OneAssetOption::setupArguments : "
                   "null underlying price given");
        arguments->underlying = underlying_->value();

        // should I require !IsNull(TS) ???
        arguments->dividendTS = dividendTS_;
        arguments->riskFreeTS = riskFreeTS_;

        arguments->exercise = exercise_;


        // shouldn't be here
        // it should be moved elsewhere
        arguments->stoppingTimes =
            std::vector<Time>(exercise_->dates().size());
        for (Size i=0; i<exercise_->dates().size(); i++) {
            arguments->stoppingTimes[i] =
                riskFreeTS_->dayCounter().yearFraction(
                             riskFreeTS_->referenceDate(), exercise_->date(i));
        }


        arguments->volTS = volTS_;
    }

    void OneAssetOption::performCalculations() const {
        Option::performCalculations();
        const Greeks* results =
            dynamic_cast<const Greeks*>(engine_->results());
        QL_ENSURE(results != 0,
                  "OneAssetOption::performCalculations : "
                  "no greeks returned from pricing engine");
        /* no check on null values - just copy.
           this allows:
           a) to decide in derived options what to do when null
           results are returned (throw? numerical calculation?)
           b) to implement slim engines which only calculate the
           value---of course care must be taken not to call
           the greeks methods when using these.
        */
        delta_       = results->delta;
        gamma_       = results->gamma;
        theta_       = results->theta;
        vega_        = results->vega;
        rho_         = results->rho;
        dividendRho_ = results->dividendRho;

        QL_ENSURE(NPV_ != Null<double>(),
                  "OneAssetOption::performCalculations : "
                  "null value returned from option pricer");
    }


    void OneAssetOption::arguments::validate() const {
        #if defined(QL_PATCH_MICROSOFT)
        Option::arguments copy = *this;
        copy.validate();
        #else
        Option::arguments::validate();
        #endif
        QL_REQUIRE(underlying != Null<double>(),
                   "OneAssetOption::arguments::validate() : "
                   "no underlying given");
        QL_REQUIRE(underlying > 0.0,
                   "OneAssetOption::arguments::validate() : "
                   "negative or zero underlying given");
        QL_REQUIRE(!IsNull(dividendTS),
                   "OneAssetOption::arguments::validate() : "
                   "no dividend term structure given");
        QL_REQUIRE(!IsNull(riskFreeTS),
                   "OneAssetOption::arguments::validate() : "
                   "no risk free term structure given");
        QL_REQUIRE(!IsNull(volTS),
                   "OneAssetOption::arguments::validate() : "
                   "no vol term structure given");
    }


    OneAssetOption::ImpliedVolHelper::ImpliedVolHelper(
                                         const Handle<PricingEngine>& engine,
                                         double targetValue)
    : engine_(engine), targetValue_(targetValue) {
        OneAssetOption::arguments* arguments_ =
            dynamic_cast<OneAssetOption::arguments*>(engine_->arguments());
        QL_REQUIRE(arguments_ != 0,
                   "OneAssetOption::ImpliedVolHelper::ImpliedVolHelper : "
                   "pricing engine does not supply needed arguments");
        vol_ = Handle<SimpleQuote>(new SimpleQuote(0.0));
        arguments_->volTS = RelinkableHandle<BlackVolTermStructure>(
            Handle<BlackVolTermStructure>(
                new BlackConstantVol(arguments_->volTS->referenceDate(),
                                     RelinkableHandle<Quote>(vol_))));
        results_ = dynamic_cast<const Value*>(engine_->results());
        QL_REQUIRE(results_ != 0,
                   "OneAssetOption::ImpliedVolHelper::ImpliedVolHelper : "
                   "pricing engine does not supply needed results");
    }

    double OneAssetOption::ImpliedVolHelper::operator()(double x) const {
        vol_->setValue(x);
        engine_->calculate();
        return results_->value-targetValue_;
    }

}

