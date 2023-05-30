/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 StatPro Italia srl

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

#include "schedule.hpp"
#include "utilities.hpp"
#include <ql/time/schedule.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <map>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::make_pair;
using std::map;
using std::pair;
using std::vector;

namespace {

    void check_dates(const Schedule& s,
                     const std::vector<Date>& expected) {
        if (s.size() != expected.size()) {
            BOOST_FAIL("expected " << expected.size() << " dates, "
                       << "found " << s.size());
        }
        for (Size i=0; i<expected.size(); ++i) {
            if (s[i] != expected[i]) {
                BOOST_ERROR("expected " << expected[i]
                            << " at index " << i << ", "
                            "found " << s[i]);
            }
        }
    }

}


void ScheduleTest::testDailySchedule() {
    BOOST_TEST_MESSAGE("Testing schedule with daily frequency...");

    Date startDate = Date(17,January,2012);

    Schedule s =
        MakeSchedule().from(startDate).to(startDate+7)
                      .withCalendar(TARGET())
                      .withFrequency(Daily)
                      .withConvention(Preceding);

    std::vector<Date> expected(6);
    // The schedule should skip Saturday 21st and Sunday 22rd.
    // Previously, it would adjust them to Friday 20th, resulting
    // in three copies of the same date.
    expected[0] = Date(17,January,2012);
    expected[1] = Date(18,January,2012);
    expected[2] = Date(19,January,2012);
    expected[3] = Date(20,January,2012);
    expected[4] = Date(23,January,2012);
    expected[5] = Date(24,January,2012);

    check_dates(s, expected);
}

void ScheduleTest::testEndDateWithEomAdjustment() {
    BOOST_TEST_MESSAGE(
        "Testing end date for schedule with end-of-month adjustment...");

    Schedule s =
        MakeSchedule().from(Date(30,September,2009))
                      .to(Date(15,June,2012))
                      .withCalendar(Japan())
                      .withTenor(6*Months)
                      .withConvention(Following)
                      .withTerminationDateConvention(Following)
                      .forwards()
                      .endOfMonth();

    std::vector<Date> expected(7);
    expected[0] = Date(30,September,2009);
    expected[1] = Date(31,March,2010);
    expected[2] = Date(30,September,2010);
    expected[3] = Date(31,March,2011);
    expected[4] = Date(30,September,2011);
    expected[5] = Date(30,March,2012);
    expected[6] = Date(15,June,2012);

    check_dates(s, expected);
}


void ScheduleTest::testDatesPastEndDateWithEomAdjustment() {
    BOOST_TEST_MESSAGE(
        "Testing that no dates are past the end date with EOM adjustment...");

    Schedule s =
        MakeSchedule().from(Date(28,March,2013))
                      .to(Date(30,March,2015))
                      .withCalendar(TARGET())
                      .withTenor(1*Years)
                      .withConvention(Unadjusted)
                      .withTerminationDateConvention(Unadjusted)
                      .forwards()
                      .endOfMonth();

    std::vector<Date> expected(3);
    expected[0] = Date(28,March,2013);
    expected[1] = Date(31,March,2014);
    // March 31st 2015, coming from the EOM adjustment of March 28th,
    // should be discarded as past the end date.
    expected[2] = Date(30,March,2015);

    check_dates(s, expected);

    // also, the last period should not be regular.
    if (s.isRegular(2))
        BOOST_ERROR("last period should not be regular");
}

void ScheduleTest::testDatesSameAsEndDateWithEomAdjustment() {
    BOOST_TEST_MESSAGE(
        "Testing that next-to-last date same as end date is removed...");

    Schedule s =
        MakeSchedule().from(Date(28,March,2013))
                      .to(Date(31,March,2015))
                      .withCalendar(TARGET())
                      .withTenor(1*Years)
                      .withConvention(Unadjusted)
                      .withTerminationDateConvention(Unadjusted)
                      .forwards()
                      .endOfMonth();

    std::vector<Date> expected(3);
    expected[0] = Date(28,March,2013);
    expected[1] = Date(31,March,2014);
    // March 31st 2015, coming from the EOM adjustment of March 28th,
    // should be discarded as the same as the end date.
    expected[2] = Date(31,March,2015);

    check_dates(s, expected);

    // also, the last period should be regular.
    if (!s.isRegular(2))
        BOOST_ERROR("last period should be regular");
}

void ScheduleTest::testForwardDatesWithEomAdjustment() {
    BOOST_TEST_MESSAGE(
        "Testing that the last date is not adjusted for EOM when "
        "termination date convention is unadjusted...");

    Schedule s =
        MakeSchedule().from(Date(31,August,1996))
                      .to(Date(15,September,1997))
                      .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
                      .withTenor(6*Months)
                      .withConvention(Unadjusted)
                      .withTerminationDateConvention(Unadjusted)
                      .forwards()
                      .endOfMonth();

    std::vector<Date> expected(4);
    expected[0] = Date(31,August,1996);
    expected[1] = Date(28,February,1997);
    expected[2] = Date(31,August,1997);
    expected[3] = Date(15,September,1997);

    check_dates(s, expected);
}

void ScheduleTest::testBackwardDatesWithEomAdjustment() {
    BOOST_TEST_MESSAGE(
        "Testing that the first date is not adjusted for EOM "
        "going backward when termination date convention is unadjusted...");

    Schedule s =
        MakeSchedule().from(Date(22,August,1996))
                      .to(Date(31,August,1997))
                      .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
                      .withTenor(6*Months)
                      .withConvention(Unadjusted)
                      .withTerminationDateConvention(Unadjusted)
                      .backwards()
                      .endOfMonth();

    std::vector<Date> expected(4);
    expected[0] = Date(22,August,1996);
    expected[1] = Date(31,August,1996);
    expected[2] = Date(28,February,1997);
    expected[3] = Date(31,August,1997);

    check_dates(s, expected);
}

void ScheduleTest::testDoubleFirstDateWithEomAdjustment() {
    BOOST_TEST_MESSAGE(
        "Testing that the first date is not duplicated due to "
        "EOM convention when going backwards...");

    Schedule s =
        MakeSchedule().from(Date(22,August,1996))
                      .to(Date(31,August,1997))
                      .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
                      .withTenor(6*Months)
                      .withConvention(Following)
                      .withTerminationDateConvention(Following)
                      .backwards()
                      .endOfMonth();

    std::vector<Date> expected(4);
    expected[0] = Date(22, August, 1996);
    expected[1] = Date(30,August,1996);
    expected[2] = Date(28,February,1997);
    expected[3] = Date(2,September,1997);

    check_dates(s, expected);
}

