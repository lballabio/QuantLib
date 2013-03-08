/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2008 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 Neil Firth

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

#include "basketoption.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/pricingengines/basket/stulzengine.hpp>
#include <ql/pricingengines/basket/kirkengine.hpp>
#include <ql/pricingengines/basket/mceuropeanbasketengine.hpp>
#include <ql/pricingengines/basket/mcamericanbasketengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/pricingengines/basket/fd2dblackscholesvanillaengine.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <boost/progress.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE_2(greekName, basketType, payoff, exercise, \
                         s1, s2, q1, q2, r, today, v1, v2, rho, \
                         expected, calculated, error, tolerance) \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) << " " \
        << payoff->optionType() << " option on " \
        << basketTypeToString(basketType) \
        << " with " << payoffTypeToString(payoff) << " payoff:\n" \
        << "1st underlying value: " << s1 << "\n" \
        << "2nd underlying value: " << s2 << "\n" \
        << "              strike: " << payoff->strike() << "\n" \
        << "  1st dividend yield: " << io::rate(q1) << "\n" \
        << "  2nd dividend yield: " << io::rate(q2) << "\n" \
        << "      risk-free rate: " << io::rate(r) << "\n" \
        << "      reference date: " << today << "\n" \
        << "            maturity: " << exercise->lastDate() << "\n" \
        << "1st asset volatility: " << io::volatility(v1) << "\n" \
        << "2nd asset volatility: " << io::volatility(v2) << "\n" \
        << "         correlation: " << rho << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << error << "\n" \
        << "    tolerance:        " << tolerance);

#define REPORT_FAILURE_3(greekName, basketType, payoff, exercise, \
                         s1, s2, s3, r, today, v1, v2, v3, rho, \
                         expected, calculated, error, tolerance) \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) << " " \
        << payoff->optionType() << " option on " \
        << basketTypeToString(basketType) \
        << " with " << payoffTypeToString(payoff) << " payoff:\n" \
        << "1st underlying value: " << s1 << "\n" \
        << "2nd underlying value: " << s2 << "\n" \
        << "3rd underlying value: " << s3 << "\n" \
        << "              strike: " << payoff->strike() <<"\n" \
        << "      risk-free rate: " << io::rate(r) << "\n" \
        << "      reference date: " << today << "\n" \
        << "            maturity: " << exercise->lastDate() << "\n" \
        << "1st asset volatility: " << io::volatility(v1) << "\n" \
        << "2nd asset volatility: " << io::volatility(v2) << "\n" \
        << "3rd asset volatility: " << io::volatility(v3) << "\n" \
        << "         correlation: " << rho << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << error << "\n" \
        << "    tolerance:        " << tolerance);


namespace {

    enum BasketType { MinBasket, MaxBasket, SpreadBasket };

    std::string basketTypeToString(BasketType basketType) {
        switch (basketType) {
          case MinBasket:
            return "MinBasket";
          case MaxBasket:
            return "MaxBasket";
          case SpreadBasket:
            return "Spread";
        }
        QL_FAIL("unknown basket option type");
    }

    boost::shared_ptr<BasketPayoff> basketTypeToPayoff(
                                         BasketType basketType,
                                         const boost::shared_ptr<Payoff> &p) {
        switch (basketType) {
          case MinBasket:
            return boost::shared_ptr<BasketPayoff>(new MinBasketPayoff(p));
          case MaxBasket:
            return boost::shared_ptr<BasketPayoff>(new MaxBasketPayoff(p));
          case SpreadBasket:
            return boost::shared_ptr<BasketPayoff>(new SpreadBasketPayoff(p));
        }
        QL_FAIL("unknown basket option type");
    }

    struct BasketOptionOneData {
        Option::Type type;
        Real strike;
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        Volatility v;  // volatility
        Real result;   // expected result
        Real tol;      // tolerance
    };

    struct BasketOptionTwoData {
        BasketType basketType;
        Option::Type type;
        Real strike;
        Real s1;
        Real s2;
        Rate q1;
        Rate q2;
        Rate r;
        Time t; // years
        Volatility v1;
        Volatility v2;
        Real rho;
        Real result;
        Real tol;
    };

