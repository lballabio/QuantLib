
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "old_pricers.hpp"
#include "utilities.hpp"
#include <ql/Pricers/fdeuropean.hpp>
#include <ql/Pricers/fdamericanoption.hpp>
#include <ql/Pricers/fdshoutoption.hpp>
#include <ql/Pricers/fddividendamericanoption.hpp>
#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/mceverest.hpp>
#include <ql/Pricers/mcmaxbasket.hpp>
#include <ql/Pricers/mcpagoda.hpp>
#include <ql/Pricers/mchimalaya.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/RandomNumbers/rngtraits.hpp>
#include <ql/MonteCarlo/getcovariance.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <boost/progress.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void OldPricerTest::testFdEuropeanPricer() {

    BOOST_MESSAGE("Testing old-style finite-difference European pricer...");

    QL_TEST_START_TIMING
    Real under = 100.0;
    Real strikeMin = 60.0, strikeRange = 100.0;
    Rate rRateMin = 0.0,  rRateRange = 0.18;
    Rate qRateMin = 0.0,  qRateRange = 0.02;
    Volatility volMin = 0.0, volRange = 1.2;
    Time timeMin = 0.5, timeRange = 2.0;

    Real tolerance = 1.0e-2;
    Size totCases = 200;

    PseudoRandom::urng_type rng(56789012);

    Option::Type types[] = { Option::Call, Option::Put };

    for (Size i=0; i<totCases; i++) {

        Real strike = strikeMin + strikeRange * rng.next().value;
        Rate qRate = qRateMin + qRateRange * rng.next().value;
        Rate rRate = rRateMin + rRateRange * rng.next().value;
        Volatility vol = volMin + volRange * rng.next().value;
        Time resTime = timeMin + timeRange * rng.next().value;

        for (Size j=0; j<LENGTH(types); j++) {

            DiscountFactor rDiscount = std::exp(-rRate*resTime);
            DiscountFactor qDiscount = std::exp(-qRate*resTime);
            Real forward = under*qDiscount/rDiscount;
            Real variance = vol*vol*resTime;
            boost::shared_ptr<StrikedTypePayoff> payoff(
                                    new PlainVanillaPayoff(types[j], strike));
            Real anValue = BlackFormula(forward, rDiscount,
                                        variance, payoff).value();
            Real numValue = FdEuropean(types[j], under, strike,
                                       qRate, rRate, resTime,
                                       vol, 100, 400).value();
            if (std::fabs(anValue-numValue) > tolerance)
                BOOST_FAIL("Option details: \n"
                           << "    type:           " << types[j] << "\n"
                           << "    underlying:     " << under << "\n"
                           << "    strike:         " << strike << "\n"
                           << "    dividend yield: " << io::rate(qRate) << "\n"
                           << "    risk-free rate: " << io::rate(rRate) << "\n"
                           << "    residual time:  " << resTime << "\n"
                           << "    volatility:     " << io::volatility(vol)
                           << "\n\n"
                           << "    calculated value: " << numValue << "\n"
                           << "    expected:         " << anValue);
        }
    }
}

namespace {

