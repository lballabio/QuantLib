/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2015 Thema Consulting SA

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

#include "mclongstaffschwartzengine.hpp"
#include "utilities.hpp"
#include <ql/instruments/vanillaoption.hpp>
#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <ql/pricingengines/mclongstaffschwartzengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/mcamericanengine.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <utility>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    class AmericanMaxPathPricer : public EarlyExercisePathPricer<MultiPath>  {
      public:
        explicit AmericanMaxPathPricer(ext::shared_ptr<Payoff> payoff)
        : payoff_(std::move(payoff)) {}

        StateType state(const MultiPath& path, Size t) const override {
            Array tmp(path.assetNumber());
            for (Size i=0; i<path.assetNumber(); ++i) {
                tmp[i]=path[i][t];
            }

            return tmp;
        }

        Real operator()(const MultiPath& path, Size t) const override {
            const Array tmp = state(path, t);
            return (*payoff_)(*std::max_element(tmp.begin(), tmp.end()));
        }

        std::vector<ext::function<Real(StateType)> > basisSystem() const override {
            return LsmBasisSystem::multiPathBasisSystem(2, 2,
                                                        LsmBasisSystem::Monomial);
        }

      protected:
        const ext::shared_ptr<Payoff> payoff_;
    };

    template <class RNG>
    class MCAmericanMaxEngine
        : public MCLongstaffSchwartzEngine<VanillaOption::engine,
                                           MultiVariate,RNG>{
      public:
        MCAmericanMaxEngine(
                            const ext::shared_ptr<StochasticProcessArray>& processes,
                            Size timeSteps,
                            Size timeStepsPerYear,
                            bool brownianbridge,
                            bool antitheticVariate,
                            bool controlVariate,
                            Size requiredSamples,
                            Real requiredTolerance,
                            Size maxSamples,
                            BigNatural seed,
                            Size nCalibrationSamples = Null<Size>())
        : MCLongstaffSchwartzEngine<VanillaOption::engine,
                                    MultiVariate,RNG>(processes,
                                                      timeSteps,
                                                      timeStepsPerYear,
                                                      brownianbridge,
                                                      antitheticVariate,
                                                      controlVariate,
                                                      requiredSamples,
                                                      requiredTolerance,
                                                      maxSamples,
                                                      seed, nCalibrationSamples)
        { }

      protected:
        ext::shared_ptr<LongstaffSchwartzPathPricer<MultiPath> > lsmPathPricer() const override {
            ext::shared_ptr<StochasticProcessArray> processArray =
            ext::dynamic_pointer_cast<StochasticProcessArray>(this->process_);
            QL_REQUIRE(processArray && processArray->size() > 0,
                       "Stochastic process array required");

            ext::shared_ptr<GeneralizedBlackScholesProcess> process =
                ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                    processArray->process(0));
            QL_REQUIRE(process, "generalized Black-Scholes proces required");

            ext::shared_ptr<AmericanMaxPathPricer> earlyExercisePathPricer(
                          new AmericanMaxPathPricer(this->arguments_.payoff));

            return ext::make_shared<LongstaffSchwartzPathPricer<MultiPath> > (
                
                    this->timeGrid(),
                    earlyExercisePathPricer,
                    process->riskFreeRate().currentLink());
        }
    };

}


void MCLongstaffSchwartzEngineTest::testAmericanOption() {
    BOOST_TEST_MESSAGE("Testing Monte-Carlo pricing of American options...");

    // most of the example taken from the EquityOption.cpp
    const Option::Type type(Option::Put);
    const Real underlying = 36;
    const Spread dividendYield = 0.00;
    const Rate riskFreeRate = 0.06;
    const Volatility volatility = 0.20;

    const Date todaysDate(15, May, 1998);
    const Date settlementDate(17, May, 1998);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturity(17, May, 1999);
    const DayCounter dayCounter = Actual365Fixed();

    ext::shared_ptr<Exercise> americanExercise(
        new AmericanExercise(settlementDate, maturity));

    // bootstrap the yield/dividend/vol curves
    Handle<YieldTermStructure> flatTermStructure(
            ext::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, riskFreeRate, dayCounter)));
    Handle<YieldTermStructure> flatDividendTS(
            ext::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, dividendYield, dayCounter)));

    // expected results for exercise probability, evaluated with third-party
    // product (using Cox-Rubinstein binomial tree)
    Matrix expectedExProb(2, 3);
    expectedExProb[0][0] = 0.48013; // (price: 2.105)
    expectedExProb[0][1] = 0.51678; // (price: 3.451)
    expectedExProb[0][2] = 0.54598; // (price: 4.807)
    expectedExProb[1][0] = 0.75549; // (price: 4.505)
    expectedExProb[1][1] = 0.67569; // (price: 5.764)
    expectedExProb[1][2] = 0.65562; // (price: 7.138)

    LsmBasisSystem::PolynomialType polynomialTypes[]
        = { LsmBasisSystem::Monomial, LsmBasisSystem::Laguerre,
            LsmBasisSystem::Hermite, LsmBasisSystem::Hyperbolic,
            LsmBasisSystem::Chebyshev2nd };

    for (Integer i=0; i<2; ++i) {
        for (Integer j=0; j<3; ++j) {
            Handle<BlackVolTermStructure> flatVolTS(
                ext::shared_ptr<BlackVolTermStructure>(
                    new BlackConstantVol(settlementDate, NullCalendar(),
                                         volatility+0.1*j, dayCounter)));

            ext::shared_ptr<StrikedTypePayoff> payoff(
                new PlainVanillaPayoff(type, underlying+4*i));

            Handle<Quote> underlyingH(
                ext::shared_ptr<Quote>(new SimpleQuote(underlying)));

            ext::shared_ptr<GeneralizedBlackScholesProcess>
                stochasticProcess(new GeneralizedBlackScholesProcess(
                                      underlyingH, flatDividendTS,
                                      flatTermStructure, flatVolTS));

            VanillaOption americanOption(payoff, americanExercise);

            ext::shared_ptr<PricingEngine> mcengine =
                MakeMCAmericanEngine<PseudoRandom>(stochasticProcess)
                  .withSteps(75)
                  .withAntitheticVariate()
                  .withAbsoluteTolerance(0.02)
                  .withSeed(42)
                  .withPolynomialOrder(3)
                  .withBasisSystem(polynomialTypes[0*(i*3+j)%LENGTH(polynomialTypes)]);

            americanOption.setPricingEngine(mcengine);
            const Real calculated = americanOption.NPV();
            const Real errorEstimate = americanOption.errorEstimate();
            const Real exerciseProbability =
                americanOption.result<QuantLib::Real>("exerciseProbability");

            americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                        new FdBlackScholesVanillaEngine(stochasticProcess, 401, 200)));
            const Real expected = americanOption.NPV();

            // Check price
            if (std::fabs(calculated - expected) > 2.34*errorEstimate) {
                BOOST_ERROR("Failed to reproduce american option prices"
                            << "\n    expected: " << expected
                            << "\n    calculated:   " << calculated
                            << " +/- " << errorEstimate);
            }

            // Check exercise probability (tolerance 1.5%)
            if (std::fabs(exerciseProbability - expectedExProb[i][j]) > 0.015) {
                BOOST_ERROR("Failed to reproduce american option "
                            << "exercise probability"
                            << "\n    expected: " << expectedExProb[i][j]
                            << "\n    calculated:   " << exerciseProbability);
            }
        }
    }
}

