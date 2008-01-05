/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

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

#include "utilities.hpp"
#include "hybridhestonhullwhiteprocess.hpp"

#include <ql/time/schedule.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/hybridhestonhullwhiteprocess.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/statistics/generalstatistics.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/methods/montecarlo/multipathgenerator.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>

#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>

#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/mchestonhullwhiteengine.hpp>
#include <ql/pricingengines/vanilla/analyticbsmhullwhiteengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonhullwhiteengine.hpp>

#include <boost/bind.hpp>
#include <boost/timer.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;


QL_BEGIN_TEST_LOCALS(HybridHestonHullWhiteProcessTest)

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(HybridHestonHullWhiteProcessTest)



void HybridHestonHullWhiteProcessTest::testBsmHullWhiteEngine() {
    BOOST_MESSAGE("Testing European option pricing for a BSM process"
                  " with one factor Hull-White Model...");

    SavedSettings backup;

    DayCounter dc = Actual365Fixed();

    const Date today = Date::todaysDate();
    const Date maturity = today + Period(20, Years);

    const Handle<Quote> spot(
                         boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    const Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0525));
    const Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    // FLOATING_POINT_EXCEPTION
    boost::shared_ptr<HullWhite> hullWhiteModel(
        new HullWhite(Handle<YieldTermStructure>(rTS), 0.00883, 0.00526));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                      new BlackScholesMertonProcess(spot, qTS, rTS, volTS));

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(maturity));

    Real fwd = spot->value()*qTS->discount(maturity)/rTS->discount(maturity);
    boost::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Call, fwd));

    EuropeanOption option(payoff, exercise);

    const Real tol = 1e-8;
    const Real corr[] = {-0.75, -0.25, 0.0, 0.25, 0.75 };
    const Volatility expectedVol[] = { 0.217064577, 0.243995801,
                                       0.256402830, 0.268236596, 0.290461343 };

    for (Size i=0; i < LENGTH(corr); ++i) {
        boost::shared_ptr<PricingEngine> bsmhwEngine(
                         new AnalyticBSMHullWhiteEngine(corr[i], stochProcess,
                                                        hullWhiteModel));

        option.setPricingEngine(bsmhwEngine);
        const Real npv = option.NPV();

        const Handle<BlackVolTermStructure> compVolTS(
                                        flatVol(today, expectedVol[i], dc));

        boost::shared_ptr<BlackScholesMertonProcess> bsProcess(
                    new BlackScholesMertonProcess(spot, qTS, rTS, compVolTS));
        boost::shared_ptr<PricingEngine> bsEngine(
                                       new AnalyticEuropeanEngine(bsProcess));

        EuropeanOption comp(payoff, exercise);
        comp.setPricingEngine(bsEngine);

        Volatility impliedVol =
            comp.impliedVolatility(npv, bsProcess, 1e-10, 100);

        if (std::fabs(impliedVol - expectedVol[i]) > tol) {
            BOOST_FAIL("Failed to reproduce implied volatility"
                       << "\n    calculated: " << impliedVol
                       << "\n    expected  : " << expectedVol[i]);
        }
        if (std::fabs((comp.NPV() - npv)/npv) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
        if (std::fabs(comp.delta() - option.delta()) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
        if (std::fabs((comp.gamma() - option.gamma())/npv) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
        if (std::fabs((comp.theta() - option.theta())/npv) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
        if (std::fabs((comp.vega() - option.vega())/npv) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
   }
}

void HybridHestonHullWhiteProcessTest::testCompareBsmHWandHestonHW() {
    BOOST_MESSAGE("Testing Comparing European option pricing for a BSM process"
                  " with one factor Hull-White Model...");

    SavedSettings backup;

    DayCounter dc = Actual365Fixed();

    const Date today = Date::todaysDate();

    const Handle<Quote> spot(
                         boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    std::vector<Date> dates;
    std::vector<Time> times;
    std::vector<Rate> rates, divRates;

    for (Size i=0; i <= 40; ++i) {
        dates.push_back(today+Period(i, Years));
        // FLOATING_POINT_EXCEPTION
        rates.push_back(0.01 + 0.02*std::exp(std::sin(i/4.0)));
        divRates.push_back(0.02 + 0.01*std::exp(std::sin(i/5.0)));
        times.push_back(dc.yearFraction(today, dates.back()));
    }

    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));
    const Handle<YieldTermStructure> rTS(
       boost::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
    const Handle<YieldTermStructure> qTS(
       boost::shared_ptr<YieldTermStructure>(
                                          new ZeroCurve(dates, divRates, dc)));

    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
                      new BlackScholesMertonProcess(spot, qTS, rTS, volTS));

    boost::shared_ptr<HestonProcess> hestonProcess(
                   new HestonProcess(rTS, qTS, spot,
                                     vol->value()*vol->value(), 1.0,
                                     vol->value()*vol->value(), 1e-4, 0.0));

    boost::shared_ptr<HestonModel> hestonModel(new HestonModel(hestonProcess));

    boost::shared_ptr<HullWhite> hullWhiteModel(
        new HullWhite(Handle<YieldTermStructure>(rTS), 0.01, 0.01));

    boost::shared_ptr<PricingEngine> bsmhwEngine(
             new AnalyticBSMHullWhiteEngine(0.0, bsmProcess, hullWhiteModel));

    boost::shared_ptr<PricingEngine> hestonHwEngine(
          new AnalyticHestonHullWhiteEngine(hestonModel, hullWhiteModel, 192));


    const Real tol = 1e-6;
    const Real strike[] = { 0.25, 0.5, 0.75, 0.8, 0.9,
                            1.0, 1.1, 1.2, 1.5, 2.0, 4.0 };
    const Size maturity[] = { 1, 2, 3, 5, 10, 15, 20, 25, 30 };
    const Option::Type types[] = { Option::Put, Option::Call };

    for (Size i=0; i < LENGTH(types); ++i) {
        for (Size j=0; j < LENGTH(strike); ++j) {
            for (Size l=0; l < LENGTH(maturity); ++l) {
                const Date maturityDate = today + Period(maturity[l], Years);

                boost::shared_ptr<Exercise> exercise(
                                         new EuropeanExercise(maturityDate));

                Real fwd = strike[j] * spot->value()
                    *qTS->discount(maturityDate)/rTS->discount(maturityDate);

                boost::shared_ptr<StrikedTypePayoff> payoff(new
                                          PlainVanillaPayoff(types[i], fwd));

                EuropeanOption option(payoff, exercise);

                option.setPricingEngine(bsmhwEngine);
                const Real calculated = option.NPV();

                option.setPricingEngine(hestonHwEngine);
                const Real expected = option.NPV();

                if (std::fabs(calculated-expected) > calculated*tol &&
                    std::fabs(calculated-expected) > tol) {
                    BOOST_ERROR("Failed to reproduce npvs"
                                << "\n    calculated: " << calculated
                                << "\n    expected  : " << expected
                                << "\n    strike    : " << strike[j]
                                << "\n    maturity  : " << maturity[l]
                                << "\n    type      : "
                                << ((types[i] == Option::Put)? "Put" : "Call")
                                );
                }
            }
        }
   }
}

void HybridHestonHullWhiteProcessTest::testZeroBondPricing() {
    BOOST_MESSAGE("Testing Monte-Carlo Zero Bond Pricing...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    const Date today = Date::todaysDate();

    // construct a strange yield curve to check drifts and discounting
    // of the joint stochastic process

    std::vector<Date> dates;
    std::vector<Time> times;
    std::vector<Rate> rates;

    dates.push_back(today);
    rates.push_back(0.02);
    times.push_back(0.0);
    for (Size i=120; i < 240; ++i) {
        dates.push_back(today+Period(i, Months));
        rates.push_back(0.02 + 0.02*std::exp(std::sin(i/8.0)));
        times.push_back(dc.yearFraction(today, dates.back()));
    }

    const Date maturity = dates.back() + Period(10, Years);
    dates.push_back(maturity);
    rates.push_back(0.04);
    times.push_back(dc.yearFraction(today, dates.back()));

    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));

    const Handle<YieldTermStructure> ts(
       boost::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
    const Handle<YieldTermStructure> ds(flatRate(today, 0.0, dc));

    const boost::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(ts, ds, s0, 0.02, 1.0, 0.02, 0.5, -0.8));
    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
                   new HullWhiteForwardProcess(ts, 0.05, 0.05));
    hwProcess->setForwardMeasureTime(dc.yearFraction(today, maturity));
    const boost::shared_ptr<HullWhite> hwModel(new HullWhite(ts, 0.05, 0.05));

    const boost::shared_ptr<JointStochasticProcess> jointProcess(
        new HybridHestonHullWhiteProcess(hestonProcess, hwProcess, -0.4, 3));

    TimeGrid grid(times.begin(), times.end()-1);

    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    BigNatural seed = 1234;
    rsg_type rsg = PseudoRandom::make_sequence_generator(
                              jointProcess->factors()*(grid.size()-1), seed);

    MultiPathGenerator<rsg_type> generator(jointProcess, grid, rsg, false);
    std::vector<GeneralStatistics> zeroStat(90);
    std::vector<GeneralStatistics> optionStat(90);

    const Size nrTrails = 1000;
    const Size optionTenor = 24;
    const DiscountFactor strike = 0.5;

    std::vector<DiscountFactor> tmpZero(90);
    std::vector<DiscountFactor> tmpOption(90);

    for (Size i=0; i < nrTrails; ++i) {
        const bool antithetic = (i%2)==0 ? false : true;
        sample_type path = (!antithetic) ? generator.next()
                                         : generator.antithetic();

        for (Size j=1; j < 90; ++j) {
            const Time t = grid[j];            // zero end and option maturity
            const Time T = grid[j+optionTenor];// maturity of zero bond
                                               // of option

            Array states(3);
            Array optionStates(3);
            for (Size k=0; k < jointProcess->size(); ++k) {
                states[k]       = path.value[k][j];
                optionStates[k] = path.value[k][j+optionTenor];
            }

            const DiscountFactor zeroBond
                = 1.0/jointProcess->numeraire(t, states);
            const DiscountFactor zeroOption = zeroBond
                * std::max(0.0, hwModel->discountBond(t, T, states[2])-strike);

            if (antithetic) {
                zeroStat[j].add(0.5*(tmpZero[j] + zeroBond));
                optionStat[j].add(0.5*(tmpOption[j] + zeroOption));
            }
            else {
                tmpZero[j] = zeroBond;
                tmpOption[j] = zeroOption;
            }
        }
    }

    for (Size j=1; j < 90; ++j) {
        const Time t = grid[j];
        Real calculated = zeroStat[j].mean();
        Real error = zeroStat[j].errorEstimate();
        Real expected = ts->discount(t);

        if (std::fabs(calculated - expected) > 3*error) {
            BOOST_ERROR("Failed to reproduce expected zero bond prices"
                        << "\n   t:          " << t
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
        }

        const Time T = grid[j+optionTenor];

        calculated = optionStat[j].mean();
        error = optionStat[j].errorEstimate();
        expected = hwModel->discountBondOption(Option::Call, strike, t, T);

        if (std::fabs(calculated - expected) > 3*error) {
            BOOST_ERROR("Failed to reproduce expected zero bond option prices"
                        << "\n   t:          " << t
                        << "\n   T:          " << T
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
        }
    }
}

void HybridHestonHullWhiteProcessTest::testMcVanillaPricing() {
    BOOST_MESSAGE("Testing Monte-Carlo Vanilla Option Pricing...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    const Date today = Date::todaysDate();

    // construct a strange yield curve to check drifts and discounting
    // of the joint stochastic process

    std::vector<Date> dates;
    std::vector<Time> times;
    std::vector<Rate> rates, divRates;

    for (Size i=0; i <= 40; ++i) {
        dates.push_back(today+Period(i, Years));
        // FLOATING_POINT_EXCEPTION
        rates.push_back(0.01 + 0.02*std::exp(std::sin(i/4.0)));
        divRates.push_back(0.02 + 0.01*std::exp(std::sin(i/5.0)));
        times.push_back(dc.yearFraction(today, dates.back()));
    }

    const Date maturity = today + Period(20, Years);

    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));
    const Handle<YieldTermStructure> rTS(
       boost::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
    const Handle<YieldTermStructure> qTS(
       boost::shared_ptr<YieldTermStructure>(
                                          new ZeroCurve(dates, divRates, dc)));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    const boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
              new BlackScholesMertonProcess(s0, qTS, rTS, volTS));
    const boost::shared_ptr<HestonProcess> hestonProcess(
              new HestonProcess(rTS, qTS, s0, 0.0625, 0.5, 0.0625, 1e-5, 0));
    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
              new HullWhiteForwardProcess(rTS, 0.01, 0.01));
    hwProcess->setForwardMeasureTime(dc.yearFraction(today, maturity));

    const Real tol = 0.1;
    const Real corr[] = {-0.9, -0.5, 0.0, 0.5, 0.9 };
    const Real strike[] = { 100 };

    for (Size i=0; i < LENGTH(corr); ++i) {
        for (Size j=0; j < LENGTH(strike); ++j) {
            boost::shared_ptr<HybridHestonHullWhiteProcess> jointProcess(
                new HybridHestonHullWhiteProcess(hestonProcess,
                                                 hwProcess, corr[i], 3));

            boost::shared_ptr<StrikedTypePayoff> payoff(
                               new PlainVanillaPayoff(Option::Put, strike[j]));
            boost::shared_ptr<Exercise> exercise(
                               new EuropeanExercise(maturity));

            VanillaOption optionHestonHW(payoff, exercise);
            optionHestonHW.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                    new MCHestonHullWhiteEngine<PseudoRandom>(
                                          jointProcess,
                                          5, Null<Size>(), true, true, 1,
                                          tol, Null<Size>(), 42)));

            const boost::shared_ptr<HullWhite> hwModel(
                        new HullWhite(Handle<YieldTermStructure>(rTS),
                                      hwProcess->a(), hwProcess->sigma()));

            VanillaOption optionBsmHW(payoff, exercise);
            optionBsmHW.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                           new AnalyticBSMHullWhiteEngine(corr[i], bsmProcess,
                                                          hwModel)));

            const Real calculated = optionHestonHW.NPV();
            const Real error      = optionHestonHW.errorEstimate();
            const Real expected   = optionBsmHW.NPV();

            if (   (corr[i] == 0.0 
                    && (   std::fabs(calculated - expected) > 1e-3
                        || error > 1e-3))
                || ( corr[i] != 0.0 
                     && std::fabs(calculated - expected) > 3*error)) {
                BOOST_ERROR("Failed to reproduce BSM-HW vanilla prices"
                        << "\n   corr:       " << corr[i]
                        << "\n   strike:     " << strike[j]
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
            }
        }
    }
}


