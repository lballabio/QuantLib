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
        Integer n;
        TimeUnit units;
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

            std::vector<Datum> swapData;
            swapData.push_back({1, Years, 0.0454});
            swapData.push_back({5, Years, 0.0499});
            swapData.push_back({10, Years, 0.0547});
            swapData.push_back({20, Years, 0.0589});
            swapData.push_back({30, Years, 0.0596});

            ufrRate = ext::shared_ptr<Quote>(new SimpleQuote(0.023));

            today = calendar.adjust(Date(31, December, 2018));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            Size swaps = LENGTH(swapData);
            std::vector<ext::shared_ptr<RateHelper> > instruments(swaps);

            for (Size i = 0; i < swaps; ++i) {
                instruments[i] = ext::shared_ptr<RateHelper>(new SwapRateHelper(
                    swapData[i].rate, swapData[i].n * swapData[i].units, calendar, fixedFrequency,
                    businessConvention, dayCount, index));
            }
            ext::shared_ptr<YieldTermStructure> ftkTermStructure(
                new PiecewiseYieldCurve<Discount, LogLinear>(settlement, instruments, dayCount));
            ftkTermStructure->enableExtrapolation();
            ftkTermStructureHandle.linkTo(ftkTermStructure);
        }
    };

    ext::shared_ptr<Quote> calculateLLFR(const Handle<YieldTermStructure>& t,
                                         const Period& fsp,
                                         const std::vector<LLFRWeight>& weights,
                                         Real omega) {
        Real llfr;
        DayCounter dc = t->dayCounter();
        Date ref = t->referenceDate();
        Calendar cal = t->calendar();
        Date fspDt = cal.advance(ref, fsp);
        Time timeToFsp = dc.yearFraction(ref, fspDt);

        for (Size i = 0; i < weights.size(); i++) {
            LLFRWeight w = weights[i];
            Date forwardDt = cal.advance(fspDt, w.n * w.units);
            Real timeToForward = dc.yearFraction(fspDt, forwardDt);
            llfr += w.weight * t->forwardRate(timeToFsp, timeToForward, Continuous, NoFrequency);
        }
        return ext::shared_ptr<Quote>(new SimpleQuote(omega * llfr));
    }
}

void UFRTermStructureTest::testDnbReplication() {

    BOOST_TEST_MESSAGE("Testing DNB replication of UFR zero annually compounded rates...");

    using namespace ufr_term_structure_test;

    CommonVars vars;

    std::vector<LLFRWeight> llfrWeights;
    llfrWeights.push_back({5, Years, 1.});
    llfrWeights.push_back({10, Years, .5});
    llfrWeights.push_back({20, Years, .25});
    llfrWeights.push_back({30, Years, .125});

    Real omega = 8. / 15.;

    Period fsp = 20 * Years;

    ext::shared_ptr<Quote> llfr =
        calculateLLFR(vars.ftkTermStructureHandle, fsp, llfrWeights, omega);

    Real alpha = 0.1;

    ext::shared_ptr<YieldTermStructure> ufrTs(new UFRTermStructure(
        vars.ftkTermStructureHandle, Handle<Quote>(llfr), Handle<Quote>(vars.ufrRate), fsp, alpha));

    std::vector<Datum> expZeroes;
    expZeroes.push_back({1, Years, -0.00235});
    expZeroes.push_back({5, Years, 0.00196});
    expZeroes.push_back({10, Years, 0.00828});
    expZeroes.push_back({20, Years, 0.01357});
    expZeroes.push_back({30, Years, 0.01509});
    expZeroes.push_back({50, Years, 0.01776});
    expZeroes.push_back({60, Years, 0.01859});
    expZeroes.push_back({70, Years, 0.0192});
    expZeroes.push_back({80, Years, 0.01967});
    expZeroes.push_back({90, Years, 0.02004});
    expZeroes.push_back({100, Years, 0.02034});

    Real tolerance = 1.0e-5;

    for (Size i = 0; i < expZeroes.size(); ++i) {
        Period p = expZeroes[i].n * expZeroes[i].units;

        Date maturity = vars.settlement + p;
        Rate actual = ufrTs->zeroRate(maturity, vars.dayCount, Compounded, Annual).rate();
        Rate expected = expZeroes[i].rate;

        if (std::fabs(actual - expected) > tolerance)
            BOOST_ERROR(
                "unable to reproduce zero yield rate from the UFR curve\n"
                << std::fixed << std::setprecision(10)
                << "    calculated: " << actual << "\n"
                << "    expected:   " << expected);
    }
}