    template<class O>
    void testStepOption(Option::Type type, Real u, Real k,
                        Rate q, Rate r, Time T, Volatility v,
                        const std::string& name) {

        Size nstp = 145;
        Size ngrd = nstp+1;

        std::map<std::string,Real> calculated, expected, tolerance;
        tolerance["delta"]  = 2.0e-3;
        tolerance["gamma"]  = 2.0e-3;
        tolerance["theta"]  = 2.0e-3;
        tolerance["rho"]    = 2.0e-3;
        tolerance["divRho"] = 2.0e-3;
        tolerance["vega"]   = 2.0e-3;

        Real du = u*1.0e-4;
        Volatility dv = v*1.0e-4;
        Spread dr = r*1.0e-4;
        Spread dq = q*1.0e-4;

        O option(type,u,k,q,r,T,v,nstp,ngrd);
        if (option.value() > u*1.0e-5) {
            // greeks
            calculated["delta"]  = option.delta();
            calculated["gamma"]  = option.gamma();
            calculated["theta"]  = option.theta();
            calculated["rho"]    = option.rho();
            calculated["divRho"] = option.dividendRho();
            calculated["vega"]   = option.vega();
            // recalculate greeks numerically
            O optPs(type,u+du,k,q,   r,   T,v,   nstp,ngrd);
            O optMs(type,u-du,k,q,   r,   T,v,   nstp,ngrd);
            O optPr(type,u,   k,q,   r+dr,T,v,   nstp,ngrd);
            O optMr(type,u,   k,q,   r-dr,T,v,   nstp,ngrd);
            O optPq(type,u,   k,q+dq,r,   T,v,   nstp,ngrd);
            O optMq(type,u,   k,q-dq,r,   T,v,   nstp,ngrd);
            O optPv(type,u,   k,q,   r,   T,v+dv,nstp,ngrd);
            O optMv(type,u,   k,q,   r,   T,v-dv,nstp,ngrd);

            expected["delta"]  = (optPs.value()-optMs.value())/(2*du);
            expected["gamma"]  = (optPs.delta()-optMs.delta())/(2*du);
            expected["theta"]  = r*option.value() - (r-q)*u*option.delta()
                                 - 0.5*v*v*u*u*option.gamma();
            expected["rho"]    = (optPr.value()-optMr.value())/(2*dr);
            expected["divRho"] = (optPq.value()-optMq.value())/(2*dq);
            expected["vega"]   = (optPv.value()-optMv.value())/(2*dv);

            // check
            std::map<std::string,Real>::iterator it;
            for (it = expected.begin(); it != expected.end(); ++it) {
                std::string greek = it->first;
                Real expct = expected[greek];
                Real calcl = calculated[greek];
                Real tol = tolerance[greek];
                if (relativeError(expct,calcl,u) > tol)
                    BOOST_FAIL(
                        "Option details:"
                        << "\n    type:           " << name << " " << type
                        << "\n    underlying:     " << u
                        << "\n    strike:         " << k
                        << "\n    dividend yield: " << io::rate(q)
                        << "\n    risk-free rate: " << io::rate(r)
                        << "\n    residual time:  " << T
                        << "\n    volatility:     " << io::volatility(v)
                        << "\n\n"
                        << "    calculated " << greek << ": " << calcl << "\n"
                        << "    expected:  " << greek << ": " << expct);
            }
        }
    }

}

void OldPricerTest::testFdAmericanPricers() {

    BOOST_MESSAGE(
              "Testing old-style finite-difference American-type pricers...");

    QL_TEST_START_TIMING

    Option::Type types[] = { Option::Call, Option::Put };
    Real underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Time residualTimes[] = { 1.0 };
    Real strikes[] = { 50, 100, 150 };
    Volatility volatilities[] = { 0.05, 0.5, 1.2 };

    for (Size i1=0; i1<LENGTH(types); i1++) {
      for (Size i2=0; i2<LENGTH(underlyings); i2++) {
        for (Size i3=0; i3<LENGTH(rRates); i3++) {
          for (Size i4=0; i4<LENGTH(qRates); i4++) {
            for (Size i5=0; i5<LENGTH(residualTimes); i5++) {
              for (Size i6=0; i6<LENGTH(strikes); i6++) {
                for (Size i7=0; i7<LENGTH(volatilities); i7++) {
                  // test data
                  Option::Type type = types[i1];
                  Real u = underlyings[i2];
                  Rate r = rRates[i3];
                  Rate q = qRates[i4];
                  Time T = residualTimes[i5];
                  Real k = strikes[i6];
                  Volatility v = volatilities[i7];

                  testStepOption<FdAmericanOption>(type,u,k,q,r,T,v,
                      std::string("American"));
                  testStepOption<FdShoutOption>(type,u,k,q,r,T,v,
                      std::string("shout"));

                }
              }
            }
          }
        }
      }
    }
}

