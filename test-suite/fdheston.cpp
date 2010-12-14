/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2008, 2009 Klaus Spanderen

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

#include "fdheston.hpp"
#include "utilities.hpp"

#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/experimental/finitedifferences/fdhestonbarrierengine.hpp>
#include <ql/experimental/finitedifferences/fdhestonvanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdblackscholesbarrierengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {
    struct NewBarrierOptionData {
        Barrier::Type barrierType;
        Real barrier;
        Real rebate;
        Option::Type type;
        Real strike;
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        Volatility v;  // volatility
    };
}

void FdHestonTest::testFdmHestonBarrierVsBlackScholes() {

    BOOST_MESSAGE("Testing FDM with barrier option in Heston model...");

    SavedSettings backup;

    NewBarrierOptionData values[] = {
        /* The data below are from
          "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag. 72
        */
        //     barrierType, barrier, rebate,         type, strike,     s,    q,    r,    t,    v
        { Barrier::DownOut,    95.0,    3.0, Option::Call,     90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,    100, 100.0, 0.00, 0.08, 1.00, 0.30},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,     90, 100.0, 0.00, 0.08, 0.25, 0.25},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,     90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25},

        { Barrier::DownIn,     95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,    90, 100.0, 0.00, 0.08, 0.25, 0.25},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   100, 100.0, 0.00, 0.08, 0.40, 0.25},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.15},

        { Barrier::DownOut,    95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,   100, 100.0, 0.00, 0.08, 0.40, 0.35},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.15},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,   110, 100.0, 0.00, 0.00, 1.00, 0.20},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30},

        { Barrier::DownIn,     95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   110, 100.0, 0.00, 0.08, 1.00, 0.30},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30},

        { Barrier::DownOut,    95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},

        { Barrier::DownIn,     95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   110, 100.0, 0.00, 0.04, 1.00, 0.15},

        { Barrier::DownOut,    95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30},

        { Barrier::DownIn,     95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 1.00, 0.15},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30}
    };
    
    const DayCounter dc = Actual365Fixed();     
    const Date todaysDate(28, March, 2004);
    const Date exerciseDate(28, March, 2005);
    Settings::instance().evaluationDate() = todaysDate;

    Handle<Quote> spot(
            boost::shared_ptr<Quote>(new SimpleQuote(0.0)));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    boost::shared_ptr<BlackScholesMertonProcess> bsProcess(
                      new BlackScholesMertonProcess(spot, qTS, rTS, volTS));

    boost::shared_ptr<PricingEngine> analyticEngine(
                                        new AnalyticBarrierEngine(bsProcess));
    
    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = todaysDate + Integer(values[i].t*365+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        boost::dynamic_pointer_cast<SimpleQuote>(spot .currentLink())
                                                    ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<StrikedTypePayoff> payoff(new
                    PlainVanillaPayoff(values[i].type, values[i].strike));

        BarrierOption barrierOption(values[i].barrierType, values[i].barrier,
                                    values[i].rebate, payoff, exercise);

        const Real v0 = vol->value()*vol->value();
        boost::shared_ptr<HestonProcess> hestonProcess(
             new HestonProcess(rTS, qTS, spot, v0, 1.0, v0, 0.00001, 0.0));

        barrierOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdHestonBarrierEngine(boost::shared_ptr<HestonModel>(
                              new HestonModel(hestonProcess)), 200, 400, 3)));

        const Real calculatedHE = barrierOption.NPV();
    
        barrierOption.setPricingEngine(analyticEngine);
        const Real expected = barrierOption.NPV();
    
        const Real tol = 0.002;
        if (std::fabs(calculatedHE - expected)/expected > tol) {
            BOOST_ERROR("Failed to reproduce expected Heston npv"
                        << "\n    calculated: " << calculatedHE
                        << "\n    expected:   " << expected
                        << "\n    tolerance:  " << tol); 
        }
    }
}

