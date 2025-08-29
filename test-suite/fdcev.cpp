/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2018 Klaus Spanderen

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/statistics/generalstatistics.hpp>
#include <ql/pricingengines/vanilla/analyticcevengine.hpp>
#include <ql/pricingengines/vanilla/fdcevvanillaengine.hpp>
#include <ql/methods/finitedifferences/utilities/cevrndcalculator.hpp>
#include <ql/shared_ptr.hpp>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FdCevTests)

class ExpectationFct {
  public:
    ExpectationFct(const CEVRNDCalculator& calculator, Time t)
    : t_(t), calculator_(calculator) { }

    Real operator()(Real f) const { return f*calculator_.pdf(f, t_); }

  private:
    const Time t_;
    const CEVRNDCalculator& calculator_;
};


BOOST_AUTO_TEST_CASE(testLocalMartingale) {
    BOOST_TEST_MESSAGE(
        "Testing local martingale property of CEV process with PDF...");

    const Time t = 1.0;

    const Real f0 = 2.1;
    const Real alpha = 1.75;
    const Real betas[] = {-2.4, 0.23, 0.9, 1.1, 1.5};

    for (Real beta : betas) {
        const CEVRNDCalculator rndCalculator(f0, alpha, beta);

        const Real eps = 1e-10;
        const Real tol = 100*eps;

        const Real upperBound = 10*rndCalculator.invcdf(1-eps, t);

        const Real expectationValue = GaussLobattoIntegral(10000, eps)(
            ExpectationFct(rndCalculator, t), QL_EPSILON, upperBound);

        const Real diff = expectationValue-f0;


        if (beta < 1.0 && std::fabs(diff) > tol) {
            BOOST_ERROR("CEV process should be a martingale for beta < 1.0"
                        << "\n    expected:   " << f0
                        << std::scientific
                        << "\n    difference  " << diff
                        << "\n    tolerance:  " << tol);
        }

        if (beta > 1.0 && diff > -tol) {
            BOOST_ERROR("CEV process should only be a local martingale "
                        "for beta > 1.0. Expectation is E[F_t|F_0] < F_0"
                        << "\n    E[F_t|F_0]: " << expectationValue
                        << "\n    F_0:        " << f0);
        }

        // check local martingale property with Monte-Carlo simulation
        const Size nSims = 5000;

        const Size nSteps = 2000;
        const Real dt = t / nSteps;
        const Real sqrtDt = std::sqrt(dt);

        GeneralStatistics stat;
        const PseudoRandom::rng_type mt(MersenneTwisterUniformRng(42));

        if (beta > 1.2) {
            for (Size i=0; i < nSims; ++i) {
                Real f = f0;
                for (Size j=0; j < nSteps; ++j) {
                    f += alpha * std::pow(f, beta) * mt.next().value * sqrtDt;
                    f = std::max(0.0, f);

                    if (f == 0.0) break; // absorbing boundary
                }
                stat.add(f - f0);
            }

            const Real calculated = stat.mean();
            const Real error = stat.errorEstimate();

            if (std::fabs(calculated - diff) > 2.35*error) {
                BOOST_ERROR(
                    "failed to calculate local martingale property "
                    "by Monte-Carlo Simulation for beta > 1.0. "
                            << "\n    E[F_t|F_0]   : " << expectationValue
                            << "\n    E_MC[F_t|F_0]: " << calculated + f0
                            << "\n    error_MC     : " << error
                            << "\n    difference   : " << std::fabs(calculated - diff)
                            << "\n    tolerance    : " << 2.35*error);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testFdmCevOp) {
    BOOST_TEST_MESSAGE(
            "Testing FDM constant elasticity of variance (CEV) operator...");

    const Date today = Date(22, February, 2018);
    const DayCounter dc = Actual365Fixed();
    Settings::instance().evaluationDate() = today;

    const Date maturityDate = today + Period(12, Months);
    const Real strike = 2.3;

    const Option::Type optionTypes[] = { Option::Call, Option::Put};

    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturityDate);

    for (auto optionType : optionTypes) {
        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(optionType, strike);

        const ext::shared_ptr<YieldTermStructure> rTS =
            flatRate(today, 0.15, dc);

        const Real f0 = 2.1;
        const Real alpha = 0.75;

        const Real betas[] = { -2.0, -0.5, 0.45, 0.6, 0.9, 1.45 };
        for (Real beta : betas) {

            VanillaOption option(payoff, exercise);
            option.setPricingEngine(ext::make_shared<AnalyticCEVEngine>(
                f0, alpha, beta, Handle<YieldTermStructure>(rTS)));

            const Real analyticNPV = option.NPV();

            const Real eps = 1e-3;

            option.setPricingEngine(ext::make_shared<AnalyticCEVEngine>(
                f0*(1+eps), alpha, beta, Handle<YieldTermStructure>(rTS)));
            const Real analyticUpNPV = option.NPV();

            option.setPricingEngine(ext::make_shared<AnalyticCEVEngine>(
                f0*(1-eps), alpha, beta, Handle<YieldTermStructure>(rTS)));
            const Real analyticDownNPV = option.NPV();

            const Real analyticDelta = (analyticUpNPV - analyticDownNPV)
                /(2*eps*f0);

            option.setPricingEngine(ext::make_shared<FdCEVVanillaEngine>(
                f0, alpha, beta, Handle<YieldTermStructure>(rTS),
                100, 1000, 1, 1.0, 1e-6));

            const Real calculatedNPV = option.NPV();
            const Real calculatedDelta = option.delta();

            const Real tol = 0.01;
            if (std::fabs(calculatedNPV - analyticNPV) > tol
                || std::fabs(calculatedDelta - analyticDelta) > tol) {
                BOOST_ERROR(
                    "failed to calculate vanilla option prices/delta "
                    << "\n    beta            : " << beta
                    << "\n    option type     : "
                    << ((payoff->optionType() == Option::Call) ? "Call" : "Put")
                    << "\n    analytic npv    : " << analyticNPV
                    << "\n    pde npv         : " << calculatedNPV
                    << "\n    npv difference  : "
                    << std::fabs(calculatedNPV - analyticNPV)
                    << "\n    tolerance       : " << tol
                    << "\n    analytic delta  : " << analyticDelta
                    << "\n    pde delta       : " << calculatedDelta
                    << "\n    delta difference: "
                    << std::fabs(calculatedDelta - analyticDelta)
                    << "\n    tolerance       : " << tol);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