void OldPricerTest::testFdAmericanDividendPricers() {
    BOOST_MESSAGE(
               "Testing finite-difference American pricer with dividends...");

    QL_TEST_START_TIMING

    std::vector<Rate> dividendA, blankA;
    std::vector<Time> dividendTimeA, blankTimeA;

    Real under = 54.625;
    Real strike = 55;
    Rate dividendYield = 0.0;
    Rate interestRate = 0.052706;
    Time expirationTime = 0.126027;
    Volatility sigma = 0.282922;
    Size timeSteps = 40;
    Size assetSteps = 300;
    Real tolerance = 1.0e-6;

    FdDividendAmericanOption opt1(Option::Call, under, strike,
                                  dividendYield, interestRate,
                                  expirationTime, sigma,
                                  blankA, blankTimeA,
                                  timeSteps, assetSteps);
    Real refValue = opt1.value();

    for (Size i=0; i <= 6; i++) {
        FdDividendAmericanOption opt(Option::Call, under, strike,
                                     dividendYield, interestRate,
                                     expirationTime, sigma,
                                     dividendA, dividendTimeA,
                                     timeSteps, assetSteps);

        Real value = opt.value();

        if (std::fabs(refValue-value) > tolerance)
            BOOST_FAIL("DividendAmericanOption :\n"
                       << "    calculated value: " << value << "\n"
                       << "    expected:         " << refValue);

        dividendA.push_back(0.0);
        dividendTimeA.push_back(1.0e-3 * i);
    }
}


namespace {

    struct Batch4Data {
        Option::Type type;
        Real underlying;
        Real strike;
        Rate dividendYield;
        Rate riskFreeRate;
        Time first;
        Time length;
        Size fixings;
        Volatility volatility;
        bool controlVariate;
        Real result;
    };

    typedef Batch4Data Batch5Data;
}

