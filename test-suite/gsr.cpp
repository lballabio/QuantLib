/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/processes/gsrprocess.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/instruments/nonstandardswap.hpp>
#include <ql/instruments/nonstandardswaption.hpp>
#include <ql/pricingengines/swaption/gaussian1dswaptionengine.hpp>
#include <ql/pricingengines/swaption/gaussian1djamshidianswaptionengine.hpp>
#include <ql/pricingengines/swaption/gaussian1dnonstandardswaptionengine.hpp>
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;

using std::fabs;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(GsrTests)

BOOST_AUTO_TEST_CASE(testGsrProcess) {

    BOOST_TEST_MESSAGE("Testing GSR process...");

    Date refDate = Settings::instance().evaluationDate();

    // constant reversion, constant volatility, test conditional expectation and
    // variance against
    // existing HullWhiteForwardProcess
    // technically we test two representations of the same constant reversion
    // and volatility structure,
    // namely with and without step dates

    Real tol = 1E-8;

    Real reversion = 0.01;
    Real modelvol = 0.01;

    Handle<YieldTermStructure> yts0(ext::shared_ptr<YieldTermStructure>(
        new FlatForward(0, TARGET(), 0.00, Actual365Fixed())));

    std::vector<Date> stepDates0;
    std::vector<Real> vols0(1, modelvol);
    std::vector<Real> reversions0(1, reversion);

    std::vector<Date> stepDates1;
    for (Size i = 1; i < 60; i++)
        stepDates1.push_back(refDate + (i * 6 * Months));
    std::vector<Real> vols1(stepDates1.size() + 1, modelvol);
    std::vector<Real> reversions1(stepDates1.size() + 1, reversion);

    Real T = 10.0;
    do {

        ext::shared_ptr<Gsr> model(
            new Gsr(yts0, stepDates0, vols0, reversions0, T));
        ext::shared_ptr<StochasticProcess1D> gsrProcess =
            model->stateProcess();
        ext::shared_ptr<Gsr> model2(
            new Gsr(yts0, stepDates1, vols1, reversions1, T));
        ext::shared_ptr<StochasticProcess1D> gsrProcess2 =
            model2->stateProcess();

        ext::shared_ptr<HullWhiteForwardProcess> hwProcess(
            new HullWhiteForwardProcess(yts0, reversion, modelvol));
        hwProcess->setForwardMeasureTime(T);

        Real w, t, xw, hwVal, gsrVal, gsr2Val;

        t = 0.5;
        do {
            w = 0.0;
            do {
                xw = -0.1;
                do {
                    hwVal = hwProcess->expectation(w, xw, t - w);
                    gsrVal = gsrProcess->expectation(w, xw, t - w);
                    gsr2Val = gsrProcess2->expectation(w, xw, t - w);
                    if (fabs(hwVal - gsrVal) > tol)
                        BOOST_ERROR(
                            "Expectation E^{T="
                            << T << "}(x(" << t << ") | x(" << w << ") = " << xw
                            << " is different in HullWhiteProcess(" << hwVal
                            << ") and GsrProcess (" << gsrVal << ")");
                    if (fabs(hwVal - gsr2Val) > tol)
                        BOOST_ERROR(
                            "Expectation E^{T="
                            << T << "}(x(" << t << ") | x(" << w << ") = " << xw
                            << " is different in HullWhiteProcess(" << hwVal
                            << ") and GsrProcess2 (" << gsr2Val << ")");

                    hwVal = hwProcess->variance(w, xw, t - w);
                    gsrVal = gsrProcess->variance(w, xw, t - w);
                    gsr2Val = gsrProcess2->variance(w, xw, t - w);
                    if (fabs(hwVal - gsrVal) > tol)
                        BOOST_ERROR("Variance V((x("
                                    << t << ") | x(" << w << ") = " << xw
                                    << " is different in HullWhiteProcess("
                                    << hwVal << ") and GsrProcess (" << gsrVal
                                    << ")");
                    if (fabs(hwVal - gsr2Val) > tol)
                        BOOST_ERROR("Variance V((x("
                                    << t << ") | x(" << w << ") = " << xw
                                    << " is different in HullWhiteProcess("
                                    << hwVal << ") and GsrProcess2 (" << gsr2Val
                                    << ")");
                    xw += 0.01;
                } while (xw <= 0.1);
                w += t / 5.0;
            } while (w <= t - 0.1);
            t += T / 20.0;
        } while (t <= T - 0.1);
        T += 10.0;
    } while (T <= 30.0);

    // time dependent reversion and volatility (test cases to be added)

    Array times(2);
    Array vols(3);
    Array reversions(3);

    times[0] = 1.0;
    times[1] = 2.0;
    vols[0] = 0.2;
    vols[1] = 0.3;
    vols[2] = 0.4;
    reversions[0] = 0.50;
    reversions[1] = 0.80;
    reversions[2] = 1.30;

    GsrProcess p(times, vols, reversions);
    p.setForwardMeasureTime(10.0);

    // add more test cases here ...
}

