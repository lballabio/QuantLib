/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2008 Klaus Spanderen

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
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/experimental/finitedifferences/fdhestonbarrierengine.hpp>
#include <ql/experimental/finitedifferences/fdhestonvanillaengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;


void FdHestonTest::testFdmHestonBarrier() {

    BOOST_MESSAGE("Testing FDM with Barrier Option in Heston model ...");

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

    boost::shared_ptr<PricingEngine> engine(
            new FdHestonBarrierEngine(boost::shared_ptr<HestonModel>(
                              new HestonModel(hestonProcess)), 50, 400, 100));
    barrierOption.setPricingEngine(engine);

    const Real tol = 0.01;
    const Real npvExpected   =  9.1783;
    const Real deltaExpected =  0.5244;
    const Real gammaExpected = -0.0365;
    
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

    BOOST_MESSAGE("Testing FDM with American Option in Heston model ...");

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
                                      PlainVanillaPayoff(Option::Put, 100));

    VanillaOption option(payoff, exercise);
    boost::shared_ptr<PricingEngine> engine(
         new FdHestonVanillaEngine(boost::shared_ptr<HestonModel>(
                             new HestonModel(hestonProcess)), 200, 100, 50));
    option.setPricingEngine(engine);
    
    const Real tol = 0.01;
    const Real npvExpected   =  5.1138;
    const Real deltaExpected = -0.2514;
    const Real gammaExpected =  0.0161;

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


void FdHestonTest::testFdmHestonEuropeanWithDividends() {

    BOOST_MESSAGE("Testing FDM with European Option with dividends"
                  " in Heston model ...");

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
    const Real npvExpected   =  6.7506;
    const Real deltaExpected = -0.3486;
    const Real gammaExpected =  0.0232;

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
    
    BOOST_MESSAGE("Testing FDM with European Option with dividends"
                  " in Heston model ...");

    SavedSettings backup;
    
    HestonTestData values[] = {
        { 1.5   , 0.04  , 0.3   , -0.9   , 0.025 , 0.0   , 1.0 , 100 },
        { 3.0   , 0.12  , 0.04  , 0.6    , 0.01  , 0.04  , 1.0 , 100 },
        { 0.6067, 0.0707, 0.2928, -0.7571, 0.03  , 0.0   , 3.0 , 100 },
        { 2.5   , 0.06  , 0.5   , -0.1   , 0.0507, 0.0469, 0.25, 100 }
    };
    
    Size tn[] = { 50 };
    Real v0[] = { 0.01, 0.04 };
    
    const Date todaysDate(28, March, 2004); 
    Settings::instance().evaluationDate() = todaysDate;
    
    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(75.0)));

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
            
                Date exerciseDate = todaysDate + Period(values[i].T*365, Days);
                boost::shared_ptr<Exercise> exercise(
                                           new EuropeanExercise(exerciseDate));
            
                boost::shared_ptr<StrikedTypePayoff> payoff(new
                                PlainVanillaPayoff(Option::Call, values[i].K));
        
                VanillaOption option(payoff, exercise);
                boost::shared_ptr<PricingEngine> engine(
                     new FdHestonVanillaEngine(boost::shared_ptr<HestonModel>(
                         new HestonModel(hestonProcess)), tn[j], 400, 100));
                option.setPricingEngine(engine);
                
                const Real calculated = option.NPV();
                
                boost::shared_ptr<PricingEngine> analyticEngine(
                    new AnalyticHestonEngine(boost::shared_ptr<HestonModel>(
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


test_suite* FdHestonTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Finite Difference Heston tests");

    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testFdmHestonBarrier));
    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testFdmHestonAmerican));
    suite->add(QUANTLIB_TEST_CASE(
                    &FdHestonTest::testFdmHestonEuropeanWithDividends));

    suite->add(QUANTLIB_TEST_CASE(&FdHestonTest::testFdmHestonConvergence));

    return suite;
}