void OldPricerTest::testMcSingleFactorPricers() {

    BOOST_MESSAGE("Testing old-style Monte Carlo single-factor pricers...");

    QL_TEST_START_TIMING

    DayCounter dc = Actual360();

    BigNatural seed = 3456789;

    // cannot be too low, or one cannot compare numbers when
    // switching to a new default generator
    Size fixedSamples = 1023;
    Real minimumTol = 1.0e-2;

    // batch 5
    //
    // data from "Asian Option", Levy, 1997
    // in "Exotic Options: The State of the Art",
    // edited by Clewlow, Strickland

    Batch5Data cases5[] = {
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 2,
          0.13, true, 1.51917595129 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 4,
          0.13, true, 1.67940165674 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 8,
          0.13, true, 1.75371215251 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 12,
          0.13, true, 1.77595318693 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 26,
          0.13, true, 1.81430536630 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 52,
          0.13, true, 1.82269246898 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 100,
          0.13, true, 1.83822402464 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 250,
          0.13, true, 1.83875059026 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 500,
          0.13, true, 1.83750703638 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 1000,
          0.13, true, 1.83887181884 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 2,
          0.13, true, 1.51154400089 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 4,
          0.13, true, 1.67103508506 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 8,
          0.13, true, 1.74529684070 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 12,
          0.13, true, 1.76667074564 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 26,
          0.13, true, 1.80528400613 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 52,
          0.13, true, 1.81400883891 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 100,
          0.13, true, 1.82922901451 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 250,
          0.13, true, 1.82937111773 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 500,
          0.13, true, 1.82826193186 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 1000,
          0.13, true, 1.82967846654 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 2,
          0.13, true, 1.49648170891 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 4,
          0.13, true, 1.65443100462 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 8,
          0.13, true, 1.72817806731 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 12,
          0.13, true, 1.74877367895 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 26,
          0.13, true, 1.78733801988 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 52,
          0.13, true, 1.79624826757 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 100,
          0.13, true, 1.81114186876 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 250,
          0.13, true, 1.81101152587 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 500,
          0.13, true, 1.81002311939 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 1000,
          0.13, true, 1.81145760308 }
    };

    for (Size l=0; l<LENGTH(cases5); l++) {
        Time dt = cases5[l].length/(cases5[l].fixings-1);
        std::vector<Time> timeIncrements(cases5[l].fixings);
        for (Size i=0; i<cases5[l].fixings; i++)
            timeIncrements[i] = i*dt + cases5[l].first;

        Date today = Date::todaysDate();
        Handle<YieldTermStructure> riskFreeRate(
                                flatRate(today, cases5[l].riskFreeRate, dc));
        Handle<YieldTermStructure> dividendYield(
                                flatRate(today, cases5[l].dividendYield, dc));
        Handle<BlackVolTermStructure> volatility(
                                flatVol(today, cases5[l].volatility, dc));

        McDiscreteArithmeticASO pricer(cases5[l].type,
                                       cases5[l].underlying,
                                       dividendYield,
                                       riskFreeRate,
                                       volatility,
                                       timeIncrements,
                                       cases5[l].controlVariate,
                                       seed);
        Real value = pricer.valueWithSamples(fixedSamples);
        if (std::fabs(value-cases5[l].result) > 2.0e-2)
            BOOST_FAIL("Batch 5, case " << l+1 << ":\n"
                       << std::setprecision(10)
                       << "    calculated value: " << value << "\n"
                       << "    expected:         " << cases5[l].result);
        Real tolerance = pricer.errorEstimate()/value;
        tolerance = std::min<Real>(tolerance/2.0, minimumTol);
        value = pricer.value(tolerance);
        Real accuracy = pricer.errorEstimate()/value;
        if (accuracy > tolerance)
            BOOST_FAIL("Batch 5, case " << l+1 << ":\n"
                       << std::setprecision(10)
                       << "    reached accuracy: " << accuracy << "\n"
                       << "    expected:         " << tolerance);
    }

}

namespace {

    template <class P>
    void testMcMFPricer(const P& pricer, Real storedValue,
                        Real tolerance, const std::string& name) {

        // cannot be too low, or one cannot compare numbers when
        // switching to a new default generator
        Size fixedSamples = 1023;
        Real minimumTol = 1.0e-2;

        Real value = pricer.valueWithSamples(fixedSamples);
        if (std::fabs(value-storedValue) > tolerance)
            BOOST_FAIL(name << ":\n"
                       << std::setprecision(10)
                       << "    calculated value: " << value << "\n"
                       << "    expected:         " << storedValue);

        tolerance = pricer.errorEstimate()/value;
        tolerance = std::min<Real>(tolerance/2.0, minimumTol);
        value = pricer.value(tolerance);
        Real accuracy = pricer.errorEstimate()/value;
        if (accuracy > tolerance)
            BOOST_FAIL(name << ":\n"
                       << std::setprecision(10)
                       << "    reached accuracy: " << accuracy << "\n"
                       << "    expected:         " << tolerance);

    }

}

