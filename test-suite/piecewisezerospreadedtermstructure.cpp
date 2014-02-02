
/*
 Copyright (C) 2014 StatPro Italia srl

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

#include "piecewisezerospreadedtermstructure.hpp"
#include "utilities.hpp"
#include <ql/termstructures/yield/piecewisezerospreadedtermstructure.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/math/interpolations/all.hpp>
#include <boost/make_shared.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct Datum {
        Integer n;
        TimeUnit units;
        Rate rate;
    };

    struct CommonVars {
        // common data
        Calendar calendar;
        Natural settlementDays;
        DayCounter dayCount;
        Compounding compounding;
        boost::shared_ptr<YieldTermStructure> termStructure;
        Date today;
        Date settlementDate;

        // cleanup
        SavedSettings backup;

        // setup
        CommonVars() {
            calendar = TARGET();
            settlementDays = 2;
            today =Date(9,June,2009);
            compounding = Continuous;
            dayCount = Actual360();
            settlementDate = calendar.advance(today,settlementDays,Days);

            Settings::instance().evaluationDate() = today;

            Integer ts[] = { 13,    41,  75,   165,   256 , 345,  524,  703 };
            Rate r[] = { 0.035,0.033,0.034, 0.034, 0.036,0.037,0.039,0.040 };
            std::vector<Rate> rates(1, 0.035);
            std::vector<Date> dates(1, settlementDate);
            for (Size i = 0; i < 8; ++i) {
                dates.push_back(calendar.advance(today,ts[i],Days));
                rates.push_back(r[i]);
            }
            termStructure = boost::make_shared<ZeroCurve>(dates, rates, dayCount);
        }
    };

}

void PiecewiseZeroSpreadedTermStructureTest::testFlatInterpolationLeft() {

    BOOST_MESSAGE("Testing flat interpolation before the first spreaded date...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.03);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 8,  Months));
    spreadDates.push_back(vars.calendar.advance(vars.today, 15, Months));

    Date interpolationDate = vars.calendar.advance(vars.today, 6, Months);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<PiecewiseZeroSpreadedTermStructure>(
                           Handle<YieldTermStructure>(vars.termStructure),
                           spreads, spreadDates);

    Time t = vars.dayCount.yearFraction(vars.today, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);

    Real tolerance = 1e-9;
    Real expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                        spread1->value();

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

void PiecewiseZeroSpreadedTermStructureTest::testFlatInterpolationRight() {

    BOOST_MESSAGE("Testing flat interpolation after the last spreaded date...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.03);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 8,  Months));
    spreadDates.push_back(vars.calendar.advance(vars.today, 15, Months));

    Date interpolationDate = vars.calendar.advance(vars.today, 20, Months);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<PiecewiseZeroSpreadedTermStructure>(
                           Handle<YieldTermStructure>(vars.termStructure),
                           spreads, spreadDates);
    spreadedTermStructure->enableExtrapolation();

    Time t = vars.dayCount.yearFraction(vars.today, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);

    Real tolerance = 1e-9;
    Real expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                        spread2->value();

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

void PiecewiseZeroSpreadedTermStructureTest::testLinearInterpolationMultipleSpreads() {

    BOOST_MESSAGE("Testing linear interpolation with more than two spreaded dates...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread3 = boost::make_shared<SimpleQuote>(0.035);
    boost::shared_ptr<SimpleQuote> spread4 = boost::make_shared<SimpleQuote>(0.04);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));
    spreads.push_back(Handle<Quote>(spread3));
    spreads.push_back(Handle<Quote>(spread4));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 90,  Days));
    spreadDates.push_back(vars.calendar.advance(vars.today, 150, Days));
    spreadDates.push_back(vars.calendar.advance(vars.today, 30, Months));
    spreadDates.push_back(vars.calendar.advance(vars.today, 40, Months));

    Date interpolationDate = vars.calendar.advance(vars.today, 120, Days);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<PiecewiseZeroSpreadedTermStructure>(
                           Handle<YieldTermStructure>(vars.termStructure),
                           spreads, spreadDates);

    Time t = vars.dayCount.yearFraction(vars.today, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);

    Real tolerance = 1e-9;
    Real expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                        spread1->value();

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

void PiecewiseZeroSpreadedTermStructureTest::testLinearInterpolation() {

    BOOST_MESSAGE("Testing linear interpolation between two dates...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.03);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 100,  Days));
    spreadDates.push_back(vars.calendar.advance(vars.today, 150, Days));

    Date interpolationDate = vars.calendar.advance(vars.today, 120, Days);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<InterpolatedPiecewiseZeroSpreadedTermStructure<Linear> >(
                        Handle<YieldTermStructure>(vars.termStructure),
                        spreads, spreadDates);

    Date d0 = vars.calendar.advance(vars.today, 100,  Days);
    Date d1 = vars.calendar.advance(vars.today, 150,  Days);
    Date d2 = vars.calendar.advance(vars.today, 120,  Days);

    Real m = (0.03-0.02)/vars.dayCount.yearFraction(d0,d1);
    Real expectedRate = m * vars.dayCount.yearFraction(d0, d2) + 0.054;

    Time t = vars.dayCount.yearFraction(vars.settlementDate, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);

    Real tolerance = 1e-9;

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

void PiecewiseZeroSpreadedTermStructureTest::testForwardFlatInterpolation() {

    BOOST_MESSAGE("Testing forward flat interpolation between two dates...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.03);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 75,  Days));
    spreadDates.push_back(vars.calendar.advance(vars.today, 260, Days));

    Date interpolationDate = vars.calendar.advance(vars.today, 100, Days);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<InterpolatedPiecewiseZeroSpreadedTermStructure<ForwardFlat> >(
                        Handle<YieldTermStructure>(vars.termStructure),
                        spreads, spreadDates);

    Time t = vars.dayCount.yearFraction(vars.today, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);

    Real tolerance = 1e-9;
    Real expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                        spread1->value();

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

void PiecewiseZeroSpreadedTermStructureTest::testBackwardFlatInterpolation() {

    BOOST_MESSAGE("Testing backward flat interpolation between two dates...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.03);
    boost::shared_ptr<SimpleQuote> spread3 = boost::make_shared<SimpleQuote>(0.04);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));
    spreads.push_back(Handle<Quote>(spread3));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 100,  Days));
    spreadDates.push_back(vars.calendar.advance(vars.today, 200, Days));
    spreadDates.push_back(vars.calendar.advance(vars.today, 300, Days));

    Date interpolationDate = vars.calendar.advance(vars.today, 110, Days);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<InterpolatedPiecewiseZeroSpreadedTermStructure<BackwardFlat> >(
                        Handle<YieldTermStructure>(vars.termStructure),
                        spreads, spreadDates);

    Time t = vars.dayCount.yearFraction(vars.today, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);

    Real tolerance = 1e-9;
    Real expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                        spread2->value();

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

void PiecewiseZeroSpreadedTermStructureTest::testDefaultInterpolation() {

    BOOST_MESSAGE("Testing default interpolation between two dates...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.02);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 75,  Days));
    spreadDates.push_back(vars.calendar.advance(vars.today, 160, Days));

    Date interpolationDate = vars.calendar.advance(vars.today, 100, Days);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<PiecewiseZeroSpreadedTermStructure>(
                               Handle<YieldTermStructure>(vars.termStructure),
                               spreads, spreadDates);

    Time t = vars.dayCount.yearFraction(vars.today, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);

    Real tolerance = 1e-9;
    Real expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                        spread1->value();

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

void PiecewiseZeroSpreadedTermStructureTest::testSetInterpolationFactory() {

    BOOST_MESSAGE("Testing factory constructor with additional parameters...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.03);
    boost::shared_ptr<SimpleQuote> spread3 = boost::make_shared<SimpleQuote>(0.01);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));
    spreads.push_back(Handle<Quote>(spread3));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 8,  Months));
    spreadDates.push_back(vars.calendar.advance(vars.today, 15, Months));
    spreadDates.push_back(vars.calendar.advance(vars.today, 25, Months));

    Date interpolationDate = vars.calendar.advance(vars.today, 11, Months);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure;

    Frequency freq = NoFrequency;

    Cubic factory;
    factory = Cubic(CubicInterpolation::Spline, false);

    spreadedTermStructure =
        boost::make_shared<InterpolatedPiecewiseZeroSpreadedTermStructure<Cubic> >(
                               Handle<YieldTermStructure>(vars.termStructure),
                               spreads, spreadDates, vars.compounding,
                               freq, vars.dayCount,factory);

    Time t = vars.dayCount.yearFraction(vars.today, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);

    Real tolerance = 1e-9;
    Real expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                        0.026065770863;

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

void PiecewiseZeroSpreadedTermStructureTest::testMaxDate() {

    BOOST_MESSAGE("Testing term structure max date...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.03);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 8,  Months));
    spreadDates.push_back(vars.calendar.advance(vars.today, 15, Months));

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<PiecewiseZeroSpreadedTermStructure>(
                        Handle<YieldTermStructure>(vars.termStructure),
                        spreads, spreadDates);

    Date maxDate = spreadedTermStructure->maxDate();

    Date expectedDate =
        std::min(vars.termStructure->maxDate(), spreadDates.back());

    if (maxDate != expectedDate)
        BOOST_ERROR(
            "unable to reproduce max date\n"
            << "    calculated: " << maxDate << "\n"
            << "    expected: "   << expectedDate);

}

void PiecewiseZeroSpreadedTermStructureTest::testQuoteChanging() {

    BOOST_MESSAGE("Testing quote update...");

    CommonVars vars;

    std::vector<Handle<Quote> > spreads;
    boost::shared_ptr<SimpleQuote> spread1 = boost::make_shared<SimpleQuote>(0.02);
    boost::shared_ptr<SimpleQuote> spread2 = boost::make_shared<SimpleQuote>(0.03);
    spreads.push_back(Handle<Quote>(spread1));
    spreads.push_back(Handle<Quote>(spread2));

    std::vector<Date> spreadDates;
    spreadDates.push_back(vars.calendar.advance(vars.today, 100,  Days));
    spreadDates.push_back(vars.calendar.advance(vars.today, 150, Days));

    Date interpolationDate = vars.calendar.advance(vars.today, 120, Days);

    boost::shared_ptr<ZeroYieldStructure> spreadedTermStructure =
        boost::make_shared<InterpolatedPiecewiseZeroSpreadedTermStructure<BackwardFlat> >(
                        Handle<YieldTermStructure>(vars.termStructure),
                        spreads, spreadDates);

    Time t = vars.dayCount.yearFraction(vars.settlementDate, interpolationDate);
    Rate interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);
    Real tolerance = 1e-9;
    Real expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                        0.03;

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

    spread2->setValue(0.025);

    interpolatedZeroRate = spreadedTermStructure->zeroRate(t,vars.compounding);
    expectedRate = vars.termStructure->zeroRate(t,vars.compounding) +
                   0.025;

    if (std::fabs(interpolatedZeroRate - expectedRate) > tolerance)
        BOOST_ERROR(
            "unable to reproduce interpolated rate\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(interpolatedZeroRate) << "\n"
            << "    expected: "   << io::rate(expectedRate));

}

test_suite* PiecewiseZeroSpreadedTermStructureTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Interpolated piecewise zero spreaded yield curve tests");
    suite->add(QUANTLIB_TEST_CASE(
        &PiecewiseZeroSpreadedTermStructureTest::testFlatInterpolationLeft));
    suite->add(QUANTLIB_TEST_CASE(
        &PiecewiseZeroSpreadedTermStructureTest::testFlatInterpolationRight));
    suite->add(QUANTLIB_TEST_CASE(
        &PiecewiseZeroSpreadedTermStructureTest::testLinearInterpolationMultipleSpreads));
    suite->add(QUANTLIB_TEST_CASE(
        &PiecewiseZeroSpreadedTermStructureTest::testLinearInterpolation));
    suite->add(QUANTLIB_TEST_CASE(
        &PiecewiseZeroSpreadedTermStructureTest::testBackwardFlatInterpolation));
    suite->add(QUANTLIB_TEST_CASE(
        &PiecewiseZeroSpreadedTermStructureTest::testForwardFlatInterpolation));
    suite->add(QUANTLIB_TEST_CASE(
        &PiecewiseZeroSpreadedTermStructureTest::testDefaultInterpolation));
    suite->add(QUANTLIB_TEST_CASE(
        &PiecewiseZeroSpreadedTermStructureTest::testSetInterpolationFactory));
    suite->add(QUANTLIB_TEST_CASE(&PiecewiseZeroSpreadedTermStructureTest::testMaxDate));
    suite->add(QUANTLIB_TEST_CASE(&PiecewiseZeroSpreadedTermStructureTest::testQuoteChanging));
    return suite;
}