void HybridHestonHullWhiteProcessTest::testMcPureHestonPricing() {
    BOOST_MESSAGE("Testing Monte-Carlo Heston Option Pricing...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    const Date today = Date::todaysDate();

    // construct a strange yield curve to check drifts and discounting
    // of the joint stochastic process

    std::vector<Date> dates;
    std::vector<Time> times;
    std::vector<Rate> rates, divRates;

    for (Size i=0; i <= 40; ++i) {
        dates.push_back(today+Period(i, Months));
        // FLOATING_POINT_EXCEPTION
        rates.push_back(0.01 + 0.02*std::exp(std::sin(i/10.0)));
        divRates.push_back(0.02 + 0.01*std::exp(std::sin(i/20.0)));
        times.push_back(dc.yearFraction(today, dates.back()));
    }

    const Date maturity = today + Period(2, Years);

    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));
    const Handle<YieldTermStructure> rTS(
       boost::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
    const Handle<YieldTermStructure> qTS(
       boost::shared_ptr<YieldTermStructure>(
                                          new ZeroCurve(dates, divRates, dc)));

    const boost::shared_ptr<HestonProcess> hestonProcess(
              new HestonProcess(rTS, qTS, s0, 0.08, 1.5, 0.0625, 0.5, -0.8));
    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
              new HullWhiteForwardProcess(rTS, 0.1, 1e-8));
    hwProcess->setForwardMeasureTime(dc.yearFraction(today, maturity));

    const Real tol = 0.1;
    const Real corr[] = { -0.45, 0.45, 0.25 };
    const Real strike[] = { 100, 75, 50, 150 };

    for (Size i=0; i < LENGTH(corr); ++i) {
        for (Size j=0; j < LENGTH(strike); ++j) {
            boost::shared_ptr<HybridHestonHullWhiteProcess> jointProcess(
                new HybridHestonHullWhiteProcess(hestonProcess,
                                                 hwProcess, corr[i], 3));

            boost::shared_ptr<StrikedTypePayoff> payoff(
                               new PlainVanillaPayoff(Option::Put, strike[j]));
            boost::shared_ptr<Exercise> exercise(
                               new EuropeanExercise(maturity));

            VanillaOption optionHestonHW(payoff, exercise);
            VanillaOption optionPureHeston(payoff, exercise);
            optionPureHeston.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                    new AnalyticHestonEngine(
                          boost::shared_ptr<HestonModel>(
                                           new HestonModel(hestonProcess)))));

            Real expected   = optionPureHeston.NPV();

            optionHestonHW.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                    new MCHestonHullWhiteEngine<PseudoRandom>(
                                          jointProcess,
                                          20, Null<Size>(), true, false, 1,
                                          tol, Null<Size>(), 42)));

            
            Real calculated = optionHestonHW.NPV();
            Real error      = optionHestonHW.errorEstimate();

            if (std::fabs(calculated - expected) > 3*error) {
                BOOST_ERROR("Failed to reproduce heston vanilla prices"
                        << "\n   corr:       " << corr[i]
                        << "\n   strike:     " << strike[j]
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
            }

            // using the Heston Hull-White pricer as a control variate
            // yields to almost the exact price within ms
            optionHestonHW.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                    new MCHestonHullWhiteEngine<PseudoRandom>(
                           jointProcess,
                           2, Null<Size>(), true, true, 1,
                           tol, Null<Size>(), 42)));
           
            calculated = optionHestonHW.NPV();
            error      = optionHestonHW.errorEstimate();

            if (   std::fabs(calculated - expected) > 1e-5
                 || error > 1e-5) {
                BOOST_ERROR("Failed to reproduce heston vanilla prices "
                            "using control variate"
                        << "\n   corr:       " << corr[i]
                        << "\n   strike:     " << strike[j]
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
            }
        }
    }
}