void FdHestonTest::testFdmHestonBarrier() {

    BOOST_MESSAGE("Testing FDM with barrier option for Heston model vs "
                  "Black-Scholes model...");

    SavedSettings backup;

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    boost::shared_ptr<HestonProcess> hestonProcess(
        new HestonProcess(rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    boost::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Call, 100));

    BarrierOption barrierOption(Barrier::UpOut, 135, 0.0, payoff, exercise);

    barrierOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdHestonBarrierEngine(boost::shared_ptr<HestonModel>(
                              new HestonModel(hestonProcess)), 50, 400, 100)));

    const Real tol = 0.01;
    const Real npvExpected   =  9.1530;
    const Real deltaExpected =  0.5218;
    const Real gammaExpected = -0.0354;

    if (std::fabs(barrierOption.NPV() - npvExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected npv"
                    << "\n    calculated: " << barrierOption.NPV()
                    << "\n    expected:   " << npvExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(barrierOption.delta() - deltaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected delta"
                    << "\n    calculated: " << barrierOption.delta()
                    << "\n    expected:   " << deltaExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(barrierOption.gamma() - gammaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected gamma"
                    << "\n    calculated: " << barrierOption.gamma()
                    << "\n    expected:   " << gammaExpected
                    << "\n    tolerance:  " << tol); 
    }
}

void FdHestonTest::testFdmHestonAmerican() {

    BOOST_MESSAGE("Testing FDM with American option in Heston model...");

    SavedSettings backup;

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    boost::shared_ptr<HestonProcess> hestonProcess(
        new HestonProcess(rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<Exercise> exercise(new AmericanExercise(exerciseDate));

    boost::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Put, 100));

    VanillaOption option(payoff, exercise);
    boost::shared_ptr<PricingEngine> engine(
         new FdHestonVanillaEngine(boost::shared_ptr<HestonModel>(
                             new HestonModel(hestonProcess)), 200, 100, 50));
    option.setPricingEngine(engine);
    
    const Real tol = 0.01;
    const Real npvExpected   =  5.66032;
    const Real deltaExpected = -0.30065;
    const Real gammaExpected =  0.02202;
    
    if (std::fabs(option.NPV() - npvExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected npv"
                    << "\n    calculated: " << option.NPV()
                    << "\n    expected:   " << npvExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(option.delta() - deltaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected delta"
                    << "\n    calculated: " << option.delta()
                    << "\n    expected:   " << deltaExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(option.gamma() - gammaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected gamma"
                    << "\n    calculated: " << option.gamma()
                    << "\n    expected:   " << gammaExpected
                    << "\n    tolerance:  " << tol); 
    }
}


void FdHestonTest::testFdmHestonIkonenToivanen() {

    BOOST_MESSAGE("Testing FDM Heston for Ikonen and Toivanen tests...");

    /* check prices of american puts as given in:
       From Efficient numerical methods for pricing American options under 
       stochastic volatility, Samuli Ikonen, Jari Toivanen, 
       http://users.jyu.fi/~tene/papers/reportB12-05.pdf
    */
    SavedSettings backup;

    Handle<YieldTermStructure> rTS(flatRate(0.10, Actual360()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual360()));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(26, June, 2004);

    boost::shared_ptr<Exercise> exercise(new AmericanExercise(exerciseDate));

    boost::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Put, 10));

    VanillaOption option(payoff, exercise);

    Real strikes[]  = { 8, 9, 10, 11, 12 };
    Real expected[] = { 2.00000, 1.10763, 0.520038, 0.213681, 0.082046 };
    const Real tol = 0.001;
    
    for (Size i=0; i < LENGTH(strikes); ++i) {
        Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(strikes[i])));
        boost::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, s0, 0.0625, 5, 0.16, 0.9, 0.1));
    
        boost::shared_ptr<PricingEngine> engine(
             new FdHestonVanillaEngine(boost::shared_ptr<HestonModel>(
                                 new HestonModel(hestonProcess)), 100, 400));
        option.setPricingEngine(engine);
        
        Real calculated = option.NPV();
        if (std::fabs(calculated - expected[i]) > tol) {
            BOOST_ERROR("Failed to reproduce expected npv"
                        << "\n    strike:     " << strikes[i]
                        << "\n    calculated: " << calculated
                        << "\n    expected:   " << expected[i]
                        << "\n    tolerance:  " << tol); 
        }
    }
}

