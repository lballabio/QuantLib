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

#include <ql/experimental/convertiblebonds/discretizedconvertible.hpp>
#include <ql/experimental/convertiblebonds/convertiblebond.hpp>
#include <ql/experimental/convertiblebonds/tflattice.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! Binomial Tsiveriotis-Fernandes engine for convertible bonds
    /*  \ingroup hybridengines

        \test the correctness of the returned value is tested by
              checking it against known results in a few corner cases.
    */
    template <class T>
    class BinomialConvertibleEngine : public ConvertibleBond::option::engine {
      public:
        BinomialConvertibleEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps)
        : process_(process), timeSteps_(timeSteps) {
            QL_REQUIRE(timeSteps>0,
                       "timeSteps must be positive, " << timeSteps <<
                       " not allowed");
            registerWith(process_);
        }
        void calculate() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size timeSteps_;
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
        for (i=0; i<arguments_.dividends.size(); i++) {
            if (arguments_.dividends[i]->date() >= referenceDate)
                s0 -= arguments_.dividends[i]->amount() *
                      process_->riskFreeRate()->discount(
                                             arguments_.dividends[i]->date());
        }
        QL_REQUIRE(s0 > 0.0,
                   "negative value after subtracting dividends");

        // binomial trees with constant coefficient
        Handle<Quote> underlying(boost::shared_ptr<Quote>(new SimpleQuote(s0)));
        Handle<YieldTermStructure> flatRiskFree(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(referenceDate, riskFreeRate, rfdc)));
        Handle<YieldTermStructure> flatDividends(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(referenceDate, q, divdc)));
        Handle<BlackVolTermStructure> flatVol(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(referenceDate, volcal, v, voldc)));

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Time maturity = rfdc.yearFraction(arguments_.settlementDate,
                                          maturityDate);

        boost::shared_ptr<GeneralizedBlackScholesProcess> bs(
                 new GeneralizedBlackScholesProcess(underlying, flatDividends,
                                                    flatRiskFree, flatVol));
        boost::shared_ptr<T> tree(new T(bs, maturity, timeSteps_,
                                        payoff->strike()));

        Real creditSpread = arguments_.creditSpread->value();

        boost::shared_ptr<Lattice> lattice(
              new TsiveriotisFernandesLattice<T>(tree,riskFreeRate,maturity,
                                                 timeSteps_,creditSpread,v,q));

        DiscretizedConvertible convertible(arguments_, bs,
                                           TimeGrid(maturity, timeSteps_));

        convertible.initialize(lattice, maturity);
        convertible.rollback(0.0);
        results_.value = convertible.presentValue();
        QL_ENSURE(results_.value < std::numeric_limits<Real>::max(),
                  "floating-point overflow on tree grid");
    }

}


#endif
