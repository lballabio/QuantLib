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
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/math/incompletegamma.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/trapezoidintegral.hpp>
#include <ql/math/integrals/twodimensionalintegral.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/schemes/douglasscheme.hpp>
#include <ql/methods/finitedifferences/schemes/hundsdorferscheme.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesfwdop.hpp>
#include <ql/experimental/finitedifferences/fdmsquarerootfwdop.hpp>
#include <ql/experimental/finitedifferences/fdmhestonfwdop.hpp>

#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/distributions/non_central_chi_squared.hpp>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;

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

    BOOST_TEST_MESSAGE("Testing FDM with barrier option in Heston model...");

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

    BOOST_TEST_MESSAGE("Testing FDM with barrier option for Heston model vs "
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

    BOOST_TEST_MESSAGE("Testing FDM with American option in Heston model...");

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

    BOOST_TEST_MESSAGE("Testing FDM Heston for Ikonen and Toivanen tests...");

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

    BOOST_TEST_MESSAGE("Testing FDM Heston with Black Scholes model...");

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

    BOOST_TEST_MESSAGE("Testing FDM with European option with dividends"
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
    
    BOOST_TEST_MESSAGE("Testing FDM Heston convergence...");

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

namespace {
    Real fokkerPlanckPrice1D(const boost::shared_ptr<FdmMesher>& mesher,
                             const boost::shared_ptr<FdmLinearOpComposite>& op,
                             const boost::shared_ptr<StrikedTypePayoff>& payoff,
                             Real x0, Time maturity, Size tGrid) {

        const Array x = mesher->locations(0);
        Array p(x.size(), 0.0);

        QL_REQUIRE(x.size() > 3 && x[1] <= x0 && x[x.size()-2] >= x0,
                   "insufficient mesher");

        const Array::const_iterator upperb
            = std::upper_bound(x.begin(), x.end(), x0);
        const Array::const_iterator lowerb = upperb-1;

        if (close_enough(*upperb, x0)) {
            const Size idx = std::distance(x.begin(), upperb);
            const Real dx = (x[idx+1]-x[idx-1])/2.0;
            p[idx] = 1.0/dx;
        }
        else if (close_enough(*lowerb, x0)) {
            const Size idx = std::distance(x.begin(), lowerb);
            const Real dx = (x[idx+1]-x[idx-1])/2.0;
            p[idx] = 1.0/dx;
        } else {
            const Real dx = *upperb - *lowerb;
            const Real lowerP = (*upperb - x0)/dx;
            const Real upperP = (x0 - *lowerb)/dx;

            const Size lowerIdx = std::distance(x.begin(), lowerb);
            const Size upperIdx = std::distance(x.begin(), upperb);

            const Real lowerDx = (x[lowerIdx+1]-x[lowerIdx-1])/2.0;
            const Real upperDx = (x[upperIdx+1]-x[upperIdx-1])/2.0;

            p[lowerIdx] = lowerP/lowerDx;
            p[upperIdx] = upperP/upperDx;
        }

        DouglasScheme evolver(FdmSchemeDesc::Douglas().theta, op);
        const Time dt = maturity/tGrid;
        evolver.setStep(dt);

        for (Time t=dt; t <= maturity+20*QL_EPSILON; t+=dt) {
            evolver.step(p, t);
        }

        Array payoffTimesDensity(x.size());
        for (Size i=0; i < x.size(); ++i) {
            payoffTimesDensity[i] = payoff->operator()(std::exp(x[i]))*p[i];
        }

        CubicNaturalSpline f(x.begin(), x.end(), payoffTimesDensity.begin());
        f.enableExtrapolation();
        return GaussLobattoIntegral(1000, 1e-6)(f, x.front(), x.back());
    }
}

void FdHestonTest::testBlackScholesFokkerPlanckFwdEquation() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation for BS process...");

    SavedSettings backup;

    const DayCounter dc = ActualActual();
    const Date todaysDate = Date(28, Dec, 2012);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturityDate = todaysDate + Period(2, Years);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = 100;
    const Real x0 = std::log(s0);
    const Rate r = 0.035;
    const Rate q = 0.01;
    const Volatility v = 0.35;

    const Size xGrid = 2*100+1;
    const Size tGrid = 400;

    const Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(s0)));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));
    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<BlackVolTermStructure> vTS(flatVol(v, dc));

    const boost::shared_ptr<GeneralizedBlackScholesProcess> process(
        new GeneralizedBlackScholesProcess(spot, qTS, rTS, vTS));

    const boost::shared_ptr<PricingEngine> engine(
        new AnalyticEuropeanEngine(process));

    const boost::shared_ptr<FdmMesher> uniformMesher(
        new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
            new FdmBlackScholesMesher(xGrid, process, maturity, s0))));

    const boost::shared_ptr<FdmLinearOpComposite> uniformBSFwdOp(
        new FdmBlackScholesFwdOp(uniformMesher, process, s0, 0));

    const boost::shared_ptr<FdmMesher> concentratedMesher(
        new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
            new FdmBlackScholesMesher(xGrid, process, maturity, s0,
                                      Null<Real>(), Null<Real>(), 0.0001, 1.5,
                                      std::pair<Real, Real>(s0, 0.1)))));

    const boost::shared_ptr<FdmLinearOpComposite> concentratedBSFwdOp(
        new FdmBlackScholesFwdOp(concentratedMesher, process, s0, 0));

    const boost::shared_ptr<FdmMesher> shiftedMesher(
        new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
            new FdmBlackScholesMesher(xGrid, process, maturity, s0,
                                      Null<Real>(), Null<Real>(), 0.0001, 1.5,
                                      std::pair<Real, Real>(s0*1.1, 0.2)))));

    const boost::shared_ptr<FdmLinearOpComposite> shiftedBSFwdOp(
        new FdmBlackScholesFwdOp(shiftedMesher, process, s0, 0));

    const boost::shared_ptr<Exercise> exercise(
        new EuropeanExercise(maturityDate));
    const Real strikes[] = { 50, 80, 100, 130, 150 };

    for (Size i=0; i < LENGTH(strikes); ++i) {
        const boost::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(Option::Call, strikes[i]));

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        const Real expected = option.NPV()/rTS->discount(maturityDate);
        const Real calcUniform
            = fokkerPlanckPrice1D(uniformMesher, uniformBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real calcConcentrated
            = fokkerPlanckPrice1D(concentratedMesher, concentratedBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real calcShifted
            = fokkerPlanckPrice1D(shiftedMesher, shiftedBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real tol = 0.02;

        if (std::fabs(expected - calcUniform) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with an uniform mesher"
                       << "\n   strike:     " << strikes[i]
                       << QL_FIXED << std::setprecision(8)
                       << "\n   calculated: " << calcUniform
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
        if (std::fabs(expected - calcConcentrated) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with a concentrated mesher"
                       << "\n   strike:     " << strikes[i]
                       << QL_FIXED << std::setprecision(8)
                       << "\n   calculated: " << calcConcentrated
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
        if (std::fabs(expected - calcShifted) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with a shifted mesher"
                       << "\n   strike:     " << strikes[i]
                       << QL_FIXED << std::setprecision(8)
                       << "\n   calculated: " << calcShifted
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
    }
}


namespace {

    Real squareRootGreensFct(Real v0, Real kappa, Real theta,
                             Real sigma, Real t, Real x) {

        const Real ncp = 4*kappa*std::exp(-kappa*t)
            /((sigma*sigma)*(1-std::exp(-kappa*t)))*v0;
        const Real df  = 4*theta*kappa/(sigma*sigma);
        const Real k = sigma*sigma*(1-std::exp(-kappa*t))/(4*kappa);

        const boost::math::non_central_chi_squared_distribution<Real>
            dist(df, ncp);

        return boost::math::pdf(dist, x/k) / k;
    }

    Real stationaryProbabilityFct(Real kappa, Real theta,
                                   Real sigma, Real v) {
        const Real alpha = 2*kappa*theta/(sigma*sigma);
        const Real beta = alpha/theta;

        return std::pow(beta, alpha)*std::pow(v, alpha-1.0)
                *std::exp(-beta*v-GammaFunction().logValue(alpha));
    }

    class StationaryDistributionFct : public std::unary_function<Real,Real> {
      public:
        StationaryDistributionFct(Real kappa, Real theta, Real sigma)
        : kappa_(kappa), theta_(theta), sigma_(sigma) {}

        Real operator()(Real v) const {
            const Real alpha = 2*kappa_*theta_/(sigma_*sigma_);
            const Real beta = alpha/theta_;

            return boost::math::gamma_p(alpha, beta*v);
        }
      private:
        const Real kappa_, theta_, sigma_;
    };

    Real invStationaryDistributionFct(Real kappa, Real theta,
                                      Real sigma, Real q) {
        const Real alpha = 2*kappa*theta/(sigma*sigma);
        const Real beta = alpha/theta;

        return boost::math::gamma_p_inv(alpha, q)/beta;
    }
}

void FdHestonTest::testSquareRootZeroFlowBC() {
    BOOST_TEST_MESSAGE("Testing zero-flow BC for the square root process...");

    SavedSettings backup;

    const Real kappa = 1.0;
    const Real theta = 0.4;
    const Real sigma = 0.8;
    const Real v_0   = 0.1;
    const Time t     = 1.0;

    const Real vmin = 0.0005;
    const Real h    = 0.0001;

    const Real expected[5][5]
        = {{ 0.000548, -0.000245, -0.005657, -0.001167, -0.000024},
           {-0.000595, -0.000701, -0.003296, -0.000883, -0.000691},
           {-0.001277, -0.001320, -0.003128, -0.001399, -0.001318},
           {-0.001979, -0.002002, -0.003425, -0.002047, -0.002001},
           {-0.002715, -0.002730, -0.003920, -0.002760, -0.002730} };

    for (Size i=0; i < 5; ++i) {
        const Real v = vmin + i*0.001;
        const Real vm2 = v - 2*h;
        const Real vm1 = v - h;
        const Real v0  = v;
        const Real v1  = v + h;
        const Real v2  = v + 2*h;

        const Real pm2= squareRootGreensFct(v_0, kappa, theta, sigma, t, vm2);
        const Real pm1= squareRootGreensFct(v_0, kappa, theta, sigma, t, vm1);
        const Real p0 = squareRootGreensFct(v_0, kappa, theta, sigma, t, v0);
        const Real p1 = squareRootGreensFct(v_0, kappa, theta, sigma, t, v1);
        const Real p2 = squareRootGreensFct(v_0, kappa, theta, sigma, t, v2);

        // test derivatives
        const Real flowSym2Order = sigma*sigma*v0/(4*h)*(p1-pm1)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real flowSym4Order
            = sigma*sigma*v0/(24*h)*(-p2 + 8*p1 - 8*pm1 + pm2)
              + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd1Order = sigma*sigma*v0/(2*h)*(p1-p0)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd2Order = sigma*sigma*v0/(4*h)*(4*p1-3*p0-p2)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd3Order
            = sigma*sigma*v0/(12*h)*(-p2 + 6*p1 - 3*p0 - 2*pm1)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real tol = 0.000002;
        if (   std::fabs(expected[i][0] - flowSym2Order) > tol
            || std::fabs(expected[i][1] - flowSym4Order) > tol
            || std::fabs(expected[i][2] - fwd1Order) > tol
            || std::fabs(expected[i][3] - fwd2Order) > tol
            || std::fabs(expected[i][4] - fwd3Order) > tol ) {
            BOOST_ERROR("failed to reproduce Zero Flow BC at"
                       << "\n   v:          " << v
                       << "\n   tolerance:  " << tol);
        }
    }
}


namespace {
    boost::shared_ptr<FdmMesher> createStationaryDistributionMesher(
        Real kappa, Real theta, Real sigma, Size vGrid) {

        const Real qMin = 0.01;
        const Real qMax = 0.99;
        const Real dq = (qMax-qMin)/(vGrid-1);

        std::vector<Real> v(vGrid);
        for (Size i=0; i < vGrid; ++i) {
            v[i] = invStationaryDistributionFct(kappa, theta,
                                                sigma, qMin + i*dq);
        }

        return boost::shared_ptr<FdmMesher>(
            new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
                new Predefined1dMesher(v))));
    }
}


void FdHestonTest::testTransformedZeroFlowBC() {
    BOOST_TEST_MESSAGE("Testing zero-flow BC for transformed "
                       "Fokker-Planck forward equation...");

    SavedSettings backup;

    const Real kappa = 1.0;
    const Real theta = 0.4;
    const Real sigma = 2.0;
    const Size vGrid = 100;

    const boost::shared_ptr<FdmMesher> mesher
        = createStationaryDistributionMesher(kappa, theta, sigma, vGrid);
    const Array v = mesher->locations(0);

    Array p(vGrid);
    for (Size i=0; i < v.size(); ++i)
        p[i] =  stationaryProbabilityFct(kappa, theta, sigma, v[i]);


    const Real alpha = 1.0 - 2*kappa*theta/(sigma*sigma);
    const Array q = Pow(v, alpha)*p;

    for (Size i=0; i < vGrid/2; ++i) {
        const Real hm = v[i+1] - v[i];
        const Real hp = v[i+2] - v[i+1];

        const Real eta=1.0/(hm*(hm+hp)*hp);
        const Real a = -eta*(square<Real>()(hm+hp) - hm*hm);
        const Real b  = eta*square<Real>()(hm+hp);
        const Real c = -eta*hm*hm;

        const Real df = a*q[i] + b*q[i+1] + c*q[i+2];
        const Real flow = 0.5*sigma*sigma*v[i]*df + kappa*v[i]*q[i];

        const Real tol = 1e-6;
        if (std::fabs(flow) > tol) {
            BOOST_ERROR("failed to reproduce Zero Flow BC at"
                       << "\n v:          " << v
                       << "\n flow:       " << flow
                       << "\n tolerance:  " << tol);
        }
    }
}

namespace {
    class q_fct : public std::unary_function<Real, Real> {
      public:
        q_fct(const Array& v, const Array& p, const Real alpha)
        : v_(v), q_(Pow(v, alpha)*p), alpha_(alpha) {
            spline_ = boost::shared_ptr<CubicInterpolation>(
                new CubicNaturalSpline(v_.begin(), v_.end(), q_.begin()));
        }

        Real operator()(Real v) {
            return (*spline_)(v, true)*std::pow(v, -alpha_);
        }
      private:

        const Array v_, q_;
        const Real alpha_;
        boost::shared_ptr<CubicInterpolation> spline_;
    };
}

void FdHestonTest::testSquareRootEvolveWithStationaryDensity() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the square root process with stationary density...");

    // Documentation for this test case:
    // http://www.spanderen.de/2013/05/04/fokker-planck-equation-feller-constraint-and-boundary-conditions/
    SavedSettings backup;

    const Real kappa = 2.5;
    const Real theta = 0.2;
    const Size vGrid = 100;
    const Real eps = 1e-2;

    for (Real sigma = 0.2; sigma < 2.01; sigma+=0.1) {
        const Real vMin
            = invStationaryDistributionFct(kappa, theta, sigma, eps);
        const Real vMax
            = invStationaryDistributionFct(kappa, theta, sigma, 1-eps);

        const boost::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
                    new Uniform1dMesher(vMin, vMax, vGrid))));

        const Array v = mesher->locations(0);

        Array p(vGrid);
        for (Size i=0; i < v.size(); ++i)
            p[i] =  stationaryProbabilityFct(kappa, theta, sigma, v[i]);

        const boost::shared_ptr<FdmSquareRootFwdOp> op(
            new FdmSquareRootFwdOp(mesher, kappa, theta,
                                   sigma, 0, sigma > 0.75));

        const Array eP = p;

        const Size n = 100;
        const Time dt = 0.01;
        DouglasScheme evolver(0.5, op);
        evolver.setStep(dt);

        for (Size i=1; i <= n; ++i) {
            evolver.step(p, i*dt);
        }

        const Real expected = 1-2*eps;
        const Real alpha = 1-2*kappa*theta/(sigma*sigma);
        const Real calculated = GaussLobattoIntegral(1000000, 1e-6)(
                                        q_fct(v,p,alpha), v.front(), v.back());

        const Real tol = 0.005;
        if (std::fabs(calculated-expected) > tol) {
            BOOST_ERROR("failed to reproduce stationary probability function"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    tolerance:  " << tol);
        }
    }
}