void ScheduleTest::testFirstDateWithEomAdjustment() {
    BOOST_TEST_MESSAGE("Testing schedule with first date and EOM adjustments ...");

    Schedule schedule = MakeSchedule()
                            .from(Date(10, August, 1996))
                            .to(Date(10, August, 1998))
                            .withFirstDate(Date(28, February, 1997))
                            .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
                            .withTenor(6 * Months)
                            .withConvention(Following)
                            .withTerminationDateConvention(Following)
                            .forwards()
                            .endOfMonth();

    std::vector<Date> expected(5);
    expected[0] = Date(12, August, 1996);
    expected[1] = Date(28, February, 1997);
    expected[2] = Date(29, August, 1997);
    expected[3] = Date(27, February, 1998);
    expected[4] = Date(10, August, 1998);

    check_dates(schedule, expected);
}

void ScheduleTest::testNextToLastWithEomAdjustment() {
    BOOST_TEST_MESSAGE("Testing schedule with next to last date and EOM adjustments ...");

    Schedule schedule = MakeSchedule()
                            .from(Date(10, August, 1996))
                            .to(Date(10, August, 1998))
                            .withNextToLastDate(Date(28, February, 1998))
                            .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
                            .withTenor(6 * Months)
                            .withConvention(Following)
                            .withTerminationDateConvention(Following)
                            .backwards()
                            .endOfMonth();

    std::vector<Date> expected(6);
    expected[0] = Date(12, August, 1996);
    expected[1] = Date(30, August, 1996);
    expected[2] = Date(28, February, 1997);
    expected[3] = Date(29, August, 1997);
    expected[4] = Date(27, February, 1998);
    expected[5] = Date(10, August, 1998);

    check_dates(schedule, expected);
}

void ScheduleTest::testEffectiveDateWithEomAdjustment() {
    BOOST_TEST_MESSAGE(
        "Testing forward schedule with EOM adjustment and effective date and first date in the same month...");

    Schedule s =
        MakeSchedule().from(Date(16,January,2023))
                      .to(Date(16,March,2023))
                      .withFirstDate(Date(31,January,2023))
                      .withCalendar(NullCalendar())
                      .withTenor(1*Months)
                      .withConvention(Unadjusted)
                      .withTerminationDateConvention(Unadjusted)
                      .forwards()
                      .endOfMonth();

    std::vector<Date> expected(4);
    // check that the effective date is not moved at the end of the month
    expected[0] = Date(16,January,2023);
    expected[1] = Date(31,January,2023);
    expected[2] = Date(28,February,2023);
    expected[3] = Date(16,March,2023);

    check_dates(s, expected);
}

namespace CdsTests {

    Schedule makeCdsSchedule(const Date& from, const Date& to, DateGeneration::Rule rule) {

        return MakeSchedule()
            .from(from)
            .to(to)
            .withCalendar(WeekendsOnly())
            .withTenor(3 * Months)
            .withConvention(Following)
            .withTerminationDateConvention(Unadjusted)
            .withRule(rule);
    }

    typedef map<pair<Date, Period>, pair<Date, Date> > InputData;

    void testCDSConventions(const InputData& inputs, DateGeneration::Rule rule) {

        // Test the generated start and end date against the expected start and end date.
        for (const auto& input : inputs) {

            Date from = input.first.first;
            Period tenor = input.first.second;

            Date maturity = cdsMaturity(from, tenor, rule);
            Date expEnd = input.second.second;
            BOOST_CHECK_EQUAL(maturity, expEnd);

            Schedule s = makeCdsSchedule(from, maturity, rule);

            Date expStart = input.second.first;
            Date start = s.startDate();
            Date end = s.endDate();
            BOOST_CHECK_EQUAL(start, expStart);
            BOOST_CHECK_EQUAL(end, expEnd);
        }
    }
}

void ScheduleTest::testCDS2015Convention() {

    using CdsTests::makeCdsSchedule;

    BOOST_TEST_MESSAGE("Testing CDS2015 semi-annual rolling convention...");

    DateGeneration::Rule rule = DateGeneration::CDS2015;
    Period tenor(5, Years);

    // From September 20th 2016 to March 19th 2017 of the next year, end date is December 20th 2021 for a 5 year CDS.
    // To get the correct schedule, you can first use the cdsMaturity function to get the maturity from the tenor.
    Date tradeDate(12, Dec, 2016);
    Date maturity = cdsMaturity(tradeDate, tenor, rule);
    Date expStart(20, Sep, 2016);
    Date expMaturity(20, Dec, 2021);
    BOOST_CHECK_EQUAL(maturity, expMaturity);
    Schedule s = makeCdsSchedule(tradeDate, maturity, rule);
    BOOST_CHECK_EQUAL(s.startDate(), expStart);
    BOOST_CHECK_EQUAL(s.endDate(), expMaturity);

    // If we just use 12 Dec 2016 + 5Y = 12 Dec 2021 as termination date in the schedule, the schedule constructor can 
    // use any of the allowable CDS dates i.e. 20 Mar, Jun, Sep and Dec. In the constructor, we just use the next one 
    // here i.e. 20 Dec 2021. We get the same results as above.
    maturity = tradeDate + tenor;
    s = makeCdsSchedule(tradeDate, maturity, rule);
    BOOST_CHECK_EQUAL(s.startDate(), expStart);
    BOOST_CHECK_EQUAL(s.endDate(), expMaturity);

    // We do the same tests but with a trade date of 1 Mar 2017. Using cdsMaturity to get maturity date from 5Y tenor, 
    // we get the same maturity as above.
    tradeDate = Date(1, Mar, 2017);
    maturity = cdsMaturity(tradeDate, tenor, rule);
    BOOST_CHECK_EQUAL(maturity, expMaturity);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    expStart = Date(20, Dec, 2016);
    BOOST_CHECK_EQUAL(s.startDate(), expStart);
    BOOST_CHECK_EQUAL(s.endDate(), expMaturity);

    // Using 1 Mar 2017 + 5Y = 1 Mar 2022 as termination date in the schedule, the constructor just uses the next 
    // allowable CDS date i.e. 20 Mar 2022. We must update the expected maturity.
    maturity = tradeDate + tenor;
    s = makeCdsSchedule(tradeDate, maturity, rule);
    BOOST_CHECK_EQUAL(s.startDate(), expStart);
    expMaturity = Date(20, Mar, 2022);
    BOOST_CHECK_EQUAL(s.endDate(), expMaturity);

    // From March 20th 2017 to September 19th 2017, end date is June 20th 2022 for a 5 year CDS.
    tradeDate = Date(20, Mar, 2017);
    maturity = cdsMaturity(tradeDate, tenor, rule);
    expStart = Date(20, Mar, 2017);
    expMaturity = Date(20, Jun, 2022);
    BOOST_CHECK_EQUAL(maturity, expMaturity);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    BOOST_CHECK_EQUAL(s.startDate(), expStart);
    BOOST_CHECK_EQUAL(s.endDate(), expMaturity);
}

