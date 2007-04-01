/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/varianceswap.hpp>

namespace QuantLib {

    VarianceSwap::VarianceSwap(
                          Position::Type position,
                          Real strike,
                          Real notional,
                          const boost::shared_ptr<StochasticProcess>& process,
                          const Date& maturityDate,
                          const boost::shared_ptr<PricingEngine>& engine)
    : position_(position), strike_(strike), notional_(notional),
      maturityDate_(maturityDate) {
        process_ =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                                     process);
        QL_REQUIRE(process_, "Black-Scholes process required");

        setPricingEngine(engine);
    }

    void VarianceSwap::performCalculations() const {
        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(maturityDate_);
        Instrument::performCalculations();
        errorEstimate_ = Null<Real>();
        Real multiplier;
        switch (position_) {
          case Position::Long:
            multiplier = 1.0;
            break;
          case Position::Short:
            multiplier = -1.0;
            break;
          default:
            QL_FAIL("Unknown position");
        }
        NPV_ = multiplier * riskFreeDiscount * notional_ *
            (fairVariance_ - strike_);
    }

    Real VarianceSwap::fairVariance() const {
        calculate();
        QL_REQUIRE(fairVariance_ != Null<Real>(), "result not available");
        return fairVariance_;
    }

    std::vector<std::pair<Real, Real> >
    VarianceSwap::optionWeights(Option::Type type) const {
        calculate();
        QL_REQUIRE(optionWeights_ != WeightsType(), "result not available");
        std::vector<std::pair<Real, Real> > w;
        for (WeightsType::const_iterator i=optionWeights_.begin();
             i<optionWeights_.end();
             i++) {
            boost::shared_ptr<StrikedTypePayoff> payoff = i->first;
            if (payoff->optionType() == type)
                w.push_back(std::make_pair(payoff->strike(), i->second));
        }
        return w;
    }

    void VarianceSwap::setupExpired() const {
        Instrument::setupExpired();
        fairVariance_ = Null<Real>();
        optionWeights_.clear();
    }

    void VarianceSwap::setupArguments(PricingEngine::arguments* args) const {
        VarianceSwap::arguments* arguments =
            dynamic_cast<VarianceSwap::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->stochasticProcess = process_;
        arguments->position = position_;
        arguments->strike = strike_;
        arguments->notional = notional_;
        arguments->maturityDate = maturityDate_;
    }

    void VarianceSwap::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);
        const VarianceSwap::results* results =
            dynamic_cast<const VarianceSwap::results*>(r);
        fairVariance_ = results->fairVariance;
        optionWeights_ = results->optionWeights;
    }

    void VarianceSwap::arguments::validate() const {
        QL_REQUIRE(!stochasticProcess->stateVariable().empty(),
                   "no underlying given");
        QL_REQUIRE(stochasticProcess->stateVariable()->value() > 0.0,
                   "negative or zero underlying given");
        QL_REQUIRE(strike != Null<Real>(), "no strike given");
        QL_REQUIRE(strike > 0.0, "negative or null strike given");
        QL_REQUIRE(notional != Null<Real>(), "no notional given");
        QL_REQUIRE(notional > 0.0, "negative or null notional given");
    }

    bool VarianceSwap::isExpired() const {
        return maturityDate_ < Settings::instance().evaluationDate();
    }

}