void FdHestonTest::testSquareRootFokkerPlanckFwdEquation() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the square root process with Dirac start...");

    SavedSettings backup;

    const Real kappa = 1.2;
    const Real theta = 0.4;
    const Real sigma = 0.7;
    const Real v0 = theta;
    const Real alpha = 1.0 - 2*kappa*theta/(sigma*sigma);

    const Time maturity = 1.0;

    const Size xGrid = 1001;
    const Size tGrid = 500;

    const Real vol = sigma*std::sqrt(theta/(2*kappa));
    const Real upperBound = theta+6*vol;
    const Real lowerBound = std::max(0.0002, theta-6*vol);

    const boost::shared_ptr<FdmMesher> mesher(
        new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
                new Uniform1dMesher(lowerBound, upperBound, xGrid))));

    const Array x(mesher->locations(0));

    const boost::shared_ptr<FdmSquareRootFwdOp> op(
        new FdmSquareRootFwdOp(mesher, kappa, theta, sigma, 0));

    const Time dt = maturity/tGrid;
    const Size n = 5;

    Array p(xGrid);
    for (Size i=0; i < p.size(); ++i) {
        p[i] = squareRootGreensFct(v0, kappa, theta,
                                   sigma, n*dt, x[i]);
    }
    Array q = Pow(x, alpha)*p;

    DouglasScheme evolver(0.5, op);
    evolver.setStep(dt);

    for (Time t=(n+1)*dt; t <= maturity+20*QL_EPSILON; t+=dt) {
        evolver.step(p, t);
        evolver.step(q, t);
    }

    const Real tol = 0.002;

    Array y(x.size());
    for (Size i=0; i < x.size(); ++i) {
        const Real expected = squareRootGreensFct(v0, kappa, theta,
                                                  sigma, maturity, x[i]);

        const Real calculated = p[i];
        if (std::fabs(expected - calculated) > tol) {
            BOOST_FAIL("failed to reproduce pdf at"
                       << QL_FIXED << std::setprecision(5)
                       << "\n   x:          " << x[i]
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
    }
}



namespace {
    Real fokkerPlanckPrice2D(const Array& p,
                       const boost::shared_ptr<FdmMesherComposite>& mesher) {

        std::vector<Real> x, y;
        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();

        x.reserve(layout->dim()[0]);
        y.reserve(layout->dim()[1]);

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
              ++iter) {
            if (!iter.coordinates()[1]) {
                x.push_back(mesher->location(iter, 0));
            }
            if (!iter.coordinates()[0]) {
                y.push_back(mesher->location(iter, 1));
            }
        }

        Matrix m(y.size(), x.size());
        std::copy(p.begin(), p.end(), m.begin());

        const Real tolerance = 1e-3;
        const Size maxEvaluations = 1000;

        return TwoDimensionalIntegral(
             boost::shared_ptr<Integrator>(
                 new TrapezoidIntegral<Default>(tolerance, maxEvaluations)),
             boost::shared_ptr<Integrator>(
                 new TrapezoidIntegral<Default>(tolerance, maxEvaluations)))(
             Bilinear().interpolate(x.begin(), x.end(), y.begin(), y.end(), m),
             std::make_pair(x.front(), y.front()),
             std::make_pair(x.back(), y.back()));
    }
}