void HybridHestonHullWhiteProcessTest::testAnalyticHestonHullWhitePricing() {
    BOOST_MESSAGE("Testing analytic Heston Hull White Option Pricing...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    const Date today = Date::todaysDate();

    // construct a strange yield curve to check drifts and discounting
    // of the joint stochastic process

    std::vector<Date> dates;
    std::vector<Time> times;
    std::vector<Rate> rates, divRates;

    for (Size i=0; i <= 40; ++i) {
        dates.push_back(today+Period(i, Years));
        // FLOATING_POINT_EXCEPTION
        rates.push_back(0.03 + 0.01*std::exp(std::sin(i/4.0)));
        divRates.push_back(0.02 + 0.02*std::exp(std::sin(i/3.0)));
        times.push_back(dc.yearFraction(today, dates.back()));
    }

    const Date maturity = today + Period(5, Years);
    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));
    const Handle<YieldTermStructure> rTS(
       boost::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
    const Handle<YieldTermStructure> qTS(
       boost::shared_ptr<YieldTermStructure>(
                                          new ZeroCurve(dates, divRates, dc)));

    const boost::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, s0, 0.08, 1.5, 0.0625, 0.5, -0.8));
    const boost::shared_ptr<HestonModel> hestonModel(
                                            new HestonModel(hestonProcess));

    const boost::shared_ptr<HullWhiteForwardProcess> hwFwdProcess(
              new HullWhiteForwardProcess(rTS, 0.01, 0.01));
    hwFwdProcess->setForwardMeasureTime(dc.yearFraction(today, maturity));
    const boost::shared_ptr<HullWhite> hullWhiteModel(new HullWhite(
                               rTS, hwFwdProcess->a(), hwFwdProcess->sigma()));

    const Real tol = 0.3;
    const Real strike[] = { 80, 90, 100, 110, 120 };
    const Option::Type types[] = { Option::Put, Option::Call };

    for (Size i=0; i < LENGTH(types); ++i) {
        for (Size j=0; j < LENGTH(strike); ++j) {
            boost::shared_ptr<HybridHestonHullWhiteProcess> jointProcess(
                new HybridHestonHullWhiteProcess(hestonProcess,
                                                 hwFwdProcess, 0.0, 3));

            boost::shared_ptr<StrikedTypePayoff> payoff(
                               new PlainVanillaPayoff(types[i], strike[j]));
            boost::shared_ptr<Exercise> exercise(
                               new EuropeanExercise(maturity));

            VanillaOption optionHestonHW(payoff, exercise);
            optionHestonHW.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                    new MCHestonHullWhiteEngine<PseudoRandom>(
                                          jointProcess,
                                          4, Null<Size>(), true, true, 1,
                                          tol, Null<Size>(), 42)));

            VanillaOption optionPureHeston(payoff, exercise);
            optionPureHeston.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                    new AnalyticHestonHullWhiteEngine(hestonModel,
                                                      hullWhiteModel, 192)));

            Real calculated = optionHestonHW.NPV();
            Real error      = optionHestonHW.errorEstimate();
            Real expected   = optionPureHeston.NPV();

            if (std::fabs(calculated - expected) > 1e-8 || error > 1e-8) {
                BOOST_ERROR("Failed to reproduce heston vanilla prices"
                        << "\n   strike:     " << strike[j]
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
            }
        }
    }
}

