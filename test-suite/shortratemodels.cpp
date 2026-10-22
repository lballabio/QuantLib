/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2005 StatPro Italia srl
 Copyright (C) 2013 Peter Caspers

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
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/models/shortrate/onefactormodels/vasicek.hpp>
#include <ql/models/shortrate/onefactormodels/extendedcoxingersollross.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <ql/pricingengines/swap/treeswapengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ShortRateModelTests)

struct CalibrationData {
    Integer start;
    Integer length;
    Volatility volatility;
};

BOOST_AUTO_TEST_CASE(testHullWhiteUpdatesR0WhenTermStructureRelinks) {
    BOOST_TEST_MESSAGE("Testing Hull-White r0 update when the term structure is relinked...");

    Date today(19, May, 2026);
    Settings::instance().evaluationDate() = today;

    RelinkableHandle<YieldTermStructure> termStructure;
    termStructure.linkTo(flatRate(today, 0.02, Actual365Fixed()));

    HullWhite model(termStructure);

    termStructure.linkTo(flatRate(today, 0.05, Actual365Fixed()));

    const Rate expected =
        termStructure->forwardRate(0.0, 0.0, Continuous, NoFrequency);
    const Real tolerance = 1.0e-12;

    if (std::fabs(model.r0() - expected) > tolerance) {
        BOOST_ERROR("failed to update r0 after relinking the term structure:\n"
                    << "expected:   " << expected << "\n"
                    << "calculated: " << model.r0());
    }
}

BOOST_AUTO_TEST_CASE(testExtendedCoxIngersollRossUpdatesWhenTermStructureRelinks) {
    BOOST_TEST_MESSAGE(
        "Testing extended CIR updates when the term structure is relinked...");

    Date today(19, May, 2026);
    Settings::instance().evaluationDate() = today;

    RelinkableHandle<YieldTermStructure> termStructure;
    termStructure.linkTo(flatRate(today, 0.02, Actual365Fixed()));

    auto model = ext::make_shared<ExtendedCoxIngersollRoss>(
        termStructure, 0.02, 1.0, 1e-4, 0.02);

    Flag flag;
    flag.registerWith(model);

    termStructure.linkTo(flatRate(today, 0.05, Actual365Fixed()));

    if (!flag.isUp())
        BOOST_FAIL("extended CIR model was not notified of the curve relink");
}


