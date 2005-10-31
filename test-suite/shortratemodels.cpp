/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "shortratemodels.hpp"
#include "utilities.hpp"
#include <ql/ShortRateModels/OneFactorModels/hullwhite.hpp>
#include <ql/ShortRateModels/CalibrationHelpers/swaptionhelper.hpp>
#include <ql/PricingEngines/Swaption/all.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Indexes/indexmanager.hpp>
#include <ql/DayCounters/all.hpp>
#include <ql/Optimization/simplex.hpp>
#include <ql/TermStructures/discountcurve.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(ShortRateModelTest)

struct CalibrationData {
    Integer start;
    Integer length;
    Volatility volatility;
};

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(ShortRateModelTest)


void ShortRateModelTest::testCachedHullWhite() {
    #if !defined(QL_PATCH_BORLAND)

    BOOST_MESSAGE("Testing Hull-White calibration against cached values...");

    QL_TEST_BEGIN

    Date today(15, February, 2002);
    Date settlement(19, February, 2002);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> termStructure;
    termStructure.linkTo(flatRate(settlement,0.04875825,
                                  Actual365Fixed()));

    boost::shared_ptr<HullWhite> model(new HullWhite(termStructure));

    CalibrationData data[] = {{ 1, 5, 0.1148 },
                              { 2, 4, 0.1108 },
                              { 3, 3, 0.1070 },
                              { 4, 2, 0.1021 },
                              { 5, 1, 0.1000 }};

    boost::shared_ptr<Xibor> index(new Euribor(6, Months, termStructure));

    std::vector<boost::shared_ptr<CalibrationHelper> > swaptions;
    for (Size i=0; i<LENGTH(data); i++) {
        boost::shared_ptr<Quote> vol(new SimpleQuote(data[i].volatility));
        boost::shared_ptr<CalibrationHelper> helper(
                             new SwaptionHelper(Period(data[i].start, Years),
                                                Period(data[i].length, Years),
                                                Handle<Quote>(vol),
                                                index,
                                                Annual, Thirty360(),
                                                termStructure));
        helper->setPricingEngine(boost::shared_ptr<PricingEngine>(
                                        new JamshidianSwaptionEngine(model)));
        swaptions.push_back(helper);
    }

    Real lambda = 0.5;
    Simplex simplex(lambda, 1e-9);
    simplex.setEndCriteria(EndCriteria(10000, 1e-7));
    model->calibrate(swaptions, simplex);

    #if defined(QL_USE_INDEXED_COUPON)
    Real cachedA = 0.0484921, cachedSigma = 0.00591608;
    #else
    Real cachedA = 0.0458489, cachedSigma = 0.00582708;
    #endif
    Real tolerance = 1.0e-6;

    if (std::fabs(model->params()[0]-cachedA) > tolerance
        || std::fabs(model->params()[1]-cachedSigma) > tolerance) {
        BOOST_ERROR("Failed to reproduce cached calibration results:\n"
                    << "calculated: a = " << model->params()[0] << ", "
                    << "sigma = " << model->params()[1] << "\n"
                    << "expected:   a = " << cachedA << ", "
                    << "sigma = " << cachedSigma);
    }

    QL_TEST_TEARDOWN
    #endif
}


void ShortRateModelTest::testSwaps() {
    #if !defined(QL_PATCH_BORLAND)

    BOOST_MESSAGE("Testing Hull-White swap pricing against known values...");

    QL_TEST_BEGIN

    Date today = Settings::instance().evaluationDate();
    Calendar calendar = TARGET();
    Date settlement = calendar.advance(today,2,Days);

    Date dates[] = {
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
    DiscountFactor discounts[] = {
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

    Handle<YieldTermStructure> termStructure;
    termStructure.linkTo(boost::shared_ptr<YieldTermStructure>(
       new DiscountCurve(
           std::vector<Date>(dates,dates+LENGTH(dates)),
           std::vector<DiscountFactor>(discounts,discounts+LENGTH(discounts)),
           Actual365Fixed())));

    boost::shared_ptr<HullWhite> model(new HullWhite(termStructure));

    Integer start[] = { -3, 0, 3 };
    Integer length[] = { 2, 5, 10 };
    Rate rates[] = { 0.02, 0.04, 0.06 };
    boost::shared_ptr<Xibor> euribor(new Euribor(6,Months,termStructure));

    boost::shared_ptr<PricingEngine> engine(
                                         new TreeSimpleSwapEngine(model,120));

    Real tolerance = 1.0e-9;
    for (Size i=0; i<LENGTH(start); i++) {

        Date startDate = calendar.advance(settlement,start[i],Months);
        if (startDate < today) {
            Date fixingDate = calendar.advance(startDate,-2,Days);
            History pastFixings(std::vector<Date>(1,fixingDate),
                                std::vector<Rate>(1,0.03));
            IndexManager::instance().setHistory(euribor->name(),
                                                pastFixings);
        }

        for (Size j=0; j<LENGTH(length); j++) {

            Date maturity = calendar.advance(startDate,length[i],Years);
            Schedule fixedSchedule(calendar,startDate,maturity,
                                   Annual,Unadjusted);
            Schedule floatSchedule(calendar,startDate,maturity,
                                   Semiannual,Following);

            for (Size k=0; k<LENGTH(rates); k++) {

                SimpleSwap swap(true, 1000000.0,
                                fixedSchedule, rates[k], Thirty360(),
                                floatSchedule, euribor, 2, 0.0,
                                termStructure);
                Real expected = swap.NPV();
                swap.setPricingEngine(engine);
                Real calculated = swap.NPV();

                Real error = std::fabs(expected-calculated);
                if (error > tolerance) {
                    BOOST_ERROR("Failed to reproduce swap NPV:"
                                << QL_FIXED << std::setprecision(9)
                                << "\n    calculated: " << calculated
                                << "\n    expected:   " << expected
                                << QL_SCIENTIFIC
                                << "\n    error:      " << error);
                }
            }
        }
    }

    QL_TEST_TEARDOWN
    #endif
}


test_suite* ShortRateModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Short-rate model tests");
    #if !defined(QL_PATCH_BORLAND)
    suite->add(BOOST_TEST_CASE(&ShortRateModelTest::testCachedHullWhite));
    suite->add(BOOST_TEST_CASE(&ShortRateModelTest::testSwaps));
    #endif
    return suite;
}

