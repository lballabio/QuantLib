
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

#include "capfloor.hpp"
#include "utilities.hpp"
#include <ql/Instruments/capfloor.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#ifdef QL_USE_INDEXED_COUPON
#include <ql/CashFlows/indexedcashflowvectors.hpp>
#include <ql/CashFlows/upfrontindexedcoupon.hpp>
#endif
#include <ql/Utilities/dataformatters.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    // global data

    Date today_, settlement_;
    std::vector<Real> nominals_;
    BusinessDayConvention convention_;
    Frequency frequency_;
    boost::shared_ptr<Xibor> index_;
    Calendar calendar_;
    Integer settlementDays_, fixingDays_;
    Handle<YieldTermStructure> termStructure_;

    // utilities

    std::vector<boost::shared_ptr<CashFlow> > makeLeg(const Date& startDate,
                                                      Integer length) {
        Date endDate = calendar_.advance(startDate,length,Years,convention_);
        Schedule schedule(calendar_,startDate,endDate,frequency_,convention_);
        return FloatingRateCouponVector(schedule, convention_, nominals_,
                                        index_, fixingDays_,
                                        std::vector<Spread>(),
                                        index_->dayCounter());
    }

    boost::shared_ptr<PricingEngine> makeEngine(Volatility volatility) {
        Handle<Quote> vol(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
        boost::shared_ptr<BlackModel> model(
                                         new BlackModel(vol, termStructure_));
        return boost::shared_ptr<PricingEngine>(
                                              new BlackCapFloorEngine(model));
    }

    boost::shared_ptr<CapFloor> makeCapFloor(
                         CapFloor::Type type,
                         const std::vector<boost::shared_ptr<CashFlow> >& leg,
                         Rate strike,
                         Volatility volatility) {
        switch (type) {
          case CapFloor::Cap:
            return boost::shared_ptr<CapFloor>(
               new Cap(leg, std::vector<Rate>(1, strike),
                       termStructure_, makeEngine(volatility)));
          case CapFloor::Floor:
            return boost::shared_ptr<CapFloor>(
                new Floor(leg, std::vector<Rate>(1, strike),
                          termStructure_, makeEngine(volatility)));
          default:
            QL_FAIL("unknown cap/floor type");
        }
    }

    std::string typeToString(CapFloor::Type type) {
        switch (type) {
          case CapFloor::Cap:
            return "cap";
          case CapFloor::Floor:
            return "floor";
          case CapFloor::Collar:
            return "collar";
          default:
            QL_FAIL("unknown cap/floor type");
        }
    }


    void setup() {
        nominals_ = std::vector<Real>(1,100.0);
        frequency_ = Semiannual;
        index_ = boost::shared_ptr<Xibor>(
                            new Euribor(12/frequency_,Months,termStructure_));
        calendar_ = index_->calendar();
        convention_ = ModifiedFollowing;
        today_ = calendar_.adjust(Date::todaysDate());
        Settings::instance().setEvaluationDate(today_);
        settlementDays_ = 2;
        fixingDays_ = 2;
        settlement_ = calendar_.advance(today_,settlementDays_,Days);
        termStructure_.linkTo(flatRate(settlement_,0.05,Actual360()));
    }

    void teardown() {
        Settings::instance().setEvaluationDate(Date());
    }

}


void CapFloorTest::testStrikeDependency() {

    BOOST_MESSAGE("Testing cap/floor dependency on strike...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };
    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(vols); j++) {
            // store the results for different strikes...
            std::vector<Real> cap_values, floor_values;
            for (Size k=0; k<LENGTH(strikes); k++) {
                std::vector<boost::shared_ptr<CashFlow> > leg =
                    makeLeg(startDate,lengths[i]);
                boost::shared_ptr<Instrument> cap =
                    makeCapFloor(CapFloor::Cap,leg,
                                 strikes[k],vols[j]);
                cap_values.push_back(cap->NPV());
                boost::shared_ptr<Instrument> floor =
                    makeCapFloor(CapFloor::Floor,leg,
                                 strikes[k],vols[j]);
                floor_values.push_back(floor->NPV());
            }
            // and check that they go the right way
            std::vector<Real>::iterator it =
                std::adjacent_find(cap_values.begin(),cap_values.end(),
                                   std::less<Real>());
            if (it != cap_values.end()) {
                Size n = it - cap_values.begin();
                BOOST_FAIL(
                    "NPV is increasing with the strike in a cap: \n"
                    << std::setprecision(2)
                    << "    length:     " << lengths[i] << " years\n"
                    << "    volatility: " << io::volatility(vols[j]) << "\n"
                    << "    value:      " << cap_values[n]
                    << " at strike: " << io::rate(strikes[n]) << "\n"
                    << "    value:      " << cap_values[n+1]
                    << " at strike: " << io::rate(strikes[n+1]));
            }
            // same for floors
            it = std::adjacent_find(floor_values.begin(),floor_values.end(),
                                    std::greater<Real>());
            if (it != floor_values.end()) {
                Size n = it - floor_values.begin();
                BOOST_FAIL(
                    "NPV is decreasing with the strike in a floor: \n"
                    << std::setprecision(2)
                    << "    length:     " << lengths[i] << " years\n"
                    << "    volatility: " << io::volatility(vols[j]) << "\n"
                    << "    value:      " << floor_values[n]
                    << " at strike: " << io::rate(strikes[n]) << "\n"
                    << "    value:      " << floor_values[n+1]
                    << " at strike: " << io::rate(strikes[n+1]));
            }
        }
    }

    QL_TEST_TEARDOWN
}

