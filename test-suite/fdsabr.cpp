/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2019 Klaus Spanderen

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

#include "fdsabr.hpp"
#include "utilities.hpp"

#include <ql/functional.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/statistics/generalstatistics.hpp>
#include <ql/pricingengines/vanilla/analyticcevengine.hpp>
#include <ql/pricingengines/vanilla/fdsabrvanillaengine.hpp>

#include <ql/math/richardsonextrapolation.hpp>
#include <ql/math/randomnumbers/sobolbrownianbridgersg.hpp>
#include <ql/methods/finitedifferences/utilities/cevrndcalculator.hpp>

#include <boost/make_shared.hpp>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;

namespace {
    class SabrMonteCarloPricer {
      public:
        SabrMonteCarloPricer(
            Real f0, Time maturity,
            const ext::shared_ptr<Payoff>& payoff,
            Real alpha, Real beta, Real nu, Real rho)
        : f0_(f0), maturity_(maturity), payoff_(payoff),
          alpha_(alpha), beta_(beta), nu_(nu), rho_(rho) { }

        Real operator()(Real dt) const {
            const Size nSims = 64*1024;

            const Real timeStepsPerYear = 1./dt;
            const Size timeSteps = Size(maturity_*timeStepsPerYear+1e-8);

            const Real sqrtDt = std::sqrt(dt);
            const Real w = std::sqrt(1.0-rho_*rho_);

            const Real logAlpha = std::log(alpha_);

            SobolBrownianBridgeRsg rsg(2, timeSteps, SobolBrownianGenerator::Diagonal, 12345U);

            GeneralStatistics stats;

            for (Size i=0; i < nSims; ++i) {
                Real f = f0_;
                Real a = logAlpha;

                const std::vector<Real> n = rsg.nextSequence().value;

                for (Size j=0; j < timeSteps && f > 0.0; ++j) {

                    const Real r1 = n[j];
                    const Real r2 = rho_*r1 + n[j+timeSteps]*w;

                    //Sample CEV distribution: accurate but slow
                    //
                    //const CEVRNDCalculator calc(f, std::exp(a), beta_);
                    //const Real u = CumulativeNormalDistribution()(r1);
                    //f = calc.invcdf(u, dt);

                    // simple Euler method
                    f += std::exp(a)*std::pow(f, beta_)*r1*sqrtDt;
                    a += - 0.5*nu_*nu_*dt + nu_*r2*sqrtDt;
                }
                f = std::max(0.0, f);
                stats.add((*payoff_)(f));
            }

            return stats.mean();
        }

      private:
        const Real f0_;
        const Time maturity_;
        const ext::shared_ptr<Payoff> payoff_;
        const Real alpha_, beta_, nu_, rho_;
    };

}


void FdSabrTest::testFdmSabrOp() {
    BOOST_TEST_MESSAGE("Testing FDM SABR operator...");

    SavedSettings backup;

    const Date today = Date(22, February, 2018);
    const DayCounter dc = Actual365Fixed();
    Settings::instance().evaluationDate() = today;

    const Date maturityDate = today + Period(2, Years);
    const Time maturityTime = dc.yearFraction(today, maturityDate);

    const Real strike = 1.5;

    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturityDate);

    const ext::shared_ptr<PlainVanillaPayoff> putPayoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike);
    const ext::shared_ptr<PlainVanillaPayoff> callPayoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Call, strike);

    VanillaOption optionPut(putPayoff, exercise);
    VanillaOption optionCall(callPayoff, exercise);

    const Handle<YieldTermStructure> rTS =
        Handle<YieldTermStructure>(flatRate(today, 0.0, dc));

    const Real f0    = 1.0;
    const Real alpha = 0.35;
    const Real nu    = 1.0;
    const Real rho   = 0.25;

    const Real betas[] = { 0.25, 0.6 };

    const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess =
        ext::make_shared<GeneralizedBlackScholesProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(f0)),
            rTS, rTS, Handle<BlackVolTermStructure>(flatVol(0.2, dc)));

    for (Size j=0; j < LENGTH(betas); ++j) {

        const Real beta = betas[j];

        const ext::shared_ptr<PricingEngine> pdeEngine =
            ext::make_shared<FdSabrVanillaEngine>(
                f0, alpha, beta, nu, rho, rTS, 100, 400, 100);

        optionPut.setPricingEngine(pdeEngine);
        const Real pdePut = optionPut.NPV();

        // check put/call parity
        optionCall.setPricingEngine(pdeEngine);
        const Real pdeCall = optionCall.NPV();

        const Real pdeFwd = pdeCall - pdePut;

        const Real parityDiff = std::fabs(pdeFwd - (f0 - strike));
        const Real parityTol = 1e-4;
        if (parityDiff > parityTol) {
            BOOST_ERROR(
                "failed to validate the call/put parity"
                << "\n    beta           : " << beta
                << "\n    strike         : " << strike
                << "\n    fwd (call/put) : " << pdeFwd
                << "\n    fwd (f0-strike): " << f0-strike
                << "\n    diff           : " << parityDiff
                << "\n    tol            : " << parityTol);
        }

        const Real putPdeImplVol =
            optionPut.impliedVolatility(optionPut.NPV(), bsProcess, 1e-6);

        const ext::function<Real(Real)> mcSabr(
            SabrMonteCarloPricer(f0, maturityTime, putPayoff,
                                 alpha, beta, nu, rho));

        const Real mcNPV = RichardsonExtrapolation(
            mcSabr, 1/4.0)(4.0, 2.0);

        const Real putMcImplVol =
            optionPut.impliedVolatility(mcNPV, bsProcess, 1e-6);

        const Real volDiff = std::fabs(putPdeImplVol - putMcImplVol);

        const Real volTol = 5e-3;
        if (volDiff > volTol) {
            BOOST_ERROR(
                "failed to validate PDE against MC implied volatility"
                << "\n    beta         : " << beta
                << "\n    strike       : " << strike
                << "\n    PDE impl vol : " << putPdeImplVol
                << "\n    MC  impl vol : " << putMcImplVol
                << "\n    diff         : " << volDiff
                << "\n    tol          : " << volTol);
        }
    }
}