BOOST_AUTO_TEST_CASE(testCachedHullWhite) {
    BOOST_TEST_MESSAGE("Testing Hull-White calibration against cached values using swaptions with start delay...");

    bool usingAtParCoupons  = IborCoupon::Settings::instance().usingAtParCoupons();

    Date today(15, February, 2002);
    Date settlement(19, February, 2002);
    Settings::instance().evaluationDate() = today;
    Handle<YieldTermStructure> termStructure(flatRate(settlement,0.04875825,
                                                      Actual365Fixed()));
    ext::shared_ptr<HullWhite> model(new HullWhite(termStructure));
    CalibrationData data[] = {{ 1, 5, 0.1148 },
                              { 2, 4, 0.1108 },
                              { 3, 3, 0.1070 },
                              { 4, 2, 0.1021 },
                              { 5, 1, 0.1000 }};
    ext::shared_ptr<IborIndex> index(new Euribor6M(termStructure));

    ext::shared_ptr<PricingEngine> engine(
                                     new JamshidianSwaptionEngine(model));

    std::vector<ext::shared_ptr<CalibrationHelper> > swaptions;
    for (auto& i : data) {
        ext::shared_ptr<Quote> vol(new SimpleQuote(i.volatility));
        ext::shared_ptr<BlackCalibrationHelper> helper(
            new SwaptionHelper(Period(i.start, Years), Period(i.length, Years), Handle<Quote>(vol),
                               index, Period(1, Years), Thirty360(Thirty360::BondBasis), Actual360(), termStructure));
        helper->setPricingEngine(engine);
        swaptions.push_back(helper);
    }

    // Set up the optimization problem
    // Real simplexLambda = 0.1;
    // Simplex optimizationMethod(simplexLambda);
    LevenbergMarquardt optimizationMethod(1.0e-8,1.0e-8,1.0e-8);
    EndCriteria endCriteria(10000, 100, 1e-6, 1e-8, 1e-8);

    //Optimize
    model->calibrate(swaptions, optimizationMethod, endCriteria);
    EndCriteria::Type ecType = model->endCriteria();

    // Check and print out results
    Real cachedA, cachedSigma;
    if (!usingAtParCoupons) {
        cachedA = 0.0463679, cachedSigma = 0.00579831;
    } else {
        cachedA = 0.0464041, cachedSigma = 0.00579912;
    }

    Real tolerance = 1.3e-5;
    Array xMinCalculated = model->params();
    Real yMinCalculated = model->value(xMinCalculated, swaptions);
    Array xMinExpected(2);
    xMinExpected[0]= cachedA;
    xMinExpected[1]= cachedSigma;
    Real yMinExpected = model->value(xMinExpected, swaptions);
    if (std::fabs(xMinCalculated[0]-cachedA) > tolerance
        || std::fabs(xMinCalculated[1]-cachedSigma) > tolerance) {
        BOOST_ERROR("Failed to reproduce cached calibration results:\n"
                    << "calculated: a = " << xMinCalculated[0] << ", "
                    << "sigma = " << xMinCalculated[1] << ", "
                    << "f(a) = " << yMinCalculated << ",\n"
                    << "expected:   a = " << xMinExpected[0] << ", "
                    << "sigma = " << xMinExpected[1] << ", "
                    << "f(a) = " << yMinExpected << ",\n"
                    << "difference: a = " << xMinCalculated[0]-xMinExpected[0] << ", "
                    << "sigma = " << xMinCalculated[1]-xMinExpected[1] << ", "
                    << "f(a) = " << yMinCalculated - yMinExpected << ",\n"
                    << "end criteria = " << ecType );
    }
}

BOOST_AUTO_TEST_CASE(testCachedHullWhiteFixedReversion) {
    BOOST_TEST_MESSAGE("Testing Hull-White calibration with fixed reversion against cached values...");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    Date today(15, February, 2002);
    Date settlement(19, February, 2002);
    Settings::instance().evaluationDate() = today;
    Handle<YieldTermStructure> termStructure(flatRate(settlement,0.04875825,
                                                      Actual365Fixed()));
    ext::shared_ptr<HullWhite> model(new HullWhite(termStructure,0.05,0.01));
    CalibrationData data[] = {{ 1, 5, 0.1148 },
                              { 2, 4, 0.1108 },
                              { 3, 3, 0.1070 },
                              { 4, 2, 0.1021 },
                              { 5, 1, 0.1000 }};
    ext::shared_ptr<IborIndex> index(new Euribor6M(termStructure));

    ext::shared_ptr<PricingEngine> engine(
                                     new JamshidianSwaptionEngine(model));

    std::vector<ext::shared_ptr<CalibrationHelper> > swaptions;
    for (auto& i : data) {
        ext::shared_ptr<Quote> vol(new SimpleQuote(i.volatility));
        ext::shared_ptr<BlackCalibrationHelper> helper(
            new SwaptionHelper(Period(i.start, Years), Period(i.length, Years), Handle<Quote>(vol),
                               index, Period(1, Years), Thirty360(Thirty360::BondBasis),
                               Actual360(), termStructure));
        helper->setPricingEngine(engine);
        swaptions.push_back(helper);
    }

    // Set up the optimization problem
    //Real simplexLambda = 0.1;
    //Simplex optimizationMethod(simplexLambda);
    LevenbergMarquardt optimizationMethod;//(1.0e-18,1.0e-18,1.0e-18);
    EndCriteria endCriteria(1000,500,1E-8,1E-8,1E-8);

    //Optimize
    model->calibrate(swaptions, optimizationMethod, endCriteria, Constraint(), std::vector<Real>(),
        HullWhite::FixedReversion());
    EndCriteria::Type ecType = model->endCriteria();

    // Check and print out results
    Real cachedA, cachedSigma;
    if (!usingAtParCoupons) {
        cachedA = 0.05, cachedSigma = 0.00585835;
    } else {
        cachedA = 0.05, cachedSigma = 0.00585858;
    }

    Real tolerance = 1.0e-5;
    Array xMinCalculated = model->params();
    Real yMinCalculated = model->value(xMinCalculated, swaptions);
    Array xMinExpected(2);
    xMinExpected[0]= cachedA;
    xMinExpected[1]= cachedSigma;
    Real yMinExpected = model->value(xMinExpected, swaptions);
    if (std::fabs(xMinCalculated[0]-cachedA) > tolerance
        || std::fabs(xMinCalculated[1]-cachedSigma) > tolerance) {
        BOOST_ERROR("Failed to reproduce cached calibration results:\n"
                    << "calculated: a = " << xMinCalculated[0] << ", "
                    << "sigma = " << xMinCalculated[1] << ", "
                    << "f(a) = " << yMinCalculated << ",\n"
                    << "expected:   a = " << xMinExpected[0] << ", "
                    << "sigma = " << xMinExpected[1] << ", "
                    << "f(a) = " << yMinExpected << ",\n"
                    << "difference: a = " << xMinCalculated[0]-xMinExpected[0] << ", "
                    << "sigma = " << xMinCalculated[1]-xMinExpected[1] << ", "
                    << "f(a) = " << yMinCalculated - yMinExpected << ",\n"
                    << "end criteria = " << ecType );
    }
}