void OldPricerTest::testMcMultiFactorPricers() {

    BOOST_MESSAGE("Testing old-style Monte Carlo multi-factor pricers...");

    QL_TEST_START_TIMING;

    DayCounter dc = Actual360();
    Matrix correlation(4,4);
    correlation[0][0] = 1.00;
                    correlation[0][1] = 0.50;
                                    correlation[0][2] = 0.30;
                                                    correlation[0][3] = 0.10;
    correlation[1][0] = 0.50;
                    correlation[1][1] = 1.00;
                                    correlation[1][2] = 0.20;
                                                    correlation[1][3] = 0.40;
    correlation[2][0] = 0.30;
                    correlation[2][1] = 0.20;
                                    correlation[2][2] = 1.00;
                                                    correlation[2][3] = 0.60;
    correlation[3][0] = 0.10;
                    correlation[3][1] = 0.40;
                                    correlation[3][2] = 0.60;
                                                    correlation[3][3] = 1.00;

    Date today = Date::todaysDate();
    std::vector<Handle<BlackVolTermStructure> > volatilities(4);
    volatilities[0] = Handle<BlackVolTermStructure>(flatVol(today, 0.30, dc));
    volatilities[1] = Handle<BlackVolTermStructure>(flatVol(today, 0.35, dc));
    volatilities[2] = Handle<BlackVolTermStructure>(flatVol(today, 0.25, dc));
    volatilities[3] = Handle<BlackVolTermStructure>(flatVol(today, 0.20, dc));

    std::vector<Handle<YieldTermStructure> > dividendYields(4);
    dividendYields[0] = Handle<YieldTermStructure>(flatRate(today, 0.01, dc));
    dividendYields[1] = Handle<YieldTermStructure>(flatRate(today, 0.05, dc));
    dividendYields[2] = Handle<YieldTermStructure>(flatRate(today, 0.04, dc));
    dividendYields[3] = Handle<YieldTermStructure>(flatRate(today, 0.03, dc));

    Handle<YieldTermStructure> riskFreeRate(flatRate(today, 0.05, dc));
    Time resTime = 1.0;

    // degenerate portfolio
    Matrix perfectCorrelation(4,4,1.0);
    Handle<BlackVolTermStructure> sameVol(flatVol(today, 0.30, dc));
    Handle<YieldTermStructure> sameDividend(flatRate(today, 0.03, dc));

    std::vector<Handle<BlackVolTermStructure> > sameAssetVols(4, sameVol);
    std::vector<Handle<YieldTermStructure> > sameAssetDividend(4, sameDividend);

    BigNatural seed = 86421;

    // McEverest
    testMcMFPricer(McEverest(dividendYields, riskFreeRate, volatilities,
                             correlation, resTime, seed),
                   0.75784944,
                   1.0e-8,
                   "McEverest");

    std::vector<Real> sameAssetValues(4,25.0);
    Real strike;

    // McMaxBasket
    std::vector<Real> assetValues(4);
    assetValues[0] = 100.0;
    assetValues[1] = 110.0;
    assetValues[2] =  90.0;
    assetValues[3] = 105.0;
    testMcMFPricer(McMaxBasket(assetValues, dividendYields, riskFreeRate,
                               volatilities, correlation, resTime, seed),
                   122.87781492,
                   1.0e-8,
                   "McMaxBasket");

    // McPagoda
    std::vector<Real> portfolio(4);
    portfolio[0] = 0.15;
    portfolio[1] = 0.20;
    portfolio[2] = 0.35;
    portfolio[3] = 0.30;
    Real fraction = 0.62;
    Real roof = 0.20;
    std::vector<Time> timeIncrements(4);
    timeIncrements[0] = 0.25;
    timeIncrements[1] = 0.50;
    timeIncrements[2] = 0.75;
    timeIncrements[3] = 1.00;
    testMcMFPricer(McPagoda(portfolio, fraction, roof, dividendYields,
                            riskFreeRate, volatilities, correlation,
                            timeIncrements, seed),
                   0.03829317,
                   1.0e-8,
                   "McPagoda");

    // McHimalaya
    strike = 101.0;
    testMcMFPricer(McHimalaya(assetValues, dividendYields, riskFreeRate,
                              volatilities, correlation, strike,
                              timeIncrements, seed),
                   5.80409038,
                   1.0e-8,
                   "McHimalaya");
}


test_suite* OldPricerTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Old-style pricer tests");
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testFdEuropeanPricer));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testFdAmericanPricers));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testFdAmericanDividendPricers));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testMcSingleFactorPricers));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testMcMultiFactorPricers));
    return suite;
}

