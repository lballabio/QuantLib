
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

/*! \file vanillaoption.cpp
    \brief Vanilla option on a single asset
*/

#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Instruments/vanillaoption.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    VanillaOption::VanillaOption(
                         Option::Type type,
                         const RelinkableHandle<Quote>& underlying,
                         double strike,
                         const RelinkableHandle<TermStructure>& dividendTS,
                         const RelinkableHandle<TermStructure>& riskFreeTS,
                         const Exercise& exercise,
                         const RelinkableHandle<BlackVolTermStructure>& volTS,
                         const Handle<PricingEngine>& engine,
                         const std::string& isinCode, 
                         const std::string& description)
    : Option(engine, isinCode, description),
      type_(type), underlying_(underlying),
      strike_(strike), exercise_(exercise), 
      riskFreeTS_(riskFreeTS), dividendTS_(dividendTS),
      volTS_(volTS) {
        registerWith(underlying_);
        registerWith(dividendTS_);
        registerWith(riskFreeTS_);
        registerWith(volTS_);
    }

    bool VanillaOption::isExpired() const {
        return exercise_.lastDate() < riskFreeTS_->referenceDate();
    }

    double VanillaOption::delta() const {
        calculate();
        QL_REQUIRE(delta_ != Null<double>(),
                   "VanillaOption: delta not provided");
        return delta_;
    }

    double VanillaOption::gamma() const {
        calculate();
        QL_REQUIRE(gamma_ != Null<double>(),
                   "VanillaOption: gamma not provided");
        return gamma_;
    }

    double VanillaOption::theta() const {
        calculate();
        QL_REQUIRE(theta_ != Null<double>(),
                   "VanillaOption: theta not provided");
        return theta_;
    }

    double VanillaOption::vega() const {
        calculate();
        QL_REQUIRE(vega_ != Null<double>(),
                   "VanillaOption: vega not provided");
        return vega_;
    }

    double VanillaOption::rho() const {
        calculate();
        QL_REQUIRE(rho_ != Null<double>(),
                   "VanillaOption: rho not provided");
        return rho_;
    }

    double VanillaOption::dividendRho() const {
        calculate();
        QL_REQUIRE(dividendRho_ != Null<double>(),
                   "VanillaOption: dividend rho not provided");
        return dividendRho_;
    }

    double VanillaOption::strikeSensitivity() const {
        calculate();
        QL_REQUIRE(strikeSensitivity_ != Null<double>(),
                   "VanillaOption: strike sensitivity not provided");
        return strikeSensitivity_;
    }

    double VanillaOption::impliedVolatility(double targetValue,
                                            double accuracy, 
                                            Size maxEvaluations,
                                            double minVol, 
                                            double maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(),
                   "VanillaOption::impliedVolatility : "
                   "option expired");

        double guess = volTS_->blackVol(exercise_.lastDate(),
                                        underlying_->value());

        ImpliedVolHelper f(engine_,targetValue);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }

    void VanillaOption::setupExpired() const {
        NPV_ = delta_ = gamma_ = theta_ =
            vega_ = rho_ = dividendRho_ = strikeSensitivity_ = 0.0;
    }

    void VanillaOption::setupArguments(Arguments* args) const {
        VanillaOption::arguments* arguments =
            dynamic_cast<VanillaOption::arguments*>(args);
        QL_REQUIRE(arguments != 0,
                   "VanillaOption::setupArguments : "
                   "wrong argument type");

        arguments->payoff = Handle<Payoff>(
                                       new PlainVanillaPayoff(type_,strike_));

        QL_REQUIRE(!IsNull(underlying_),
                   "VanillaOption::setupArguments : "
                   "null underlying price given");
        arguments->underlying = underlying_->value();

        // should I require !IsNull(TS) ???
        arguments->dividendTS = dividendTS_;
        arguments->riskFreeTS = riskFreeTS_;

        arguments->maturity = riskFreeTS_->dayCounter().yearFraction(
                          riskFreeTS_->referenceDate(), exercise_.lastDate());
        arguments->exerciseType = exercise_.type();
        arguments->stoppingTimes = 
            std::vector<Time>(exercise_.dates().size());
        for (Size i=0; i<exercise_.dates().size(); i++) {
            arguments->stoppingTimes[i] = 
                riskFreeTS_->dayCounter().yearFraction(
                             riskFreeTS_->referenceDate(), exercise_.date(i));
        }

        arguments->volTS = volTS_;
    }

    void VanillaOption::performCalculations() const {
        Option::performCalculations();
        const Greeks* results =
            dynamic_cast<const Greeks*>(engine_->results());
        QL_ENSURE(results != 0,
                  "VanillaOption::performCalculations : "
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
        strikeSensitivity_ = results->strikeSensitivity;

        QL_ENSURE(NPV_ != Null<double>(),
                  "VanillaOption::performCalculations : "
                  "null value returned from option pricer");
    }


    void VanillaOption::arguments::validate() const {
        QL_REQUIRE(!IsNull(payoff),
                   "VanillaOption::arguments::validate() : "
                   "null payoff given");
        QL_REQUIRE(underlying != Null<double>(),
                   "VanillaOption::arguments::validate() : "
                   "no underlying given");
        QL_REQUIRE(underlying > 0.0,
                   "VanillaOption::arguments::validate() : "
                   "negative or zero underlying given");
        QL_REQUIRE(!IsNull(dividendTS),
                   "VanillaOption::arguments::validate() : "
                   "no dividend term structure given");
        QL_REQUIRE(!IsNull(riskFreeTS),
                   "VanillaOption::arguments::validate() : "
                   "no risk free term structure given");
        QL_REQUIRE(maturity != Null<double>(),
                   "VanillaOption::arguments::validate() : "
                   "no maturity given");
        QL_REQUIRE(maturity>=0.0,
                   "VanillaOption::arguments::validate() : "
                   "negative maturity");
        QL_REQUIRE(!IsNull(volTS),
                   "VanillaOption::arguments::validate() : "
                   "no vol term structure given");
    }


    VanillaOption::ImpliedVolHelper::ImpliedVolHelper(
                                         const Handle<PricingEngine>& engine, 
                                         double targetValue)
    : engine_(engine), targetValue_(targetValue) {
        VanillaOption::arguments* arguments_ = 
            dynamic_cast<VanillaOption::arguments*>(engine_->arguments());
        QL_REQUIRE(arguments_ != 0,
                   "VanillaOption::ImpliedVolHelper::ImpliedVolHelper : "
                   "pricing engine does not supply needed arguments");
        vol_ = Handle<SimpleQuote>(new SimpleQuote(0.0));
        arguments_->volTS = RelinkableHandle<BlackVolTermStructure>(
            Handle<BlackVolTermStructure>(
                new BlackConstantVol(arguments_->volTS->referenceDate(), 
                                     RelinkableHandle<Quote>(vol_))));
        results_ = dynamic_cast<const Value*>(engine_->results());
        QL_REQUIRE(results_ != 0,
                   "VanillaOption::ImpliedVolHelper::ImpliedVolHelper : "
                   "pricing engine does not supply needed results");
    }

    double VanillaOption::ImpliedVolHelper::operator()(double x) const {
        vol_->setValue(x);
        engine_->calculate();
        return results_->value-targetValue_;
    }

}

