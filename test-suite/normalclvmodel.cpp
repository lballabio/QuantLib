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
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/math/statistics/statistics.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/randomnumbers/sobolbrownianbridgersg.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>

#include <ql/experimental/models/normalclvmodel.hpp>
#include <ql/experimental/finitedifferences/bsmrndcalculator.hpp>
#include <ql/experimental/finitedifferences/hestonrndcalculator.hpp>
#include <ql/experimental/finitedifferences/fdornsteinuhlenbeckvanillaengine.hpp>
#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/experimental/barrieroption/analyticdoublebarrierbinaryengine.hpp>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/std/vector.hpp>

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
        bsProcess, ouProcess, std::vector<Date>(), 5);
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
        boost::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, hestonVolTS),
        boost::shared_ptr<OrnsteinUhlenbeckProcess>(),
        std::vector<Date>(), 5);

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

    const NormalCLVModel m(bsProcess, ouProcess, maturityDates, 4);
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

namespace {
    class NormalCLVModelPayoff : public PlainVanillaPayoff {
      public:
        NormalCLVModelPayoff(Option::Type type, Real strike,
                             const boost::function<Real(Real)> g)
        : PlainVanillaPayoff(type, strike),
          g_(g) { }

        Real operator()(Real x) const {
            return PlainVanillaPayoff::operator()(g_(x));
        }

      private:
        const boost::function<Real(Real)> g_;
    };
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
    const Real level = 100;
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

    const NormalCLVModel m(bsProcess, ouProcess, maturities, 8);
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

    Real calculated = stat.mean() * rTS->discount(maturity);

    VanillaOption option(payoff, exercise);
    option.setPricingEngine(
        boost::make_shared<AnalyticEuropeanEngine>(bsProcess));
    const Real expected = option.NPV();

