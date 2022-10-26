/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Grzegorz Andruszkiewicz

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

#include "catbonds.hpp"
#include "utilities.hpp"
#include <ql/types.hpp>
#include <ql/experimental/catbonds/catbond.hpp>
#include <ql/experimental/catbonds/catrisk.hpp>
#include <ql/experimental/catbonds/montecarlocatbondengine.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/business252.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/time/schedule.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/pricingengines/bond/bondfunctions.hpp>
#include <ql/termstructures/yield/flatforward.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace catbonds_test {
    std::pair<Date, Real> data[] = {std::pair<Date, Real>(Date(1, February, 2012), 100), std::pair<Date, Real>(Date(1, July, 2013), 150), std::pair<Date, Real>(Date(5, January, 2014), 50)};
    ext::shared_ptr<std::vector<std::pair<Date, Real> > > sampleEvents(new std::vector<std::pair<Date, Real> >(data, data+3));

    Date eventsStart(1, January, 2011);
    Date eventsEnd(31, December, 2014);
}

void CatBondTest::testEventSetForWholeYears() {
    BOOST_TEST_MESSAGE("Testing that catastrophe events are split correctly for periods of whole years...");

    using namespace catbonds_test;

    EventSet catRisk(sampleEvents, eventsStart, eventsEnd);
    ext::shared_ptr<CatSimulation> simulation = catRisk.newSimulation(Date(1, January, 2015), Date(31, December, 2015));

    BOOST_REQUIRE(simulation);

    std::vector<std::pair<Date, Real> > path;

    BOOST_REQUIRE(simulation->nextPath(path));
    BOOST_CHECK_EQUAL(Size(0), path.size());

    BOOST_REQUIRE(simulation->nextPath(path));
    BOOST_CHECK_EQUAL(Size(1), path.size());
    BOOST_CHECK_EQUAL(Date(1, February, 2015), path.at(0).first);
    BOOST_CHECK_EQUAL(100, path.at(0).second);

    BOOST_REQUIRE(simulation->nextPath(path));
    BOOST_CHECK_EQUAL(Size(1), path.size());
    BOOST_CHECK_EQUAL(Date(1, July, 2015), path.at(0).first);
    BOOST_CHECK_EQUAL(150, path.at(0).second);

    BOOST_REQUIRE(simulation->nextPath(path));
    BOOST_CHECK_EQUAL(Size(1), path.size());
    BOOST_CHECK_EQUAL(Date(5, January, 2015), path.at(0).first);
    BOOST_CHECK_EQUAL(50, path.at(0).second);

    BOOST_REQUIRE(!simulation->nextPath(path));
}


void CatBondTest::testEventSetForIrregularPeriods() {
    BOOST_TEST_MESSAGE("Testing that catastrophe events are split correctly for irregular periods...");

    using namespace catbonds_test;
    
    EventSet catRisk(sampleEvents, eventsStart, eventsEnd);
    ext::shared_ptr<CatSimulation> simulation = catRisk.newSimulation(Date(2, January, 2015), Date(5, January, 2016));

    BOOST_REQUIRE(simulation);

    std::vector<std::pair<Date, Real> > path;

    BOOST_REQUIRE(simulation->nextPath(path));
    BOOST_CHECK_EQUAL(Size(0), path.size());

    BOOST_REQUIRE(simulation->nextPath(path));
    BOOST_CHECK_EQUAL(Size(2), path.size());
    BOOST_CHECK_EQUAL(Date(1, July, 2015), path.at(0).first);
    BOOST_CHECK_EQUAL(150, path.at(0).second);
    BOOST_CHECK_EQUAL(Date(5, January, 2016), path.at(1).first);
    BOOST_CHECK_EQUAL(50, path.at(1).second);

    BOOST_REQUIRE(!simulation->nextPath(path));
}