void FdHestonTest::testFdmHestonBlackScholes() {

    BOOST_MESSAGE("Testing FDM Heston with Black Scholes model...");

    SavedSettings backup;


    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(26, June, 2004);

    Handle<YieldTermStructure> rTS(flatRate(0.10, Actual360()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual360()));
    Handle<BlackVolTermStructure> volTS(
                    flatVol(rTS->referenceDate(), 0.25, rTS->dayCounter()));
    
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    boost::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Put, 10));

    VanillaOption option(payoff, exercise);

    Real strikes[]  = { 8, 9, 10, 11, 12 };
    const Real tol = 0.0001;
    
    for (Size i=0; i < LENGTH(strikes); ++i) {
        Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(strikes[i])));

        boost::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
                       new GeneralizedBlackScholesProcess(s0, qTS, rTS, volTS));

        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                        new AnalyticEuropeanEngine(bsProcess)));
        
        const Real expected = option.NPV();
        
        boost::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, s0, 0.0625, 1, 0.0625, 0.0001, 0.0));

        // Hundsdorfer scheme
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
             new FdHestonVanillaEngine(boost::shared_ptr<HestonModel>(
                                           new HestonModel(hestonProcess)), 
                                       100, 400)));
        
        Real calculated = option.NPV();
        if (std::fabs(calculated - expected) > tol) {
            BOOST_ERROR("Failed to reproduce expected npv"
                        << "\n    strike:     " << strikes[i]
                        << "\n    calculated: " << calculated
                        << "\n    expected:   " << expected
                        << "\n    tolerance:  " << tol); 
        }
        
        // Explicit scheme
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
             new FdHestonVanillaEngine(boost::shared_ptr<HestonModel>(
                                           new HestonModel(hestonProcess)), 
                                       10000, 400, 5, 0, 
                                       FdmSchemeDesc::ExplicitEuler())));
        
        calculated = option.NPV();
        if (std::fabs(calculated - expected) > tol) {
            BOOST_ERROR("Failed to reproduce expected npv"
                        << "\n    strike:     " << strikes[i]
                        << "\n    calculated: " << calculated
                        << "\n    expected:   " << expected
                        << "\n    tolerance:  " << tol); 
        }
    }
}