    const Real tol = 0.01;
    if (std::fabs(calculated - expected) > tol) {
        BOOST_FAIL("Failed to reproduce Monte-Carlo vanilla option price "
                   << "\n    time:       " << maturity
                   << "\n    strike:     " << strike
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }

    VanillaOption fdmOption(
         boost::make_shared<NormalCLVModelPayoff>(
             payoff->optionType(), payoff->strike(), boost::bind(g, t, _1)),
         exercise);

    fdmOption.setPricingEngine(
        boost::make_shared<FdOrnsteinUhlenbeckVanillaEngine>(
            ouProcess, rTS.currentLink(), 50, 800));

    calculated = fdmOption.NPV();
    if (std::fabs(calculated - expected) > tol) {
        BOOST_FAIL("Failed to reproduce FDM vanilla option price "
                   << "\n    time:       " << maturity
                   << "\n    strike:     " << strike
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }
}

void NormalCLVModelTest::testMoustacheGraph() {
    BOOST_TEST_MESSAGE(
        "Testing Double no Touch pricing with Normal CLV Model...");

    SavedSettings backup;

    /*
     The comparison of Black-Scholes and normal CLV prices is derived
     from figure 8.8 in Iain J. Clark's book,
     Foreign Exchange Option Pricing: A Practitioner’s Guide
    */

    const DayCounter dc = ActualActual();
    const Date todaysDate(5, Aug, 2016);
    const Date maturityDate = todaysDate + Period(1, Years);
    const Time maturityTime = dc.yearFraction(todaysDate, maturityDate);

    Settings::instance().evaluationDate() = todaysDate;

    const Real s0 = 100;
    const Handle<Quote> spot(boost::make_shared<SimpleQuote>(s0));
    const Rate r = 0.02;
    const Rate q = 0.01;

    // parameter of the "calibrated" Heston model
    const Real kappa =   1.0;
    const Real theta =   0.06;
    const Real rho   =  -0.8;
    const Real sigma =   0.8;
    const Real v0    =   0.09;

    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    const boost::shared_ptr<HestonModel> hestonModel(
        boost::make_shared<HestonModel>(
            boost::make_shared<HestonProcess>(
                rTS, qTS, spot, v0, kappa, theta, sigma, rho)));

    const Handle<BlackVolTermStructure> vTS(
        boost::make_shared<HestonBlackVolSurface>(
            Handle<HestonModel>(hestonModel)));

    const boost::shared_ptr<GeneralizedBlackScholesProcess> bsProcess =
        boost::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, vTS);

    // Ornstein-Uhlenbeck
    const Real speed   = -0.80;
    const Real level   = 100;
    const Real sigmaOU = 0.15;
    const Real x0      = 100;

    const boost::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess(
        boost::make_shared<OrnsteinUhlenbeckProcess>(
            speed, sigmaOU, x0, level));

    const boost::shared_ptr<Exercise> europeanExercise(
        boost::make_shared<EuropeanExercise>(maturityDate));

    VanillaOption vanillaOption(
        boost::make_shared<PlainVanillaPayoff>(Option::Call, s0),
        europeanExercise);

    vanillaOption.setPricingEngine(
        boost::make_shared<AnalyticHestonEngine>(hestonModel));

    const Volatility atmVol = vanillaOption.impliedVolatility(
        vanillaOption.NPV(),
        boost::make_shared<GeneralizedBlackScholesProcess>(spot, qTS, rTS,
            Handle<BlackVolTermStructure>(flatVol(std::sqrt(theta), dc))));

    const boost::shared_ptr<PricingEngine> analyticEngine(
        boost::make_shared<AnalyticDoubleBarrierBinaryEngine>(
            boost::make_shared<GeneralizedBlackScholesProcess>(
                spot, qTS, rTS,
                Handle<BlackVolTermStructure>(flatVol(atmVol, dc)))));


    std::vector<Date> maturities(1, todaysDate + Period(2, Weeks));
    while (maturities.back() < maturityDate)
        maturities.push_back(maturities.back() + Period(2, Weeks));

    const NormalCLVModel m(bsProcess, ouProcess, maturities, 8);
    const boost::function<Real(Real, Real)> g = m.g();

    const Size n = 18;
    Array barrier_lo(n), barrier_hi(n), bsNPV(n);

    const boost::shared_ptr<CashOrNothingPayoff> payoff =
        boost::make_shared<CashOrNothingPayoff>(Option::Call, 0.0, 1.0);

    for (Size i=0; i < n; ++i) {
        const Real dist = 10.0+5.0*i;

        barrier_lo[i] = std::max(s0 - dist, 1e-2);
        barrier_hi[i] = s0 + dist;
        DoubleBarrierOption doubleBarrier(
            DoubleBarrier::KnockOut, barrier_lo[i], barrier_hi[i], 0.0,
            payoff,
            europeanExercise);

        doubleBarrier.setPricingEngine(analyticEngine);
        bsNPV[i] = doubleBarrier.NPV();
    }

    typedef SobolBrownianBridgeRsg rsg_type;
    typedef PathGenerator<rsg_type>::sample_type sample_type;

    const Size factors = 1;
    const Size tSteps = 200;
    const TimeGrid grid(maturityTime, tSteps);

    const boost::shared_ptr<PathGenerator<rsg_type> > pathGenerator =
        boost::make_shared<PathGenerator<rsg_type> >(
            ouProcess, grid, rsg_type(factors, tSteps), false);

    const Size nSims = 400000;
    std::vector<GeneralStatistics> stats(n);
    const DiscountFactor df = rTS->discount(maturityDate);

    for (Size i=0; i < nSims; ++i) {
        std::vector<bool> touch(n, false);

        const sample_type& path = pathGenerator->next();

        Real s;
        for (Size j=1; j <= tSteps; ++j) {
            const Time t = grid.at(j);
            s = g(t, path.value.at(j));

            for (Size u=0; u < n; ++u) {
                if (s <= barrier_lo[u] || s >= barrier_hi[u]) {
                    touch[u] = true;
                }
            }
        }
        for (Size u=0; u < n; ++u) {
            if (touch[u]) {
                stats[u].add(0.0);
            }
            else {
                stats[u].add(df*payoff->operator()(s));
            }
        }
    }

    const Real expected[] = {
        0.00907689,  0.0892242,  0.139655,   0.111603,   0.059872,
        0.016652,   -0.0094134, -0.0207528, -0.0230873, -0.0201951,
        -0.0160556, -0.0116482, -0.00741535,-0.00355287,-0.000165914,
        0.00493464,  0.00342315, 0.0023405 };

    const Real tol = 1e-5;
    for (Size u=0; u < n; ++u) {
        const Real calculated = stats[u].mean() - bsNPV[u];

        if (std::fabs(calculated - expected[u]) > tol) {
            BOOST_FAIL("Failed to reproduce Double no Touch prices"
                   << "\n    time:          " << maturityDate
                   << "\n    barrier lower: " << barrier_lo[u]
                   << "\n    barrier high:  " << barrier_hi[u]
                   << "\n    calculated:    " << calculated
                   << "\n    expected:      " << expected[u]);
        }
    }
}


test_suite* NormalCLVModelTest::experimental() {
    test_suite* suite = BOOST_TEST_SUITE("NormalCLVModel tests");

    suite->add(QUANTLIB_TEST_CASE(
        &NormalCLVModelTest::testBSCumlativeDistributionFunction));
    suite->add(QUANTLIB_TEST_CASE(
        &NormalCLVModelTest::testHestonCumlativeDistributionFunction));
    suite->add(QUANTLIB_TEST_CASE(
        &NormalCLVModelTest::testIllustrative1DExample));
    suite->add(QUANTLIB_TEST_CASE(
            NormalCLVModelTest::testMonteCarloBSOptionPricing));
    suite->add(QUANTLIB_TEST_CASE(
            NormalCLVModelTest::testMoustacheGraph));

    return suite;
}