void FdHestonTest::testHestonFokkerPlanckFwdEquation() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the Heston process...");

    SavedSettings backup;

    const DayCounter dc = ActualActual();
    const Date todaysDate = Date(28, Dec, 2012);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturityDate = todaysDate + Period(1, Years);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = 100;
    const Rate r = 0.10;
    const Rate q = 0.05;

    const Real kappa =  1.0;
    const Real theta =  0.4;
    const Real rho   = -0.9;
    const Real sigma =  0.4;
    const Real v0    =  theta;

    const Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(s0)));
    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    boost::shared_ptr<HestonProcess> process(
        new HestonProcess(rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const Size xGrid = 101;
    const Size vGrid = 501;
    const Size tGrid = 200;

    const Real vol = sigma*std::sqrt(theta/(2*kappa));
    const Real upperBound = std::max(v0+6*vol, theta+6*vol);
    const Real lowerBound = std::max(0.0025, std::min(v0-6*vol, theta-6*vol));

    const boost::shared_ptr<Fdm1dMesher> varianceMesher(
        new Uniform1dMesher(lowerBound, upperBound, vGrid));
    const boost::shared_ptr<Fdm1dMesher> equityMesher(
        new FdmBlackScholesMesher(
            xGrid,
            FdmBlackScholesMesher::processHelper(
              process->s0(), process->dividendYield(),
              process->riskFreeRate(), std::sqrt(v0)),
              maturity, s0));

    const boost::shared_ptr<FdmMesherComposite>
        mesher(new FdmMesherComposite(equityMesher, varianceMesher));

    Array p(mesher->layout()->size(), 0.0);

    const Size xIdx = xGrid/2;
    const Size vIdx
        = std::distance(varianceMesher->locations().begin(),
                        std::lower_bound(varianceMesher->locations().begin(),
                                         varianceMesher->locations().end(),
                                         v0));
    const Real dx = 0.5*(equityMesher->location(xIdx+1)
                         - equityMesher->location(xIdx-1));
    const Real dy = 0.5*(varianceMesher->location(vIdx+1)
                         - varianceMesher->location(vIdx-1));

    p[xIdx + vIdx*xGrid] = 1.0/(dx*dy);
    Array pd(p.size());

    const boost::shared_ptr<FdmLinearOpComposite> hestonFwdOp(
        new FdmHestonFwdOp(mesher, process));

    HundsdorferScheme evolver(FdmSchemeDesc::Hundsdorfer().theta,
                              FdmSchemeDesc::Hundsdorfer().mu,
                              hestonFwdOp);

    const Time dt = maturity/tGrid;
    evolver.setStep(dt);

    for (Time t=dt; t <= maturity+20*QL_EPSILON; t+=dt) {
        evolver.step(p, t);
    }

    const boost::shared_ptr<PricingEngine> engine(
        new AnalyticHestonEngine(boost::shared_ptr<HestonModel>(
            new HestonModel(boost::shared_ptr<HestonProcess>(
                new HestonProcess(rTS, qTS, spot,
                                  varianceMesher->location(vIdx),
                                  kappa, theta, sigma, rho))))));

    const boost::shared_ptr<Exercise> exercise(
        new EuropeanExercise(maturityDate));

    const Real strikes[] = { 50, 80, 100, 120, 150, 200 };

    for (Size i=0; i < LENGTH(strikes); ++i) {
        const Real strike = strikes[i];
        const boost::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(Option::Call, strike));

        const FdmLinearOpIterator endIter = mesher->layout()->end();
        for (FdmLinearOpIterator iter = mesher->layout()->begin();
            iter != endIter; ++iter) {
            const Size idx = iter.index();
            const Real s = std::exp(mesher->location(iter, 0));

            pd[idx] = payoff->operator()(s)*p[idx];
        }

        const Real calculated
            = fokkerPlanckPrice2D(pd, mesher)*rTS->discount(maturityDate);

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);
        const Real expected = option.NPV();

        const Real tol = 0.1;
        if (std::fabs(expected - calculated ) > tol) {
            BOOST_FAIL("failed to reproduce Heston prices at"
                       << "\n   strike      " << strike
                       << QL_FIXED << std::setprecision(5)
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
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

test_suite* FdHestonTest::experimental() {
    test_suite* suite = BOOST_TEST_SUITE("Finite Difference Heston tests");
    suite->add(QUANTLIB_TEST_CASE(
        &FdHestonTest::testBlackScholesFokkerPlanckFwdEquation));
    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testSquareRootZeroFlowBC));
    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testTransformedZeroFlowBC));
    suite->add(QUANTLIB_TEST_CASE(
          &FdHestonTest::testSquareRootEvolveWithStationaryDensity));
    suite->add(QUANTLIB_TEST_CASE(
        &FdHestonTest::testSquareRootFokkerPlanckFwdEquation));
    suite->add(QUANTLIB_TEST_CASE(
        &FdHestonTest::testHestonFokkerPlanckFwdEquation));

    return suite;
}