BOOST_AUTO_TEST_CASE(testGsrModel) {

    BOOST_TEST_MESSAGE("Testing GSR model...");

    Date refDate = Settings::instance().evaluationDate();

    Real modelvol = 0.01;
    Real reversion = 0.01;

    std::vector<Date> stepDates; // no step dates
    std::vector<Real> vols(1, modelvol);
    std::vector<Real> reversions(1, reversion);

    std::vector<Date> stepDates1; // artificial step dates (should yield the
                                  // same result)
    for (Size i = 1; i < 60; i++)
        stepDates1.push_back(refDate + (i * 6 * Months));
    std::vector<Real> vols1(stepDates1.size() + 1, modelvol);
    std::vector<Real> reversions1(stepDates1.size() + 1, reversion);

    Handle<YieldTermStructure> yts(ext::shared_ptr<YieldTermStructure>(
        new FlatForward(0, TARGET(), 0.03, Actual365Fixed())));
    ext::shared_ptr<Gsr> model(
        new Gsr(yts, stepDates, vols, reversions, 50.0));
    ext::shared_ptr<Gsr> model2(
        new Gsr(yts, stepDates1, vols1, reversions1, 50.0));
    ext::shared_ptr<HullWhite> hw(new HullWhite(yts, reversion, modelvol));

    // test zerobond prices against existing HullWhite model
    // technically we test two representations of the same constant reversion
    // and volatility structure,
    // namely with and without step dates

    Real tol0 = 1E-8;

    Real w, t, xw;

    w = 0.1;
    do {
        t = w + 0.1;
        do {
            xw = -0.10;
            do {
                Real yw =
                    (xw - model->stateProcess()->expectation(0.0, 0.0, w)) /
                    model->stateProcess()->stdDeviation(0.0, 0.0, w);
                Real rw = xw + 0.03; // instantaneous forward is 0.03
                Real gsrVal = model->zerobond(t, w, yw);
                Real gsr2Val = model2->zerobond(t, w, yw);
                Real hwVal = hw->discountBond(w, t, rw);
                if (fabs(gsrVal - hwVal) > tol0)
                    BOOST_ERROR("Zerobond P("
                                << w << "," << t << " | x=" << xw << " / y="
                                << yw << ") is different in HullWhite ("
                                << hwVal << ") and Gsr (" << gsrVal << ")");
                if (fabs(gsr2Val - hwVal) > tol0)
                    BOOST_ERROR("Zerobond P("
                                << w << "," << t << " | x=" << xw << " / y="
                                << yw << ") is different in HullWhite ("
                                << hwVal << ") and Gsr2 (" << gsr2Val << ")");
                xw += 0.01;
            } while (xw <= 0.10);
            t += 2.5;
        } while (t <= 50.0);
        w += 5.0;
    } while (w <= 50.0);

    // test standard, nonstandard and jamshidian engine against existing Hull
    // White Jamshidian engine

    Date expiry = TARGET().advance(refDate, 5 * Years);
    Period tenor = 10 * Years;
    ext::shared_ptr<SwapIndex> swpIdx(new EuriborSwapIsdaFixA(tenor, yts));
    Real forward = swpIdx->fixing(expiry);

    ext::shared_ptr<VanillaSwap> underlying = swpIdx->underlyingSwap(expiry);
    ext::shared_ptr<VanillaSwap> underlyingFixed =
        MakeVanillaSwap(10 * Years, swpIdx->iborIndex(), forward)
            .withEffectiveDate(swpIdx->valueDate(expiry))
            .withFixedLegCalendar(swpIdx->fixingCalendar())
            .withFixedLegDayCount(swpIdx->dayCounter())
            .withFixedLegTenor(swpIdx->fixedLegTenor())
            .withFixedLegConvention(swpIdx->fixedLegConvention())
            .withFixedLegTerminationDateConvention(
                 swpIdx->fixedLegConvention());
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(expiry));
    ext::shared_ptr<Swaption> stdswaption(
        new Swaption(underlyingFixed, exercise));
    ext::shared_ptr<NonstandardSwaption> nonstdswaption(
        new NonstandardSwaption(*stdswaption));

    stdswaption->setPricingEngine(ext::shared_ptr<PricingEngine>(
        new JamshidianSwaptionEngine(hw, yts)));
    Real HwJamNpv = stdswaption->NPV();

    nonstdswaption->setPricingEngine(ext::shared_ptr<PricingEngine>(
        new Gaussian1dNonstandardSwaptionEngine(model, 64, 7.0, true, false)));
    stdswaption->setPricingEngine(ext::shared_ptr<PricingEngine>(
        new Gaussian1dSwaptionEngine(model, 64, 7.0, true, false)));
    Real GsrNonStdNpv = nonstdswaption->NPV();
    Real GsrStdNpv = stdswaption->NPV();
    stdswaption->setPricingEngine(ext::shared_ptr<PricingEngine>(
        new Gaussian1dJamshidianSwaptionEngine(model)));
    Real GsrJamNpv = stdswaption->NPV();

    if (fabs(HwJamNpv - GsrNonStdNpv) > 0.00005)
        BOOST_ERROR(
            "Jamshidian HW NPV ("
            << HwJamNpv
            << ") deviates from Gaussian1dNonstandardSwaptionEngine NPV ("
            << GsrNonStdNpv << ")");
    if (fabs(HwJamNpv - GsrStdNpv) > 0.00005)
        BOOST_ERROR("Jamshidian HW NPV ("
                    << HwJamNpv
                    << ") deviates from Gaussian1dSwaptionEngine NPV ("
                    << GsrStdNpv << ")");
    if (fabs(HwJamNpv - GsrJamNpv) > 0.00005)
        BOOST_ERROR("Jamshidian HW NPV ("
                    << HwJamNpv
                    << ") deviates from Gaussian1dJamshidianEngine NPV ("
                    << GsrJamNpv << ")");
}

