/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) Theo Boafo

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

/*! \file binomialconvertible.hpp
    \brief discretized convertible
*/

#ifndef quantlib_binomial_convertible_engine_hpp
#define quantlib_binomial_convertible_engine_hpp

#include <ql/Lattices/binomialtree.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/PricingEngines/Hybrid/discretizedconvertible.hpp>
#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Instruments/convertiblebond.hpp>


namespace QuantLib {

    //! Pricing engine for convertibles using binomial trees

    /*    \test the correctness of the returned value is tested by
              checking it against analytic results.
    */
    template <class T>
    class BinomialConvertibleEngine : public ConvertibleBond::option::engine {
      public:
        BinomialConvertibleEngine(Size timeSteps)
        : timeSteps_(timeSteps) {}
        void calculate() const;
      private:
        Size timeSteps_;
    };


    template <class T>
    void BinomialConvertibleEngine<T>::calculate() const {

        boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                                          this->arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        DayCounter rfdc  = process->riskFreeRate()->dayCounter();
        DayCounter divdc = process->dividendYield()->dayCounter();
        DayCounter voldc = process->blackVolatility()->dayCounter();

        Real s0 = process->stateVariable()->value();
        Volatility v = process->blackVolatility()->blackVol(
            arguments_.exercise->lastDate(), s0);
        Date maturityDate = arguments_.exercise->lastDate();
        Rate riskFreeRate = process->riskFreeRate()->zeroRate(maturityDate,
            rfdc, Continuous, NoFrequency);
        Rate q = process->dividendYield()->zeroRate(maturityDate,
            divdc, Continuous, NoFrequency);
        Date referenceDate = process->riskFreeRate()->referenceDate();

        // binomial trees with constant coefficient
        Handle<YieldTermStructure> flatRiskFree(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(referenceDate, riskFreeRate, rfdc)));
        Handle<YieldTermStructure> flatDividends(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(referenceDate, q, divdc)));
        Handle<BlackVolTermStructure> flatVol(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(referenceDate, v, voldc)));

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Time maturity = rfdc.yearFraction(referenceDate, maturityDate);

        boost::shared_ptr<StochasticProcess1D> bs(new
            BlackScholesProcess(Handle<Quote>(process->stateVariable()),
                                flatDividends, flatRiskFree, flatVol));
        boost::shared_ptr<T> tree(new T(bs, maturity, timeSteps_,
                                        payoff->strike(),
                                        arguments_.dividends));

        Real creditSpread = arguments_.creditSpread->value();

		boost::shared_ptr<NumericalMethod> lattice(
              new TsiveriotisFernandesLattice<T>(tree,riskFreeRate,maturity,
                                                 timeSteps_,creditSpread,v,q));

        DiscretizedConvertible convertible(arguments_);

        convertible.initialize(lattice, maturity);
        convertible.rollback(0.0);
        results_.value = convertible.presentValue();
    }

}


#endif