    struct BasketOptionThreeData {
        BasketType basketType;
        Option::Type type;
        Real strike;
        Real s1;
        Real s2;
        Real s3;
        Rate r;
        Time t; // months
        Volatility v1;
        Volatility v2;
        Volatility v3;
        Real rho;
        Real euroValue;
        Real amValue;
    };

}


void BasketOptionTest::testEuroTwoValues() {

    BOOST_TEST_MESSAGE("Testing two-asset European basket options...");

    /*
        Data from:
        Excel spreadsheet www.maths.ox.ac.uk/~firth/computing/excel.shtml
        and
        "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 56-58
        European two asset max basket options
    */
    BasketOptionTwoData values[] = {
        //      basketType,   optionType, strike,    s1,    s2,   q1,   q2,    r,    t,   v1,   v2,  rho, result, tol
        // data from http://www.maths.ox.ac.uk/~firth/computing/excel.shtml
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 10.898, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70,  8.483, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50,  6.844, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  5.531, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10,  4.413, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.70, 0.00,  4.981, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.30, 0.00,  4.159, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.10, 0.00,  2.597, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.10, 0.50,  4.030, 1.0e-3},

        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 17.565, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70, 19.980, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50, 21.619, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 22.932, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10, 24.049, 1.1e-3},
        {MaxBasket, Option::Call,  100.0,  80.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 16.508, 1.0e-3},
        {MaxBasket, Option::Call,  100.0,  80.0,  80.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  8.049, 1.0e-3},
        {MaxBasket, Option::Call,  100.0,  80.0, 120.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 30.141, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 120.0, 120.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 42.889, 1.0e-3},

        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 11.369, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70, 12.856, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50, 13.890, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 14.741, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10, 15.485, 1.0e-3},

        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 0.50, 0.30, 0.30, 0.10, 11.893, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 0.25, 0.30, 0.30, 0.10,  8.881, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 2.00, 0.30, 0.30, 0.10, 19.268, 1.0e-3},

        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90,  7.339, 1.0e-3},
        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70,  5.853, 1.0e-3},
        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50,  4.818, 1.0e-3},
        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  3.967, 1.1e-3},
        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10,  3.223, 1.0e-3},

        //      basketType,   optionType, strike,    s1,    s2,   q1,   q2,    r,    t,   v1,   v2,  rho,  result, tol
        // data from "Option pricing formulas" VB code + spreadsheet
        {MinBasket, Option::Call,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  4.8177, 1.0e-4},
        {MaxBasket, Option::Call,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63, 11.6323, 1.0e-4},
        {MinBasket,  Option::Put,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  2.0376, 1.0e-4},
        {MaxBasket,  Option::Put,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  0.5731, 1.0e-4},
        {MinBasket, Option::Call,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  2.9340, 1.0e-4},
        {MinBasket,  Option::Put,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  3.5224, 1.0e-4},
        // data from "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 58
        {MaxBasket, Option::Call,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  8.0701, 1.0e-4},
        {MaxBasket,  Option::Put,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  1.2181, 1.0e-4},

        /* "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 59-60
            Kirk approx. for a european spread option on two futures*/

        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.20, -0.5, 4.7530, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.20,  0.0, 3.7970, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.20,  0.5, 2.5537, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.25, 0.20, -0.5, 5.4275, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.25, 0.20,  0.0, 4.3712, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.25, 0.20,  0.5, 3.0086, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.25, -0.5, 5.4061, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.25,  0.0, 4.3451, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.25,  0.5, 2.9723, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.20, -0.5,10.7517, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.20,  0.0, 8.7020, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.20,  0.5, 6.0257, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.25, 0.20, -0.5,12.1941, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.25, 0.20,  0.0, 9.9340, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.25, 0.20,  0.5, 7.0067, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.25, -0.5,12.1483, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.25,  0.0, 9.8780, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.25,  0.5, 6.9284, 1.0e-3}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));

    boost::shared_ptr<SimpleQuote> qRate1(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS1 = flatRate(today, qRate1, dc);
    boost::shared_ptr<SimpleQuote> qRate2(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS2 = flatRate(today, qRate2, dc);

    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    boost::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);
    boost::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);

    const Real mcRelativeErrorTolerance = 0.01;
    const Real fdRelativeErrorTolerance = 0.01;

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<PlainVanillaPayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot1 ->setValue(values[i].s1);
        spot2 ->setValue(values[i].s2);
        qRate1->setValue(values[i].q1);
        qRate2->setValue(values[i].q2);
        rRate ->setValue(values[i].r );
        vol1  ->setValue(values[i].v1);
        vol2  ->setValue(values[i].v2);

        
        boost::shared_ptr<PricingEngine> analyticEngine;
        boost::shared_ptr<GeneralizedBlackScholesProcess> p1, p2;
        switch(values[i].basketType) {
          case MaxBasket: 
          case MinBasket:
            p1 = boost::shared_ptr<GeneralizedBlackScholesProcess>(
                new BlackScholesMertonProcess(
                                        Handle<Quote>(spot1),
                                        Handle<YieldTermStructure>(qTS1),
                                        Handle<YieldTermStructure>(rTS),
                                        Handle<BlackVolTermStructure>(volTS1)));
            p2 = boost::shared_ptr<GeneralizedBlackScholesProcess>(
                new BlackScholesMertonProcess(
                                        Handle<Quote>(spot2),
                                        Handle<YieldTermStructure>(qTS2),
                                        Handle<YieldTermStructure>(rTS),
                                        Handle<BlackVolTermStructure>(volTS2)));
            analyticEngine=boost::shared_ptr<PricingEngine>(
                new StulzEngine(p1, p2, values[i].rho));
            break;
          case SpreadBasket:
              p1 = boost::shared_ptr<GeneralizedBlackScholesProcess>(
                  new BlackProcess(Handle<Quote>(spot1),
                                   Handle<YieldTermStructure>(rTS),
                                   Handle<BlackVolTermStructure>(volTS1)));
              p2 = boost::shared_ptr<GeneralizedBlackScholesProcess>(
                  new BlackProcess(Handle<Quote>(spot2),
                                   Handle<YieldTermStructure>(rTS),
                                   Handle<BlackVolTermStructure>(volTS2)));
              
              analyticEngine=boost::shared_ptr<PricingEngine>(
                  new KirkEngine(boost::dynamic_pointer_cast<BlackProcess>(p1), 
                                 boost::dynamic_pointer_cast<BlackProcess>(p2), 
                                 values[i].rho));
            break;
          default:
              QL_FAIL("unknown basket type");
        }
        
        std::vector<boost::shared_ptr<StochasticProcess1D> > procs;
        procs.push_back(p1);
        procs.push_back(p2);

        Matrix correlationMatrix(2,2, values[i].rho);
        for (Integer j=0; j < 2; j++) {
            correlationMatrix[j][j] = 1.0;
        }

        boost::shared_ptr<StochasticProcessArray> process(
                         new StochasticProcessArray(procs,correlationMatrix));

        boost::shared_ptr<PricingEngine> mcEngine =
            MakeMCEuropeanBasketEngine<PseudoRandom, Statistics>(process)
            .withStepsPerYear(1)
            .withSamples(10000)
            .withSeed(42);

        boost::shared_ptr<PricingEngine> fdEngine(
                    new Fd2dBlackScholesVanillaEngine(p1, p2, values[i].rho,
                                                      50, 50, 15));
        
        BasketOption basketOption(basketTypeToPayoff(values[i].basketType,
                                                     payoff),
                                  exercise);

        // analytic engine
        basketOption.setPricingEngine(analyticEngine);
        Real calculated = basketOption.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error > values[i].tol) {
            REPORT_FAILURE_2("value", values[i].basketType, payoff, exercise,
                             values[i].s1, values[i].s2, values[i].q1,
                             values[i].q2, values[i].r, today, values[i].v1,
                             values[i].v2, values[i].rho, values[i].result,
                             calculated, error, values[i].tol);
        }

        // fd engine
        basketOption.setPricingEngine(fdEngine);
        calculated = basketOption.NPV();
        Real relError = relativeError(calculated, expected, expected);
        if (relError > mcRelativeErrorTolerance ) {
            REPORT_FAILURE_2("FD value", values[i].basketType, payoff,
                             exercise, values[i].s1, values[i].s2,
                             values[i].q1, values[i].q2, values[i].r,
                             today, values[i].v1, values[i].v2, values[i].rho,
                             values[i].result, calculated, relError,
                             fdRelativeErrorTolerance);
        }

        // mc engine
        basketOption.setPricingEngine(mcEngine);
        calculated = basketOption.NPV();
        relError = relativeError(calculated, expected, values[i].s1);
        if (relError > mcRelativeErrorTolerance ) {
            REPORT_FAILURE_2("MC value", values[i].basketType, payoff,
                             exercise, values[i].s1, values[i].s2,
                             values[i].q1, values[i].q2, values[i].r,
                             today, values[i].v1, values[i].v2, values[i].rho,
                             values[i].result, calculated, relError,
                             mcRelativeErrorTolerance);
        }

    }
}