void HybridHestonHullWhiteProcessTest::testCallableEquityPricing() {
    BOOST_MESSAGE("Testing the pricing of a callable equity product...");

    SavedSettings backup;

    /*
       For the definition of the example product see
       Alexander Giese, On the Pricing of Auto-Callable Equity
       Structures in the Presence of Stochastic Volatility and
       Stochastic Interest Rates .
       http://workshop.mathfinance.de/2006/papers/giese/slides.pdf
    */

    const Size maturity = 7;
    DayCounter dc = Actual365Fixed();
    const Date today = Date::todaysDate();

    Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.04));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));

    const boost::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, spot, 0.0625, 1.0, 0.0625, 1e-4, 0.0));
    // FLOATING_POINT_EXCEPTION
    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
            new HullWhiteForwardProcess(rTS, 0.00883, 0.00526));
    hwProcess->setForwardMeasureTime(
                      dc.yearFraction(today, today+Period(maturity+1, Years)));

    const boost::shared_ptr<JointStochasticProcess> jointProcess(
        new HybridHestonHullWhiteProcess(hestonProcess, hwProcess, -0.4, 3));

    Schedule schedule(today, today + Period(maturity, Years),
                      Period(1, Years), TARGET(),
                      Following, Following,
                      DateGeneration::Forward, false);

    std::vector<Time> times(maturity+1);
    std::transform(schedule.begin(), schedule.end(), times.begin(),
                   boost::bind(&Actual365Fixed::yearFraction,
                               dc, today, _1, Date(), Date()));

    for (Size i=0; i<=maturity; ++i) times[i] = i;

    TimeGrid grid(times.begin(), times.end());

    std::vector<Real> redemption(maturity);
    for (Size i=0; i < maturity; ++i) {
        redemption[i] = 1.07 + 0.03*i;
    }

    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    BigNatural seed = 42;
    rsg_type rsg = PseudoRandom::make_sequence_generator(
                              jointProcess->factors()*(grid.size()-1), seed);

    MultiPathGenerator<rsg_type> generator(jointProcess, grid, rsg, false);
    GeneralStatistics stat;

    Real antitheticPayoff=0;
    const Size nrTrails = 5000;
    for (Size i=0; i < nrTrails; ++i) {
        const bool antithetic = (i%2)==0 ? false : true;

        sample_type path = antithetic ? generator.antithetic()
                                      : generator.next();

        Real payoff=0;
        for (Size j=1; j <= maturity; ++j) {
            if (path.value[0][j] > spot->value()) {
                Array states(3);
                for (Size k=0; k < 3; ++k) {
                    states[k] = path.value[k][j];
                }
                payoff = redemption[j-1]
                    / jointProcess->numeraire(grid[j], states);
                break;
            }
            else if (j == maturity) {
                Array states(3);
                for (Size k=0; k < 3; ++k) {
                    states[k] = path.value[k][j];
                }
                payoff = 1.0 / jointProcess->numeraire(grid[j], states);
            }
        }

        if (antithetic){
            stat.add(0.5*(antitheticPayoff + payoff));
        }
        else {
            antitheticPayoff = payoff;
        }
    }

    const Real expected = 0.938;
    const Real calculated = stat.mean();
    const Real error = stat.errorEstimate();

    if (std::fabs(expected - calculated) > 3*error) {
        BOOST_ERROR("Failed to reproduce auto-callable equity structure price"
                    << "\n   calculated: " << calculated
                    << "\n   error:      " << error
                    << "\n   expected:   " << expected);
    }
}

void HybridHestonHullWhiteProcessTest::testDiscretizationError() {
    BOOST_MESSAGE("Testing the Discretization Error of the "
                  "Heston Hull-White Process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    const Date today = Date::todaysDate();

    // construct a strange yield curve to check drifts and discounting
    // of the joint stochastic process

    std::vector<Date> dates;
    std::vector<Time> times;
    std::vector<Rate> rates, divRates;

    for (Size i=0; i <= 31; ++i) {
        dates.push_back(today+Period(i, Years));
        // FLOATING_POINT_EXCEPTION
        rates.push_back(0.01 + 0.02*std::exp(std::sin(double(i))));
        divRates.push_back(0.02 + 0.01*std::exp(std::sin(double(i))));
        times.push_back(dc.yearFraction(today, dates.back()));
    }

    const Date maturity = today + Period(10, Years);

    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));
    const boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));
    const Handle<YieldTermStructure> rTS(
       boost::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
    const Handle<YieldTermStructure> qTS(
       boost::shared_ptr<YieldTermStructure>(
                                          new ZeroCurve(dates, divRates, dc)));

    const boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
                          new BlackScholesMertonProcess(s0, qTS, rTS, volTS));

    const boost::shared_ptr<HestonProcess> hestonProcess(
           new HestonProcess(rTS, qTS, s0, 0.0625, 5, 0.0625, 1e-4, 0.0));

    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
              new HullWhiteForwardProcess(rTS, 0.01, 0.01));
    hwProcess->setForwardMeasureTime(30);

    const Real tol = 0.25;
    const Real corr[] = { 0.0, 0.5, -0.25 };
    const Real strike[] = { 100, 50, 125 };

    for (Size i=0; i < LENGTH(corr); ++i) {
        for (Size j=0; j < LENGTH(strike); ++j) {
            boost::shared_ptr<StrikedTypePayoff> payoff(
                               new PlainVanillaPayoff(Option::Put, strike[j]));
            boost::shared_ptr<Exercise> exercise(
                               new EuropeanExercise(maturity));

            VanillaOption optionBsmHW(payoff, exercise);
            const boost::shared_ptr<HullWhite> hwModel(new HullWhite(
                               rTS, hwProcess->a(), hwProcess->sigma()));
            optionBsmHW.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                         new AnalyticBSMHullWhiteEngine(corr[i], bsmProcess,
                                                        hwModel)));

            Real expected   = optionBsmHW.NPV();


            VanillaOption optionHestonHW(payoff, exercise);
            boost::shared_ptr<HybridHestonHullWhiteProcess> jointProcess(
                new HybridHestonHullWhiteProcess(hestonProcess,
                                                 hwProcess, corr[i], 3));
            optionHestonHW.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                    new MCHestonHullWhiteEngine<PseudoRandom>(
                                          jointProcess,
                                          20, Null<Size>(), false, true, 1,
                                          tol, Null<Size>(), 42)));

            Real calculated = optionHestonHW.NPV();
            Real error      = optionHestonHW.errorEstimate();

            if (   std::fabs(calculated - expected) > 3*error 
                && std::fabs(calculated - expected) > 1e-5){
                BOOST_ERROR("Failed to reproduce heston vanilla prices"
                        << "\n   corr:       " << corr[i]
                        << "\n   strike:     " << strike[j]
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
            }
        }
    }
}



