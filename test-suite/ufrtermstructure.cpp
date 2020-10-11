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

    LLFRWeight initLlfrWeight(Time ttm, Real w) {
        LLFRWeight d;
        d.ttm = ttm;
        d.weight = w;
        return d;
    }

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

            std::vector<Datum> swapData;
            // Data source: https://fred.stlouisfed.org/
            swapData.push_back({1, Years, -0.00315});
            swapData.push_back({2, Years, -0.00205});
            swapData.push_back({3, Years, -0.00144});
            swapData.push_back({4, Years, -0.00068});
            swapData.push_back({5, Years, 0.00014});
            swapData.push_back({6, Years, 0.00103});
            swapData.push_back({7, Years, 0.00194});
            swapData.push_back({8, Years, 0.00288});
            swapData.push_back({9, Years, 0.00381});
            swapData.push_back({10, Years, 0.00471});
            swapData.push_back({12, Years, 0.0063});
            swapData.push_back({15, Years, 0.00808});
            swapData.push_back({20, Years, 0.00973});
            swapData.push_back({25, Years, 0.01035});
            swapData.push_back({30, Years, 0.01055});
            swapData.push_back({40, Years, 0.0103});
            swapData.push_back({50, Years, 0.0103});

            InterestRate ufr(0.023, dayCount, Compounded, Annual);
            ufrRate = ext::shared_ptr<Quote>(
                new SimpleQuote(ufr.equivalentRate(Continuous, Annual, 1.0)));

            today = calendar.adjust(Date(29, March, 2019));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            Size swaps = LENGTH(swapData);
            std::vector<ext::shared_ptr<RateHelper> > instruments;
            for (std::vector<Datum>::iterator it = swapData.begin(); it != swapData.end(); ++it) {
                instruments.push_back(ext::shared_ptr<RateHelper>(
                    new SwapRateHelper((*it).rate, Period((*it).n, (*it).units), calendar,
                                       fixedFrequency, businessConvention, dayCount, index)));
            }

            ext::shared_ptr<YieldTermStructure> ftkTermStructure(
                new PiecewiseYieldCurve<Discount, LogLinear>(settlement, instruments, dayCount));
            ftkTermStructure->enableExtrapolation();
            ftkTermStructureHandle.linkTo(ftkTermStructure);
        }
    };

    ext::shared_ptr<Quote> calculateLLFR(const Handle<YieldTermStructure>& ts,
                                         Time fsp,
                                         const std::vector<LLFRWeight>& weights,
                                         Real omega) {
        Rate llfr = 0.0;
        DayCounter dc = ts->dayCounter();
        Date ref = ts->referenceDate();

        for (Size i = 0; i < weights.size(); i++) {
            LLFRWeight w = weights[i];
            llfr += w.weight * ts->forwardRate(fsp, w.ttm, Continuous, NoFrequency, true);
        }
        return ext::shared_ptr<Quote>(new SimpleQuote(omega * llfr));
    }
}

void UFRTermStructureTest::testDnbReplication() {
    BOOST_TEST_MESSAGE("Testing DNB replication of UFR zero annually compounded rates...");

    using namespace ufr_term_structure_test;

    CommonVars vars;

    std::vector<LLFRWeight> llfrWeights;
    llfrWeights.push_back(initLlfrWeight(25.0, 1.0));
    llfrWeights.push_back(initLlfrWeight(30.0, 0.5));
    llfrWeights.push_back(initLlfrWeight(40.0, 0.25));
    llfrWeights.push_back(initLlfrWeight(50.0, 0.125));

    Real omega = 8.0 / 15.0;
    Time fsp = 20.0;
    Real alpha = 0.1;

    ext::shared_ptr<Quote> llfr =
        calculateLLFR(vars.ftkTermStructureHandle, fsp, llfrWeights, omega);

    ext::shared_ptr<YieldTermStructure> ufrTs(new UFRTermStructure(
        vars.ftkTermStructureHandle, Handle<Quote>(llfr), Handle<Quote>(vars.ufrRate), fsp, alpha));

    std::vector<Datum> expZeroes;
    expZeroes.push_back({10, Years, 0.00477});
    expZeroes.push_back({20, Years, 0.01004});
    expZeroes.push_back({30, Years, 0.01223});
    expZeroes.push_back({40, Years, 0.01433});
    expZeroes.push_back({50, Years, 0.01589});
    expZeroes.push_back({60, Years, 0.01702});
    expZeroes.push_back({70, Years, 0.01785});
    expZeroes.push_back({80, Years, 0.01849});
    expZeroes.push_back({90, Years, 0.01899});
    expZeroes.push_back({100, Years, 0.01939});

    Real tolerance = 1.0e-4;

    for (Size i = 0; i < expZeroes.size(); ++i) {
        Period p = expZeroes[i].n * expZeroes[i].units;

        Date maturity = vars.settlement + p;
        Rate actual = ufrTs->zeroRate(maturity, vars.dayCount, Compounded, Annual).rate();
        Rate expected = expZeroes[i].rate;

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