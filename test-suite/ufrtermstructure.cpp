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
    struct ZeroRatePoint {
        Date date;
        Rate rate;
    };

    ZeroRatePoint initZeroRate(Date d, Rate r) {
        ZeroRatePoint z;
        z.date = d;
        z.rate = r;
        return z;
    }

    struct Datum {
        Integer n;
        TimeUnit units;
        Rate rate;
    };

    Datum initDatum(Integer n, TimeUnit u, Rate r) {
        Datum d;
        d.n = n;
        d.units = u;
        d.rate = r;
        return d;
    }

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
        Date today, settlement;
        Calendar calendar;
        DayCounter dayCount;

        Natural settlementDays;
        RelinkableHandle<YieldTermStructure> ftkTermStructureHandle;

        ext::shared_ptr<Quote> ufrRate;

        SavedSettings backup;

        CommonVars() {
            settlementDays = 2;
            dayCount = SimpleDayCounter();
            calendar = NullCalendar();

            std::vector<ZeroRatePoint> zeroData;
            zeroData.push_back(initZeroRate(Date(2, January, 2019), -0.00235176));
            zeroData.push_back(initZeroRate(Date(2, January, 2020), -0.00235176));
            zeroData.push_back(initZeroRate(Date(2, January, 2021), -0.00178209));
            zeroData.push_back(initZeroRate(Date(2, January, 2022), -0.000921391));
            zeroData.push_back(initZeroRate(Date(2, January, 2023), 0.000339853));
            zeroData.push_back(initZeroRate(Date(2, January, 2024), 0.00195862));
            zeroData.push_back(initZeroRate(Date(2, January, 2025), 0.00323966));
            zeroData.push_back(initZeroRate(Date(2, January, 2026), 0.00457807));
            zeroData.push_back(initZeroRate(Date(2, January, 2027), 0.00585332));
            zeroData.push_back(initZeroRate(Date(2, January, 2028), 0.0070809));
            zeroData.push_back(initZeroRate(Date(2, January, 2029), 0.00824992));
            zeroData.push_back(initZeroRate(Date(2, January, 2031), 0.00991469));
            zeroData.push_back(initZeroRate(Date(2, January, 2034), 0.0118056));
            zeroData.push_back(initZeroRate(Date(2, January, 2039), 0.0134759));
            zeroData.push_back(initZeroRate(Date(2, January, 2044), 0.013918));
            zeroData.push_back(initZeroRate(Date(2, January, 2049), 0.0140356));
            zeroData.push_back(initZeroRate(Date(2, January, 2059), 0.0138101));
            zeroData.push_back(initZeroRate(Date(2, January, 2069), 0.0135371));

            InterestRate ufr(0.023, dayCount, Compounded, Annual);
            ufrRate = ext::shared_ptr<Quote>(
                new SimpleQuote(ufr.equivalentRate(Continuous, Annual, 1.0)));

            today = calendar.adjust(Date(31, December, 2018));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            std::vector<Date> nomD;
            std::vector<Rate> nomR;
            for (Size i = 0; i < zeroData.size(); i++) {
                nomD.push_back(zeroData[i].date);
                nomR.push_back(zeroData[i].rate);
            }
            ext::shared_ptr<YieldTermStructure> ftkTermStructure(
                new InterpolatedZeroCurve<Linear>(nomD, nomR, dayCount));
            ftkTermStructure->enableExtrapolation();

            ftkTermStructureHandle.linkTo(ftkTermStructure);
        }
    };

    ext::shared_ptr<Quote> calculateLLFR(const Handle<YieldTermStructure>& ts,
                                         Time fsp,
                                         const std::vector<LLFRWeight>& weights,
                                         Real omega) {
        Real llfr;
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

    ext::shared_ptr<Quote> llfr =
        calculateLLFR(vars.ftkTermStructureHandle, fsp, llfrWeights, omega);

    Real alpha = 0.1;

    ext::shared_ptr<YieldTermStructure> ufrTs(new UFRTermStructure(
        vars.ftkTermStructureHandle, Handle<Quote>(llfr), Handle<Quote>(vars.ufrRate), fsp, alpha));

    std::vector<Datum> expectedZeroes;
    expectedZeroes.push_back(initDatum(1, Years, -0.00235));
    expectedZeroes.push_back(initDatum(5, Years, 0.00196));
    expectedZeroes.push_back(initDatum(10, Years, 0.00828));
    expectedZeroes.push_back(initDatum(20, Years, 0.01357));
    expectedZeroes.push_back(initDatum(30, Years, 0.01509));
    expectedZeroes.push_back(initDatum(50, Years, 0.01776));
    expectedZeroes.push_back(initDatum(60, Years, 0.01859));
    expectedZeroes.push_back(initDatum(70, Years, 0.0192));
    expectedZeroes.push_back(initDatum(80, Years, 0.01967));
    expectedZeroes.push_back(initDatum(90, Years, 0.02004));
    expectedZeroes.push_back(initDatum(100, Years, 0.02034));

    Real tolerance = 1.0e-5;

    for (Size i = 0; i < expectedZeroes.size(); ++i) {
        Datum z = expectedZeroes[i];
        Period p = Period(z.n, z.units);

        Date maturity = vars.settlement + p;
        Rate actual = ufrTs->zeroRate(maturity, vars.dayCount, Compounded, Annual).rate();
        Rate expected = z.rate;

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