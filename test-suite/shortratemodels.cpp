
/*
 Copyright (C) 2005 StatPro Italia srl

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

#include "shortratemodels.hpp"
#include "utilities.hpp"
#include <ql/ShortRateModels/OneFactorModels/hullwhite.hpp>
#include <ql/ShortRateModels/CalibrationHelpers/swaptionhelper.hpp>
#include <ql/PricingEngines/Swaption/jamshidianswaptionengine.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Optimization/simplex.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct CalibrationData {
        Integer start;
        Integer length;
        Volatility volatility;
    };

    void teardown() {
        Settings::instance().setEvaluationDate(Date());
    }

}

void ShortRateModelTest::testCachedHullWhite() {

    BOOST_MESSAGE("Testing Hull-White calibration against cached values...");

    QL_TEST_BEGIN

    Date today(15, February, 2002);
    Date settlement(19, February, 2002);
    Settings::instance().setEvaluationDate(today);

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
                                                termStructure));
        helper->setPricingEngine(boost::shared_ptr<PricingEngine>(
                                        new JamshidianSwaptionEngine(model)));
        swaptions.push_back(helper);
    }

    Real lambda = 0.5;
    Simplex simplex(lambda, 1e-9);
    simplex.setEndCriteria(EndCriteria(10000, 1e-7));
    model->calibrate(swaptions, simplex);

    Real cachedA = 0.0486909, cachedSigma = 0.00589026;
    Real tolerance = 1.0e-6;

    if (std::fabs(model->params()[0]-cachedA) > tolerance
        || std::fabs(model->params()[1]-cachedSigma) > tolerance) {
        BOOST_FAIL("Failed to reproduce cached calibration results:\n"
                   << "calculated: a = " << model->params()[0] << ", "
                   << "sigma = " << model->params()[1] << "\n"
                   << "expected:   a = " << cachedA << ", "
                   << "sigma = " << cachedSigma);
    }

    QL_TEST_TEARDOWN
}


test_suite* ShortRateModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Short-rate model tests");
    suite->add(BOOST_TEST_CASE(&ShortRateModelTest::testCachedHullWhite));
    return suite;
}