void ScheduleTest::testCDS2015ConventionGrid() {

    using CdsTests::InputData;

    // Testing against section 11 of ISDA doc FAQs Amending when Single Name CDS roll to new on-the-run contracts
    // December 20, 2015 Go-Live
    BOOST_TEST_MESSAGE("Testing CDS2015 convention against ISDA doc...");

    // Test inputs and expected outputs
    // The map key is a pair with 1st element equal to trade date and 2nd element equal to CDS tenor.
    // The map value is a pair with 1st and 2nd element equal to expected start and end date respectively.
    // The trade dates are from the transition dates in the doc i.e. 20th Mar, Jun, Sep and Dec in 2016 and a day 
    // either side. The tenors are selected tenors from the doc i.e. short quarterly tenors less than 1Y, 1Y and 5Y.
    InputData inputs = {
        { make_pair(Date(19, Mar, 2016), 3 * Months), make_pair(Date(21, Dec, 2015), Date(20, Mar, 2016)) },
        { make_pair(Date(20, Mar, 2016), 3 * Months), make_pair(Date(21, Dec, 2015), Date(20, Sep, 2016)) },
        { make_pair(Date(21, Mar, 2016), 3 * Months), make_pair(Date(21, Mar, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(19, Jun, 2016), 3 * Months), make_pair(Date(21, Mar, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(20, Jun, 2016), 3 * Months), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(21, Jun, 2016), 3 * Months), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(19, Sep, 2016), 3 * Months), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(20, Sep, 2016), 3 * Months), make_pair(Date(20, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Sep, 2016), 3 * Months), make_pair(Date(20, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Dec, 2016), 3 * Months), make_pair(Date(20, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Dec, 2016), 3 * Months), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Dec, 2016), 3 * Months), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Mar, 2016), 6 * Months), make_pair(Date(21, Dec, 2015), Date(20, Jun, 2016)) },
        { make_pair(Date(20, Mar, 2016), 6 * Months), make_pair(Date(21, Dec, 2015), Date(20, Dec, 2016)) },
        { make_pair(Date(21, Mar, 2016), 6 * Months), make_pair(Date(21, Mar, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(19, Jun, 2016), 6 * Months), make_pair(Date(21, Mar, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Jun, 2016), 6 * Months), make_pair(Date(20, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(21, Jun, 2016), 6 * Months), make_pair(Date(20, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(19, Sep, 2016), 6 * Months), make_pair(Date(20, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Sep, 2016), 6 * Months), make_pair(Date(20, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Sep, 2016), 6 * Months), make_pair(Date(20, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Dec, 2016), 6 * Months), make_pair(Date(20, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Dec, 2016), 6 * Months), make_pair(Date(20, Dec, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Dec, 2016), 6 * Months), make_pair(Date(20, Dec, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Mar, 2016), 9 * Months), make_pair(Date(21, Dec, 2015), Date(20, Sep, 2016)) },
        { make_pair(Date(20, Mar, 2016), 9 * Months), make_pair(Date(21, Dec, 2015), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Mar, 2016), 9 * Months), make_pair(Date(21, Mar, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Jun, 2016), 9 * Months), make_pair(Date(21, Mar, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Jun, 2016), 9 * Months), make_pair(Date(20, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Jun, 2016), 9 * Months), make_pair(Date(20, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Sep, 2016), 9 * Months), make_pair(Date(20, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Sep, 2016), 9 * Months), make_pair(Date(20, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(21, Sep, 2016), 9 * Months), make_pair(Date(20, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(19, Dec, 2016), 9 * Months), make_pair(Date(20, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(20, Dec, 2016), 9 * Months), make_pair(Date(20, Dec, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(21, Dec, 2016), 9 * Months), make_pair(Date(20, Dec, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(19, Mar, 2016), 1 * Years), make_pair(Date(21, Dec, 2015), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Mar, 2016), 1 * Years), make_pair(Date(21, Dec, 2015), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Mar, 2016), 1 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Jun, 2016), 1 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Jun, 2016), 1 * Years), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Jun, 2016), 1 * Years), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Sep, 2016), 1 * Years), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Sep, 2016), 1 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(21, Sep, 2016), 1 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(19, Dec, 2016), 1 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(20, Dec, 2016), 1 * Years), make_pair(Date(20, Dec, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(21, Dec, 2016), 1 * Years), make_pair(Date(20, Dec, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(19, Mar, 2016), 5 * Years), make_pair(Date(21, Dec, 2015), Date(20, Dec, 2020)) },
        { make_pair(Date(20, Mar, 2016), 5 * Years), make_pair(Date(21, Dec, 2015), Date(20, Jun, 2021)) },
        { make_pair(Date(21, Mar, 2016), 5 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(19, Jun, 2016), 5 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(20, Jun, 2016), 5 * Years), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(21, Jun, 2016), 5 * Years), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(19, Sep, 2016), 5 * Years), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(20, Sep, 2016), 5 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(21, Sep, 2016), 5 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(19, Dec, 2016), 5 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(20, Dec, 2016), 5 * Years), make_pair(Date(20, Dec, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(21, Dec, 2016), 5 * Years), make_pair(Date(20, Dec, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(20, Mar, 2016), 0 * Months), make_pair(Date(21, Dec, 2015), Date(20, Jun, 2016)) },
        { make_pair(Date(21, Mar, 2016), 0 * Months), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2016)) },
        { make_pair(Date(19, Jun, 2016), 0 * Months), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2016)) },
        { make_pair(Date(20, Sep, 2016), 0 * Months), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(21, Sep, 2016), 0 * Months), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(19, Dec, 2016), 0 * Months), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2016)) }
    };

    CdsTests::testCDSConventions(inputs, DateGeneration::CDS2015);
}