namespace {

    // Multi vanilla option instrument allows to price sevaral
    // vanilla options using one Monte-Carlo simulation run.
    // Needed here to measure the calibration missmatch of all
    // calibration instruments within one Monte-Carlo simulation.
    // Having one Monte-Carlo simulation for each calibration instrument
    // would take far too long.

     class MultiVanillaOption : public Instrument {
      public:
        class engine;
        MultiVanillaOption(
            const std::vector<boost::shared_ptr<StrikedTypePayoff> > & payoffs,
            const std::vector<boost::shared_ptr<Exercise> > & exercises);

        void setupArguments(PricingEngine::arguments* args) const;
        void fetchResults(const PricingEngine::results* results) const;

        bool isExpired() const;
        const std::vector<Real> & NPVs() const;

        class arguments;
        class results;

      protected:
        const std::vector<boost::shared_ptr<StrikedTypePayoff> >  payoffs_;
        const std::vector<boost::shared_ptr<Exercise> >           exercises_;

        mutable std::vector<Real> value_;
        mutable std::vector<Real> errorEstimate_;
    };

    class MultiVanillaOption::arguments
        : public virtual PricingEngine::arguments {
      public:
        arguments() {}
        void validate() const;

        boost::shared_ptr<Exercise> exercise;
        std::vector<boost::shared_ptr<VanillaOption::arguments> > optionArgs;
    };

    class MultiVanillaOption::results : public virtual PricingEngine::results {
      public:
        results() {}
        void reset();

        std::vector<Real> value;
        std::vector<Real> errorEstimate;
    };

    class MultiVanillaOption::engine :
        public GenericEngine<MultiVanillaOption::arguments,
                             MultiVanillaOption::results> {};

    MultiVanillaOption::MultiVanillaOption(
         const std::vector<boost::shared_ptr<StrikedTypePayoff> > & payoffs,
         const std::vector<boost::shared_ptr<Exercise> > & exercises)
    : payoffs_(payoffs),
      exercises_(exercises) {
        QL_REQUIRE(payoffs_.size() == exercises_.size(),
                   "number of payoffs and exercises must match");
    }

    void MultiVanillaOption::setupArguments(
                                     PricingEngine::arguments* args) const {
        MultiVanillaOption::arguments* arguments =
            dynamic_cast<MultiVanillaOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->optionArgs.resize(payoffs_.size());

        for (Size i=0; i < payoffs_.size(); ++i) {
            arguments->optionArgs[i]
              = boost::shared_ptr<VanillaOption::arguments>(
                                              new VanillaOption::arguments());

            arguments->optionArgs[i]->payoff   = payoffs_[i];
            arguments->optionArgs[i]->exercise = exercises_[i];
        }
        arguments->exercise = exercises_.back();
    }

    void MultiVanillaOption::fetchResults(
                                     const PricingEngine::results* r) const {
        const MultiVanillaOption::results* results =
            dynamic_cast<const MultiVanillaOption::results*>(r);
        QL_REQUIRE(results != 0,
                   "incorrect result type return from pricing engine");

        value_.resize(results->value.size());
        errorEstimate_.resize(results->value.size());

        std::copy(results->value.begin(), results->value.end(),
                  value_.begin());
        std::copy(results->errorEstimate.begin(),
                  results->errorEstimate.end(), errorEstimate_.begin());

        NPV_ = value_[0];
    }

    bool MultiVanillaOption::isExpired() const {
        const Date evaluationDate = Settings::instance().evaluationDate();
        for (std::vector<boost::shared_ptr<Exercise> >::const_iterator
                 iter = exercises_.begin(); iter != exercises_.end(); ++iter) {
            if ((*iter)->lastDate() >= evaluationDate) {
                return false;
            }
        }
        return true;
    }

    void MultiVanillaOption::arguments::validate() const {
        for (std::vector<
                 boost::shared_ptr<VanillaOption::arguments> >::const_iterator
                 iter = optionArgs.begin(); iter != optionArgs.end(); ++iter) {
            (*iter)->validate();
        }
    }

    void MultiVanillaOption::results::reset() {
        std::fill(value.begin(), value.end(), Null<Real>());
        std::fill(errorEstimate.begin(), errorEstimate.end(), Null<Real>());
    }

    const std::vector<Real> & MultiVanillaOption::NPVs() const {
        NPV();
        Array t = Array(errorEstimate_)/Array(value_);
        return value_;
    }

    class MultiVanillaOptionEngine : public MultiVanillaOption::engine {
      public:
        MultiVanillaOptionEngine(
                           const boost::shared_ptr<PricingEngine> & engine);

        void calculate() const;
      private:
        const boost::shared_ptr<PricingEngine> engine_;
    };


    MultiVanillaOptionEngine::MultiVanillaOptionEngine(
                             const boost::shared_ptr<PricingEngine> & engine)
    : engine_(engine) {
    }

    void MultiVanillaOptionEngine::calculate() const {
        const std::vector<boost::shared_ptr<VanillaOption::arguments> > &
            optionArgs = arguments_.optionArgs;

        results_.value.resize(optionArgs.size());
        results_.errorEstimate.resize(optionArgs.size());

        for (std::vector<
                 boost::shared_ptr<VanillaOption::arguments> >::const_iterator
                 iter=optionArgs.begin(); iter !=optionArgs.end(); ++iter) {

            const boost::shared_ptr<StrikedTypePayoff> payoff =
               boost::dynamic_pointer_cast<StrikedTypePayoff>((*iter)->payoff);

            const boost::shared_ptr<Exercise> exercise = (*iter)->exercise;

            VanillaOption option(payoff, exercise);
            option.setPricingEngine(engine_);

            const Size i = iter - optionArgs.begin();

            results_.errorEstimate[i] = 0;
            results_.value[i] = option.NPV();
        }
    }