void FdSabrTest::testFdmSabrCevPricing() {
    BOOST_TEST_MESSAGE("Testing FDM CEV pricing with trivial SABR model...");

    SavedSettings backup;

    const Date today = Date(3, January, 2019);
    const DayCounter dc = Actual365Fixed();
    Settings::instance().evaluationDate() = today;

    const Date maturityDate = today + Period(12, Months);

    const Real betas[]   = { 0.1, 0.9 };
    const Real strikes[] = { 0.9, 1.5 };

    const Real f0    = 1.2;
    const Real alpha = 0.35;
    const Real nu    = 1e-3;
    const Real rho   = 0.25;

    const Handle<YieldTermStructure> rTS = Handle<YieldTermStructure>(
        flatRate(today, 0.05, dc));

    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturityDate);

    const Option::Type optionTypes[] = {Option::Put, Option::Call};

    const Real tol = 5e-5;

    for (Size i=0; i < LENGTH(optionTypes); ++i) {
        const Option::Type optionType = optionTypes[i];

        for (Size j=0; j < LENGTH(strikes); ++j) {
            const Real strike = strikes[j];

            const ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::make_shared<PlainVanillaPayoff>(optionType, strike);

            VanillaOption option(payoff, exercise);

            for (Size k=0; k < LENGTH(betas); ++k) {
                const Real beta = betas[k];

                option.setPricingEngine(ext::make_shared<FdSabrVanillaEngine>(
                    f0, alpha, beta, nu, rho, rTS, 100, 400, 3));

                const Real calculated = option.NPV();

                option.setPricingEngine(ext::make_shared<AnalyticCEVEngine>(
                    f0, alpha, beta, rTS));

                const Real expected = option.NPV();

                if (std::fabs(expected-calculated) > tol) {
                    BOOST_ERROR(
                        "failed to calculate vanilla CEV option prices"
                        << "\n    beta            : " << beta
                        << "\n    strike          : " << strike
                        << "\n    option type     : "
                        << ((payoff->optionType() == Option::Call) ? "Call" : "Put")
                        << "\n    analytic npv    : " << expected
                        << "\n    pde npv         : " << calculated
                        << "\n    npv difference  : "
                        << std::fabs(expected - calculated)
                        << "\n    tolerance       : " << tol);
                }
            }
        }
    }
}