void BasketOptionTest::testBarraquandThreeValues() {

    BOOST_TEST_MESSAGE("Testing three-asset basket options "
                       "against Barraquand's values...");

    QL_TEST_START_TIMING

    /*
        Data from:
        "Numerical Valuation of High Dimensional American Securities"
        Barraquand, J. and Martineau, D.
        Journal of Financial and Quantitative Analysis 1995 3(30) 383-405
    */
    BasketOptionThreeData  values[] = {
        // time in months is with 30 days to the month..
        // basketType, optionType,       strike,    s1,    s2,   s3,    r,    t,   v1,   v2,  v3,  rho, euro, american,
        // Table 2
        // not using 4 month case to speed up test
/*
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 8.59, 8.59},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 3.84, 3.84},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.89, 0.89},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 12.55, 12.55},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 7.87, 7.87},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 4.26, 4.26},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 15.29, 15.29},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 10.72, 10.72},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 6.96, 6.96},
*/
/*
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 7.78, 7.78},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 3.18, 3.18},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.82, 0.82},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 10.97, 10.97},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 6.69, 6.69},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 3.70, 3.70},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 13.23, 13.23},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 9.11, 9.11},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 5.98, 5.98},
*/
/*
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 6.53, 6.53},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 2.38, 2.38},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.74, 0.74},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 8.51, 8.51},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 4.92, 4.92},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 2.97, 2.97},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 10.04, 10.04},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 6.64, 6.64},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 4.61, 4.61},
*/
        // Table 3

        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.00, 0.00},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.13, 0.23},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 2.26, 5.00},
        //{MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 0.01, 0.01},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 0.25, 0.44},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 1.55, 5.00},
        //{MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 0.03, 0.04},
        //{MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 0.31, 0.57},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 1.41, 5.00},