/* To be re-enabled when we find a solution to #2408 that doesn't break calibration

BOOST_AUTO_TEST_CASE(testGsrProcessWithPathGenerator) {

    BOOST_TEST_MESSAGE("Testing GSR process path generation...");

    // This test verifies that GsrProcess works correctly with PathGenerator.
    // Previously, GsrProcessCore stored references to the input arrays instead
    // of copies, which caused crashes when temporary arrays were passed
    // (common in language bindings like Python/SWIG).

    Size timeSteps = 4;
    Time length = 2.0;

    // Create GsrProcess with temporary Array objects
    // This simulates what happens in SWIG bindings when Python lists are converted
    ext::shared_ptr<GsrProcess> process(
        new GsrProcess(Array(1, 1.0),           // times (temporary)
                       Array(2, 0.005),          // vols (temporary)
                       Array(1, 0.03)));         // reversions (temporary)

    // Create path generator
    typedef PseudoRandom::rsg_type rsg_type;
    typedef PathGenerator<rsg_type>::sample_type sample_type;

    rsg_type rsg = PseudoRandom::make_sequence_generator(timeSteps, 42);
    PathGenerator<rsg_type> generator(process, length, timeSteps, rsg, false);

    // Generate paths - this would crash before the fix due to dangling references
    const sample_type& sample = generator.next();
    const Path& path = sample.value;

    // Verify the path has the expected structure
    BOOST_CHECK_EQUAL(path.length(), timeSteps + 1);
    BOOST_CHECK_EQUAL(path.front(), 0.0);  // x0 is always 0 for GsrProcess

    // Verify path values are finite (not NaN or Inf)
    for (Size i = 0; i < path.length(); ++i) {
        BOOST_CHECK_MESSAGE(std::isfinite(path[i]),
                          "Path value at index " << i << " is not finite: " << path[i]);
    }

    // Generate a few more paths to ensure stability
    for (Size n = 0; n < 10; ++n) {
        const sample_type& s = generator.next();
        for (Size i = 0; i < s.value.length(); ++i) {
            BOOST_CHECK_MESSAGE(std::isfinite(s.value[i]),
                              "Path " << n << " value at index " << i
                              << " is not finite: " << s.value[i]);
        }
    }
}
*/

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
