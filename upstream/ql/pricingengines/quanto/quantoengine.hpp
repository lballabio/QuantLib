/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file quantoengine.hpp
    \brief Quanto option engine
*/

#ifndef quantlib_quanto_engine_hpp
#define quantlib_quanto_engine_hpp

#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/quantovanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yield/quantotermstructure.hpp>
#include <utility>

namespace QuantLib {

    //! Quanto engine
    /*! \warning for the time being, this engine will only work with
                 simple Black-Scholes processes (i.e., no Merton.)

        \ingroup quantoengines

        \test
        - the correctness of the returned value is tested by
          reproducing results available in literature.
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
    */
    template <class Instr, class Engine>
    class QuantoEngine :
        public GenericEngine<typename Instr::arguments,
                             QuantoOptionResults<typename Instr::results> > {
      public:
        QuantoEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>,
                     Handle<YieldTermStructure> foreignRiskFreeRate,
                     Handle<BlackVolTermStructure> exchangeRateVolatility,
                     Handle<Quote> correlation);
        void calculate() const override;

      protected:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Handle<YieldTermStructure> foreignRiskFreeRate_;
        Handle<BlackVolTermStructure> exchangeRateVolatility_;
        Handle<Quote> correlation_;
    };


    // template definitions

    template <class Instr, class Engine>
    QuantoEngine<Instr, Engine>::QuantoEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Handle<YieldTermStructure> foreignRiskFreeRate,
        Handle<BlackVolTermStructure> exchangeRateVolatility,
        Handle<Quote> correlation)
    : process_(std::move(process)), foreignRiskFreeRate_(std::move(foreignRiskFreeRate)),
      exchangeRateVolatility_(std::move(exchangeRateVolatility)),
      correlation_(std::move(correlation)) {
        this->registerWith(process_);
        this->registerWith(foreignRiskFreeRate_);
        this->registerWith(exchangeRateVolatility_);
        this->registerWith(correlation_);
    }

    template <class Instr, class Engine>
    void QuantoEngine<Instr,Engine>::calculate() const {

        // ATM exchangeRate level needed here
        Real exchangeRateATMlevel = 1.0;

        // determine strike from payoff
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");
        Real strike = payoff->strike();

        Handle<Quote> spot = process_->stateVariable();
        QL_REQUIRE(spot->value() > 0.0, "negative or null underlying");
        Handle<YieldTermStructure> riskFreeRate = process_->riskFreeRate();
        // dividendTS needs modification
        Handle<YieldTermStructure> dividendYield(
            ext::shared_ptr<YieldTermStructure>(
                new QuantoTermStructure(process_->dividendYield(),
                                        process_->riskFreeRate(),
                                        foreignRiskFreeRate_,
                                        process_->blackVolatility(),
                                        strike,
                                        exchangeRateVolatility_,
                                        exchangeRateATMlevel,
                                        correlation_->value())));
        Handle<BlackVolTermStructure> blackVol = process_->blackVolatility();

        ext::shared_ptr<GeneralizedBlackScholesProcess> quantoProcess(
                  new GeneralizedBlackScholesProcess(spot, dividendYield,
                                                     riskFreeRate, blackVol));

        ext::shared_ptr<Engine> originalEngine(new Engine(quantoProcess));
        originalEngine->reset();
        auto* originalArguments =
            dynamic_cast<typename Instr::arguments*>(originalEngine->getArguments());
        QL_REQUIRE(originalArguments, "wrong engine type");

        *originalArguments = this->arguments_;

        originalArguments->validate();
        originalEngine->calculate();

        const auto* originalResults =
            dynamic_cast<const typename Instr::results*>(originalEngine->getResults());
        QL_REQUIRE(originalResults, "wrong engine type");

        this->results_.value = originalResults->value;
        this->results_.delta = originalResults->delta;
        this->results_.gamma = originalResults->gamma;
        this->results_.theta = originalResults->theta;
        if (originalResults->rho != Null<Real>() &&
            originalResults->dividendRho != Null<Real>()) {
            this->results_.rho = originalResults->rho +
                originalResults->dividendRho;
            this->results_.dividendRho = originalResults->dividendRho;
        } else {
            this->results_.rho = this->results_.dividendRho = Null<Real>();
        }
        Volatility exchangeRateFlatVol =
            exchangeRateVolatility_->blackVol(
                                        this->arguments_.exercise->lastDate(),
                                        exchangeRateATMlevel);
        if (originalResults->vega != Null<Real>()
            && originalResults->dividendRho != Null<Real>()) {
            this->results_.vega = originalResults->vega +
                correlation_->value() * exchangeRateFlatVol *
                originalResults->dividendRho;
        } else {
            this->results_.vega = Null<Real>();
        }

        if (originalResults->dividendRho != Null<Real>()) {
            Volatility volatility = process_->blackVolatility()->blackVol(
                                        this->arguments_.exercise->lastDate(),
                                        process_->stateVariable()->value());
            this->results_.qvega = correlation_->value() *
                process_->blackVolatility()->blackVol(
                                        this->arguments_.exercise->lastDate(),
                                        process_->stateVariable()->value()) *
                originalResults->dividendRho;
            this->results_.qrho = - originalResults->dividendRho;
            this->results_.qlambda = exchangeRateFlatVol *
                volatility * originalResults->dividendRho;
        } else {
            this->results_.qvega = this->results_.qrho =
                this->results_.qlambda = Null<Real>();
        }
    }

}


#endif