    class MultiEuropeanPathPricer : public PathPricer<MultiPath, Array> {
      public:
        MultiEuropeanPathPricer(
            const TimeGrid& timeGrid,
            const boost::shared_ptr<JointStochasticProcess> & process,
            const std::vector<boost::shared_ptr<VanillaOption::arguments> > &);

        Array operator()(const MultiPath& path) const;

      private:
        const boost::shared_ptr<JointStochasticProcess> process_;

        std::vector<Time> exerciseTimes_;
        std::vector<Size> exerciseIndices_;
        std::vector<boost::shared_ptr<Payoff> > payoffs_;
    };

    MultiEuropeanPathPricer::MultiEuropeanPathPricer(
        const TimeGrid& timeGrid,
        const boost::shared_ptr<JointStochasticProcess> & process,
        const std::vector<boost::shared_ptr<VanillaOption::arguments> > & args)
    : process_(process) {
        for (std::vector<
                 boost::shared_ptr<VanillaOption::arguments> >::const_iterator
                 iter = args.begin(); iter != args.end(); ++iter) {

            boost::shared_ptr<Exercise> exercise = (*iter)->exercise;
            QL_REQUIRE(exercise->type() == Exercise::European,
                       "only european exercise is supported");
            const Time exerciseTime = process_->time(exercise->lastDate());

            payoffs_.push_back((*iter)->payoff);
            exerciseTimes_.push_back(exerciseTime);
            exerciseIndices_.push_back(timeGrid.index(exerciseTime));
        }
    }

    Array MultiEuropeanPathPricer::operator()(const MultiPath& path) const {
        QL_REQUIRE(path.pathSize() > 0, "the path cannot be empty");

        Array retVal(payoffs_.size());
        for (Size i=0; i < payoffs_.size(); ++i) {
            const Time t = exerciseTimes_[i];
            const Size index = exerciseIndices_[i];

            Array states(path.assetNumber());
            for (Size j=0; j < states.size(); ++j) {
                states[j] = path[j][index];
            }
            const DiscountFactor df(1.0/process_->numeraire(t, states));

            retVal[i] = (*payoffs_[i])(states[0]) * df;
        }

        return retVal;
    }


    //! default Monte Carlo traits for multi-variate models
    template <class RNG = PseudoRandom>
    struct MultiVariateMultiPricer {
        typedef RNG rng_traits;
        typedef MultiPath path_type;
        typedef PathPricer<path_type, Array> path_pricer_type;
        typedef typename RNG::rsg_type rsg_type;
        typedef MultiPathGenerator<rsg_type> path_generator_type;
        enum { allowsErrorEstimate = RNG::allowsErrorEstimate };
    };
    template<class RNG,class S = Statistics>
    class MCEuropeanMultiEngine
        : public MCVanillaEngine<MultiVariateMultiPricer,RNG,
                                 GenericSequenceStatistics<S>,
                                 MultiVanillaOption> {

      public:
        typedef MCVanillaEngine<MultiVariateMultiPricer,RNG,
                                GenericSequenceStatistics<S>,
                                MultiVanillaOption> base_type;
        typedef typename base_type::path_generator_type path_generator_type;
        typedef typename base_type::path_pricer_type path_pricer_type;
        typedef typename base_type::stats_type stats_type;
        typedef typename base_type::result_type result_type;

        MCEuropeanMultiEngine(const boost::shared_ptr<JointStochasticProcess>&,
                              Size timeSteps,
                              Size timeStepsPerYear,
                              bool antitheticVariate,
                              bool controlVariate,
                              Size requiredSamples,
                              Real requiredTolerance,
                              Size maxSamples,
                              BigNatural seed);

      protected:
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_pricer_type> pathPricer() const;
    };


    template <class RNG, class S = Statistics>
    class MCMultiEuropeanHestonEngine
        : public MCEuropeanMultiEngine<RNG,S> {
      public:
        typedef MCEuropeanMultiEngine<RNG,S> base_type;
        typedef typename base_type::path_generator_type path_generator_type;
        typedef typename base_type::path_pricer_type path_pricer_type;
        typedef typename base_type::stats_type stats_type;
        typedef typename base_type::result_type result_type;

        MCMultiEuropeanHestonEngine(
                             const boost::shared_ptr<JointStochasticProcess>&,
                             Size timeSteps,
                             Size timeStepsPerYear,
                             bool antitheticVariate,
                             bool controlVariate,
                             Size requiredSamples,
                             Real requiredTolerance,
                             Size maxSamples,
                             BigNatural seed);

      protected:
        boost::shared_ptr<path_pricer_type> controlPathPricer() const;
        boost::shared_ptr<PricingEngine> controlPricingEngine() const;
        boost::shared_ptr<path_generator_type> controlPathGenerator() const;
    };



    template<class RNG,class S>
    inline MCEuropeanMultiEngine<RNG,S>::MCEuropeanMultiEngine(
        const boost::shared_ptr<JointStochasticProcess>& process,
        Size timeSteps,
        Size timeStepsPerYear,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : base_type(process, timeSteps, timeStepsPerYear,
                false,antitheticVariate,
                controlVariate, requiredSamples,
                requiredTolerance, maxSamples, seed) {}


    template <class RNG,class S>
    inline TimeGrid MCEuropeanMultiEngine<RNG,S>::timeGrid() const {

        // first get time steps from super class
        const TimeGrid tmpGrid
            = this->MCVanillaEngine<MultiVariateMultiPricer,RNG,
                                    GenericSequenceStatistics<S>,
                                    MultiVanillaOption>::timeGrid();

        // add option expiry as mandatory dates
        const std::vector<boost::shared_ptr<VanillaOption::arguments> > &
            optionArgs = this->arguments_.optionArgs;

        std::vector<Time> tmpTimes(tmpGrid.begin(), tmpGrid.end());

        for (std::vector<
                 boost::shared_ptr<VanillaOption::arguments> >::const_iterator
                 iter = optionArgs.begin(); iter != optionArgs.end(); ++iter) {
            const std::vector<Date> & dates = (*iter)->exercise->dates();
            for (std::vector<Date>::const_iterator iter = dates.begin();
                 iter != dates.end(); ++iter) {
                tmpTimes.push_back(this->process_->time(*iter));
            }
        }

        return TimeGrid(tmpTimes.begin(), tmpTimes.end());
    }

    template <class RNG,class S> inline
    boost::shared_ptr<typename MCEuropeanMultiEngine<RNG,S>::path_pricer_type>
    MCEuropeanMultiEngine<RNG,S>::pathPricer() const {
       boost::shared_ptr<JointStochasticProcess> process =
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                                                              this->process_);

       QL_REQUIRE(process, "joint stochastic process required");

       return boost::shared_ptr<path_pricer_type>(
            new MultiEuropeanPathPricer(timeGrid(), process,
                                        this->arguments_.optionArgs));
    }

