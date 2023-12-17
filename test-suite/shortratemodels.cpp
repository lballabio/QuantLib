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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/models/shortrate/onefactormodels/extendedcoxingersollross.hpp>
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

    Real tolerance = 5.0e-6; 
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

    for (Size i=0; i<LENGTH(start); i++) {

        Date startDate = calendar.advance(settlement,start[i],Months);
        if (startDate < today) {
            Date fixingDate = calendar.advance(startDate,-2,Days);
            TimeSeries<Real> pastFixings;
            pastFixings[fixingDate] = 0.03;
            IndexManager::instance().setHistory(euribor->name(),
                                                pastFixings);
        }

        for (Size j=0; j<LENGTH(length); j++) {

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
    Real a = 0.03;
    Real sigma = 0.015;
    Time t = 5.0;
    Time T = 5.25;

    Rate expectedForward = 0.0573037;
    Real tolerance       = 0.0000001;

    Rate futureImpliedRate = (100.0-futureQuote)/100.0;
    Rate calculatedForward =
        futureImpliedRate - HullWhite::convexityBias(futureQuote,t,T,sigma,a);

    Real error = std::fabs(calculatedForward-expectedForward);

    if (error > tolerance) {
        BOOST_ERROR("Failed to reproduce convexity bias:"
                    << "\ncalculated: " << calculatedForward
                    << "\n  expected: " << expectedForward
                    << std::scientific
                    << "\n     error: " << error
                    << "\n tolerance: " << tolerance);
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
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