void FdHestonTest::testFdmHestonEuropeanWithDividends() {

    BOOST_MESSAGE("Testing FDM with European option with dividends"
                  " in Heston model...");

    SavedSettings backup;

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    boost::shared_ptr<HestonProcess> hestonProcess(
        new HestonProcess(rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<Exercise> exercise(new AmericanExercise(exerciseDate));

    boost::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Put, 100));

    const std::vector<Real> dividends(1, 5);
    const std::vector<Date> dividendDates(1, Date(28, September, 2004));

    DividendVanillaOption option(payoff, exercise, dividendDates, dividends);
    boost::shared_ptr<PricingEngine> engine(
         new FdHestonVanillaEngine(boost::shared_ptr<HestonModel>(
                             new HestonModel(hestonProcess)), 50, 100, 50));
    option.setPricingEngine(engine);
    
    const Real tol = 0.01;
    const Real gammaTol = 0.001;
    const Real npvExpected   =  7.365075;
    const Real deltaExpected = -0.396678;
    const Real gammaExpected =  0.027681;
        
    if (std::fabs(option.NPV() - npvExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected npv"
                    << "\n    calculated: " << option.NPV()
                    << "\n    expected:   " << npvExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(option.delta() - deltaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected delta"
                    << "\n    calculated: " << option.delta()
                    << "\n    expected:   " << deltaExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(option.gamma() - gammaExpected) > gammaTol) {
        BOOST_ERROR("Failed to reproduce expected gamma"
                    << "\n    calculated: " << option.gamma()
                    << "\n    expected:   " << gammaExpected
                    << "\n    tolerance:  " << tol); 
    }
}

namespace {
    struct HestonTestData {
        Real kappa;
        Real theta;
        Real sigma;
        Real rho;
        Real r;
        Real q;
        Real T;
        Real K;
    };    
}

void FdHestonTest::testFdmHestonConvergence() {

    /* convergence tests based on 
       ADI finite difference schemes for option pricing in the
       Heston model with correlation, K.J. in t'Hout and S. Foulon
    */
    
    BOOST_MESSAGE("Testing FDM Heston convergence...");

    SavedSettings backup;
    
    HestonTestData values[] = {
        { 1.5   , 0.04  , 0.3   , -0.9   , 0.025 , 0.0   , 1.0 , 100 },
        { 3.0   , 0.12  , 0.04  , 0.6    , 0.01  , 0.04  , 1.0 , 100 },
        { 0.6067, 0.0707, 0.2928, -0.7571, 0.03  , 0.0   , 3.0 , 100 },
        { 2.5   , 0.06  , 0.5   , -0.1   , 0.0507, 0.0469, 0.25, 100 }
    };

    FdmSchemeDesc schemes[] = { FdmSchemeDesc::Hundsdorfer(), 
                                FdmSchemeDesc::ModifiedCraigSneyd(),
                                FdmSchemeDesc::ModifiedHundsdorfer(), 
                                FdmSchemeDesc::CraigSneyd() };
    
    Size tn[] = { 100 };
    Real v0[] = { 0.04 };
    
    const Date todaysDate(28, March, 2004); 
    Settings::instance().evaluationDate() = todaysDate;
    
    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(75.0)));

    for (Size l=0; l < LENGTH(schemes); ++l) {
        for (Size i=0; i < LENGTH(values); ++i) {
            for (Size j=0; j < LENGTH(tn); ++j) {
                for (Size k=0; k < LENGTH(v0); ++k) {
                    Handle<YieldTermStructure> rTS(
                        flatRate(values[i].r, Actual365Fixed()));
                    Handle<YieldTermStructure> qTS(
                        flatRate(values[i].q, Actual365Fixed()));
                
                    boost::shared_ptr<HestonProcess> hestonProcess(
                        new HestonProcess(rTS, qTS, s0, 
                                          v0[k], 
                                          values[i].kappa, 
                                          values[i].theta, 
                                          values[i].sigma, 
                                          values[i].rho));
                
                    Date exerciseDate = todaysDate 
                        + Period(static_cast<Integer>(values[i].T*365), Days);
                    boost::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));
                
                    boost::shared_ptr<StrikedTypePayoff> payoff(new
                               PlainVanillaPayoff(Option::Call, values[i].K));
            
                    VanillaOption option(payoff, exercise);
                    boost::shared_ptr<PricingEngine> engine(
                         new FdHestonVanillaEngine(
                             boost::shared_ptr<HestonModel>(
                                 new HestonModel(hestonProcess)), 
                             tn[j], 400, 100, 0, 
                             schemes[l]));
                    option.setPricingEngine(engine);
                    
                    const Real calculated = option.NPV();
                    
                    boost::shared_ptr<PricingEngine> analyticEngine(
                        new AnalyticHestonEngine(
                            boost::shared_ptr<HestonModel>(
                                new HestonModel(hestonProcess)), 144));
                    
                    option.setPricingEngine(analyticEngine);
                    const Real expected = option.NPV();
                    if (   std::fabs(expected - calculated)/expected > 0.02
                        && std::fabs(expected - calculated) > 0.002) {
                        BOOST_ERROR("Failed to reproduce expected npv"
                                    << "\n    calculated: " << calculated
                                    << "\n    expected:   " << expected
                                    << "\n    tolerance:  " << 0.01); 
                    }
                }
            }
        }
    }
 }


test_suite* FdHestonTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Finite Difference Heston tests");
    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testFdmHestonBarrier));
    suite->add(QUANTLIB_TEST_CASE(
                         &FdHestonTest::testFdmHestonBarrierVsBlackScholes));
    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testFdmHestonAmerican));
    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testFdmHestonIkonenToivanen));
    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testFdmHestonBlackScholes));
    suite->add(QUANTLIB_TEST_CASE(
                    &FdHestonTest::testFdmHestonEuropeanWithDividends));

    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testFdmHestonConvergence));
    return suite;
}