/*
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.00, 0.00},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.38, 0.48},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 3.00, 5.00},
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 0.07, 0.09},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 0.72, 0.93},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 2.65, 5.00},
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 0.17, 0.20},
*/
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 0.91, 1.19},
/*
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 2.63, 5.00},

        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.01, 0.01},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.84, 0.08},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 4.18, 5.00},
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 0.19, 0.19},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 1.51, 1.56},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 4.49, 5.00},
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 0.41, 0.42},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 1.87, 1.96},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 4.70, 5.20}
*/
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> spot3(new SimpleQuote(0.0));

    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);

    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    boost::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);
    boost::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);
    boost::shared_ptr<SimpleQuote> vol3(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS3 = flatVol(today, vol3, dc);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<PlainVanillaPayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today + Integer(values[i].t)*30;
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        boost::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate));

        spot1 ->setValue(values[i].s1);
        spot2 ->setValue(values[i].s2);
        spot3 ->setValue(values[i].s3);
        rRate ->setValue(values[i].r );
        vol1  ->setValue(values[i].v1);
        vol2  ->setValue(values[i].v2);
        vol3  ->setValue(values[i].v3);

        boost::shared_ptr<StochasticProcess1D> stochProcess1(new
            BlackScholesMertonProcess(Handle<Quote>(spot1),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS1)));

        boost::shared_ptr<StochasticProcess1D> stochProcess2(new
            BlackScholesMertonProcess(Handle<Quote>(spot2),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS2)));

        boost::shared_ptr<StochasticProcess1D> stochProcess3(new
            BlackScholesMertonProcess(Handle<Quote>(spot3),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS3)));

        std::vector<boost::shared_ptr<StochasticProcess1D> > procs;
        procs.push_back(stochProcess1);
        procs.push_back(stochProcess2);
        procs.push_back(stochProcess3);

        Matrix correlation(3,3, values[i].rho);
        for (Integer j=0; j < 3; j++) {
            correlation[j][j] = 1.0;
        }

        // FLOATING_POINT_EXCEPTION
        boost::shared_ptr<StochasticProcessArray> process(
                               new StochasticProcessArray(procs,correlation));

        // use a 3D sobol sequence...
        // Think long and hard before moving to more than 1 timestep....
        boost::shared_ptr<PricingEngine> mcQuasiEngine =
            MakeMCEuropeanBasketEngine<LowDiscrepancy>(process)
            .withStepsPerYear(1)
            .withSamples(8091)
            .withSeed(42);

        BasketOption euroBasketOption(basketTypeToPayoff(values[i].basketType,
                                                         payoff),
                                      exercise);
        euroBasketOption.setPricingEngine(mcQuasiEngine);

        Real expected = values[i].euroValue;
        Real calculated = euroBasketOption.NPV();
        Real relError = relativeError(calculated, expected, values[i].s1);
        Real mcRelativeErrorTolerance = 0.01;
        if (relError > mcRelativeErrorTolerance ) {
            REPORT_FAILURE_3("MC Quasi value", values[i].basketType, payoff,
                             exercise, values[i].s1, values[i].s2,
                             values[i].s3, values[i].r, today, values[i].v1,
                             values[i].v2, values[i].v3, values[i].rho,
                             values[i].euroValue, calculated, relError,
                             mcRelativeErrorTolerance);
        }


        Size requiredSamples = 1000;
        Size timeSteps = 500;
        BigNatural seed = 1;
        boost::shared_ptr<PricingEngine> mcLSMCEngine =
            MakeMCAmericanBasketEngine<>(process)
            .withSteps(timeSteps)
            .withAntitheticVariate()
            .withSamples(requiredSamples)
            .withCalibrationSamples(requiredSamples/4)
            .withSeed(seed);

        BasketOption amBasketOption(basketTypeToPayoff(values[i].basketType,
                                                       payoff),
                                    amExercise);
        amBasketOption.setPricingEngine(mcLSMCEngine);

        expected = values[i].amValue;
        calculated = amBasketOption.NPV();
        relError = relativeError(calculated, expected, values[i].s1);
        Real mcAmericanRelativeErrorTolerance = 0.01;
        if (relError > mcAmericanRelativeErrorTolerance) {
            REPORT_FAILURE_3("MC LSMC Value", values[i].basketType, payoff,
                             exercise, values[i].s1, values[i].s2,
                             values[i].s3, values[i].r, today, values[i].v1,
                             values[i].v2, values[i].v3, values[i].rho,
                             values[i].amValue, calculated, relError,
                             mcRelativeErrorTolerance);
        }

    }
}