void CatBondTest::testEventSetForNoEvents () {
    BOOST_TEST_MESSAGE("Testing that catastrophe events are split correctly when there are no simulated events...");

    using namespace catbonds_test;

    ext::shared_ptr<std::vector<std::pair<Date, Real> > > emptyEvents(new std::vector<std::pair<Date, Real> >());
    EventSet catRisk(emptyEvents, eventsStart, eventsEnd);
    ext::shared_ptr<CatSimulation> simulation = catRisk.newSimulation(Date(2, January, 2015), Date(5, January, 2016));

    BOOST_REQUIRE(simulation);

    std::vector<std::pair<Date, Real> > path;

    BOOST_REQUIRE(simulation->nextPath(path));
    BOOST_CHECK_EQUAL(Size(0), path.size());

    BOOST_REQUIRE(simulation->nextPath(path));
    BOOST_CHECK_EQUAL(Size(0), path.size());

    BOOST_REQUIRE(!simulation->nextPath(path));
}

void CatBondTest::testBetaRisk() {
    BOOST_TEST_MESSAGE("Testing that beta risk gives correct terminal distribution...");

    const size_t PATHS = 1000000;
    BetaRisk catRisk(100.0, 100.0, 10.0, 15.0);
    ext::shared_ptr<CatSimulation> simulation = catRisk.newSimulation(Date(2, January, 2015), Date(2, January, 2018));
    BOOST_REQUIRE(simulation);

    std::vector<std::pair<Date, Real> > path;
    Real sum = 0.0;
    Real sumSquares = 0.0;
    Real poissonSum = 0.0;
    Real poissonSumSquares = 0.0;
    
    
    for(size_t i=0; i<PATHS; ++i)
    {
        if (!simulation->nextPath(path))
            BOOST_FAIL("No next path available");
        Real processValue = 0.0;
        for (auto& j : path)
            processValue += j.second;
        sum+=processValue;
        sumSquares+=processValue*processValue;
        poissonSum+=path.size();
        poissonSumSquares+=path.size()*path.size();
    }
    Real poissonMean = poissonSum/PATHS;
    QL_CHECK_CLOSE(Real(3.0/100.0), poissonMean, 2);
    Real poissonVar = poissonSumSquares/PATHS - poissonMean*poissonMean;
    QL_CHECK_CLOSE(Real(3.0/100.0), poissonVar, 5);
    
    Real expectedMean = 3.0*10.0/100.0;
    Real actualMean = sum/PATHS;
    #ifdef _LIBCPP_VERSION
    QL_CHECK_CLOSE(expectedMean, actualMean, 5);
    #else
    QL_CHECK_CLOSE(expectedMean, actualMean, 1);
    #endif
    
    Real expectedVar = 3.0*(15.0*15.0+10*10)/100.0;
    Real actualVar = sumSquares/PATHS - actualMean*actualMean;
    #ifdef _LIBCPP_VERSION
    QL_CHECK_CLOSE(expectedVar, actualVar, 10);
    #else
    QL_CHECK_CLOSE(expectedVar, actualVar, 1);
    #endif
}

namespace catbonds_test {

    struct CommonVars {
        // common data
        Calendar calendar;
        Date today;
        Real faceAmount;

        // cleanup
        SavedSettings backup;

        // setup
        CommonVars() {
            calendar = TARGET();
            today = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = today;
            faceAmount = 1000000.0;
        }
    };
}