void MCLongstaffSchwartzEngineTest::testAmericanMaxOption() {

    // reference values taken from
    // "Monte Carlo Methods in Financial Engineering",
    // by Paul Glasserman, 2004 Springer Verlag, p. 462

    BOOST_TEST_MESSAGE("Testing Monte-Carlo pricing of American max options...");

    // most of the example taken from the EquityOption.cpp
    const Option::Type type(Option::Call);
    const Real strike = 100;
    const Spread dividendYield = 0.10;
    const Rate riskFreeRate = 0.05;
    const Volatility volatility = 0.20;

    const Date todaysDate(15, May, 1998);
    const Date settlementDate(17, May, 1998);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturity(16, May, 2001);
    const DayCounter dayCounter = Actual365Fixed();

    ext::shared_ptr<Exercise> americanExercise(
        new AmericanExercise(settlementDate, maturity));

    // bootstrap the yield/dividend/vol curves
    Handle<YieldTermStructure> flatTermStructure(
        ext::shared_ptr<YieldTermStructure>(
            new FlatForward(settlementDate, riskFreeRate, dayCounter)));
    Handle<YieldTermStructure> flatDividendTS(
        ext::shared_ptr<YieldTermStructure>(
            new FlatForward(settlementDate, dividendYield, dayCounter)));

    Handle<BlackVolTermStructure> flatVolTS(
        ext::shared_ptr<BlackVolTermStructure>(new
            BlackConstantVol(settlementDate, NullCalendar(),
                             volatility, dayCounter)));

    ext::shared_ptr<StrikedTypePayoff> payoff(
        new PlainVanillaPayoff(type, strike));

    RelinkableHandle<Quote> underlyingH;

    ext::shared_ptr<GeneralizedBlackScholesProcess> stochasticProcess(new
        GeneralizedBlackScholesProcess(
            underlyingH, flatDividendTS, flatTermStructure, flatVolTS));

    const Size numberAssets = 2;
    Matrix corr(numberAssets, numberAssets, 0.0);
    std::vector<ext::shared_ptr<StochasticProcess1D> > v;

    for (Size i=0; i<numberAssets; ++i) {
        v.push_back(stochasticProcess);
        corr[i][i] = 1.0;
    }

    ext::shared_ptr<StochasticProcessArray> process(
        new StochasticProcessArray(v, corr));
    VanillaOption americanMaxOption(payoff, americanExercise);

    ext::shared_ptr<PricingEngine> mcengine(
        new MCAmericanMaxEngine<PseudoRandom>(process, 25, Null<Size>(), false,
                                              true, false, 4096,
                                              Null<Real>(), Null<Size>(),
                                              42, 1024));
    americanMaxOption.setPricingEngine(mcengine);

    const Real expected[] = {8.08, 13.90, 21.34};
    for (Size i = 0; i < 3; ++i) {

        const Real underlying = 90.0 + i*10.0;
        underlyingH.linkTo(
            ext::shared_ptr<Quote>(new SimpleQuote(underlying)));

        const Real calculated  = americanMaxOption.NPV();
        const Real errorEstimate = americanMaxOption.errorEstimate();
        if (std::fabs(calculated - expected[i]) > 2.34*errorEstimate) {
                BOOST_ERROR("Failed to reproduce american option prices"
                            << "\n    expected: " << expected[i]
                            << "\n    calculated:   " << calculated
                            << " +/- " << errorEstimate);
        }
    }
}

test_suite* MCLongstaffSchwartzEngineTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("Longstaff Schwartz MC engine tests");

    suite->add(QUANTLIB_TEST_CASE(&MCLongstaffSchwartzEngineTest::testAmericanMaxOption));

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&MCLongstaffSchwartzEngineTest::testAmericanOption));
    }

    return suite;
}