void BasketOptionTest::testTavellaValues() {

    BOOST_TEST_MESSAGE("Testing three-asset American basket options "
                       "against Tavella's values...");

    QL_TEST_START_TIMING

    /*
        Data from:
        "Quantitative Methods in Derivatives Pricing"
        Tavella, D. A.   -   Wiley (2002)
    */
    BasketOptionThreeData  values[] = {
        // time in months is with 30 days to the month..
        // basketType, optionType,       strike,    s1,    s2,   s3,    r,    t,   v1,   v2,  v3,  rho, euroValue, american Value,
        {MaxBasket, Option::Call,  100,    100,   100, 100,  0.05, 3.00, 0.20, 0.20, 0.20, 0.0, -999, 18.082}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> spot3(new SimpleQuote(0.0));

    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.1));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);

    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    boost::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);
    boost::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);
    boost::shared_ptr<SimpleQuote> vol3(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS3 = flatVol(today, vol3, dc);

    Real mcRelativeErrorTolerance = 0.01;
    Size requiredSamples = 10000;
    Size timeSteps = 20;
    BigNatural seed = 0;


    boost::shared_ptr<PlainVanillaPayoff> payoff(new
        PlainVanillaPayoff(values[0].type, values[0].strike));

    Date exDate = today + Integer(values[0].t*360+0.5);
    boost::shared_ptr<Exercise> exercise(new AmericanExercise(today, exDate));

    spot1 ->setValue(values[0].s1);
    spot2 ->setValue(values[0].s2);
    spot3 ->setValue(values[0].s3);
    vol1  ->setValue(values[0].v1);
    vol2  ->setValue(values[0].v2);
    vol3  ->setValue(values[0].v3);

    boost::shared_ptr<StochasticProcess1D> stochProcess1(new
        BlackScholesMertonProcess(Handle<Quote>(spot1),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS1)));

    boost::shared_ptr<StochasticProcess1D> stochProcess2(new
        BlackScholesMertonProcess(Handle<Quote>(spot2),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS2)));

    boost::shared_ptr<StochasticProcess1D> stochProcess3(new
        BlackScholesMertonProcess(Handle<Quote>(spot3),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS3)));

    std::vector<boost::shared_ptr<StochasticProcess1D> > procs;
    procs.push_back(stochProcess1);
    procs.push_back(stochProcess2);
    procs.push_back(stochProcess3);

    Matrix correlation(3,3, 0.0);
    for (Integer j=0; j < 3; j++) {
        correlation[j][j] = 1.0;
    }
    correlation[1][0] = -0.25;
    correlation[0][1] = -0.25;
    correlation[2][0] = 0.25;
    correlation[0][2] = 0.25;
    correlation[2][1] = 0.3;
    correlation[1][2] = 0.3;

    boost::shared_ptr<StochasticProcessArray> process(
                               new StochasticProcessArray(procs,correlation));
    boost::shared_ptr<PricingEngine> mcLSMCEngine =
        MakeMCAmericanBasketEngine<>(process)
        .withSteps(timeSteps)
        .withAntitheticVariate()
        .withSamples(requiredSamples)
        .withCalibrationSamples(requiredSamples/4)
        .withSeed(seed);

    BasketOption basketOption(basketTypeToPayoff(values[0].basketType,
                                                 payoff),
                              exercise);
    basketOption.setPricingEngine(mcLSMCEngine);

    Real calculated = basketOption.NPV();
    Real expected = values[0].amValue;
    Real errorEstimate = basketOption.errorEstimate();
    Real relError = relativeError(calculated, expected, values[0].s1);
    if (relError > mcRelativeErrorTolerance ) {
        REPORT_FAILURE_3("MC LSMC Tavella value", values[0].basketType,
                         payoff, exercise, values[0].s1, values[0].s2,
                         values[0].s3, values[0].r, today, values[0].v1,
                         values[0].v2, values[0].v3, values[0].rho,
                         values[0].amValue, calculated, errorEstimate,
                         mcRelativeErrorTolerance);
    }
}

