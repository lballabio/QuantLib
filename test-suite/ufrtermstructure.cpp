/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2020 Marcin Rybacki

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

#include "ufrtermstructure.hpp"
#include "utilities.hpp"
#include <ql/currencies/europe.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/ufrtermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace ufr_term_structure_test {

    struct Datum {
        Integer n;
        TimeUnit units;
        Rate rate;
    };

    struct LLFRWeight {
        Time ttm;
        Real weight;
    };

    struct CommonVars {
        // global data
        Date today, settlement;
        Calendar calendar;
        Currency ccy;
        BusinessDayConvention businessConvention;
        DayCounter dayCount;
        Frequency fixedFrequency;
        Period floatingTenor;
        ext::shared_ptr<IborIndex> index;
        Natural settlementDays;
        RelinkableHandle<YieldTermStructure> ftkTermStructureHandle;
        ext::shared_ptr<Quote> ufrRate;

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars() {
            settlementDays = 2;
            businessConvention = Unadjusted;
            dayCount = SimpleDayCounter();
            calendar = NullCalendar();
            ccy = EURCurrency();
            fixedFrequency = Annual;
            floatingTenor = 6 * Months;

            index = ext::shared_ptr<IborIndex>(
                new IborIndex("FTK_IDX", floatingTenor, settlementDays, ccy, calendar,
                              businessConvention, false, dayCount, ftkTermStructureHandle));

            // Data source: https://fred.stlouisfed.org/
            Datum swapData[] = {{1, Years, -0.00315}, {2, Years, -0.00205}, {3, Years, -0.00144},
                                {4, Years, -0.00068}, {5, Years, 0.00014},  {6, Years, 0.00103},
                                {7, Years, 0.00194},  {8, Years, 0.00288},  {9, Years, 0.00381},
                                {10, Years, 0.00471}, {12, Years, 0.0063},  {15, Years, 0.00808},
                                {20, Years, 0.00973}, {25, Years, 0.01035}, {30, Years, 0.01055},
                                {40, Years, 0.0103},  {50, Years, 0.0103}};

            InterestRate ufr(0.023, dayCount, Compounded, Annual);
            ufrRate = ext::shared_ptr<Quote>(
                new SimpleQuote(ufr.equivalentRate(Continuous, Annual, 1.0)));

            today = calendar.adjust(Date(29, March, 2019));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            Size nInstruments = LENGTH(swapData);
            std::vector<ext::shared_ptr<RateHelper> > instruments(nInstruments);
            for (Size i = 0; i < nInstruments; i++) {
                instruments[i] = ext::shared_ptr<RateHelper>(new SwapRateHelper(
                    swapData[i].rate, Period(swapData[i].n, swapData[i].units), calendar,
                    fixedFrequency, businessConvention, dayCount, index));
            }

            ext::shared_ptr<YieldTermStructure> ftkTermStructure(
                new PiecewiseYieldCurve<Discount, LogLinear>(settlement, instruments, dayCount));
            ftkTermStructure->enableExtrapolation();
            ftkTermStructureHandle.linkTo(ftkTermStructure);
        }
    };

    ext::shared_ptr<Quote> calculateLLFR(const Handle<YieldTermStructure>& ts, Time fsp) {
        DayCounter dc = ts->dayCounter();
        Date ref = ts->referenceDate();
        Real omega = 8.0 / 15.0;

        LLFRWeight llfrWeights[] = {{25.0, 1.0}, {30.0, 0.5}, {40.0, 0.25}, {50.0, 0.125}};
        Size nWeights = LENGTH(llfrWeights);

        Rate llfr = 0.0;
        for (Size j = 0; j < nWeights; j++) {
            LLFRWeight w = llfrWeights[j];
            llfr += w.weight * ts->forwardRate(fsp, w.ttm, Continuous, NoFrequency, true);
        }
        return ext::shared_ptr<Quote>(new SimpleQuote(omega * llfr));
    }
}

void UFRTermStructureTest::testDnbReplication() {
    BOOST_TEST_MESSAGE("Testing DNB replication of UFR zero annually compounded rates...");

    using namespace ufr_term_structure_test;

    CommonVars vars;

    Time fsp = 20.0;
    Real alpha = 0.1;

    ext::shared_ptr<Quote> llfr = calculateLLFR(vars.ftkTermStructureHandle, fsp);

    ext::shared_ptr<YieldTermStructure> ufrTs(new UFRTermStructure(
        vars.ftkTermStructureHandle, Handle<Quote>(llfr), Handle<Quote>(vars.ufrRate), fsp, alpha));

    Datum expectedZeroes[] = {{10, Years, 0.00477}, {20, Years, 0.01004}, {30, Years, 0.01223},
                         {40, Years, 0.01433}, {50, Years, 0.01589}, {60, Years, 0.01702},
                         {70, Years, 0.01785}, {80, Years, 0.01849}, {90, Years, 0.01899},
                         {100, Years, 0.01939}};

    Real tolerance = 1.0e-4;
    Size nRates = LENGTH(expectedZeroes);

    for (Size k = 0; k < nRates; ++k) {
        Period p = expectedZeroes[k].n * expectedZeroes[k].units;
        Date maturity = vars.settlement + p;
        
        Rate actual = ufrTs->zeroRate(maturity, vars.dayCount, Compounded, Annual).rate();
        Rate expected = expectedZeroes[k].rate;

        if (std::fabs(actual - expected) > tolerance)
            BOOST_ERROR("unable to reproduce zero yield rate from the UFR curve\n"
                        << std::setprecision(10) << "    calculated: " << actual << "\n"
                        << "    expected:   " << expected << "\n"
                        << "    tenor:       " << p << "\n");
    }
}

test_suite* UFRTermStructureTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("UFR term structure tests");

    suite->add(QUANTLIB_TEST_CASE(&UFRTermStructureTest::testDnbReplication));

    return suite;
}