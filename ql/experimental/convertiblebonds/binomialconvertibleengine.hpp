/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file binomialconvertibleengine.hpp
    \brief binomial engine for convertible bonds
*/

#ifndef quantlib_binomial_convertible_engine_hpp
#define quantlib_binomial_convertible_engine_hpp

#include <ql/experimental/convertiblebonds/convertiblebond.hpp>
#include <ql/experimental/convertiblebonds/discretizedconvertible.hpp>
#include <ql/experimental/convertiblebonds/tflattice.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <utility>

namespace QuantLib {

    //! Binomial Tsiveriotis-Fernandes engine for convertible bonds
    /*  \ingroup hybridengines

        \test the correctness of the returned value is tested by
              checking it against known results in a few corner cases.
    */
    template <class T>
    class BinomialConvertibleEngine : public ConvertibleBond::option::engine {
      public:
        BinomialConvertibleEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                                  Size timeSteps,
                                  const Handle<Quote>& creditSpread,
                                  DividendSchedule dividends)
        : process_(std::move(process)), timeSteps_(timeSteps),
          dividends_(std::move(dividends), creditSpread_(creditSpread))
           {
            QL_REQUIRE(timeSteps>0,
                       "timeSteps must be positive, " << timeSteps <<
                       " not allowed");

            registerWith(process_);
            registerWith(creditSpread);
        }
        void calculate() const override;
        const Handle<Quote>& creditSpread() const { return creditSpread_; }
        const DividendSchedule& dividends() const { return dividends_; }

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size timeSteps_;
        DividendSchedule dividends_;
        Handle<Quote> creditSpread_;
    };


    template <class T>
    void BinomialConvertibleEngine<T>::calculate() const {

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        Calendar volcal = process_->blackVolatility()->calendar();

        Real s0 = process_->x0();
        QL_REQUIRE(s0 > 0.0, "negative or null underlying");
        Volatility v = process_->blackVolatility()->blackVol(
            arguments_.exercise->lastDate(), s0);
        Date maturityDate = arguments_.exercise->lastDate();
        Rate riskFreeRate = process_->riskFreeRate()->zeroRate(
            maturityDate, rfdc, Continuous, NoFrequency);
        Rate q = process_->dividendYield()->zeroRate(
            maturityDate, divdc, Continuous, NoFrequency);
        Date referenceDate = process_->riskFreeRate()->referenceDate();

        // subtract dividends
        Size i;
        for (i=0; i<dividends_.size(); i++) {
            if (dividends_[i]->date() >= referenceDate)
                s0 -= dividends_[i]->amount() *
                      process_->riskFreeRate()->discount(dividends_[i]->date());
        }
        QL_REQUIRE(s0 > 0.0,
            "negative value after subtracting dividends");

        // binomial trees with constant coefficient
        Handle<Quote> underlying(ext::shared_ptr<Quote>(new SimpleQuote(s0)));
        Handle<YieldTermStructure> flatRiskFree(ext::shared_ptr<YieldTermStructure>(
            new FlatForward(referenceDate, riskFreeRate, rfdc)));
        Handle<YieldTermStructure> flatDividends(
            ext::shared_ptr<YieldTermStructure>(new FlatForward(referenceDate, q, divdc)));
        Handle<BlackVolTermStructure> flatVol(ext::shared_ptr<BlackVolTermStructure>(
            new BlackConstantVol(referenceDate, volcal, v, voldc)));

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Time maturity = rfdc.yearFraction(arguments_.settlementDate, maturityDate);

        ext::shared_ptr<GeneralizedBlackScholesProcess> bs(
            new GeneralizedBlackScholesProcess(underlying, flatDividends, flatRiskFree, flatVol));
        ext::shared_ptr<T> tree(new T(bs, maturity, timeSteps_, payoff->strike()));

        Real creditSpread = creditSpread_->value();

        ext::shared_ptr<Lattice> lattice(new TsiveriotisFernandesLattice<T>(
            tree, riskFreeRate, maturity, timeSteps_, creditSpread, v, q));

        // Setup arguments for convertible
        arguments_.dividends.clear();
        arguments_.dividendDates.clear();
        for (const auto& dividend : dividends_) {
            if (!dividend->hasOccurred(arguments_.settlementDate, false)) {
                arguments_.dividends.push_back(dividend);
                arguments_.dividendDates.push_back(dividend->date());
            }
        }
        arguments_.creditSpread = creditSpread_;

        DiscretizedConvertible convertible(arguments_, bs, TimeGrid(maturity, timeSteps_));

        convertible.initialize(lattice, maturity);
        convertible.rollback(0.0);
        results_.value = convertible.presentValue();
        QL_ENSURE(results_.value < std::numeric_limits<Real>::max(),
                  "floating-point overflow on tree grid");
    }

}


#endif