void BasketOptionTest::testOneDAmericanValues() {

    BOOST_TEST_MESSAGE("Testing basket American options against 1-D case...");

    QL_TEST_START_TIMING

    BasketOptionOneData values[] = {
        //        type, strike,   spot,    q,    r,    t,  vol,   value, tol
        { Option::Put, 100.00,  80.00,   0.0, 0.06,   0.5, 0.4,  21.6059, 1e-2 },
        { Option::Put, 100.00,  85.00,   0.0, 0.06,   0.5, 0.4,  18.0374, 1e-2 },
        { Option::Put, 100.00,  90.00,   0.0, 0.06,   0.5, 0.4,  14.9187, 1e-2 },
        { Option::Put, 100.00,  95.00,   0.0, 0.06,   0.5, 0.4,  12.2314, 1e-2 },
        { Option::Put, 100.00, 100.00,   0.0, 0.06,   0.5, 0.4,  9.9458, 1e-2 },
        { Option::Put, 100.00, 105.00,   0.0, 0.06,   0.5, 0.4,  8.0281, 1e-2 },
        { Option::Put, 100.00, 110.00,   0.0, 0.06,   0.5, 0.4,  6.4352, 1e-2 },
        { Option::Put, 100.00, 115.00,   0.0, 0.06,   0.5, 0.4,  5.1265, 1e-2 },
        { Option::Put, 100.00, 120.00,   0.0, 0.06,   0.5, 0.4,  4.0611, 1e-2 },

        // Longstaff Schwartz 1D example
        // use constant and three Laguerre polynomials
        // 100,000 paths and 50 timesteps per year
        { Option::Put, 40.00, 36.00,   0.0, 0.06,   1.0, 0.2,  4.478, 1e-2 },
        { Option::Put, 40.00, 36.00,   0.0, 0.06,   2.0, 0.2,  4.840, 1e-2 },
        { Option::Put, 40.00, 36.00,   0.0, 0.06,   1.0, 0.4,  7.101, 1e-2 },
        { Option::Put, 40.00, 36.00,   0.0, 0.06,   2.0, 0.4,  8.508, 1e-2 },

        { Option::Put, 40.00, 38.00,   0.0, 0.06,   1.0, 0.2,  3.250, 1e-2 },
        { Option::Put, 40.00, 38.00,   0.0, 0.06,   2.0, 0.2,  3.745, 1e-2 },
        { Option::Put, 40.00, 38.00,   0.0, 0.06,   1.0, 0.4,  6.148, 1e-2 },
        { Option::Put, 40.00, 38.00,   0.0, 0.06,   2.0, 0.4,  7.670, 1e-2 },

        { Option::Put, 40.00, 40.00,   0.0, 0.06,   1.0, 0.2,  2.314, 1e-2 },
        { Option::Put, 40.00, 40.00,   0.0, 0.06,   2.0, 0.2,  2.885, 1e-2 },
        { Option::Put, 40.00, 40.00,   0.0, 0.06,   1.0, 0.4,  5.312, 1e-2 },
        { Option::Put, 40.00, 40.00,   0.0, 0.06,   2.0, 0.4,  6.920, 1e-2 },

        { Option::Put, 40.00, 42.00,   0.0, 0.06,   1.0, 0.2,  1.617, 1e-2 },
        { Option::Put, 40.00, 42.00,   0.0, 0.06,   2.0, 0.2,  2.212, 1e-2 },
        { Option::Put, 40.00, 42.00,   0.0, 0.06,   1.0, 0.4,  4.582, 1e-2 },
        { Option::Put, 40.00, 42.00,   0.0, 0.06,   2.0, 0.4,  6.248, 1e-2 },

        { Option::Put, 40.00, 44.00,   0.0, 0.06,   1.0, 0.2,  1.110, 1e-2 },
        { Option::Put, 40.00, 44.00,   0.0, 0.06,   2.0, 0.2,  1.690, 1e-2 },
        { Option::Put, 40.00, 44.00,   0.0, 0.06,   1.0, 0.4,  3.948, 1e-2 },
        { Option::Put, 40.00, 44.00,   0.0, 0.06,   2.0, 0.4,  5.647, 1e-2 }
    };


    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));

    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);

    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    boost::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);

    Size requiredSamples = 10000;
    Size timeSteps = 52;
    BigNatural seed = 0;

    boost::shared_ptr<StochasticProcess1D> stochProcess1(new
        BlackScholesMertonProcess(Handle<Quote>(spot1),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS1)));

    std::vector<boost::shared_ptr<StochasticProcess1D> > procs;
    procs.push_back(stochProcess1);

    Matrix correlation(1, 1, 1.0);

    boost::shared_ptr<StochasticProcessArray> process(
                               new StochasticProcessArray(procs,correlation));

    boost::shared_ptr<PricingEngine> mcLSMCEngine =
        MakeMCAmericanBasketEngine<>(process)
        .withSteps(timeSteps)
        .withAntitheticVariate()
        .withSamples(requiredSamples)
        .withCalibrationSamples(requiredSamples/4)
        .withSeed(seed);

    for (Size i=0; i<LENGTH(values); i++) {
        boost::shared_ptr<PlainVanillaPayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(new AmericanExercise(today,
                                                                  exDate));

        spot1 ->setValue(values[i].s);
        vol1  ->setValue(values[i].v);
        rRate ->setValue(values[i].r);
        qRate ->setValue(values[i].q);

        BasketOption basketOption(// process,
                                  basketTypeToPayoff(MaxBasket, payoff),
                                  exercise);
        basketOption.setPricingEngine(mcLSMCEngine);

        Real calculated = basketOption.NPV();
        Real expected = values[i].result;
        // Real errorEstimate = basketOption.errorEstimate();
        Real relError = relativeError(calculated, expected, values[i].s);
        // Real error = std::fabs(calculated-expected);

        if (relError > values[i].tol) {
            BOOST_FAIL("expected value: " << values[i].result << "\n"
                       << "calculated:     " << calculated);
        }

    }
}

