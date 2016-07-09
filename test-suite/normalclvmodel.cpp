/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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


#include "utilities.hpp"
#include "normalclvmodel.hpp"

#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/math/statistics/statistics.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>

#include <ql/experimental/models/normalclvmodel.hpp>
#include <ql/experimental/finitedifferences/bsmrndcalculator.hpp>
#include <ql/experimental/finitedifferences/hestonrndcalculator.hpp>

#include <boost/make_shared.hpp>
#include <boost/assign/std/vector.hpp>

#include <iostream>

using namespace QuantLib;
using namespace boost::assign;
using namespace boost::unit_test_framework;

void NormalCLVModelTest::testBSCumlativeDistributionFunction() {
    BOOST_TEST_MESSAGE("Testing Black-Scholes cumulative distribution function"
                       " with constant volatility...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(22, June, 2016);
    const Date maturity = today + Period(6, Months);

    const Real s0 = 100;
    const Real rRate = 0.1;
    const Real qRate = 0.05;
    const Volatility vol = 0.25;

    const Handle<Quote> spot(boost::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    const boost::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
        boost::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, volTS));
    const boost::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess;

    const NormalCLVModel m(
        5, bsProcess, ouProcess, std::vector<Date>());
    const BSMRNDCalculator rndCalculator(bsProcess);


    const Real tol = 1e5*QL_EPSILON;
    const Time t = dc.yearFraction(today, maturity);
    for (Real x=10; x < 400; x+=10) {
        const Real calculated = m.cdf(maturity, x);
        const Real expected = rndCalculator.cdf(std::log(x), t);

        if (std::fabs(calculated - expected) > tol) {
            BOOST_FAIL("Failed to reproduce CDF for "
                       << "\n    strike:     " << x
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << expected);
        }
    }
}

void NormalCLVModelTest::testHestonCumlativeDistributionFunction() {
    BOOST_TEST_MESSAGE("Testing Heston cumulative distribution function ...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(22, June, 2016);
    const Date maturity = today + Period(1, Years);

    const Real s0 = 100;
    const Real v0 = 0.01;
    const Real rRate = 0.1;
    const Real qRate = 0.05;
    const Real kappa = 2.0;
    const Real theta = 0.09;
    const Real sigma = 0.4;
    const Real rho = -0.75;

    const Handle<Quote> spot(boost::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));

    const boost::shared_ptr<HestonProcess> process(
        boost::make_shared<HestonProcess>(
            rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const Handle<BlackVolTermStructure> hestonVolTS(
        boost::make_shared<HestonBlackVolSurface>(
            Handle<HestonModel>(boost::make_shared<HestonModel>(process))));

    const NormalCLVModel m(
        5,
        boost::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, hestonVolTS),
        boost::shared_ptr<OrnsteinUhlenbeckProcess>(),
        std::vector<Date>());

    const HestonRNDCalculator rndCalculator(process);

    const Real tol = 1e-6;
    const Time t = dc.yearFraction(today, maturity);
    for (Real x=10; x < 400; x+=25) {
        const Real calculated = m.cdf(maturity, x);
        const Real expected = rndCalculator.cdf(std::log(x), t);

        if (std::fabs(calculated - expected) > tol) {
            BOOST_FAIL("Failed to reproduce CDF for "
                       << "\n    strike:     " << x
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << expected);
        }
    }
}


namespace {
    class SABRVolTermStructure : public BlackVolatilityTermStructure {
      public:
        SABRVolTermStructure(Real alpha, Real beta, Real gamma, Real rho,
                             Real s0, Real r,
                             const Date& referenceDate, const DayCounter dc)
      : BlackVolatilityTermStructure(
            referenceDate, NullCalendar(), Following, dc),
        alpha_(alpha), beta_(beta),
        gamma_(gamma), rho_(rho),
        s0_(s0), r_(r) { }

        Date maxDate()   const { return Date::maxDate(); }
        Rate minStrike() const { return 0.0; }
        Rate maxStrike() const { return QL_MAX_REAL; }

      protected:
        Volatility blackVolImpl(Time t, Real strike) const {
            const Real fwd = s0_*std::exp(r_*t);
            return sabrVolatility(strike, fwd, t, alpha_, beta_, gamma_, rho_);
        }
      private:
        Real alpha_, beta_, gamma_, rho_, s0_, r_;
    };
}

void NormalCLVModelTest::testIllustrative1DExample() {
    BOOST_TEST_MESSAGE("Testing illustrative 1d example of NormalCLVModel ...");

    SavedSettings backup;

    // example taken from:
    // A. Grzelak, 2015, The CLV Framework -
    // A Fresh Look at Efficient Pricing with Smile
    // http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2747541

    const DayCounter dc = Actual360();
    const Date today = Date(22, June, 2016);

    //SABR
    const Real beta = 0.5;
    const Real alpha= 0.2;
    const Real rho  = -0.9;
    const Real gamma= 0.2;

    // Ornstein-Uhlenbeck
    const Real speed = 1.3;
    const Real level = 0.1;
    const Real vol   = 0.25;
    const Real x0    = 1.0;

    const Real s0    = 1.0;
    const Real rRate = 0.03;
    const Real qRate = 0.0;

    const Handle<Quote> spot(boost::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));

    const Handle<BlackVolTermStructure> sabrVol(
        boost::make_shared<SABRVolTermStructure>(
            alpha, beta, gamma, rho, s0, rRate, today, dc));

    const boost::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
        boost::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, sabrVol));

    const boost::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess(
        boost::make_shared<OrnsteinUhlenbeckProcess>(
            speed, vol, x0, level));

    std::vector<Date> maturityDates;
    maturityDates += (today + Period(18, Days)),
        today + Period(90, Days)  , today + Period(180, Days),
        today + Period(360, Days) , today + Period(720, Days);

    const NormalCLVModel m(4, bsProcess, ouProcess, maturityDates);
    const boost::function<Real(Real, Real)> g = m.g();

    // test collocation points in x_ij
    std::vector<Date> maturities;
    maturities += maturityDates[0], maturityDates[2], maturityDates[4];

    std::vector<std::vector<Real> > x(3);
    x[0] += 1.070, 0.984, 0.903, 0.817;
    x[1] += 0.879, 0.668, 0.472, 0.261;
    x[2] += 0.528, 0.282, 0.052,-0.194;

    std::vector<std::vector<Real> > s(3);
    s[0] += 1.104, 1.035, 0.969, 0.895;
    s[1] += 1.328, 1.122, 0.911, 0.668;
    s[2] += 1.657, 1.283, 0.854, 0.339;

    std::vector<Real> c;
    c += 2.3344, 0.7420, -0.7420, -2.3344;

    const Real tol = 0.001;
    for (Size i=0; i < maturities.size(); ++i) {
        const Time t = dc.yearFraction(today, maturities[i]);

        for (Size j=0; j < x.front().size(); ++j) {
            const Real calculatedX = m.collocationPointsX(maturities[i])[j];
            const Real expectedX = x[i][j];

            if (std::fabs(calculatedX - expectedX) > tol) {
                BOOST_FAIL("Failed to reproduce collocation x points for "
                           << "\n    time:       " << maturities[i]
                           << "\n    j           " << j
                           << "\n    calculated: " << calculatedX
                           << "\n    expected:   " << expectedX);
            }

            const Real calculatedS = m.collocationPointsY(maturities[i])[j];
            const Real expectedS = s[i][j];
            if (std::fabs(calculatedS - expectedS) > tol) {
                BOOST_FAIL("Failed to reproduce collocation s points for "
                           << "\n    time:       " << maturities[i]
                           << "\n    j           " << j
                           << "\n    calculated: " << calculatedS
                           << "\n    expected:   " << expectedS);
            }

            const Real expectation
                = ouProcess->expectation(0.0, ouProcess->x0(), t);
            const Real stdDeviation
                = ouProcess->stdDeviation(0.0, ouProcess->x0(), t);

            const Real calculatedG = g(t, expectation + stdDeviation*c[j]);
            if (std::fabs(calculatedG - expectedS) > tol) {
                BOOST_FAIL("Failed to reproduce g values "
                           "at collocation points for "
                           << "\n    time:       " << maturities[i]
                           << "\n    j           " << j
                           << "\n    calculated: " << calculatedG
                           << "\n    expected:   " << expectedS);
            }
        }
    }
}