void CatBondTest::testRiskFreeAgainstFloatingRateBond() {
    BOOST_TEST_MESSAGE("Testing floating-rate cat bond against risk-free floating-rate bond...");

    using namespace catbonds_test;

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    CommonVars vars;

    Date today(22,November,2004);
    Settings::instance().evaluationDate() = today;

    Natural settlementDays = 1;

    Handle<YieldTermStructure> riskFreeRate(flatRate(today,0.025,Actual360()));
    Handle<YieldTermStructure> discountCurve(flatRate(today,0.03,Actual360()));

    ext::shared_ptr<IborIndex> index(new USDLibor(6*Months, riskFreeRate));
    Natural fixingDays = 1;

    Real tolerance = 1.0e-6;

    ext::shared_ptr<IborCouponPricer> pricer(new
        BlackIborCouponPricer(Handle<OptionletVolatilityStructure>()));

    // plain

    Schedule sch(Date(30,November,2004),
                 Date(30,November,2008),
                 Period(Semiannual),
                 UnitedStates(UnitedStates::GovernmentBond),
                 ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Backward, false);

    ext::shared_ptr<CatRisk> noCatRisk(new EventSet(
        ext::make_shared<std::vector<std::pair<Date, Real> > >(), 
        Date(1, Jan, 2000), Date(31, Dec, 2010)));

    ext::shared_ptr<EventPaymentOffset> paymentOffset(new NoOffset());
    ext::shared_ptr<NotionalRisk> notionalRisk(new DigitalNotionalRisk(paymentOffset, 100));

    FloatingRateBond bond1(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           ModifiedFollowing, fixingDays,
                           std::vector<Real>(), std::vector<Spread>(),
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    FloatingCatBond catBond1(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           notionalRisk, 
                           ModifiedFollowing, fixingDays,
                           std::vector<Real>(), std::vector<Spread>(),
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    ext::shared_ptr<PricingEngine> bondEngine(
                                     new DiscountingBondEngine(riskFreeRate));
    bond1.setPricingEngine(bondEngine);
    setCouponPricer(bond1.cashflows(),pricer);

    ext::shared_ptr<PricingEngine> catBondEngine(new MonteCarloCatBondEngine(noCatRisk, riskFreeRate));
    catBond1.setPricingEngine(catBondEngine);
    setCouponPricer(catBond1.cashflows(),pricer);

    Real cachedPrice1 = usingAtParCoupons ? 99.874646 : 99.874645;

    Real price = bond1.cleanPrice();
    Real catPrice = catBond1.cleanPrice();
    if (std::fabs(price-cachedPrice1) > tolerance || std::fabs(catPrice-price) > tolerance) {
        BOOST_FAIL("failed to reproduce floating rate bond price:\n"
                   << std::fixed
                   << "    floating bond: " << price << "\n"
                   << "    catBond bond: " << catPrice << "\n"
                   << "    expected:   " << cachedPrice1 << "\n"
                   << "    error:      " << catPrice-price);
    }

    

    // different risk-free and discount curve

    FloatingRateBond bond2(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           ModifiedFollowing, fixingDays,
                           std::vector<Rate>(), std::vector<Spread>(),
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    FloatingCatBond catBond2(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           notionalRisk,
                           ModifiedFollowing, fixingDays,
                           std::vector<Rate>(), std::vector<Spread>(),
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    ext::shared_ptr<PricingEngine> bondEngine2(
                                    new DiscountingBondEngine(discountCurve));
    bond2.setPricingEngine(bondEngine2);
    setCouponPricer(bond2.cashflows(),pricer);

    ext::shared_ptr<PricingEngine> catBondEngine2(new MonteCarloCatBondEngine(noCatRisk, discountCurve));
    catBond2.setPricingEngine(catBondEngine2);
    setCouponPricer(catBond2.cashflows(),pricer);

    Real cachedPrice2 = 97.955904;

    price = bond2.cleanPrice();
    catPrice = catBond2.cleanPrice();
    if (std::fabs(price-cachedPrice2) > tolerance || std::fabs(catPrice-price) > tolerance) {
        BOOST_FAIL("failed to reproduce floating rate bond price:\n"
                   << std::fixed
                   << "    floating bond: " << price << "\n"
                   << "    catBond bond: " << catPrice << "\n"
                   << "    expected:   " << cachedPrice2 << "\n"
                   << "    error:      " << catPrice-price);
    }

    // varying spread

    std::vector<Rate> spreads(4);
    spreads[0] = 0.001;
    spreads[1] = 0.0012;
    spreads[2] = 0.0014;
    spreads[3] = 0.0016;

    FloatingRateBond bond3(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           ModifiedFollowing, fixingDays,
                           std::vector<Real>(), spreads,
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    FloatingCatBond catBond3(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           notionalRisk,
                           ModifiedFollowing, fixingDays,
                           std::vector<Real>(), spreads,
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    bond3.setPricingEngine(bondEngine2);
    setCouponPricer(bond3.cashflows(),pricer);

    catBond3.setPricingEngine(catBondEngine2);
    setCouponPricer(catBond3.cashflows(),pricer);

    Real cachedPrice3 = usingAtParCoupons ? 98.495459 : 98.495458;

    price = bond3.cleanPrice();
    catPrice = catBond3.cleanPrice();
    if (std::fabs(price-cachedPrice3) > tolerance || std::fabs(catPrice-price) > tolerance) {
        BOOST_FAIL("failed to reproduce floating rate bond price:\n"
                   << std::fixed
                   << "    floating bond: " << price << "\n"
                   << "    catBond bond: " << catPrice << "\n"
                   << "    expected:   " << cachedPrice2 << "\n"
                   << "    error:      " << catPrice-price);
    }
}



void CatBondTest::testCatBondInDoomScenario() {
    BOOST_TEST_MESSAGE("Testing floating-rate cat bond in a doom scenario (certain default)...");

    using namespace catbonds_test;

    CommonVars vars;

    Date today(22,November,2004);
    Settings::instance().evaluationDate() = today;

    Natural settlementDays = 1;

    Handle<YieldTermStructure> riskFreeRate(flatRate(today,0.025,Actual360()));
    Handle<YieldTermStructure> discountCurve(flatRate(today,0.03,Actual360()));

    ext::shared_ptr<IborIndex> index(new USDLibor(6*Months, riskFreeRate));
    Natural fixingDays = 1;

    Real tolerance = 1.0e-6;

    ext::shared_ptr<IborCouponPricer> pricer(new
        BlackIborCouponPricer(Handle<OptionletVolatilityStructure>()));

    Schedule sch(Date(30,November,2004),
                 Date(30,November,2008),
                 Period(Semiannual),
                 UnitedStates(UnitedStates::GovernmentBond),
                 ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Backward, false);

    ext::shared_ptr<std::vector<std::pair<Date, Real> > > events(new std::vector<std::pair<Date, Real> >());
    events->push_back(std::pair<Date, Real>(Date(30,November,2004), 1000));
    ext::shared_ptr<CatRisk> doomCatRisk(new EventSet(
        events, 
        Date(30,November,2004), Date(30,November,2008)));

    ext::shared_ptr<EventPaymentOffset> paymentOffset(new NoOffset());
    ext::shared_ptr<NotionalRisk> notionalRisk(new DigitalNotionalRisk(paymentOffset, 100));

    FloatingCatBond catBond(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           notionalRisk,
                           ModifiedFollowing, fixingDays,
                           std::vector<Rate>(), std::vector<Spread>(),
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    ext::shared_ptr<PricingEngine> catBondEngine(new MonteCarloCatBondEngine(doomCatRisk, discountCurve));
    catBond.setPricingEngine(catBondEngine);
    setCouponPricer(catBond.cashflows(),pricer);

    Real price = catBond.cleanPrice();
    BOOST_CHECK_EQUAL(0, price);

    Real lossProbability = catBond.lossProbability();
    Real exhaustionProbability = catBond.exhaustionProbability();
    Real expectedLoss = catBond.expectedLoss();

    QL_CHECK_CLOSE(Real(1.0), lossProbability, tolerance);
    QL_CHECK_CLOSE(Real(1.0), exhaustionProbability, tolerance);
    QL_CHECK_CLOSE(Real(1.0), expectedLoss, tolerance);
}


void CatBondTest::testCatBondWithDoomOnceInTenYears() {
    BOOST_TEST_MESSAGE("Testing floating-rate cat bond in a doom once in 10 years scenario...");

    using namespace catbonds_test;

    CommonVars vars;

    Date today(22,November,2004);
    Settings::instance().evaluationDate() = today;

    Natural settlementDays = 1;

    Handle<YieldTermStructure> riskFreeRate(flatRate(today,0.025,Actual360()));
    Handle<YieldTermStructure> discountCurve(flatRate(today,0.03,Actual360()));

    ext::shared_ptr<IborIndex> index(new USDLibor(6*Months, riskFreeRate));
    Natural fixingDays = 1;

    Real tolerance = 1.0e-6;

    ext::shared_ptr<IborCouponPricer> pricer(new
        BlackIborCouponPricer(Handle<OptionletVolatilityStructure>()));

    Schedule sch(Date(30,November,2004),
                 Date(30,November,2008),
                 Period(Semiannual),
                 UnitedStates(UnitedStates::GovernmentBond),
                 ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Backward, false);

    ext::shared_ptr<std::vector<std::pair<Date, Real> > > events(new std::vector<std::pair<Date, Real> >());
    events->push_back(std::pair<Date, Real>(Date(30,November,2008), 1000));
    ext::shared_ptr<CatRisk> doomCatRisk(new EventSet(
        events, 
        Date(30,November,2004), Date(30,November,2044)));

    ext::shared_ptr<CatRisk> noCatRisk(new EventSet(
        ext::make_shared<std::vector<std::pair<Date, Real> > >(), 
        Date(1, Jan, 2000), Date(31, Dec, 2010)));

    ext::shared_ptr<EventPaymentOffset> paymentOffset(new NoOffset());
    ext::shared_ptr<NotionalRisk> notionalRisk(new DigitalNotionalRisk(paymentOffset, 100));

    FloatingCatBond catBond(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           notionalRisk,
                           ModifiedFollowing, fixingDays,
                           std::vector<Rate>(), std::vector<Spread>(),
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    ext::shared_ptr<PricingEngine> catBondEngine(new MonteCarloCatBondEngine(doomCatRisk, discountCurve));
    catBond.setPricingEngine(catBondEngine);
    setCouponPricer(catBond.cashflows(),pricer);

    Real price = catBond.cleanPrice();
    Real yield = catBond.yield(ActualActual(ActualActual::ISMA), Simple, Annual);
    Real lossProbability = catBond.lossProbability();
    Real exhaustionProbability = catBond.exhaustionProbability();
    Real expectedLoss = catBond.expectedLoss();

    QL_CHECK_CLOSE(Real(0.1), lossProbability, tolerance);
    QL_CHECK_CLOSE(Real(0.1), exhaustionProbability, tolerance);
    QL_CHECK_CLOSE(Real(0.1), expectedLoss, tolerance);

    ext::shared_ptr<PricingEngine> catBondEngineRF(new MonteCarloCatBondEngine(noCatRisk, discountCurve));
    catBond.setPricingEngine(catBondEngineRF);

    Real riskFreePrice = catBond.cleanPrice();
    Real riskFreeYield = catBond.yield(ActualActual(ActualActual::ISMA), Simple, Annual);
    Real riskFreeLossProbability = catBond.lossProbability();
    Real riskFreeExhaustionProbability = catBond.exhaustionProbability();
    Real riskFreeExpectedLoss = catBond.expectedLoss();
    
    QL_CHECK_CLOSE(Real(0.0), riskFreeLossProbability, tolerance);
    QL_CHECK_CLOSE(Real(0.0), riskFreeExhaustionProbability, tolerance);
    BOOST_CHECK(std::abs(riskFreeExpectedLoss) < tolerance);
    
    QL_CHECK_CLOSE(riskFreePrice*0.9, price, tolerance);
    BOOST_CHECK_LT(riskFreeYield, yield);
}

void CatBondTest::testCatBondWithDoomOnceInTenYearsProportional() {
    BOOST_TEST_MESSAGE("Testing floating-rate cat bond in a doom once in 10 years scenario with proportional notional reduction...");

    using namespace catbonds_test;

    CommonVars vars;

    Date today(22,November,2004);
    Settings::instance().evaluationDate() = today;

    Natural settlementDays = 1;

    Handle<YieldTermStructure> riskFreeRate(flatRate(today,0.025,Actual360()));
    Handle<YieldTermStructure> discountCurve(flatRate(today,0.03,Actual360()));

    ext::shared_ptr<IborIndex> index(new USDLibor(6*Months, riskFreeRate));
    Natural fixingDays = 1;

    Real tolerance = 1.0e-6;

    ext::shared_ptr<IborCouponPricer> pricer(new
        BlackIborCouponPricer(Handle<OptionletVolatilityStructure>()));

    Schedule sch(Date(30,November,2004),
                 Date(30,November,2008),
                 Period(Semiannual),
                 UnitedStates(UnitedStates::GovernmentBond),
                 ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Backward, false);

    ext::shared_ptr<std::vector<std::pair<Date, Real> > > events(new std::vector<std::pair<Date, Real> >());
    events->push_back(std::pair<Date, Real>(Date(30,November,2008), 1000));
    ext::shared_ptr<CatRisk> doomCatRisk(new EventSet(
        events, 
        Date(30,November,2004), Date(30,November,2044)));

    ext::shared_ptr<CatRisk> noCatRisk(new EventSet(
        ext::make_shared<std::vector<std::pair<Date, Real> > >(), 
        Date(1, Jan, 2000), Date(31, Dec, 2010)));

    ext::shared_ptr<EventPaymentOffset> paymentOffset(new NoOffset());
    ext::shared_ptr<NotionalRisk> notionalRisk(new ProportionalNotionalRisk(paymentOffset, 500, 1500));

    FloatingCatBond catBond(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           notionalRisk,
                           ModifiedFollowing, fixingDays,
                           std::vector<Rate>(), std::vector<Spread>(),
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    ext::shared_ptr<PricingEngine> catBondEngine(new MonteCarloCatBondEngine(doomCatRisk, discountCurve));
    catBond.setPricingEngine(catBondEngine);
    setCouponPricer(catBond.cashflows(),pricer);

    Real price = catBond.cleanPrice();
    Real yield = catBond.yield(ActualActual(ActualActual::ISMA), Simple, Annual);
    Real lossProbability = catBond.lossProbability();
    Real exhaustionProbability = catBond.exhaustionProbability();
    Real expectedLoss = catBond.expectedLoss();

    QL_CHECK_CLOSE(Real(0.1), lossProbability, tolerance);
    QL_CHECK_CLOSE(Real(0.0), exhaustionProbability, tolerance);
    QL_CHECK_CLOSE(Real(0.05), expectedLoss, tolerance);

    ext::shared_ptr<PricingEngine> catBondEngineRF(new MonteCarloCatBondEngine(noCatRisk, discountCurve));
    catBond.setPricingEngine(catBondEngineRF);

    Real riskFreePrice = catBond.cleanPrice();
    Real riskFreeYield = catBond.yield(ActualActual(ActualActual::ISMA), Simple, Annual);
    Real riskFreeLossProbability = catBond.lossProbability();
    Real riskFreeExpectedLoss = catBond.expectedLoss();
    
    QL_CHECK_CLOSE(Real(0.0), riskFreeLossProbability, tolerance);
    BOOST_CHECK(std::abs(riskFreeExpectedLoss) < tolerance);
    
    QL_CHECK_CLOSE(riskFreePrice*0.95, price, tolerance);
    BOOST_CHECK_LT(riskFreeYield, yield);
}


void CatBondTest::testCatBondWithGeneratedEventsProportional() {
    BOOST_TEST_MESSAGE("Testing floating-rate cat bond in a generated scenario with proportional notional reduction...");

    using namespace catbonds_test;

    CommonVars vars;

    Date today(22,November,2004);
    Settings::instance().evaluationDate() = today;

    Natural settlementDays = 1;

    Handle<YieldTermStructure> riskFreeRate(flatRate(today,0.025,Actual360()));
    Handle<YieldTermStructure> discountCurve(flatRate(today,0.03,Actual360()));

    ext::shared_ptr<IborIndex> index(new USDLibor(6*Months, riskFreeRate));
    Natural fixingDays = 1;

    Real tolerance = 1.0e-6;

    ext::shared_ptr<IborCouponPricer> pricer(new
        BlackIborCouponPricer(Handle<OptionletVolatilityStructure>()));

    Schedule sch(Date(30,November,2004),
                 Date(30,November,2008),
                 Period(Semiannual),
                 UnitedStates(UnitedStates::GovernmentBond),
                 ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Backward, false);

    ext::shared_ptr<CatRisk> betaCatRisk(new BetaRisk(5000, 50, 500, 500));

    ext::shared_ptr<CatRisk> noCatRisk(new EventSet(
        ext::make_shared<std::vector<std::pair<Date, Real> > >(), 
        Date(1, Jan, 2000), Date(31, Dec, 2010)));

    ext::shared_ptr<EventPaymentOffset> paymentOffset(new NoOffset());
    ext::shared_ptr<NotionalRisk> notionalRisk(new ProportionalNotionalRisk(paymentOffset, 500, 1500));

    FloatingCatBond catBond(settlementDays, vars.faceAmount, sch,
                           index, ActualActual(ActualActual::ISMA),
                           notionalRisk,
                           ModifiedFollowing, fixingDays,
                           std::vector<Rate>(), std::vector<Spread>(),
                           std::vector<Rate>(), std::vector<Rate>(),
                           false,
                           100.0, Date(30,November,2004));

    ext::shared_ptr<PricingEngine> catBondEngine(new MonteCarloCatBondEngine(betaCatRisk, discountCurve));
    catBond.setPricingEngine(catBondEngine);
    setCouponPricer(catBond.cashflows(),pricer);

    Real price = catBond.cleanPrice();
    Real yield = catBond.yield(ActualActual(ActualActual::ISMA), Simple, Annual);
    Real lossProbability = catBond.lossProbability();
    Real exhaustionProbability = catBond.exhaustionProbability();
    Real expectedLoss = catBond.expectedLoss();

    BOOST_CHECK(lossProbability<1.0 && lossProbability>0.0);
    BOOST_CHECK(exhaustionProbability<1.0 && exhaustionProbability>0.0);
    BOOST_CHECK(expectedLoss>0.0);

    ext::shared_ptr<PricingEngine> catBondEngineRF(new MonteCarloCatBondEngine(noCatRisk, discountCurve));
    catBond.setPricingEngine(catBondEngineRF);

    Real riskFreePrice = catBond.cleanPrice();
    Real riskFreeYield = catBond.yield(ActualActual(ActualActual::ISMA), Simple, Annual);
    Real riskFreeLossProbability = catBond.lossProbability();
    Real riskFreeExpectedLoss = catBond.expectedLoss();
    
    QL_CHECK_CLOSE(Real(0.0), riskFreeLossProbability, tolerance);
    BOOST_CHECK(std::abs(riskFreeExpectedLoss) < tolerance);
    
    BOOST_CHECK_GT(riskFreePrice, price);
    BOOST_CHECK_LT(riskFreeYield, yield);
}

test_suite* CatBondTest::suite() {
    auto* suite = BOOST_TEST_SUITE("CatBond tests");

    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testEventSetForWholeYears));
    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testEventSetForIrregularPeriods));
    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testEventSetForNoEvents));
    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testBetaRisk));
    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testRiskFreeAgainstFloatingRateBond));
    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testCatBondInDoomScenario));
    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testCatBondWithDoomOnceInTenYears));
    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testCatBondWithDoomOnceInTenYearsProportional));
    suite->add(QUANTLIB_TEST_CASE(&CatBondTest::testCatBondWithGeneratedEventsProportional));
    return suite;
}