void CapFloorTest::testConsistency() {

    BOOST_MESSAGE("Testing consistency between cap, floor and collar...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate cap_rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Rate floor_rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
      for (Size j=0; j<LENGTH(cap_rates); j++) {
        for (Size k=0; k<LENGTH(floor_rates); k++) {
          for (Size l=0; l<LENGTH(vols); l++) {

              std::vector<boost::shared_ptr<CashFlow> > leg =
                  makeLeg(startDate,lengths[i]);
              boost::shared_ptr<Instrument> cap =
                  makeCapFloor(CapFloor::Cap,leg,
                               cap_rates[j],vols[l]);
              boost::shared_ptr<Instrument> floor =
                  makeCapFloor(CapFloor::Floor,leg,
                               floor_rates[k],vols[l]);
              Collar collar(leg,std::vector<Rate>(1,cap_rates[j]),
                            std::vector<Rate>(1,floor_rates[k]),
                            termStructure_,makeEngine(vols[l]));

              if (std::fabs((cap->NPV()-floor->NPV())-collar.NPV()) > 1e-10) {
                  BOOST_FAIL(
                    "inconsistency between cap, floor and collar:\n"
                    << "    length:       " << lengths[i] << " years\n"
                    << "    volatility:   " << io::volatility(vols[l]) << "\n"
                    << "    cap value:    " << cap->NPV()
                    << " at strike: " << io::rate(cap_rates[j]) << "\n"
                    << "    floor value:  " << floor->NPV()
                    << " at strike: " << io::rate(floor_rates[k]) << "\n"
                    << "    collar value: " << collar.NPV());
              }
          }
        }
      }
    }

    QL_TEST_TEARDOWN
}

void CapFloorTest::testParity() {

    BOOST_MESSAGE("Testing put/call parity for cap and floor...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(vols); k++) {

            std::vector<boost::shared_ptr<CashFlow> > leg =
                makeLeg(startDate,lengths[i]);
            boost::shared_ptr<Instrument> cap =
                makeCapFloor(CapFloor::Cap,leg,
                             strikes[j],vols[k]);
            boost::shared_ptr<Instrument> floor =
                makeCapFloor(CapFloor::Floor,leg,
                             strikes[j],vols[k]);
            Date maturity = calendar_.advance(startDate,lengths[i],Years,
                                              convention_);
            Schedule schedule(calendar_,startDate,maturity,
                              frequency_,convention_);
            SimpleSwap swap(true,nominals_[0],
                            schedule,strikes[j],index_->dayCounter(),
                            schedule,index_,fixingDays_,0.0,
                            termStructure_);
            if (std::fabs((cap->NPV()-floor->NPV()) - swap.NPV()) > 1.0e-10) {
                BOOST_FAIL(
                    "put/call parity violated:\n"
                    << "    length:      " << lengths[i] << " years\n"
                    << "    volatility:  " << io::volatility(vols[k]) << "\n"
                    << "    strike:      " << io::rate(strikes[j]) << "\n"
                    << "    cap value:   " << cap->NPV() << "\n"
                    << "    floor value: " << floor->NPV() << "\n"
                    << "    swap value:  " << swap.NPV());
            }
        }
      }
    }

    QL_TEST_TEARDOWN
}