void FdSabrTest::testFdmSabrVsVolApproximation() {
    BOOST_TEST_MESSAGE("Testing FDM SABR vs approximations...");

    SavedSettings backup;

    const Date today = Date(8, January, 2019);
    const DayCounter dc = Actual365Fixed();
    Settings::instance().evaluationDate() = today;

    const Date maturityDate = today + Period(6, Months);
    const Time maturityTime = dc.yearFraction(today, maturityDate);

    const Handle<YieldTermStructure> rTS = Handle<YieldTermStructure>(
        flatRate(today, 0.05, dc));

    const Real f0 = 100;

    const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess =
        ext::make_shared<GeneralizedBlackScholesProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(f0)),
            rTS, rTS, Handle<BlackVolTermStructure>(flatVol(0.2, dc)));

    const Real alpha = 0.35;
    const Real beta  = 0.85;
    const Real nu    = 0.75;
    const Real rho   = 0.85;

    const Real strikes[] = { 90, 100, 110};
    const Option::Type optionTypes[] = {Option::Put, Option::Call};

    const Real tol = 2.5e-3;
    for (Size i=0; i < LENGTH(optionTypes); ++i) {
        const Option::Type optionType = optionTypes[i];
        for (Size j=0; j < LENGTH(strikes); ++j) {
            const Real strike = strikes[j];

            VanillaOption option(
                ext::make_shared<PlainVanillaPayoff>(optionType, strike),
                ext::make_shared<EuropeanExercise>(maturityDate));

            option.setPricingEngine(ext::make_shared<FdSabrVanillaEngine>(
                f0, alpha, beta, nu, rho, rTS, 25, 100, 50));

            const Volatility fdmVol =
                option.impliedVolatility(option.NPV(), bsProcess);

            const Real hagenVol = sabrVolatility(
                strike, f0, maturityTime, alpha, beta, nu, rho);

            const Real diff = std::fabs(fdmVol - hagenVol);

            if (std::fabs(fdmVol-hagenVol) > tol) {
                BOOST_ERROR(
                    "large difference between Hagen formula and FDM"
                    << "\n    strike          : " << strike
                    << "\n    option type     : "
                    << ((optionType == Option::Call) ? "Call" : "Put")
                    << "\n    Hagen vol       : " << hagenVol
                    << "\n    pde vol         : " << fdmVol
                    << "\n    vol difference  : " << diff
                    << "\n    tolerance       : " << tol);
            }
        }
    }
}


namespace {
    /*
     * Example and reference values are taken from
     * B. Chen, C.W. Oosterlee, H. Weide,
     * Efficient unbiased simulation scheme for the SABR stochastic volatility model.
     * https://http://ta.twi.tudelft.nl/mf/users/oosterle/oosterlee/SABRMC.pdf
     */

    class OsterleeReferenceResults {
      public:
        explicit OsterleeReferenceResults(Size i) : i_(i) { }

        Real operator()(Real t) const {
            Size i;
            if (close_enough(t, 1/16.))
                i = 0;
            else if (close_enough(t, 1/32.))
                i = 1;
            else
                QL_FAIL("unmatched reference result lookup");

            return data_[i_][i];
        }

      private:
        const Size i_;
        static Real data_[9][3];
    };

    Real OsterleeReferenceResults::data_[9][3] = {
        { 0.0610, 0.0604 }, { 0.0468, 0.0463 }, { 0.0347, 0.0343 },
        { 0.0632, 0.0625 }, { 0.0512, 0.0506 }, { 0.0406, 0.0400 },
        { 0.0635, 0.0630 }, { 0.0523, 0.0520 }, { 0.0422, 0.0421 }
    };
}

void FdSabrTest::testOosterleeTestCaseIV() {
    BOOST_TEST_MESSAGE("Testing Chen, Oosterlee and Weide test case IV...");

    SavedSettings backup;

    const Date today = Date(8, January, 2019);
    const DayCounter dc = Actual365Fixed();
    Settings::instance().evaluationDate() = today;

    const Handle<YieldTermStructure> rTS =
        Handle<YieldTermStructure>(flatRate(today, 0.0, dc));

    const Real f0    =  0.07;
    const Real alpha =  0.4;
    const Real nu    =  0.8;
    const Real beta  =  0.4;
    const Real rho   = -0.6;

    const Period maturities[] = {
        Period(2, Years), Period(5, Years), Period(10, Years)
    };

    const Real strikes[] = { 0.4*f0, f0, 1.6*f0 };

    const Real tol = 0.00035;
    for (Size i=0; i < LENGTH(maturities); ++i) {
        const Date maturityDate = today + maturities[i];
        const Time maturityTime = dc.yearFraction(today, maturityDate);

        const Size timeSteps = Size(5*maturityTime);

        const ext::shared_ptr<PricingEngine> engine =
            ext::make_shared<FdSabrVanillaEngine>(
                f0, alpha, beta, nu, rho, rTS, timeSteps, 200, 21);

        const ext::shared_ptr<Exercise> exercise =
            ext::make_shared<EuropeanExercise>(maturityDate);

        for (Size j=0; j < LENGTH(strikes); ++j) {
            const ext::shared_ptr<StrikedTypePayoff> payoff =
                ext::make_shared<PlainVanillaPayoff>(Option::Call, strikes[j]);

            VanillaOption option(payoff, exercise);
            option.setPricingEngine(engine);

            const Real calculated = option.NPV();

            const OsterleeReferenceResults referenceResuts(i*3+j);

            const Real expected = RichardsonExtrapolation(
                ext::function<Real(Real)>(referenceResuts), 1/16., 1)(2.);

            const Real diff = std::fabs(calculated - expected);
            if (diff > tol) {
                BOOST_ERROR(
                    "can not reproduce reference values from Monte-Carlo"
                    << "\n    strike     : " << payoff->strike()
                    << "\n    maturity   : " << maturityDate
                    << "\n    reference  : " << expected
                    << "\n    calculated : " << calculated
                    << "\n    difference : " << diff
                    << "\n    tolerance  : " << tol);
            }
        }
    }
}