    template<class RNG,class S>
    inline MCMultiEuropeanHestonEngine<RNG,S>::MCMultiEuropeanHestonEngine(
        const boost::shared_ptr<JointStochasticProcess>& process,
        Size timeSteps,
        Size timeStepsPerYear,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : base_type(process, timeSteps, timeStepsPerYear,
                antitheticVariate, controlVariate,
                requiredSamples, requiredTolerance,
                maxSamples, seed) {}

    template <class RNG, class S> inline
    boost::shared_ptr<
        typename MCMultiEuropeanHestonEngine<RNG,S>::path_pricer_type>
    MCMultiEuropeanHestonEngine<RNG,S>::controlPathPricer() const {

        const boost::shared_ptr<HybridHestonHullWhiteProcess> process =
            boost::dynamic_pointer_cast<HybridHestonHullWhiteProcess>(
                                                              this->process_);

        QL_REQUIRE(process, "Heston Hull-White stochastic process required");

        const boost::shared_ptr<HestonProcess> hestonProcess(
                                                    process->hestonProcess());

        return boost::shared_ptr<path_pricer_type>(
            new MultiEuropeanPathPricer(
                    this->timeGrid(), process,
                    this->arguments_.optionArgs));
    }

    template <class RNG, class S> inline
    boost::shared_ptr<
        typename MCMultiEuropeanHestonEngine<RNG,S>::path_generator_type>
    MCMultiEuropeanHestonEngine<RNG,S>::controlPathGenerator() const {
        const boost::shared_ptr<HybridHestonHullWhiteProcess> process =
            boost::dynamic_pointer_cast<HybridHestonHullWhiteProcess>(
                                                              this->process_);
        Size dimensions = process->factors();
        TimeGrid grid = this->timeGrid();
        typename RNG::rsg_type generator =
            RNG::make_sequence_generator(dimensions*(grid.size()-1),
                                         this->seed_);

        boost::shared_ptr<HybridHestonHullWhiteProcess> cvProcess(
            new HybridHestonHullWhiteProcess(process->hestonProcess(),
                                             process->hullWhiteProcess(),
                                             process->correlation(),
                                             dimensions,
                                             true));

        return boost::shared_ptr<path_generator_type>(
                  new path_generator_type(cvProcess, grid, generator, false));
    }

    template <class RNG, class S> inline
    boost::shared_ptr<PricingEngine>
    MCMultiEuropeanHestonEngine<RNG,S>::controlPricingEngine() const {
        const boost::shared_ptr<HybridHestonHullWhiteProcess> process =
            boost::dynamic_pointer_cast<HybridHestonHullWhiteProcess>(
                                                              this->process_);

        QL_REQUIRE(process, "Heston Hull-White stochastic process required");

        const boost::shared_ptr<HestonProcess> hestonProcess =
            process->hestonProcess();

        boost::shared_ptr<HullWhiteForwardProcess> hullWhiteProcess =
            process->hullWhiteProcess();

        boost::shared_ptr<HestonModel> hestonModel(
                                             new HestonModel(hestonProcess));

        boost::shared_ptr<HullWhite> hwModel(
                              new HullWhite(hestonProcess->riskFreeRate(),
                                            hullWhiteProcess->a(),
                                            hullWhiteProcess->sigma()));

        return boost::shared_ptr<PricingEngine>(
            new MultiVanillaOptionEngine(boost::shared_ptr<PricingEngine>(
               new AnalyticHestonHullWhiteEngine(hestonModel, hwModel, 192))));
    }

    class CorrelationAndKappaConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            Impl(Real maxKappa, Real equityShortRateCorr)
                : maxKappa_(maxKappa),
                  maxEquityShortRateCorr_(
                    std::sqrt(1-equityShortRateCorr*equityShortRateCorr)) {}
            bool test(const Array& params) const {
                const Real kappa = params[1];
                const Real corr  = params[3];

                return    (kappa < maxKappa_) 
                       && (std::fabs(corr) <= maxEquityShortRateCorr_);
            }
          private:
            const Real maxKappa_;
            const Real maxEquityShortRateCorr_;
        };
      public:
        CorrelationAndKappaConstraint(Real maxKappa, Real equityShortRateCorr)
        : Constraint(boost::shared_ptr<Constraint::Impl>(
            new CorrelationAndKappaConstraint::Impl(maxKappa, 
                                                    equityShortRateCorr))) { }
    };
}