void ScheduleTest::testCDSConventionGrid() {

    using CdsTests::InputData;

    // Testing against section 11 of ISDA doc FAQs Amending when Single Name CDS roll to new on-the-run contracts
    // December 20, 2015 Go-Live. Amended the dates in the doc to the pre-2015 expected maturity dates.
    BOOST_TEST_MESSAGE("Testing CDS convention against ISDA doc...");

    // Test inputs and expected outputs
    // The map key is a pair with 1st element equal to trade date and 2nd element equal to CDS tenor.
    // The map value is a pair with 1st and 2nd element equal to expected start and end date respectively.
    // The trade dates are from the transition dates in the doc i.e. 20th Mar, Jun, Sep and Dec in 2016 and a day 
    // either side. The tenors are selected tenors from the doc i.e. short quarterly tenors less than 1Y, 1Y and 5Y.
    InputData inputs = {
        { make_pair(Date(19, Mar, 2016), 3 * Months), make_pair(Date(21, Dec, 2015), Date(20, Jun, 2016)) },
        { make_pair(Date(20, Mar, 2016), 3 * Months), make_pair(Date(21, Dec, 2015), Date(20, Sep, 2016)) },
        { make_pair(Date(21, Mar, 2016), 3 * Months), make_pair(Date(21, Mar, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(19, Jun, 2016), 3 * Months), make_pair(Date(21, Mar, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(20, Jun, 2016), 3 * Months), make_pair(Date(20, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(21, Jun, 2016), 3 * Months), make_pair(Date(20, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(19, Sep, 2016), 3 * Months), make_pair(Date(20, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Sep, 2016), 3 * Months), make_pair(Date(20, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Sep, 2016), 3 * Months), make_pair(Date(20, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Dec, 2016), 3 * Months), make_pair(Date(20, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Dec, 2016), 3 * Months), make_pair(Date(20, Dec, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Dec, 2016), 3 * Months), make_pair(Date(20, Dec, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Mar, 2016), 6 * Months), make_pair(Date(21, Dec, 2015), Date(20, Sep, 2016)) },
        { make_pair(Date(20, Mar, 2016), 6 * Months), make_pair(Date(21, Dec, 2015), Date(20, Dec, 2016)) },
        { make_pair(Date(21, Mar, 2016), 6 * Months), make_pair(Date(21, Mar, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(19, Jun, 2016), 6 * Months), make_pair(Date(21, Mar, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Jun, 2016), 6 * Months), make_pair(Date(20, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Jun, 2016), 6 * Months), make_pair(Date(20, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Sep, 2016), 6 * Months), make_pair(Date(20, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Sep, 2016), 6 * Months), make_pair(Date(20, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Sep, 2016), 6 * Months), make_pair(Date(20, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Dec, 2016), 6 * Months), make_pair(Date(20, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Dec, 2016), 6 * Months), make_pair(Date(20, Dec, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(21, Dec, 2016), 6 * Months), make_pair(Date(20, Dec, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(19, Mar, 2016), 9 * Months), make_pair(Date(21, Dec, 2015), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Mar, 2016), 9 * Months), make_pair(Date(21, Dec, 2015), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Mar, 2016), 9 * Months), make_pair(Date(21, Mar, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Jun, 2016), 9 * Months), make_pair(Date(21, Mar, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Jun, 2016), 9 * Months), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Jun, 2016), 9 * Months), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Sep, 2016), 9 * Months), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Sep, 2016), 9 * Months), make_pair(Date(20, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(21, Sep, 2016), 9 * Months), make_pair(Date(20, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(19, Dec, 2016), 9 * Months), make_pair(Date(20, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(20, Dec, 2016), 9 * Months), make_pair(Date(20, Dec, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(21, Dec, 2016), 9 * Months), make_pair(Date(20, Dec, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(19, Mar, 2016), 1 * Years), make_pair(Date(21, Dec, 2015), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Mar, 2016), 1 * Years), make_pair(Date(21, Dec, 2015), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Mar, 2016), 1 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Jun, 2016), 1 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Jun, 2016), 1 * Years), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(21, Jun, 2016), 1 * Years), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(19, Sep, 2016), 1 * Years), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(20, Sep, 2016), 1 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(21, Sep, 2016), 1 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(19, Dec, 2016), 1 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(20, Dec, 2016), 1 * Years), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2018)) },
        { make_pair(Date(21, Dec, 2016), 1 * Years), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2018)) },
        { make_pair(Date(19, Mar, 2016), 5 * Years), make_pair(Date(21, Dec, 2015), Date(20, Mar, 2021)) },
        { make_pair(Date(20, Mar, 2016), 5 * Years), make_pair(Date(21, Dec, 2015), Date(20, Jun, 2021)) },
        { make_pair(Date(21, Mar, 2016), 5 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(19, Jun, 2016), 5 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(20, Jun, 2016), 5 * Years), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2021)) },
        { make_pair(Date(21, Jun, 2016), 5 * Years), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2021)) },
        { make_pair(Date(19, Sep, 2016), 5 * Years), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2021)) },
        { make_pair(Date(20, Sep, 2016), 5 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(21, Sep, 2016), 5 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(19, Dec, 2016), 5 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(20, Dec, 2016), 5 * Years), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2022)) },
        { make_pair(Date(21, Dec, 2016), 5 * Years), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2022)) },
        { make_pair(Date(19, Mar, 2016), 0 * Months), make_pair(Date(21, Dec, 2015), Date(20, Mar, 2016)) },
        { make_pair(Date(20, Mar, 2016), 0 * Months), make_pair(Date(21, Dec, 2015), Date(20, Jun, 2016)) },
        { make_pair(Date(21, Mar, 2016), 0 * Months), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2016)) },
        { make_pair(Date(19, Jun, 2016), 0 * Months), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2016)) },
        { make_pair(Date(20, Jun, 2016), 0 * Months), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(21, Jun, 2016), 0 * Months), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(19, Sep, 2016), 0 * Months), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(20, Sep, 2016), 0 * Months), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(21, Sep, 2016), 0 * Months), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(19, Dec, 2016), 0 * Months), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Dec, 2016), 0 * Months), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Dec, 2016), 0 * Months), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2017)) }
    };

    CdsTests::testCDSConventions(inputs, DateGeneration::CDS);
}