/* This unit test is a a regression test to check for a crash in
   monte carlo if the required sample is odd.  The crash occurred
   because the samples array size was off by one when antithetic
   paths were added.
*/
void BasketOptionTest::testOddSamples() {

    BOOST_TEST_MESSAGE("Testing antithetic engine using odd sample number...");

    QL_TEST_START_TIMING
    Size requiredSamples = 10001; // The important line
    Size timeSteps = 53;
    BasketOptionOneData values[] = {
        //        type, strike,   spot,    q,    r,    t,  vol,   value, tol
        { Option::Put, 100.00,  80.00,   0.0, 0.06,   0.5, 0.4,  21.6059, 1e-2 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));

    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);

    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    boost::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);



    BigNatural seed = 0;

    boost::shared_ptr<StochasticProcess1D> stochProcess1(new
        BlackScholesMertonProcess(Handle<Quote>(spot1),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS1)));

    std::vector<boost::shared_ptr<StochasticProcess1D> > procs;
    procs.push_back(stochProcess1);

    Matrix correlation(1, 1, 1.0);

    boost::shared_ptr<StochasticProcessArray> process(
                               new StochasticProcessArray(procs,correlation));

    boost::shared_ptr<PricingEngine> mcLSMCEngine =
        MakeMCAmericanBasketEngine<>(process)
        .withSteps(timeSteps)
        .withAntitheticVariate()
        .withSamples(requiredSamples)
        .withCalibrationSamples(requiredSamples/4)
        .withSeed(seed);

    for (Size i=0; i<LENGTH(values); i++) {
        boost::shared_ptr<PlainVanillaPayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(new AmericanExercise(today,
                                                                  exDate));

        spot1 ->setValue(values[i].s);
        vol1  ->setValue(values[i].v);
        rRate ->setValue(values[i].r);
        qRate ->setValue(values[i].q);

        BasketOption basketOption(// process,
                                  basketTypeToPayoff(MaxBasket, payoff),
                                  exercise);
        basketOption.setPricingEngine(mcLSMCEngine);

        Real calculated = basketOption.NPV();
        Real expected = values[i].result;
        // Real errorEstimate = basketOption.errorEstimate();
        Real relError = relativeError(calculated, expected, values[i].s);
        // Real error = std::fabs(calculated-expected);

        if (relError > values[i].tol) {
            BOOST_FAIL("expected value: " << values[i].result << "\n"
                       << "calculated:     " << calculated);
        }

    }
}

test_suite* BasketOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Basket option tests");
    suite->add(QUANTLIB_TEST_CASE(&BasketOptionTest::testEuroTwoValues));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(
                               &BasketOptionTest::testBarraquandThreeValues));
    suite->add(QUANTLIB_TEST_CASE(&BasketOptionTest::testTavellaValues));
    suite->add(QUANTLIB_TEST_CASE(&BasketOptionTest::testOneDAmericanValues));
    suite->add(QUANTLIB_TEST_CASE(&BasketOptionTest::testOddSamples));

    return suite;
}
