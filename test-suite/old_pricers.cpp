
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
#include <ql/Pricers/cliquetoption.hpp>
#include <ql/Pricers/dividendeuropeanoption.hpp>
#include <ql/Pricers/fdeuropean.hpp>
#include <ql/Pricers/fdamericanoption.hpp>
#include <ql/Pricers/fdshoutoption.hpp>
#include <ql/Pricers/discretegeometricapo.hpp>
#include <ql/Pricers/continuousgeometricapo.hpp>
#include <ql/Pricers/mcdiscretearithmeticapo.hpp>
#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/mceverest.hpp>
#include <ql/Pricers/mcbasket.hpp>
#include <ql/Pricers/mcmaxbasket.hpp>
#include <ql/Pricers/mcpagoda.hpp>
#include <ql/Pricers/mchimalaya.hpp>
#include <ql/RandomNumbers/rngtypedefs.hpp>
#include <ql/MonteCarlo/getcovariance.hpp>
#include <ql/DayCounters/actual365.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void OldPricerTest::testCliquetPricer() {

    BOOST_MESSAGE("Testing old-style cliquet option pricer...");

    double spot = 60.0;
    double moneyness = 1.1;
    std::vector<Spread> divYield(2);
    divYield[0] = 0.04; divYield[1] = 0.04;
    std::vector<Rate> rRate(2);
    rRate[0] = 0.08; rRate[1] = 0.08;
    std::vector<Time> dates(2);
    dates[0] = 0.25; dates[1] = 1.00;
    std::vector<double> vol(2);
    vol[0] = 0.30; vol[1] = 0.30;
    CliquetOptionPricer cliquet(Option::Call, spot, moneyness,
                                divYield, rRate, dates, vol);
    double calculated = cliquet.value();
    double expected = 4.4064; // Haug, p.37
    if (QL_FABS(calculated-expected) > 1.0e-4)
        BOOST_FAIL(
            "calculated value: " +
            DoubleFormatter::toString(calculated) + "\n"
            "expected:         " +
            DoubleFormatter::toString(expected));
}