void FdSabrTest::testBenchOpSabrCase() {
    BOOST_TEST_MESSAGE("Testing SABR BenchOp problem...");

    /*
     * von Sydow, L, Milovanović, S, Larsson, E, In't Hout, K,
     * Wiktorsson, M, Oosterlee, C.W, Shcherbakov, V, Wyns, M,
     * Leitao Rodriguez, A, Jain, S, et al. (2018)
     * BENCHOP–SLV: the BENCHmarking project in Option
     * Pricing–Stochastic and Local Volatility problems
     * https://ir.cwi.nl/pub/28249
     */

    SavedSettings backup;

    const Date today = Date(8, January, 2019);
    const DayCounter dc = Actual365Fixed();
    Settings::instance().evaluationDate() = today;

    const Handle<YieldTermStructure> rTS =
        Handle<YieldTermStructure>(flatRate(today, 0.0, dc));

    const Size maturityInYears[] = { 2, 10 };

    const Real f0s[]    = { 0.5, 0.07 };
    const Real alphas[] = { 0.5, 0.4 };
    const Real nus[]    = { 0.4, 0.8 };
    const Real betas[]  = { 0.5, 0.5 };
    const Real rhos[]   = { 0.0, -0.6 };

    const Real expected[2][3] = {
        { 0.221383196830866, 0.193836689413803, 0.166240814653231 },
        { 0.052450313614407, 0.046585753491306, 0.039291470612989 }
    };

    const Size gridX = 400;
    const Size gridY = 25;
    const Size gridT = 10;

    const Real factor = 2;

    const Real tol = 2e-4;

    for (Size i=0; i < LENGTH(f0s); ++i) {

        const Date maturity = today + Period(maturityInYears[i]*365, Days);
        const Time T = dc.yearFraction(today, maturity);

        const Real f0    = f0s[i];
        const Real alpha = alphas[i];
        const Real nu    = nus[i];
        const Real beta  = betas[i];
        const Real rho   = rhos[i];

        const Real strikes[] = {
            f0*std::exp(-0.1*std::sqrt(T)), f0, f0*std::exp(0.1*std::sqrt(T))
        };

        for (Size j=0; j < LENGTH(strikes); ++j) {
            const Real strike = strikes[j];

            VanillaOption option(
                ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
                ext::make_shared<EuropeanExercise>(maturity));

            option.setPricingEngine(ext::make_shared<FdSabrVanillaEngine>(
                    f0, alpha, beta, nu, rho, rTS,
                    Size(gridT*factor),
                    Size(gridX*factor),
                    Size(gridY*std::sqrt(factor))));

            const Real calculated = option.NPV();
            const Real diff = std::fabs(calculated - expected[i][j]);

            if (diff > tol) {
                BOOST_ERROR(
                    "failed to reproduce reference values"
                    << "\n    strike     : " << strike
                    << "\n    maturity   : " << maturity
                    << "\n    reference  : " << expected[i][j]
                    << "\n    calculated : " << calculated
                    << "\n    difference : " << diff
                    << "\n    tolerance  : " << tol);
            }
        }
    }
}

test_suite* FdSabrTest::suite(SpeedLevel speed) {
    test_suite* suite = BOOST_TEST_SUITE("Finite Difference SABR tests");

    suite->add(QUANTLIB_TEST_CASE(&FdSabrTest::testFdmSabrOp));
    suite->add(QUANTLIB_TEST_CASE(&FdSabrTest::testFdmSabrCevPricing));
    suite->add(QUANTLIB_TEST_CASE(&FdSabrTest::testFdmSabrVsVolApproximation));
    suite->add(QUANTLIB_TEST_CASE(&FdSabrTest::testOosterleeTestCaseIV));
    suite->add(QUANTLIB_TEST_CASE(&FdSabrTest::testBenchOpSabrCase));

    return suite;
}