void HybridHestonHullWhiteProcessTest::testJointCalibration() {
    BOOST_MESSAGE("Testing Joint Calibration of an Heston Equity Process "
                  "incl. Stochastic Interest Rates via a Hull-White Model...");

    SavedSettings backup;

    Calendar calendar = TARGET();
    DayCounter dc = Actual365Fixed();
    const Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.04));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    const boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
               new BlackScholesMertonProcess(spot, qTS, rTS, volTS));

    const boost::shared_ptr<HestonProcess> hestonProcess(
                          new HestonProcess(rTS, qTS, spot, 0.0825,
                                            1.0, 0.0625, 0.1, 0.0));
    const boost::shared_ptr<HestonModel> hestonModel(
                          new HestonModel(hestonProcess));

    // FLOATING_POINT_EXCEPTION
    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
                          new HullWhiteForwardProcess(rTS, 0.012, 0.01));
    hwProcess->setForwardMeasureTime(
                      dc.yearFraction(today, today+Period(11, Years)));

    const Real corr = 0.4;
    const CorrelationAndKappaConstraint corrConstraint(2.0, corr);

    std::vector<Period> optionMaturities;
    optionMaturities.push_back(Period(1, Years));
    optionMaturities.push_back(Period(2, Years));
    optionMaturities.push_back(Period(5, Years));
    optionMaturities.push_back(Period(10, Years));

    Real strikes[] = { 50, 66.6, 80, 100, 125, 150, 200 };

    std::vector<Volatility> impliedVols;
    std::vector<Real> myNPVs;
    std::vector<RelinkableHandle<Quote> > hestonVols;
    std::vector<boost::shared_ptr<Exercise> > exercises;
    std::vector<boost::shared_ptr<StrikedTypePayoff> > payoffs;
    std::vector<boost::shared_ptr<EuropeanOption> > bsmOptions;
    std::vector<boost::shared_ptr<CalibrationHelper> > hestonOptions;

    boost::shared_ptr<HullWhite> hullWhiteModel(
        new HullWhite(Handle<YieldTermStructure>(rTS),
                      hwProcess->a(), hwProcess->sigma()));

    boost::shared_ptr<PricingEngine> bsmhwEngine(
            new AnalyticBSMHullWhiteEngine(corr, bsmProcess, hullWhiteModel));

    boost::shared_ptr<PricingEngine> bsmEngine(
                                      new AnalyticEuropeanEngine(bsmProcess));
    for (Size i=0; i < optionMaturities.size(); ++i) {
        for (Size j=0; j < LENGTH(strikes); ++j) {
            const Date maturityDate
                = calendar.advance(today, optionMaturities[i]);
            const Time t = dc.yearFraction(today, maturityDate);
            const Real fwd = spot->value()/rTS->discount(t)*qTS->discount(t);

            const Real gf = std::exp(3.5*vol->value()*std::sqrt(t));

            // use 4-sigma options only
            if (strikes[j] < fwd/gf || strikes[j] > fwd*gf) {
                continue;
            }

            // calculate market value and
            // "implied" volatility of a BSM-Hull-White option
            exercises.push_back(boost::shared_ptr<Exercise>(
                                        new EuropeanExercise(maturityDate)));

            payoffs.push_back(boost::shared_ptr<StrikedTypePayoff>(
                          new PlainVanillaPayoff(Option::Call, strikes[j])));

            EuropeanOption option(payoffs.back(), exercises.back());
            option.setPricingEngine(bsmhwEngine);
            Real npv = option.NPV();
            myNPVs.push_back(npv);

            bsmOptions.push_back(boost::shared_ptr<EuropeanOption>(
                new EuropeanOption(payoffs.back(),exercises.back())));
            bsmOptions.back()->setPricingEngine(bsmEngine);

            impliedVols.push_back(
                bsmOptions.back()->impliedVolatility(npv, bsmProcess,
                                                     1e-10, 100));

            hestonVols.push_back(RelinkableHandle<Quote>(
              boost::shared_ptr<Quote>(new SimpleQuote(impliedVols.back()))));

            hestonOptions.push_back(boost::shared_ptr<CalibrationHelper>(
                  new HestonModelHelper(optionMaturities[i], calendar,
                                        spot->value(), strikes[j],
                                        hestonVols.back(), rTS, qTS, false)));
        }
    }

    // cascade joint calibration
    boost::shared_ptr<PricingEngine> engine(
        new AnalyticHestonHullWhiteEngine(hestonModel, hullWhiteModel, 192));

    for (std::vector<boost::shared_ptr<CalibrationHelper> >::const_iterator
           iter = hestonOptions.begin(); iter != hestonOptions.end(); ++iter) {
        (*iter)->setPricingEngine(engine);
    }

    Real qualityIndex = QL_MAX_REAL;
    Real targetQuality =10;
    const Size maxCascadeSteps = 10;
    for (Size i=0; i<maxCascadeSteps && qualityIndex > targetQuality; ++i) {
        // 1. Calibrate Heston Model to match
        //    current Heston Volatility surface
        if (i == 0) {
            LevenbergMarquardt lm(1e-8, 1e-8, 1e-8);
            hestonModel->calibrate(hestonOptions, lm,
                                   EndCriteria(400, 100, 
                                               1.0e-8, 1.0e-8, 1.0e-8),
                                   corrConstraint);
        }
        else {
            Simplex sm(0.05);
            hestonModel->calibrate(hestonOptions, sm,
                                   EndCriteria(400, 100, 
                                               1.0e-3, 1.0e-3, 1.0e-3),
                                   corrConstraint);
        }
        boost::shared_ptr<HestonProcess> calibratedProcess =
            hestonModel->process();

        // 2. Calculate NPVs under the full model
        boost::shared_ptr<JointStochasticProcess> jointProcess(
                        new HybridHestonHullWhiteProcess(calibratedProcess,
                                                         hwProcess, corr, 3));

        boost::shared_ptr<MCEuropeanMultiEngine<PseudoRandom, 
            GeneralStatistics> > mcHestonEngine(
                new MCMultiEuropeanHestonEngine<
                   PseudoRandom, GeneralStatistics >(
                       jointProcess, 
                       40, Null<Size>(),
                       false, true, 1, 0.05, Null<Size>(), 123));

        MultiVanillaOption mvo(payoffs, exercises);
        mvo.setPricingEngine(mcHestonEngine);

        const std::vector<Real> npvs = mvo.NPVs();

        // 3. calculate vola implied vols
        std::vector<Volatility> diffVols(npvs.size());

        Real sse = 0.0;
        for (Size i=0; i < npvs.size(); ++i) {
            try {
            diffVols[i] = bsmOptions[i]->impliedVolatility(npvs[i],
                                                           bsmProcess,
                                                           1e-10)
                         - impliedVols[i];

            sse += (npvs[i] - myNPVs[i])*(npvs[i] - myNPVs[i]);
            }
            catch (Error&) {
                diffVols[i]=0.0;
            }
        }
        qualityIndex = 100*std::sqrt(sse/hestonOptions.size());

        // 4. correct heston option vol by diffVol to correct the plain
        //    the plain vanilla calibration
        for (Size i=0; i < npvs.size(); ++i) {
            const Volatility currVol = hestonVols[i].currentLink()->value();

            hestonVols[i].linkTo(boost::shared_ptr<Quote>(
                  new SimpleQuote(std::max(currVol - diffVols[i], 0.0011))));
        }
    }

    if (qualityIndex > targetQuality) {
        BOOST_ERROR("Failed to calibrate Heston Hull-White Model\n"
                    << "Quality index: " << qualityIndex);
    }
}

test_suite* HybridHestonHullWhiteProcessTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Hybrid Heston-HullWhite tests");

    // FLOATING_POINT_EXCEPTION
    suite->add(BOOST_TEST_CASE(
        &HybridHestonHullWhiteProcessTest::testBsmHullWhiteEngine));
    // FLOATING_POINT_EXCEPTION
    suite->add(BOOST_TEST_CASE(
        &HybridHestonHullWhiteProcessTest::testCompareBsmHWandHestonHW));
    suite->add(BOOST_TEST_CASE(
        &HybridHestonHullWhiteProcessTest::testZeroBondPricing));
    // FLOATING_POINT_EXCEPTION
    suite->add(BOOST_TEST_CASE(
        &HybridHestonHullWhiteProcessTest::testMcVanillaPricing));
    // FLOATING_POINT_EXCEPTION
    suite->add(BOOST_TEST_CASE(
        &HybridHestonHullWhiteProcessTest::testMcPureHestonPricing));
    // FLOATING_POINT_EXCEPTION
    suite->add(BOOST_TEST_CASE(
      &HybridHestonHullWhiteProcessTest::testAnalyticHestonHullWhitePricing));
    suite->add(BOOST_TEST_CASE(
        &HybridHestonHullWhiteProcessTest::testCallableEquityPricing));
    // FLOATING_POINT_EXCEPTION

    //runs through but takes far too long
    //suite->add(BOOST_TEST_CASE(
    //          &HybridHestonHullWhiteProcessTest::testJointCalibration));

    suite->add(BOOST_TEST_CASE(
        &HybridHestonHullWhiteProcessTest::testDiscretizationError));

    return suite;
}