BOOST_AUTO_TEST_CASE(testCachedHullWhite2) {
    BOOST_TEST_MESSAGE("Testing Hull-White calibration against cached "
                        "values using swaptions without start delay...");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    Date today(15, February, 2002);
    Date settlement(19, February, 2002);
    Settings::instance().evaluationDate() = today;
    Handle<YieldTermStructure> termStructure(flatRate(settlement,0.04875825,
                                                      Actual365Fixed()));
    ext::shared_ptr<HullWhite> model(new HullWhite(termStructure));
    CalibrationData data[] = {{ 1, 5, 0.1148 },
                              { 2, 4, 0.1108 },
                              { 3, 3, 0.1070 },
                              { 4, 2, 0.1021 },
                              { 5, 1, 0.1000 }};
    ext::shared_ptr<IborIndex> index(new Euribor6M(termStructure));
    ext::shared_ptr<IborIndex> index0(new IborIndex(
        index->familyName(),index->tenor(),0,index->currency(),index->fixingCalendar(),
        index->businessDayConvention(),index->endOfMonth(),index->dayCounter(),termStructure)); // Euribor 6m with zero fixing days

    ext::shared_ptr<PricingEngine> engine(
                                     new JamshidianSwaptionEngine(model));

    std::vector<ext::shared_ptr<CalibrationHelper> > swaptions;
    for (auto& i : data) {
        ext::shared_ptr<Quote> vol(new SimpleQuote(i.volatility));
        ext::shared_ptr<BlackCalibrationHelper> helper(
            new SwaptionHelper(Period(i.start, Years), Period(i.length, Years), Handle<Quote>(vol),
                               index0, Period(1, Years), Thirty360(Thirty360::BondBasis),
                               Actual360(), termStructure));
        helper->setPricingEngine(engine);
        swaptions.push_back(helper);
    }

    // Set up the optimization problem
    // Real simplexLambda = 0.1;
    // Simplex optimizationMethod(simplexLambda);
    LevenbergMarquardt optimizationMethod(1.0e-8,1.0e-8,1.0e-8);
    EndCriteria endCriteria(10000, 100, 1e-6, 1e-8, 1e-8);

    //Optimize
    model->calibrate(swaptions, optimizationMethod, endCriteria);
    EndCriteria::Type ecType = model->endCriteria();

    // Check and print out results
    // The cached values were produced with an older version of the
    // JamshidianEngine not accounting for the delay between option
    // expiry and underlying start
    Real cachedA, cachedSigma;
    if (!usingAtParCoupons)
        cachedA = 0.0481608, cachedSigma = 0.00582493;
    else
        cachedA = 0.0482063, cachedSigma = 0.00582687;

    Real tolerance = 1.0e-5;
    Array xMinCalculated = model->params();
    Real yMinCalculated = model->value(xMinCalculated, swaptions);
    Array xMinExpected(2);
    xMinExpected[0]= cachedA;
    xMinExpected[1]= cachedSigma;
    Real yMinExpected = model->value(xMinExpected, swaptions);
    if (std::fabs(xMinCalculated[0]-cachedA) > tolerance
        || std::fabs(xMinCalculated[1]-cachedSigma) > tolerance) {
        BOOST_ERROR("Failed to reproduce cached calibration results:\n"
                    << "calculated: a = " << xMinCalculated[0] << ", "
                    << "sigma = " << xMinCalculated[1] << ", "
                    << "f(a) = " << yMinCalculated << ",\n"
                    << "expected:   a = " << xMinExpected[0] << ", "
                    << "sigma = " << xMinExpected[1] << ", "
                    << "f(a) = " << yMinExpected << ",\n"
                    << "difference: a = " << xMinCalculated[0]-xMinExpected[0] << ", "
                    << "sigma = " << xMinCalculated[1]-xMinExpected[1] << ", "
                    << "f(a) = " << yMinCalculated - yMinExpected << ",\n"
                    << "end criteria = " << ecType );
    }
}