void ScheduleTest::testOldCDSConventionGrid() {

    using CdsTests::InputData;

    // Testing against section 11 of ISDA doc FAQs Amending when Single Name CDS roll to new on-the-run contracts
    // December 20, 2015 Go-Live. Amended the dates in the doc to the pre-2009 expected start and maturity dates.
    BOOST_TEST_MESSAGE("Testing old CDS convention...");

    // Test inputs and expected outputs
    // The map key is a pair with 1st element equal to trade date and 2nd element equal to CDS tenor.
    // The map value is a pair with 1st and 2nd element equal to expected start and end date respectively.
    // The trade dates are from the transition dates in the doc i.e. 20th Mar, Jun, Sep and Dec in 2016 and a day 
    // either side. The tenors are selected tenors from the doc i.e. short quarterly tenors less than 1Y, 1Y and 5Y.
    InputData inputs = {
        { make_pair(Date(19, Mar, 2016), 3 * Months), make_pair(Date(19, Mar, 2016), Date(20, Jun, 2016)) },
        { make_pair(Date(20, Mar, 2016), 3 * Months), make_pair(Date(20, Mar, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(21, Mar, 2016), 3 * Months), make_pair(Date(21, Mar, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(19, Jun, 2016), 3 * Months), make_pair(Date(19, Jun, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(20, Jun, 2016), 3 * Months), make_pair(Date(20, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(21, Jun, 2016), 3 * Months), make_pair(Date(21, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(19, Sep, 2016), 3 * Months), make_pair(Date(19, Sep, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Sep, 2016), 3 * Months), make_pair(Date(20, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Sep, 2016), 3 * Months), make_pair(Date(21, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Dec, 2016), 3 * Months), make_pair(Date(19, Dec, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Dec, 2016), 3 * Months), make_pair(Date(20, Dec, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Dec, 2016), 3 * Months), make_pair(Date(21, Dec, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Mar, 2016), 6 * Months), make_pair(Date(19, Mar, 2016), Date(20, Sep, 2016)) },
        { make_pair(Date(20, Mar, 2016), 6 * Months), make_pair(Date(20, Mar, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(21, Mar, 2016), 6 * Months), make_pair(Date(21, Mar, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(19, Jun, 2016), 6 * Months), make_pair(Date(19, Jun, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Jun, 2016), 6 * Months), make_pair(Date(20, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Jun, 2016), 6 * Months), make_pair(Date(21, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Sep, 2016), 6 * Months), make_pair(Date(19, Sep, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Sep, 2016), 6 * Months), make_pair(Date(20, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Sep, 2016), 6 * Months), make_pair(Date(21, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Dec, 2016), 6 * Months), make_pair(Date(19, Dec, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Dec, 2016), 6 * Months), make_pair(Date(20, Dec, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(21, Dec, 2016), 6 * Months), make_pair(Date(21, Dec, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(19, Mar, 2016), 9 * Months), make_pair(Date(19, Mar, 2016), Date(20, Dec, 2016)) },
        { make_pair(Date(20, Mar, 2016), 9 * Months), make_pair(Date(20, Mar, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(21, Mar, 2016), 9 * Months), make_pair(Date(21, Mar, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(19, Jun, 2016), 9 * Months), make_pair(Date(19, Jun, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Jun, 2016), 9 * Months), make_pair(Date(20, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Jun, 2016), 9 * Months), make_pair(Date(21, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Sep, 2016), 9 * Months), make_pair(Date(19, Sep, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Sep, 2016), 9 * Months), make_pair(Date(20, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(21, Sep, 2016), 9 * Months), make_pair(Date(21, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(19, Dec, 2016), 9 * Months), make_pair(Date(19, Dec, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(20, Dec, 2016), 9 * Months), make_pair(Date(20, Dec, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(21, Dec, 2016), 9 * Months), make_pair(Date(21, Dec, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(19, Mar, 2016), 1 * Years), make_pair(Date(19, Mar, 2016), Date(20, Mar, 2017)) },
        { make_pair(Date(20, Mar, 2016), 1 * Years), make_pair(Date(20, Mar, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(21, Mar, 2016), 1 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(19, Jun, 2016), 1 * Years), make_pair(Date(19, Jun, 2016), Date(20, Jun, 2017)) },
        { make_pair(Date(20, Jun, 2016), 1 * Years), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(21, Jun, 2016), 1 * Years), make_pair(Date(21, Jun, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(19, Sep, 2016), 1 * Years), make_pair(Date(19, Sep, 2016), Date(20, Sep, 2017)) },
        { make_pair(Date(20, Sep, 2016), 1 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(21, Sep, 2016), 1 * Years), make_pair(Date(21, Sep, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(19, Dec, 2016), 1 * Years), make_pair(Date(19, Dec, 2016), Date(20, Dec, 2017)) },
        { make_pair(Date(20, Dec, 2016), 1 * Years), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2018)) },
        { make_pair(Date(21, Dec, 2016), 1 * Years), make_pair(Date(21, Dec, 2016), Date(20, Mar, 2018)) },
        { make_pair(Date(19, Mar, 2016), 5 * Years), make_pair(Date(19, Mar, 2016), Date(20, Mar, 2021)) },
        { make_pair(Date(20, Mar, 2016), 5 * Years), make_pair(Date(20, Mar, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(21, Mar, 2016), 5 * Years), make_pair(Date(21, Mar, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(19, Jun, 2016), 5 * Years), make_pair(Date(19, Jun, 2016), Date(20, Jun, 2021)) },
        { make_pair(Date(20, Jun, 2016), 5 * Years), make_pair(Date(20, Jun, 2016), Date(20, Sep, 2021)) },
        { make_pair(Date(21, Jun, 2016), 5 * Years), make_pair(Date(21, Jun, 2016), Date(20, Sep, 2021)) },
        { make_pair(Date(19, Sep, 2016), 5 * Years), make_pair(Date(19, Sep, 2016), Date(20, Sep, 2021)) },
        { make_pair(Date(20, Sep, 2016), 5 * Years), make_pair(Date(20, Sep, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(21, Sep, 2016), 5 * Years), make_pair(Date(21, Sep, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(19, Dec, 2016), 5 * Years), make_pair(Date(19, Dec, 2016), Date(20, Dec, 2021)) },
        { make_pair(Date(20, Dec, 2016), 5 * Years), make_pair(Date(20, Dec, 2016), Date(20, Mar, 2022)) },
        { make_pair(Date(21, Dec, 2016), 5 * Years), make_pair(Date(21, Dec, 2016), Date(20, Mar, 2022)) }
    };

    CdsTests::testCDSConventions(inputs, DateGeneration::OldCDS);
}

void ScheduleTest::testCDS2015ConventionSampleDates() {

    BOOST_TEST_MESSAGE("Testing all dates in sample CDS schedule(s) for rule CDS2015...");

    using CdsTests::makeCdsSchedule;

    DateGeneration::Rule rule = DateGeneration::CDS2015;
    Period tenor(1, Years);

    // trade date = Fri 18 Sep 2015.
    Date tradeDate(18, Sep, 2015);
    Date maturity = cdsMaturity(tradeDate, tenor, rule);
    Schedule s = makeCdsSchedule(tradeDate, maturity, rule);
    vector<Date> expDates = {
        Date(22, Jun, 2015), Date(21, Sep, 2015), Date(21, Dec, 2015),
        Date(21, Mar, 2016), Date(20, Jun, 2016)
    };
    check_dates(s, expDates);

    // trade date = Sat 19 Sep 2015, no change.
    tradeDate = Date(19, Sep, 2015);
    maturity = cdsMaturity(tradeDate, tenor, rule);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    check_dates(s, expDates);

    // trade date = Sun 20 Sep 2015. Roll to new maturity. Trade date still before next coupon payment
    // date of Mon 21 Sep 2015, so keep the first period from 22 Jun 2015 to 21 Sep 2015 in schedule.
    tradeDate = Date(20, Sep, 2015);
    maturity = cdsMaturity(tradeDate, tenor, rule);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    expDates.emplace_back(20, Sep, 2016);
    expDates.emplace_back(20, Dec, 2016);
    check_dates(s, expDates);

    // trade date = Mon 21 Sep 2015, first period drops out of schedule.
    tradeDate = Date(21, Sep, 2015);
    maturity = cdsMaturity(tradeDate, tenor, rule);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    expDates.erase(expDates.begin());
    check_dates(s, expDates);

    // Another sample trade date, Sat 20 Jun 2009.
    tradeDate = Date(20, Jun, 2009);
    maturity = Date(20, Dec, 2009);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    vector<Date> tmp = {
        Date(20, Mar, 2009), Date(22, Jun, 2009), Date(21, Sep, 2009), Date(20, Dec, 2009)
    };
    expDates.assign(tmp.begin(), tmp.end());
    check_dates(s, expDates);

    // Move forward to Sun 21 Jun 2009
    tradeDate = Date(21, Jun, 2009);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    check_dates(s, expDates);

    // Move forward to Mon 22 Jun 2009
    tradeDate = Date(22, Jun, 2009);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    expDates.erase(expDates.begin());
    check_dates(s, expDates);
}

void ScheduleTest::testCDSConventionSampleDates() {

    BOOST_TEST_MESSAGE("Testing all dates in sample CDS schedule(s) for rule CDS...");

    using CdsTests::makeCdsSchedule;

    DateGeneration::Rule rule = DateGeneration::CDS;
    Period tenor(1, Years);

    // trade date = Fri 18 Sep 2015.
    Date tradeDate(18, Sep, 2015);
    Date maturity = cdsMaturity(tradeDate, tenor, rule);
    Schedule s = makeCdsSchedule(tradeDate, maturity, rule);
    vector<Date> expDates = {
        Date(22, Jun, 2015), Date(21, Sep, 2015), Date(21, Dec, 2015),
        Date(21, Mar, 2016), Date(20, Jun, 2016), Date(20, Sep, 2016)
    };
    check_dates(s, expDates);

    // trade date = Sat 19 Sep 2015, no change.
    tradeDate = Date(19, Sep, 2015);
    maturity = cdsMaturity(tradeDate, tenor, rule);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    check_dates(s, expDates);

    // trade date = Sun 20 Sep 2015. Roll to new maturity. Trade date still before next coupon payment
    // date of Mon 21 Sep 2015, so keep the first period from 22 Jun 2015 to 21 Sep 2015 in schedule.
    tradeDate = Date(20, Sep, 2015);
    maturity = cdsMaturity(tradeDate, tenor, rule);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    expDates.emplace_back(20, Dec, 2016);
    check_dates(s, expDates);

    // trade date = Mon 21 Sep 2015, first period drops out of schedule.
    tradeDate = Date(21, Sep, 2015);
    maturity = cdsMaturity(tradeDate, tenor, rule);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    expDates.erase(expDates.begin());
    check_dates(s, expDates);

    // Another sample trade date, Sat 20 Jun 2009.
    tradeDate = Date(20, Jun, 2009);
    maturity = Date(20, Dec, 2009);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    vector<Date> tmp = { Date(20, Mar, 2009), Date(22, Jun, 2009), Date(21, Sep, 2009), Date(20, Dec, 2009) };
    expDates.assign(tmp.begin(), tmp.end());
    check_dates(s, expDates);

    // Move forward to Sun 21 Jun 2009
    tradeDate = Date(21, Jun, 2009);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    check_dates(s, expDates);

    // Move forward to Mon 22 Jun 2009
    tradeDate = Date(22, Jun, 2009);
    s = makeCdsSchedule(tradeDate, maturity, rule);
    expDates.erase(expDates.begin());
    check_dates(s, expDates);
}

void ScheduleTest::testOldCDSConventionSampleDates() {

    BOOST_TEST_MESSAGE("Testing all dates in sample CDS schedule(s) for rule OldCDS...");

    using CdsTests::makeCdsSchedule;

    DateGeneration::Rule rule = DateGeneration::OldCDS;
    Period tenor(1, Years);

    // trade date plus 1D = Fri 18 Sep 2015.
    Date tradeDatePlusOne(18, Sep, 2015);
    Date maturity = cdsMaturity(tradeDatePlusOne, tenor, rule);
    Schedule s = makeCdsSchedule(tradeDatePlusOne, maturity, rule);
    vector<Date> expDates = {
        Date(18, Sep, 2015), Date(21, Dec, 2015),
        Date(21, Mar, 2016), Date(20, Jun, 2016), Date(20, Sep, 2016)
    };
    check_dates(s, expDates);

    // trade date plus 1D = Sat 19 Sep 2015, no change.
    // OldCDS, schedule start date is not adjusted (kept this).
    expDates[0] = tradeDatePlusOne = Date(19, Sep, 2015);
    maturity = cdsMaturity(tradeDatePlusOne, tenor, rule);
    s = makeCdsSchedule(tradeDatePlusOne, maturity, rule);
    check_dates(s, expDates);

    // trade date plus 1D = Sun 20 Sep 2015, roll.
    expDates[0] = tradeDatePlusOne = Date(20, Sep, 2015);
    maturity = cdsMaturity(tradeDatePlusOne, tenor, rule);
    s = makeCdsSchedule(tradeDatePlusOne, maturity, rule);
    expDates.emplace_back(20, Dec, 2016);
    check_dates(s, expDates);

    // trade date plus 1D = Mon 21 Sep 2015, no change.
    expDates[0] = tradeDatePlusOne = Date(21, Sep, 2015);
    maturity = cdsMaturity(tradeDatePlusOne, tenor, rule);
    s = makeCdsSchedule(tradeDatePlusOne, maturity, rule);
    check_dates(s, expDates);

    // Check the 30 day stub rule by moving closer to the first coupon payment date of Mon 21 Dec 2015.
    // The test here requires long first stub when trade date plus 1D = 21 Nov 2015. The condition in the schedule 
    // generation code is if: effective date + 30D > next 20th _unadjusted_. Not sure if we should refer to the actual 
    // coupon payment date here i.e. the next 20th _adjusted_ when making the decision.

    // 19 Nov 2015 + 30D = 19 Dec 2015 <= 20 Dec 2015 => short front stub.
    expDates[0] = tradeDatePlusOne = Date(19, Nov, 2015);
    s = makeCdsSchedule(tradeDatePlusOne, maturity, rule);
    check_dates(s, expDates);

    // 20 Nov 2015 + 30D = 20 Dec 2015 <= 20 Dec 2015 => short front stub.
    expDates[0] = tradeDatePlusOne = Date(20, Nov, 2015);
    s = makeCdsSchedule(tradeDatePlusOne, maturity, rule);
    check_dates(s, expDates);

    // 21 Nov 2015 + 30D = 21 Dec 2015 > 20 Dec 2015 => long front stub.
    // Note that if we reffered to the next coupon payment date of 21 Dec 2015, it would still be short front.
    expDates[0] = tradeDatePlusOne = Date(21, Nov, 2015);
    s = makeCdsSchedule(tradeDatePlusOne, maturity, rule);
    expDates.erase(expDates.begin() + 1);
    check_dates(s, expDates);
}

void ScheduleTest::testCDS2015ZeroMonthsMatured() {

    BOOST_TEST_MESSAGE("Testing 0M tenor for CDS2015 where matured...");

    DateGeneration::Rule rule = DateGeneration::CDS2015;
    Period tenor(0, Months);

    // Move through selected trade dates from 20 Dec 2015 to 20 Dec 2016 checking that the 0M CDS is matured.
    vector<Date> inputs = {
        Date(20, Dec, 2015),
        Date(15, Feb, 2016),
        Date(19, Mar, 2016),
        Date(20, Jun, 2016),
        Date(15, Aug, 2016),
        Date(19, Sep, 2016),
        Date(20, Dec, 2016)
    };

    for (const Date& input: inputs) {
        BOOST_CHECK_EQUAL(cdsMaturity(input, tenor, rule), Null<Date>());
    }
}

void ScheduleTest::testDateConstructor() {
    BOOST_TEST_MESSAGE("Testing the constructor taking a vector of dates and "
                       "possibly additional meta information...");

    std::vector<Date> dates = {Date(16, May, 2015),
                               Date(18, May, 2015),
                               Date(18, May, 2016),
                               Date(31, December, 2017)};

    // schedule without any additional information
    Schedule schedule1(dates);
    if (schedule1.size() != dates.size())
        BOOST_ERROR("schedule1 has size " << schedule1.size() << ", expected "
                                          << dates.size());
    for (Size i = 0; i < dates.size(); ++i)
        if (schedule1[i] != dates[i])
            BOOST_ERROR("schedule1 has " << schedule1[i] << " at position " << i
                                         << ", expected " << dates[i]);
    if (schedule1.calendar() != NullCalendar())
        BOOST_ERROR("schedule1 has calendar " << schedule1.calendar().name()
                                              << ", expected null calendar");
    if (schedule1.businessDayConvention() != Unadjusted)
        BOOST_ERROR("schedule1 has convention "
                    << schedule1.businessDayConvention()
                    << ", expected unadjusted");

    // schedule with metadata
    std::vector<bool> regular = {false, true, false};
    Schedule schedule2(dates, TARGET(), Following, ModifiedPreceding, 1 * Years,
                       DateGeneration::Backward, true, regular);
    for (Size i = 1; i < dates.size(); ++i)
        if (schedule2.isRegular(i) != regular[i - 1])
            BOOST_ERROR("schedule2 has a "
                        << (schedule2.isRegular(i) ? "regular" : "irregular")
                        << " period at position " << i << ", expected "
                        << (regular[i - 1] ? "regular" : "irregular"));
    if (schedule2.calendar() != TARGET())
        BOOST_ERROR("schedule1 has calendar " << schedule2.calendar().name()
                                              << ", expected TARGET");
    if (schedule2.businessDayConvention() != Following)
        BOOST_ERROR("schedule2 has convention "
                    << schedule2.businessDayConvention()
                    << ", expected Following");
    if (schedule2.terminationDateBusinessDayConvention() != ModifiedPreceding)
        BOOST_ERROR("schedule2 has convention "
                    << schedule2.terminationDateBusinessDayConvention()
                    << ", expected Modified Preceding");
    if (schedule2.tenor() != 1 * Years)
        BOOST_ERROR("schedule2 has tenor " << schedule2.tenor()
                                           << ", expected 1Y");
    if (schedule2.rule() != DateGeneration::Backward)
        BOOST_ERROR("schedule2 has rule " << schedule2.rule()
                                          << ", expected Backward");
    if (!schedule2.endOfMonth())
        BOOST_ERROR("schedule2 has end of month flag false, expected true");
}

void ScheduleTest::testFourWeeksTenor() {
    BOOST_TEST_MESSAGE(
        "Testing that a four-weeks tenor works...");

    try {
        Schedule s =
            MakeSchedule().from(Date(13,January,2016))
                          .to(Date(4,May,2016))
                          .withCalendar(TARGET())
                          .withTenor(4*Weeks)
                          .withConvention(Following)
                          .forwards();
    } catch (Error& e) {
        BOOST_ERROR("A four-weeks tenor caused an exception: " << e.what());
    }
}

void ScheduleTest::testScheduleAlwaysHasAStartDate() {
    BOOST_TEST_MESSAGE("Testing that variations of MakeSchedule "
                       "always produce a schedule with a start date...");
    // Attempt to establish whether the first coupoun payment date is
    // always the second element of the constructor.
    Calendar calendar = UnitedStates(UnitedStates::GovernmentBond);
    Schedule schedule = MakeSchedule()
        .from(Date(10, January, 2017))
        .withFirstDate(Date(31, August, 2017))
        .to(Date(28, February, 2026))
        .withFrequency(Semiannual)
        .withCalendar(calendar)
        .withConvention(Unadjusted)
        .backwards().endOfMonth(false);
    QL_ASSERT(schedule.date(0) == Date(10, January, 2017),
              "The first element should always be the start date");
    schedule = MakeSchedule()
        .from(Date(10, January, 2017))
        .to(Date(28, February, 2026))
        .withFrequency(Semiannual)
        .withCalendar(calendar)
        .withConvention(Unadjusted)
        .backwards().endOfMonth(false);
    QL_ASSERT(schedule.date(0) == Date(10, January, 2017),
              "The first element should always be the start date");
    schedule = MakeSchedule()
        .from(Date(31, August, 2017))
        .to(Date(28, February, 2026))
        .withFrequency(Semiannual)
        .withCalendar(calendar)
        .withConvention(Unadjusted)
        .backwards().endOfMonth(false);
    QL_ASSERT(schedule.date(0) == Date(31, August, 2017),
              "The first element should always be the start date");
}

void ScheduleTest::testShortEomSchedule() {
    BOOST_TEST_MESSAGE("Testing short end-of-month schedule...");
    Schedule s;
    // seg-faults in 1.15
    BOOST_REQUIRE_NO_THROW(s = MakeSchedule()
                                   .from(Date(21, Feb, 2019))
                                   .to(Date(28, Feb, 2019))
                                   .withCalendar(TARGET())
                                   .withTenor(1 * Years)
                                   .withConvention(ModifiedFollowing)
                                   .withTerminationDateConvention(ModifiedFollowing)
                                   .backwards()
                                   .endOfMonth(true));
    BOOST_REQUIRE(s.size() == 2);
    BOOST_CHECK(s[0] == Date(21, Feb, 2019));
    BOOST_CHECK(s[1] == Date(28, Feb, 2019));
}

void ScheduleTest::testFirstDateOnMaturity() {
    BOOST_TEST_MESSAGE("Testing schedule with first date on maturity...");
    Schedule schedule = MakeSchedule()
        .from(Date(20, September, 2016))
        .to(Date(20, December, 2016))
        .withFirstDate(Date(20, December, 2016))
        .withFrequency(Quarterly)
        .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
        .withConvention(Unadjusted)
        .backwards();

    std::vector<Date> expected(2);
    expected[0] = Date(20,September,2016);
    expected[1] = Date(20,December,2016);

    check_dates(schedule, expected);

    schedule = MakeSchedule()
        .from(Date(20, September, 2016))
        .to(Date(20, December, 2016))
        .withFirstDate(Date(20, December, 2016))
        .withFrequency(Quarterly)
        .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
        .withConvention(Unadjusted)
        .forwards();

    check_dates(schedule, expected);
}

void ScheduleTest::testNextToLastDateOnStart() {
    BOOST_TEST_MESSAGE("Testing schedule with next-to-last date on start date...");
    Schedule schedule = MakeSchedule()
        .from(Date(20, September, 2016))
        .to(Date(20, December, 2016))
        .withNextToLastDate(Date(20, September, 2016))
        .withFrequency(Quarterly)
        .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
        .withConvention(Unadjusted)
        .backwards();

    std::vector<Date> expected(2);
    expected[0] = Date(20,September,2016);
    expected[1] = Date(20,December,2016);

    check_dates(schedule, expected);

    schedule = MakeSchedule()
        .from(Date(20, September, 2016))
        .to(Date(20, December, 2016))
        .withNextToLastDate(Date(20, September, 2016))
        .withFrequency(Quarterly)
        .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
        .withConvention(Unadjusted)
        .backwards();

    check_dates(schedule, expected);
}

void ScheduleTest::testTruncation() {
    BOOST_TEST_MESSAGE("Testing schedule truncation...");
    Schedule s = MakeSchedule().from(Date(30, September, 2009))
        .to(Date(15, June, 2020))
        .withCalendar(Japan())
        .withTenor(6 * Months)
        .withConvention(Following)
        .withTerminationDateConvention(Following)
        .forwards()
        .endOfMonth();

    Schedule t;
    std::vector<Date> expected;

    // Until
    t = s.until(Date(1, Jan, 2014));
    expected = std::vector<Date>(10);
    expected[0] = Date(30, September, 2009);
    expected[1] = Date(31, March, 2010);
    expected[2] = Date(30, September, 2010);
    expected[3] = Date(31, March, 2011);
    expected[4] = Date(30, September, 2011);
    expected[5] = Date(30, March, 2012);
    expected[6] = Date(28, September, 2012);
    expected[7] = Date(29, March, 2013);
    expected[8] = Date(30, September, 2013);
    expected[9] = Date(1, January, 2014);
    check_dates(t, expected);
    BOOST_CHECK(t.isRegular().back() == false);

    // Until, with truncation date falling on a schedule date
    t = s.until(Date(30, September, 2013));
    expected = std::vector<Date>(9);
    expected[0] = Date(30, September, 2009);
    expected[1] = Date(31, March, 2010);
    expected[2] = Date(30, September, 2010);
    expected[3] = Date(31, March, 2011);
    expected[4] = Date(30, September, 2011);
    expected[5] = Date(30, March, 2012);
    expected[6] = Date(28, September, 2012);
    expected[7] = Date(29, March, 2013);
    expected[8] = Date(30, September, 2013);
    check_dates(t, expected);
    BOOST_CHECK(t.isRegular().back() == true);

    // After
    t = s.after(Date(1, Jan, 2014));
    expected = std::vector<Date>(15);
    expected[0] = Date(1, January, 2014);
    expected[1] = Date(31, March, 2014);
    expected[2] = Date(30, September, 2014);
    expected[3] = Date(31, March, 2015);
    expected[4] = Date(30, September, 2015);
    expected[5] = Date(31, March, 2016);
    expected[6] = Date(30, September, 2016);
    expected[7] = Date(31, March, 2017);
    expected[8] = Date(29, September, 2017);
    expected[9] = Date(30, March, 2018);
    expected[10] = Date(28, September, 2018);
    expected[11] = Date(29, March, 2019);
    expected[12] = Date(30, September, 2019);
    expected[13] = Date(31, March, 2020);
    expected[14] = Date(15, June, 2020);
    check_dates(t, expected);
    BOOST_CHECK(t.isRegular().front() == false);

    // After, with truncation date falling on a schedule date
    t = s.after(Date(28, September, 2018));
    expected = std::vector<Date>(5);
    expected[0] = Date(28, September, 2018);
    expected[1] = Date(29, March, 2019);
    expected[2] = Date(30, September, 2019);
    expected[3] = Date(31, March, 2020);
    expected[4] = Date(15, June, 2020);
    check_dates(t, expected);
    BOOST_CHECK(t.isRegular().front() == true);
}

test_suite* ScheduleTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Schedule tests");
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testDailySchedule));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testEndDateWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(
        &ScheduleTest::testDatesPastEndDateWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(
        &ScheduleTest::testDatesSameAsEndDateWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(
        &ScheduleTest::testForwardDatesWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(
        &ScheduleTest::testBackwardDatesWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(
        &ScheduleTest::testDoubleFirstDateWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testFirstDateWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testNextToLastWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testEffectiveDateWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testCDS2015Convention));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testCDS2015ConventionGrid));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testCDSConventionGrid));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testOldCDSConventionGrid));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testCDS2015ConventionSampleDates));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testCDSConventionSampleDates));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testOldCDSConventionSampleDates));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testCDS2015ZeroMonthsMatured));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testDateConstructor));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testFourWeeksTenor));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testScheduleAlwaysHasAStartDate));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testShortEomSchedule));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testFirstDateOnMaturity));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testNextToLastDateOnStart));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testTruncation));
    return suite;
}