void NormalCLVModelTest::testMonteCarloBSOptionPricing() {
    BOOST_TEST_MESSAGE("Testing Monte-Carlo BS option pricing ...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(22, June, 2016);
    const Date maturity = today + Period(1, Years);
    const Time t = dc.yearFraction(today, maturity);

    const Real strike = 110;
    const boost::shared_ptr<StrikedTypePayoff> payoff =
        boost::make_shared<PlainVanillaPayoff>(Option::Call, strike);
    const boost::shared_ptr<Exercise> exercise =
        boost::make_shared<EuropeanExercise>(maturity);

    // Ornstein-Uhlenbeck
    const Real speed = 2.3;
    const Real level = 10;
    const Real sigma = 0.35;
    const Real x0    = 100.0;

    const Real s0        = x0;
    const Volatility vol = 0.25;
    const Real rRate     = 0.10;
    const Real qRate     = 0.04;

    const Handle<Quote> spot(boost::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    const Handle<BlackVolTermStructure> vTS(flatVol(today, vol, dc));

    const boost::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
        boost::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, vTS));

    const boost::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess(
        boost::make_shared<OrnsteinUhlenbeckProcess>(
            speed, sigma, x0, level));

    std::vector<Date> maturities;
    maturities += today + Period(6, Months), maturity;

    const NormalCLVModel m(7, bsProcess, ouProcess, maturities);
    const boost::function<Real(Real, Real)> g = m.g();

    const Size nSims = 32767;
    const LowDiscrepancy::rsg_type ld
        = LowDiscrepancy::make_sequence_generator(1, 23455);

    Statistics stat;
    for (Size i=0; i < nSims; ++i) {
        const Real dw = ld.nextSequence().value.front();

        const Real o_t = ouProcess->evolve(0, x0, t, dw);
        const Real s = g(t, o_t);

        stat.add((*payoff)(s));

    }
    const Real calculated = stat.mean() * rTS->discount(maturity);

    VanillaOption option(payoff, exercise);
    option.setPricingEngine(
        boost::make_shared<AnalyticEuropeanEngine>(bsProcess));
    const Real expected = option.NPV();

    const Real tol = 0.01;
    if (std::fabs(calculated - expected) > tol) {
        BOOST_FAIL("Failed to reproduce vanilla option price "
                   << "\n    time:       " << maturity
                   << "\n    strike:     " << strike
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);

    }
}

test_suite* NormalCLVModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("NormalCLVModel tests");
//    suite->add(QUANTLIB_TEST_CASE(
//        &NormalCLVModelTest::testBSCumlativeDistributionFunction));
//    suite->add(QUANTLIB_TEST_CASE(
//        &NormalCLVModelTest::testHestonCumlativeDistributionFunction));
//    suite->add(QUANTLIB_TEST_CASE(
//        &NormalCLVModelTest::testIllustrative1DExample));
    suite->add(QUANTLIB_TEST_CASE(
            NormalCLVModelTest::testMonteCarloBSOptionPricing));

    return suite;
}