void OldPricerTest::testDividendEuropeanPricer() {

    BOOST_MESSAGE("Testing old-style European option pricer "
                  "with dividends...");

    Size nstp = 150;
//    Size ngrd = nstp+1;
    std::vector<double> div(2);
    div[0] = 3.92; div[1] = 4.21;
    std::vector<Time> dates(2);
    dates[0] = 0.333; dates[1] = 0.667;

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-4;
    tolerance["gamma"]  = 1.0e-4;
    tolerance["theta"]  = 1.0e-4;
    tolerance["rho"]    = 1.0e-4;
    tolerance["vega"]   = 1.0e-4;

    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.10, 0.30 };
    Rate qRates[] = { 0.00, 0.05, 0.15 };
    Time residualTimes[] = { 1.0, 2.0 };
    double strikes[] = { 50, 99.5, 100, 100.5, 150 };
    double volatilities[] = { 0.04, 0.2, 0.7 };

    for (Size i1=0; i1<LENGTH(types); i1++) {
      for (Size i2=0; i2<LENGTH(underlyings); i2++) {
        for (Size i3=0; i3<LENGTH(rRates); i3++) {
          for (Size i4=0; i4<LENGTH(qRates); i4++) {
            for (Size i5=0; i5<LENGTH(residualTimes); i5++) {
              for (Size i6=0; i6<LENGTH(strikes); i6++) {
                for (Size i7=0; i7<LENGTH(volatilities); i7++) {
                  // test data
                  Option::Type type = types[i1];
                  double u = underlyings[i2];
                  Rate r = rRates[i3];
                  Rate q = qRates[i4];
                  Time T = residualTimes[i5];
                  double k = strikes[i6];
                  double v = volatilities[i7];
                  // increments
                  double du = u*1.0e-4;
                  Time dT = T/nstp;
                  double dv = v*1.0e-4;
                  Spread dr = r*1.0e-4;

                  // reference option
                  DividendEuropeanOption opt(type,u,k,q,r,T,v,div,dates);
                  if (opt.value() > u*1.0e-5) {
                    // greeks
                    calculated["delta"]  = opt.delta();
                    calculated["gamma"]  = opt.gamma();
                    calculated["theta"]  = opt.theta();
                    calculated["rho"]    = opt.rho();
                    calculated["vega"]   = opt.vega();

                    // recalculate greeks numerically
                    std::vector<Time> datesP(2), datesM(2);
                    std::transform(dates.begin(),dates.end(),datesP.begin(),
                                   std::bind2nd(std::plus<Time>(),dT));
                    std::transform(dates.begin(),dates.end(),datesM.begin(),
                                   std::bind2nd(std::minus<Time>(),dT));
                    DividendEuropeanOption
                        optPs(type, u+du, k, q, r,    T ,   v   , div, dates),
                        optMs(type, u-du, k, q, r,    T ,   v   , div, dates),
                        optPt(type, u   , k, q, r,    T+dT, v   , div, datesP),
                        optMt(type, u   , k, q, r,    T-dT, v   , div, datesM),
                        optPr(type, u   , k, q, r+dr, T   , v   , div, dates),
                        optMr(type, u   , k, q, r-dr, T   , v   , div, dates),
                        optPv(type, u   , k, q, r   , T   , v+dv, div, dates),
                        optMv(type, u   , k, q, r   , T   , v-dv, div, dates);

                    expected["delta"]  =  (optPs.value()-optMs.value())/(2*du);
                    expected["gamma"]  =  (optPs.delta()-optMs.delta())/(2*du);
                    expected["theta"]  = -(optPt.value()-optMt.value())/(2*dT);
                    expected["rho"]    =  (optPr.value()-optMr.value())/(2*dr);
                    expected["vega"]   =  (optPv.value()-optMv.value())/(2*dv);

                    // check
                    std::map<std::string,double>::iterator it;
                    for (it = expected.begin(); it != expected.end(); ++it) {
                      std::string greek = it->first;
                      double expct = expected[greek];
                      double calcl = calculated[greek];
                      double tol = tolerance[greek];
                      if (relativeError(expct,calcl,u) > tol)
                          BOOST_FAIL(
                              "Option details: \n"
                              "    type:           " +
                              OptionTypeFormatter::toString(type) + "\n"
                              "    underlying:     " +
                              DoubleFormatter::toString(u) + "\n"
                              "    strike:         " +
                              DoubleFormatter::toString(k) + "\n"
                              "    dividend yield: " +
                              RateFormatter::toString(q) + "\n"
                              "    risk-free rate: " +
                              RateFormatter::toString(r) + "\n"
                              "    residual time:  " +
                              DoubleFormatter::toString(T) + "\n"
                              "    volatility:     " +
                              RateFormatter::toString(v) + "\n\n"
                              "    calculated " + greek + ": " +
                              DoubleFormatter::toString(calcl) + "\n"
                              "    expected:    " +
                              std::string(greek.size(),' ') +
                              DoubleFormatter::toString(expct));
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
}

void OldPricerTest::testFdEuropeanPricer() {

    BOOST_MESSAGE("Testing old-style finite-difference European pricer...");

    double under = 100.0;
    double strikeMin = 60.0, strikeRange = 100.0;
    Rate rRateMin = 0.0,  rRateRange = 0.18;
    Rate qRateMin = 0.0,  qRateRange = 0.02;
    double volMin = 0.0, volRange = 1.2;
    Time timeMin = 0.5, timeRange = 2.0;

    double tolerance = 1.0e-2;
    Size totCases = 200;

    UniformRandomGenerator rng(56789012);

    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };

    for (Size i=0; i<totCases; i++) {

        double strike = strikeMin + strikeRange * rng.next().value;
        Rate qRate = qRateMin + qRateRange * rng.next().value;
        Rate rRate = rRateMin + rRateRange * rng.next().value;
        double vol = volMin + volRange * rng.next().value;
        Time resTime = timeMin + timeRange * rng.next().value;

        for (Size j=0; j<LENGTH(types); j++) {

            double rDiscount = QL_EXP(-rRate*resTime);
            double qDiscount = QL_EXP(-qRate*resTime);
            double forward = under*qDiscount/rDiscount;
            double variance = vol*vol*resTime;
            boost::shared_ptr<StrikedTypePayoff> payoff(
                                    new PlainVanillaPayoff(types[j], strike));
            double anValue = BlackFormula(forward, rDiscount, 
                                          variance, payoff).value();
            double numValue = FdEuropean(types[j], under, strike,
                                         qRate, rRate, resTime,
                                         vol, 100, 400).value();
            if (QL_FABS(anValue-numValue) > tolerance)
                BOOST_FAIL(
                    "Option details: \n"
                    "    type:           " +
                    OptionTypeFormatter::toString(types[j]) + "\n"
                    "    underlying:     " +
                    DoubleFormatter::toString(under) + "\n"
                    "    strike:         " +
                    DoubleFormatter::toString(strike) + "\n"
                    "    dividend yield: " +
                    RateFormatter::toString(qRate) + "\n"
                    "    risk-free rate: " +
                    RateFormatter::toString(rRate) + "\n"
                    "    residual time:  " +
                    DoubleFormatter::toString(resTime) + "\n"
                    "    volatility:     " +
                    RateFormatter::toString(vol) + "\n\n"
                    "    calculated value: " +
                    DoubleFormatter::toString(numValue) + "\n"
                    "    expected:         " +
                    DoubleFormatter::toString(anValue));
        }
    }
}

namespace {

    template<class O>
    void testStepOption(Option::Type type, double u, double k,
                        Rate q, Rate r, Time T, double v,
                        const std::string& name) {

        Size nstp = 145;
        Size ngrd = nstp+1;

        std::map<std::string,double> calculated, expected, tolerance;
        tolerance["delta"]  = 2.0e-3;
        tolerance["gamma"]  = 2.0e-3;
        tolerance["theta"]  = 2.0e-3;
        tolerance["rho"]    = 2.0e-3;
        tolerance["divRho"] = 2.0e-3;
        tolerance["vega"]   = 2.0e-3;

        double du = u*1.0e-4;
        double dv = v*1.0e-4;
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
            std::map<std::string,double>::iterator it;
            for (it = expected.begin(); it != expected.end(); ++it) {
                std::string greek = it->first;
                double expct = expected[greek];
                double calcl = calculated[greek];
                double tol = tolerance[greek];
                if (relativeError(expct,calcl,u) > tol)
                    BOOST_FAIL(
                        "Option details: \n"
                        "    type:           " +
                        name + " " + OptionTypeFormatter::toString(type)
                        + "\n"
                        "    underlying:     " +
                        DoubleFormatter::toString(u) + "\n"
                        "    strike:         " +
                        DoubleFormatter::toString(k) + "\n"
                        "    dividend yield: " +
                        RateFormatter::toString(q) + "\n"
                        "    risk-free rate: " +
                        RateFormatter::toString(r) + "\n"
                        "    residual time:  " +
                        DoubleFormatter::toString(T) + "\n"
                        "    volatility:     " +
                        RateFormatter::toString(v) + "\n\n"
                        "    calculated " + greek + ": " +
                        DoubleFormatter::toString(calcl) + "\n"
                        "    expected:    " +
                        std::string(greek.size(),' ') +
                        DoubleFormatter::toString(expct));
            }
        }
    }

}

void OldPricerTest::testAmericanPricers() {

    BOOST_MESSAGE("Testing old-style American-type pricers...");

    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Time residualTimes[] = { 1.0 };
    double strikes[] = { 50, 100, 150 };
    double volatilities[] = { 0.05, 0.5, 1.2 };

    for (Size i1=0; i1<LENGTH(types); i1++) {
      for (Size i2=0; i2<LENGTH(underlyings); i2++) {
        for (Size i3=0; i3<LENGTH(rRates); i3++) {
          for (Size i4=0; i4<LENGTH(qRates); i4++) {
            for (Size i5=0; i5<LENGTH(residualTimes); i5++) {
              for (Size i6=0; i6<LENGTH(strikes); i6++) {
                for (Size i7=0; i7<LENGTH(volatilities); i7++) {
                  // test data
                  Option::Type type = types[i1];
                  double u = underlyings[i2];
                  Rate r = rRates[i3];
                  Rate q = qRates[i4];
                  Time T = residualTimes[i5];
                  double k = strikes[i6];
                  double v = volatilities[i7];

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

namespace {

    struct Batch4Data {
        Option::Type type;
        double underlying;
        double strike;
        Rate dividendYield;
        Rate riskFreeRate;
        Time first;
        Time length;
        Size fixings;
        double volatility;
        bool controlVariate;
        double result;
    };

    typedef Batch4Data Batch5Data;
}

void OldPricerTest::testMcSingleFactorPricers() {

    BOOST_MESSAGE("Testing old-style Monte Carlo single-factor pricers...");

    long seed = 3456789;

    // cannot be too low, or one cannot compare numbers when
    // switching to a new default generator
    Size fixedSamples = 1023;
    double minimumTol = 1.0e-2;

    // "batch" 1
    //
    // data from "Implementing Derivatives Model",
    // Clewlow, Strickland, p.118-123
    Option::Type type = Option::Call;
    double underlying = 100.0, strike = 100.0;
    Rate dividendYield = 0.03, riskFreeRate = 0.06;
    Time times[] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
    double volatility = 0.2;
    double storedValue = 5.3425606635;

    std::vector<Time> timeIncrements(LENGTH(times));
    std::copy(times,times+LENGTH(times),timeIncrements.begin());
    DiscreteGeometricAPO pricer(type,underlying,strike,dividendYield,
                                riskFreeRate,timeIncrements, volatility);
    if (QL_FABS(pricer.value()-storedValue) > 1.0e-10)
        BOOST_FAIL(
            "Batch 1, case 1:\n"
            "    calculated value: "
            + DoubleFormatter::toString(pricer.value(),10) + "\n"
            "    expected:         "
            + DoubleFormatter::toString(storedValue,10));

    // "batch" 2
    //
    // data from "Option Pricing Formulas", Haug, pag.96-97
    type = Option::Put;
    underlying = 80.0;
    strike = 85.0;
    dividendYield = -0.03;
    riskFreeRate = 0.05;
    Time residualTime = 0.25;
    volatility = 0.2;

    ContinuousGeometricAPO pricer2(type,underlying,strike,dividendYield,
                                   riskFreeRate,residualTime,volatility);
    storedValue = 4.6922213122;
    if (QL_FABS(pricer2.value()-storedValue) > 1.0e-10)
        BOOST_FAIL(
            "Batch 2:\n"
            "    calculated value: "
            + DoubleFormatter::toString(pricer2.value(),10) + "\n"
            "    expected:         "
            + DoubleFormatter::toString(storedValue,10));

    // "batch" 3
    //
    // trying to approximate the continuous version with the discrete version
    type = Option::Put;
    underlying = 80.0;
    strike = 85.0;
    dividendYield = -0.03;
    riskFreeRate = 0.05;
    residualTime = 0.25;
    volatility = 0.2;
    Size timeSteps = 90000;
    storedValue = 4.6922247251;

    timeIncrements = std::vector<Time>(timeSteps);
    Time dt = residualTime/timeSteps;
    for (Size i=0; i<timeSteps; i++)
        timeIncrements[i] = (i+1)*dt;
    DiscreteGeometricAPO pricer3(type,underlying,strike,dividendYield,
                                 riskFreeRate,timeIncrements,volatility);
    if (QL_FABS(pricer3.value()-storedValue) > 1.0e-10)
        BOOST_FAIL(
            "Batch 3, case 1:\n"
            "    calculated value: "
            + DoubleFormatter::toString(pricer3.value(),10) + "\n"
            "    expected:         "
            + DoubleFormatter::toString(storedValue,10));

    // batch 4
    //
    // data from "Asian Option", Levy, 1997
    // in "Exotic Options: The State of the Art",
    // edited by Clewlow, Strickland

    Batch4Data cases4[] = {
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 2,
          0.13, true, 1.3942835683 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 4,
          0.13, true, 1.5852442983 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 8,
          0.13, true, 1.66970673 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 12,
          0.13, true, 1.6980019214 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 26,
          0.13, true, 1.7255070456 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 52,
          0.13, true, 1.7401553533 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 100,
          0.13, true, 1.7478303712 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 250,
          0.13, true, 1.7490291943 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 500,
          0.13, true, 1.7515113291 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 1000,
          0.13, true, 1.7537344885 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 2,
          0.13, true, 1.8496053697 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 4,
          0.13, true, 2.0111495205 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 8,
          0.13, true, 2.0852138818 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 12,
          0.13, true, 2.1105094397 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 26,
          0.13, true, 2.1346526695 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 52,
          0.13, true, 2.147489651 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 100,
          0.13, true, 2.154728109 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 250,
          0.13, true, 2.1564276565 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 500,
          0.13, true, 2.1594238588 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 1000,
          0.13, true, 2.1595367326 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 2,
          0.13, true, 2.63315092584 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 4,
          0.13, true, 2.76723962361 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 8,
          0.13, true, 2.83124836881 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 12,
          0.13, true, 2.84290301412 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 26,
          0.13, true, 2.88179560417 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 52,
          0.13, true, 2.88447044543 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 100,
          0.13, true, 2.89985329603 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 250,
          0.13, true, 2.90047296063 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 500,
          0.13, true, 2.89813412160 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 1000,
          0.13, true, 2.89703362437 }
    };

    for (Size k=0; k<LENGTH(cases4); k++) {
        Time dt = cases4[k].length/(cases4[k].fixings-1);
        std::vector<Time> timeIncrements(cases4[k].fixings);
        for (Size i=0; i<cases4[k].fixings; i++)
            timeIncrements[i] = i*dt + cases4[k].first;

        RelinkableHandle<TermStructure> riskFreeRate(
                       makeFlatCurve(cases4[k].riskFreeRate, Actual365()));
        RelinkableHandle<TermStructure> dividendYield(
                       makeFlatCurve(cases4[k].dividendYield, Actual365()));
        RelinkableHandle<BlackVolTermStructure> volatility(
                       makeFlatVolatility(cases4[k].volatility, Actual365()));

        McDiscreteArithmeticAPO pricer(cases4[k].type,
                                       cases4[k].underlying,
                                       cases4[k].strike,
                                       dividendYield,
                                       riskFreeRate,
                                       volatility,
                                       timeIncrements,
                                       cases4[k].controlVariate,
                                       seed);
        double value = pricer.valueWithSamples(fixedSamples);
        if (QL_FABS(value-cases4[k].result) > 2.0e-2)
            BOOST_FAIL(
                "Batch 4, case " + SizeFormatter::toString(k+1) + ":\n"
                "    calculated value: "
                + DoubleFormatter::toString(value,10) + "\n"
                "    expected:         "
                + DoubleFormatter::toString(cases4[k].result,10));
        double tolerance = pricer.errorEstimate()/value;
        tolerance = QL_MIN(tolerance/2.0, minimumTol);
        value = pricer.value(tolerance);
        double accuracy = pricer.errorEstimate()/value;
        if (accuracy > tolerance)
            BOOST_FAIL(
                "Batch 4, case " + SizeFormatter::toString(k+1) + ":\n"
                "    reached accuracy: "
                + DoubleFormatter::toString(accuracy,10) + "\n"
                "    expected:         "
                + DoubleFormatter::toString(tolerance,10));
    }

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

        RelinkableHandle<TermStructure> riskFreeRate(
                       makeFlatCurve(cases5[l].riskFreeRate, Actual365()));
        RelinkableHandle<TermStructure> dividendYield(
                       makeFlatCurve(cases5[l].dividendYield, Actual365()));
        RelinkableHandle<BlackVolTermStructure> volatility(
                       makeFlatVolatility(cases5[l].volatility, Actual365()));

        McDiscreteArithmeticASO pricer(cases5[l].type,
                                       cases5[l].underlying,
                                       dividendYield,
                                       riskFreeRate,
                                       volatility,
                                       timeIncrements,
                                       cases5[l].controlVariate,
                                       seed);
        double value = pricer.valueWithSamples(fixedSamples);
        if (QL_FABS(value-cases5[l].result) > 2.0e-2)
            BOOST_FAIL(
                "Batch 5, case " + SizeFormatter::toString(l+1) + ":\n"
                "    calculated value: "
                + DoubleFormatter::toString(value,10) + "\n"
                "    expected:         "
                + DoubleFormatter::toString(cases5[l].result,10));
        double tolerance = pricer.errorEstimate()/value;
        tolerance = QL_MIN(tolerance/2.0, minimumTol);
        value = pricer.value(tolerance);
        double accuracy = pricer.errorEstimate()/value;
        if (accuracy > tolerance)
            BOOST_FAIL(
                "Batch 5, case " + SizeFormatter::toString(l+1) + ":\n"
                "    reached accuracy: "
                + DoubleFormatter::toString(accuracy,10) + "\n"
                "    expected:         "
                + DoubleFormatter::toString(tolerance,10));
    }

}

namespace {

    template <class P>
    void testMcMFPricer(const P& pricer, double storedValue,
                        double tolerance, const std::string& name) {

        // cannot be too low, or one cannot compare numbers when
        // switching to a new default generator
        Size fixedSamples = 1023;
        double minimumTol = 1.0e-2;

        double value = pricer.valueWithSamples(fixedSamples);
        if (QL_FABS(value-storedValue) > tolerance)
            BOOST_FAIL(
                name + ":\n"
                "    calculated value: "
                + DoubleFormatter::toString(value,10) + "\n"
                "    expected:         "
                + DoubleFormatter::toString(storedValue,10));

        tolerance = pricer.errorEstimate()/value;
        tolerance = QL_MIN(tolerance/2.0, minimumTol);
        value = pricer.value(tolerance);
        double accuracy = pricer.errorEstimate()/value;
        if (accuracy > tolerance)
            BOOST_FAIL(
                name + ":\n"
                "    reached accuracy: "
                + DoubleFormatter::toString(accuracy,10) + "\n"
                "    expected:         "
                + DoubleFormatter::toString(tolerance,10));

    }

}

void OldPricerTest::testMcMultiFactorPricers() {

    BOOST_MESSAGE("Testing old-style Monte Carlo multi-factor pricers...");

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

    std::vector<RelinkableHandle<BlackVolTermStructure> > volatilities(4);
    volatilities[0] = RelinkableHandle<BlackVolTermStructure>(
                                       makeFlatVolatility(0.30, Actual365()));
    volatilities[1] = RelinkableHandle<BlackVolTermStructure>(
                                       makeFlatVolatility(0.35, Actual365()));
    volatilities[2] = RelinkableHandle<BlackVolTermStructure>(
                                       makeFlatVolatility(0.25, Actual365()));
    volatilities[3] = RelinkableHandle<BlackVolTermStructure>(
                                       makeFlatVolatility(0.20, Actual365()));

    std::vector<RelinkableHandle<TermStructure> > dividendYields(4);
    dividendYields[0] = RelinkableHandle<TermStructure>(
                                            makeFlatCurve(0.01, Actual365()));
    dividendYields[1] = RelinkableHandle<TermStructure>(
                                            makeFlatCurve(0.05, Actual365()));
    dividendYields[2] = RelinkableHandle<TermStructure>(
                                            makeFlatCurve(0.04, Actual365()));
    dividendYields[3] = RelinkableHandle<TermStructure>(
                                            makeFlatCurve(0.03, Actual365()));

    RelinkableHandle<TermStructure> riskFreeRate(
                                            makeFlatCurve(0.05, Actual365()));
    Time resTime = 1.0;

    // degenerate portfolio
    Matrix perfectCorrelation(4,4,1.0);
    RelinkableHandle<BlackVolTermStructure> sameVol(
                                       makeFlatVolatility(0.30, Actual365()));
    RelinkableHandle<TermStructure> sameDividend(
                                       makeFlatCurve(0.03, Actual365()));

    std::vector<RelinkableHandle<BlackVolTermStructure> > 
        sameAssetVols(4, sameVol);
    std::vector<RelinkableHandle<TermStructure> > 
        sameAssetDividend(4, sameDividend);

    unsigned long seed = 86421;

    // McEverest
    testMcMFPricer(McEverest(dividendYields, riskFreeRate, volatilities,
                             correlation, resTime, seed),
                   0.75784944,
                   1.0e-8,
                   "McEverest");

    // McBasket
    std::vector<double> sameAssetValues(4,25.0);
    Option::Type type = Option::Call;
    double strike = 100.0;
    testMcMFPricer(McBasket(type, sameAssetValues, strike, sameAssetDividend,
                            riskFreeRate, sameAssetVols, perfectCorrelation,
                            resTime, seed),
                   12.69493382,
                   1.0e-8,
                   "McBasket");

    // McMaxBasket
    std::vector<double> assetValues(4);
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
    std::vector<double> portfolio(4);
    portfolio[0] = 0.15;
    portfolio[1] = 0.20;
    portfolio[2] = 0.35;
    portfolio[3] = 0.30;
    double fraction = 0.62;
    double roof = 0.20;
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
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testCliquetPricer));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testDividendEuropeanPricer));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testFdEuropeanPricer));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testAmericanPricers));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testMcSingleFactorPricers));
    suite->add(BOOST_TEST_CASE(&OldPricerTest::testMcMultiFactorPricers));
    return suite;
}