BOOST_AUTO_TEST_CASE(testSwaps) {
    BOOST_TEST_MESSAGE("Testing Hull-White swap pricing against known values...");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    Date today = Settings::instance().evaluationDate();
    Calendar calendar = TARGET();
    today = calendar.adjust(today);
    Settings::instance().evaluationDate() = today;

    Date settlement = calendar.advance(today,2,Days);

    std::vector<Date> dates = {
        settlement,
        calendar.advance(settlement,1,Weeks),
        calendar.advance(settlement,1,Months),
        calendar.advance(settlement,3,Months),
        calendar.advance(settlement,6,Months),
        calendar.advance(settlement,9,Months),
        calendar.advance(settlement,1,Years),
        calendar.advance(settlement,2,Years),
        calendar.advance(settlement,3,Years),
        calendar.advance(settlement,5,Years),
        calendar.advance(settlement,10,Years),
        calendar.advance(settlement,15,Years)
    };
    std::vector<DiscountFactor> discounts = {
        1.0,
        0.999258,
        0.996704,
        0.990809,
        0.981798,
        0.972570,
        0.963430,
        0.929532,
        0.889267,
        0.803693,
        0.596903,
        0.433022
    };

    Handle<YieldTermStructure> termStructure(
       ext::shared_ptr<YieldTermStructure>(
           new DiscountCurve(dates, discounts, Actual365Fixed())));

    ext::shared_ptr<HullWhite> model(new HullWhite(termStructure));

    Integer start[] = { -3, 0, 3 };
    Integer length[] = { 2, 5, 10 };
    Rate rates[] = { 0.02, 0.04, 0.06 };
    ext::shared_ptr<IborIndex> euribor(new Euribor6M(termStructure));

    ext::shared_ptr<PricingEngine> engine(
                                     new TreeVanillaSwapEngine(model,120));

    Real tolerance = usingAtParCoupons ? 1.0e-8 : 4.0e-3;

    for (Size i=0; i<std::size(start); i++) {

        Date startDate = calendar.advance(settlement,start[i],Months);
        if (startDate < today) {
            Date fixingDate = calendar.advance(startDate,-2,Days);
            euribor->addFixing(fixingDate, 0.03);
        }

        for (Size j=0; j<std::size(length); j++) {

            Date maturity = calendar.advance(startDate,length[i],Years);
            Schedule fixedSchedule(startDate, maturity, Period(Annual),
                                   calendar, Unadjusted, Unadjusted,
                                   DateGeneration::Forward, false);
            Schedule floatSchedule(startDate, maturity, Period(Semiannual),
                                   calendar, Following, Following,
                                   DateGeneration::Forward, false);
            for (Real rate : rates) {

                VanillaSwap swap(Swap::Payer, 1000000.0, fixedSchedule, rate,
                                 Thirty360(Thirty360::BondBasis),
                                 floatSchedule, euribor, 0.0, Actual360());
                swap.setPricingEngine(ext::shared_ptr<PricingEngine>(
                                     new DiscountingSwapEngine(termStructure)));
                Real expected = swap.NPV();
                swap.setPricingEngine(engine);
                Real calculated = swap.NPV();

                Real error = std::fabs((expected-calculated)/expected);
                if (error > tolerance) {
                    BOOST_ERROR("Failed to reproduce swap NPV:"
                                << std::fixed << std::setprecision(9)
                                << "\n    calculated: " << calculated
                                << "\n    expected:   " << expected
                                << std::scientific
                                << "\n    rel. error: " << error);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testFuturesConvexityBias) {
    BOOST_TEST_MESSAGE("Testing Hull-White futures convexity bias...");

    // G. Kirikos, D. Novak, "Convexity Conundrums", Risk Magazine, March 1997
    Real futureQuote = 94.0;
    Real sigma = 0.015;
    Time t = 5.0;
    Real tolerance = 0.0000001;

    for (auto [T, a, expectedForward] : {
        std::tuple{5.25,  0.03, 0.0573037},
        std::tuple{5.25,  1e-4, 0.0568627},
        std::tuple{5.25,  0.0,  0.0568611},
        std::tuple{5.001, 0.03, 0.0575736},
        std::tuple{5.0,   0.03, 0.0575747},
    }) {
        Rate futureImpliedRate = (100.0-futureQuote)/100.0;
        Rate calculatedForward =
            futureImpliedRate - HullWhite::convexityBias(futureQuote,t,T,sigma,a);

        Real error = std::fabs(calculatedForward-expectedForward);
        if (!(error < tolerance)) {
            BOOST_ERROR("Failed to reproduce convexity bias:"
                        << "\ncalculated: " << calculatedForward
                        << "\n  expected: " << expectedForward
                        << std::scientific
                        << "\n     error: " << error
                        << "\n tolerance: " << tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testG2DiscountBondForSmallMeanReversion) {
    BOOST_TEST_MESSAGE("Testing zero-bond pricing for G2 model with "
                       "mean reversion close to zero...");

    struct Case {
        Rate r;
        Real a;
        Volatility sigma;
        Real b;
        Volatility eta;
        Real rho;
        Time t, T;
        Real x, y;
        Real expected;
    };

    // reference values computed with 60-digit arithmetic
    const Case cases[] = {
        { 0.03, 1e-9, 0.01, 0.1, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88003659072582035 },
        { 0.03, 1e-8, 0.01, 0.1, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88003659138856715 },
        { 0.03, 1e-7, 0.01, 0.1, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88003659801603423 },
        { 0.03, 1e-6, 0.01, 0.1, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88003666429061573 },
        { 0.03, 1e-5, 0.01, 0.1, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88003732702749102 },
        { 0.03, 0.0001, 0.01, 0.1, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88004395350237115 },
        { 0.03, 0.001, 0.01, 0.1, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88011012895729353 },
        { 0.03, 0.1, 0.01, 1e-9, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.89274066161481982 },
        { 0.03, 0.1, 0.01, 1e-7, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.89274065456434843 },
        { 0.03, 0.1, 0.01, 1e-5, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.89273994952677022 },
        { 0.03, 0.1, 0.01, 0.001, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.89266954128089552 },
        { 0.03, 1e-7, 0.01, 2e-7, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88625548898935952 },
        { 0.03, 1e-6, 0.01, 1e-6, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.8862554988312159 },
        { 0.03, 1e-5, 0.01, 0.001, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88618543097653418 },
        { 0.03, 0.2, 0.01, 0.3, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88288328570041449 },
        { 0.03, 0.4, 0.01, 0.1, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.89837482145762172 },
        { 0.03, 0.5, 0.01, 0.125, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.899459669703478 },
        { 0.03, 0.45, 0.01, 0.04, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.90323235067173138 },
        { 0.03, 1.0, 0.01, 0.03, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.91184613030830664 },
        { 0.03, 3.0, 0.01, 5.0, 0.005, -0.5, 1.0, 5.0, 0.01, -0.01, 0.88573682792113109 },
        { 0.05, 1e-6, 0.02, 0.05, 0.01, 0.8, 0.5, 10.0, -0.02, 0.015, 0.6600529015892483 },
        { 0.05, 0.03, 0.02, 1e-7, 0.01, -0.9, 2.0, 30.0, 0.0, 0.0, 0.23888590049897576 },
    };

    const Date today = Settings::instance().evaluationDate();
    const Real tolerance = 1e-14;
    for (const auto& c : cases) {
        const Handle<YieldTermStructure> termStructure(flatRate(today, c.r, Actual365Fixed()));
        const G2 model(termStructure, c.a, c.sigma, c.b, c.eta, c.rho);
        const Real calculated = model.discountBond(c.t, c.T, c.x, c.y);
        const Real error = std::fabs(calculated/c.expected - 1.0);
        if (error > tolerance) {
            BOOST_ERROR("Failed to reproduce zero-bond price:"
                        << "\n  a, b          : " << c.a << ", " << c.b
                        << "\n  t, T          : " << c.t << ", " << c.T
                        << std::setprecision(17)
                        << "\n  calculated    : " << calculated
                        << "\n  expected      : " << c.expected
                        << std::scientific
                        << "\n  rel. error    : " << error
                        << "\n  tolerance     : " << tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testExtendedCoxIngersollRossDiscountFactor) {
    BOOST_TEST_MESSAGE("Testing zero-bond pricing for extended CIR model...");

    const Date today = Settings::instance().evaluationDate();

    const Rate rate = 0.1;
    const Handle<YieldTermStructure> rTS(
        flatRate(today, rate, Actual365Fixed()));

    const Time now = 1.5;
    const Time maturity = 2.5;

    const ExtendedCoxIngersollRoss cirModel(rTS, rate, 1.0, 1e-4, rate);

    const Real expected = rTS->discount(maturity)/rTS->discount(now);
    const Real calculated = cirModel.discountBond(now, maturity, rate);

    const Real tol = 1e-6;
    const Real diff = std::fabs(expected-calculated);

    if (diff > tol) {
        BOOST_ERROR("Failed to reproduce zero bound price:"
                    << "\n  calculated: " << calculated
                    << "\n  expected  : " << expected
                    << std::scientific
                    << "\n  difference: " << diff
                    << "\n  tolerance : " << tol);
    }
}

BOOST_AUTO_TEST_CASE(testVasicekDiscountFactorForSmallMeanReversion) {
    BOOST_TEST_MESSAGE("Testing zero-bond pricing for Vasicek model with small mean reversion...");

    const Rate r0 = 0.05;
    const Real a = 1e-12;
    const Real b = 0.05;
    const Volatility sigma = 0.01;
    const Real lambda = 0.0;
    const Time now = 0.0;
    const Time maturity = 1.0;

    const Vasicek model(r0, a, b, sigma, lambda);

    const Real expected = std::exp(-r0*maturity + sigma*sigma*maturity*maturity*maturity/6.0);
    const Real calculated = model.discountBond(now, maturity, r0);

    const Real tolerance = 1e-12;
    const Real error = std::fabs(expected-calculated);
    if (error > tolerance) {
        BOOST_ERROR("Failed to reproduce small-mean-reversion zero-bond price:"
                    << "\n  calculated: " << calculated
                    << "\n  expected  : " << expected
                    << std::scientific
                    << "\n  error     : " << error
                    << "\n  tolerance : " << tolerance);
    }
}

BOOST_AUTO_TEST_CASE(testVasicekDiscountFactorJustAboveSmallMeanReversionCutoff) {
    BOOST_TEST_MESSAGE("Testing zero-bond pricing for Vasicek model with "
                       "mean reversion close to zero...");

    struct Case {
        Rate r0;
        Real a, b;
        Volatility sigma;
        Real lambda;
        Time maturity;
        Real expected;
    };

    // reference values computed with 60-digit arithmetic
    const Case cases[] = {
        { 0.03, 1.5e-8, 0.03, 0.01, 0.0,  5.0, 0.86250298709518503 },
        { 0.03, 2.0e-8, 0.03, 0.01, 0.0,  5.0, 0.86250298706149353 },
        { 0.03, 5.0e-8, 0.03, 0.01, 0.0,  5.0, 0.86250298685934446 },
        { 0.03, 1.0e-7, 0.03, 0.01, 0.0,  5.0, 0.86250298652242929 },
        { 0.03, 1.0e-6, 0.03, 0.01, 0.0,  5.0, 0.8625029804579708  },
        { 0.03, 1.0e-5, 0.03, 0.01, 0.0,  5.0, 0.86250291981478866 },
        { 0.03, 1.0e-4, 0.03, 0.01, 0.0,  5.0, 0.86250231352326301 },
        { 0.05, 1.0e-9, 0.04, 0.02, -0.3, 10.0, 0.87517331816777411 },
        { 0.05, 1.5e-8, 0.04, 0.02, -0.3, 10.0, 0.87517330591534837 },
        { 0.05, 1.0e-7, 0.04, 0.02, -0.3, 10.0, 0.87517323152564763 },
        { 0.05, 1.0e-5, 0.04, 0.02, -0.3, 10.0, 0.87516456763064299 },
        { 0.05, 1.0e-3, 0.04, 0.02, -0.3, 10.0, 0.87430134378833069 },
        { 0.05, 0.05,   0.04, 0.02, -0.3, 10.0, 0.8382610373371705  },
        { 0.05, 0.4,    0.04, 0.02, -0.3, 10.0, 0.73828243526215487 },
        { 0.05, 2.0,    0.04, 0.02, -0.3, 10.0, 0.68657658594684234 }
    };

    const Real tolerance = 1e-14;
    for (const auto& c : cases) {
        const Vasicek model(c.r0, c.a, c.b, c.sigma, c.lambda);
        const Real calculated = model.discountBond(0.0, c.maturity, c.r0);
        const Real error = std::fabs(calculated/c.expected - 1.0);
        if (error > tolerance) {
            BOOST_ERROR("Failed to reproduce zero-bond price:"
                        << "\n  mean reversion: " << c.a
                        << "\n  maturity      : " << c.maturity
                        << std::setprecision(17)
                        << "\n  calculated    : " << calculated
                        << "\n  expected      : " << c.expected
                        << std::scientific
                        << "\n  rel. error    : " << error
                        << "\n  tolerance     : " << tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testVasicekDiscountBondOptionForSmallMeanReversion) {
    BOOST_TEST_MESSAGE("Testing bond option pricing for Vasicek model with "
                       "mean reversion close to zero...");

    const Rate r0 = 0.05;
    const Real b = 0.05;
    const Volatility sigma = 0.02;
    const Real lambda = 0.1;
    const Time maturity = 2.0;
    const Time bondMaturity = 7.0;
    const Real strike = 0.9;

    struct Case {
        Real a;
        Real call, put;
    };

    // reference values computed with 60-digit arithmetic
    const Case cases[] = {
        { 1.0e-7, 0.0061208618684224086, 0.13114554144413559 },
        { 1.0e-5, 0.006120132213223925,  0.13114486562595751 },
        { 0.1,    0.0015560169600970369, 0.12618440799746233 }
    };

    const Real tolerance = 1e-13;
    for (const auto& c : cases) {
        const Vasicek model(r0, c.a, b, sigma, lambda);
        for (Option::Type type : {Option::Call, Option::Put}) {
            const Real calculated =
                model.discountBondOption(type, strike, maturity, bondMaturity);
            const Real expected = (type == Option::Call ? c.call : c.put);
            const Real error = std::fabs(calculated - expected);
            if (error > tolerance) {
                BOOST_ERROR("Failed to reproduce bond option price:"
                            << "\n  mean reversion: " << c.a
                            << "\n  option type   : " << type
                            << std::setprecision(17)
                            << "\n  calculated    : " << calculated
                            << "\n  expected      : " << expected
                            << std::scientific
                            << "\n  error         : " << error
                            << "\n  tolerance     : " << tolerance);
            }
        }
    }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