void CapFloorTest::testImpliedVolatility() {

    BOOST_MESSAGE("Testing implied term volatility for cap and floor...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Size maxEvaluations = 100;
    Real tolerance = 1.0e-6;

    CapFloor::Type types[] = { CapFloor::Cap, CapFloor::Floor };
    Rate strikes[] = { 0.02, 0.03, 0.04 };
    Integer lengths[] = { 1, 5, 10 };

    // test data
    Rate rRates[] = { 0.02, 0.03, 0.04 };
    Volatility vols[] = { 0.01, 0.20, 0.30, 0.70, 0.90 };

    for (Size k=0; k<LENGTH(lengths); k++) {
        std::vector<boost::shared_ptr<CashFlow> > leg =
            makeLeg(settlement_, lengths[k]);

        for (Size i=0; i<LENGTH(types); i++) {
            for (Size j=0; j<LENGTH(strikes); j++) {

                boost::shared_ptr<CapFloor> capfloor =
                    makeCapFloor(types[i], leg, strikes[j], 0.0);

                for (Size n=0; n<LENGTH(rRates); n++) {
                    for (Size m=0; m<LENGTH(vols); m++) {

                        Rate r = rRates[n];
                        Volatility v = vols[m];
                        termStructure_.linkTo(flatRate(settlement_,r,
                                                       Actual360()));
                        capfloor->setPricingEngine(makeEngine(v));

                        Real value = capfloor->NPV();
                        Volatility implVol = 0.0;

                        try {
                            implVol =
                                capfloor->impliedVolatility(value,
                                                            tolerance,
                                                            maxEvaluations);
                        } catch (std::exception& e) {
                            BOOST_FAIL(
                                typeToString(types[i]) << ":\n"
                                << "    strike:           "
                                << strikes[j] << "\n"
                                << "    risk-free rate:   "
                                << io::rate(r) << "\n"
                                << "    length:         "
                                << lengths[k] << " years\n"
                                << "    volatility:       "
                                << io::volatility(v) << "\n\n"
                                << e.what());
                        }
                        if (std::fabs(implVol-v) > tolerance) {
                            // the difference might not matter
                            capfloor->setPricingEngine(makeEngine(implVol));
                            Real value2 = capfloor->NPV();
                            if (std::fabs(value-value2) > tolerance) {
                                BOOST_FAIL(
                                    typeToString(types[i]) << ":\n"
                                    << "    strike:           "
                                    << strikes[j] << "\n"
                                    << "    risk-free rate:   "
                                    << io::rate(r) << "\n"
                                    << "    length:         "
                                    << lengths[k] << " years\n\n"
                                    << "    original volatility: "
                                    << io::volatility(v) << "\n"
                                    << "    price:               "
                                    << value << "\n"
                                    << "    implied volatility:  "
                                    << io::volatility(implVol) << "\n"
                                    << "    corresponding price: " << value2);
                            }
                        }
                    }
                }
            }
        }
    }

    QL_TEST_TEARDOWN
}

void CapFloorTest::testCachedValue() {

    BOOST_MESSAGE("Testing cap/floor value against cached values...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Date cachedToday(14,March,2002),
         cachedSettlement(18,March,2002);
    Settings::instance().setEvaluationDate(cachedToday);
    termStructure_.linkTo(flatRate(cachedSettlement, 0.05, Actual360()));
    Date startDate = termStructure_->referenceDate();
    std::vector<boost::shared_ptr<CashFlow> > leg = makeLeg(startDate,20);
    boost::shared_ptr<Instrument> cap = makeCapFloor(CapFloor::Cap,leg,
                                                     0.07,0.20);
    boost::shared_ptr<Instrument> floor = makeCapFloor(CapFloor::Floor,leg,
                                                       0.03,0.20);
#ifndef QL_USE_INDEXED_COUPON
    Real cachedCapNPV   = 6.960233718984,
         cachedFloorNPV = 2.701296290808;
#else
    Real cachedCapNPV   = 6.960840451560,
         cachedFloorNPV = 2.701133385568;
#endif

    if (std::fabs(cap->NPV()-cachedCapNPV) > 1.0e-11)
        BOOST_FAIL(
            "failed to reproduce cached cap value:\n"
            << std::setprecision(12)
            << "    calculated: " << cap->NPV() << "\n"
            << "    expected:   " << cachedCapNPV);

    if (std::fabs(floor->NPV()-cachedFloorNPV) > 1.0e-11)
        BOOST_FAIL(
            "failed to reproduce cached floor value:\n"
            << std::setprecision(12)
            << "    calculated: " << floor->NPV() << "\n"
            << "    expected:   " <<cachedFloorNPV);

    QL_TEST_TEARDOWN
}


test_suite* CapFloorTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cap/floor tests");
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testStrikeDependency));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testConsistency));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testParity));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testImpliedVolatility));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testCachedValue));
    return suite;
}

