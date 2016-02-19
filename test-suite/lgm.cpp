/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include "lgm.hpp"
#include "utilities.hpp"
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/methods/montecarlo/multipathgenerator.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/pricingengines/swaption/gaussian1dswaptionengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/models/lgm1.hpp>
#include <ql/experimental/models/cclgm1.hpp>
#include <ql/experimental/models/cclgmanalyticfxoptionengine.hpp>
#include <ql/experimental/models/fxoptionhelper.hpp>

#include <boost/make_shared.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/error_of_mean.hpp>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;
using namespace boost::accumulators;

void LgmTest::testBermudanLgm1fGsr() {

    BOOST_TEST_MESSAGE("Testing consistency of Bermudan swaption pricing in "
                       "LGM1F and GSR models...");

    // for kappa (LGM) = reversion (GSR) = 0.0
    // we have alpha (LGM) = sigma (GSR), so
    // we should get equal Bermudan swaption prices

    SavedSettings backup;

    Date evalDate(12, January, 2015);
    Settings::instance().evaluationDate() = evalDate;
    Handle<YieldTermStructure> yts(
        boost::make_shared<FlatForward>(evalDate, 0.02, Actual365Fixed()));
    boost::shared_ptr<IborIndex> euribor6m =
        boost::make_shared<Euribor>(6 * Months, yts);

    Date effectiveDate = TARGET().advance(evalDate, 2 * Days);
    Date startDate = TARGET().advance(effectiveDate, 1 * Years);
    Date maturityDate = TARGET().advance(startDate, 9 * Years);

    Schedule fixedSchedule(startDate, maturityDate, 1 * Years, TARGET(),
                           ModifiedFollowing, ModifiedFollowing,
                           DateGeneration::Forward, false);
    Schedule floatingSchedule(startDate, maturityDate, 6 * Months, TARGET(),
                              ModifiedFollowing, ModifiedFollowing,
                              DateGeneration::Forward, false);
    boost::shared_ptr<VanillaSwap> underlying = boost::make_shared<VanillaSwap>(
        VanillaSwap(VanillaSwap::Payer, 1.0, fixedSchedule, 0.02, Thirty360(),
                    floatingSchedule, euribor6m, 0.0, Actual360()));

    std::vector<Date> exerciseDates;
    for (Size i = 0; i < 9; ++i) {
        exerciseDates.push_back(TARGET().advance(fixedSchedule[i], -2 * Days));
    }
    boost::shared_ptr<Exercise> exercise =
        boost::make_shared<BermudanExercise>(exerciseDates, false);

    boost::shared_ptr<Swaption> swaption =
        boost::make_shared<Swaption>(underlying, exercise);

    std::vector<Date> stepDates(exerciseDates.begin(), exerciseDates.end() - 1);
    std::vector<Real> sigmas(stepDates.size() + 1);
    for (Size i = 0; i < sigmas.size(); ++i) {
        sigmas[i] = 0.0050 +
                    (0.0080 - 0.0050) * std::exp(-0.2 * static_cast<double>(i));
    }

    Real reversion = 0.0;

    // fix any T forward measure
    boost::shared_ptr<Gsr> gsr =
        boost::make_shared<Gsr>(yts, stepDates, sigmas, reversion, 50.0);

    boost::shared_ptr<Lgm1> lgm =
        boost::make_shared<Lgm1>(yts, stepDates, sigmas, reversion);

    boost::shared_ptr<PricingEngine> swaptionEngineGsr =
        boost::make_shared<Gaussian1dSwaptionEngine>(gsr, 64, 7.0, true, false);

    boost::shared_ptr<PricingEngine> swaptionEngineLgm =
        boost::make_shared<Gaussian1dSwaptionEngine>(lgm, 64, 7.0, true, false);

    swaption->setPricingEngine(swaptionEngineGsr);
    Real npvGsr = swaption->NPV();
    swaption->setPricingEngine(swaptionEngineLgm);
    Real npvLgm = swaption->NPV();

    Real tol = 0.05E-4; // basis point tolerance

    if (std::fabs(npvGsr - npvLgm) > tol)
        BOOST_ERROR(
            "Failed to verify consistency of Bermudan swaption price in Lgm1f ("
            << npvLgm << ") and Gsr (" << npvGsr << ") models, tolerance is "
            << tol);
} // testBermudanLgm1fGsr

void LgmTest::testLgm1fCalibration() {

    BOOST_TEST_MESSAGE(
        "Testing calibration of LGM1F model against GSR parameters...");

    // for fixed kappa != 0.0 we calibrate alpha
    // and compare the effective Hull White parameters
    // with the calibration results for the Gsr model

    SavedSettings backup;

    Date evalDate(12, January, 2015);
    Settings::instance().evaluationDate() = evalDate;
    Handle<YieldTermStructure> yts(
        boost::make_shared<FlatForward>(evalDate, 0.02, Actual365Fixed()));
    boost::shared_ptr<IborIndex> euribor6m =
        boost::make_shared<Euribor>(6 * Months, yts);

    // coterminal basket 1y-9y, 2y-8y, ... 9y-1y

    std::vector<boost::shared_ptr<CalibrationHelper> > basket;
    Real impliedVols[] = {0.4, 0.39, 0.38, 0.35, 0.35, 0.34, 0.33, 0.32, 0.31};
    std::vector<Date> expiryDates;

    for (Size i = 0; i < 9; ++i) {
        boost::shared_ptr<CalibrationHelper> helper =
            boost::make_shared<SwaptionHelper>(
                (i + 1) * Years, (9 - i) * Years,
                Handle<Quote>(boost::make_shared<SimpleQuote>(impliedVols[i])),
                euribor6m, 1 * Years, Thirty360(), Actual360(), yts);
        basket.push_back(helper);
        expiryDates.push_back(boost::static_pointer_cast<SwaptionHelper>(helper)
                                  ->swaption()
                                  ->exercise()
                                  ->dates()
                                  .back());
    }

    std::vector<Date> stepDates(expiryDates.begin(), expiryDates.end() - 1);

    std::vector<Real> gsrInitialSigmas(stepDates.size() + 1, 0.0050);
    std::vector<Real> lgmInitialAlphas(stepDates.size() + 1, 0.0050);

    Real kappa = 0.05;

    // fix any T forward measure
    boost::shared_ptr<Gsr> gsr =
        boost::make_shared<Gsr>(yts, stepDates, gsrInitialSigmas, kappa, 50.0);

    boost::shared_ptr<Lgm1> lgm =
        boost::make_shared<Lgm1>(yts, stepDates, lgmInitialAlphas, kappa);

    boost::shared_ptr<PricingEngine> swaptionEngineGsr =
        boost::make_shared<Gaussian1dSwaptionEngine>(gsr, 64, 7.0, true, false);

    boost::shared_ptr<PricingEngine> swaptionEngineLgm =
        boost::make_shared<Gaussian1dSwaptionEngine>(lgm, 64, 7.0, true, false);

    // calibrate GSR

    LevenbergMarquardt lm(1E-8, 1E-8, 1E-8);
    EndCriteria ec(1000, 500, 1E-8, 1E-8, 1E-8);

    for (Size i = 0; i < basket.size(); ++i) {
        basket[i]->setPricingEngine(swaptionEngineGsr);
    }

    gsr->calibrateVolatilitiesIterative(basket, lm, ec);

    Array gsrSigmas = gsr->volatility();

    // calibrate LGM

    for (Size i = 0; i < basket.size(); ++i) {
        basket[i]->setPricingEngine(swaptionEngineLgm);
    }

    lgm->calibrateAlphasIterative(basket, lm, ec);

    std::vector<Real> lgmHwSigmas;
    std::vector<Real> lgmHwKappas;

    for (Size i = 0; i < gsrSigmas.size(); ++i) {
        lgmHwSigmas.push_back(
            lgm->hullWhiteSigma(static_cast<double>(i) + 0.5));
        lgmHwKappas.push_back(
            lgm->hullWhiteKappa(static_cast<double>(i) + 0.5));
    }

    Real tol0 = 1E-8;
    Real tol = 1E-4;

    for (Size i = 0; i < gsrSigmas.size(); ++i) {
        // check calibration itself, we should match the market prices
        // rather exactly
        if (std::fabs(basket[i]->modelValue() - basket[i]->marketValue()) >
            tol0)
            BOOST_ERROR("Failed to calibrate to market swaption #"
                        << i << ", market price is " << basket[i]->marketValue()
                        << " while model price is " << basket[i]->modelValue());
        // we can not directly compare the gsr model's sigma with
        // the lgm model's equivalent HW sigma (since the former
        // is piecewise constant, while the latter is not), but
        // we can do a rough check on the mid point of each interval
        if (std::fabs(gsrSigmas[i] - lgmHwSigmas[i]) > tol)
            BOOST_ERROR("Failed to verify LGM's equivalent Hull White sigma (#"
                        << i << "), which is " << lgmHwSigmas[i]
                        << " while GSR's sigma is " << gsrSigmas[i] << ")");
    }

} // testLgm1fCalibration

void LgmTest::testLgm3fForeignPayouts() {
    BOOST_TEST_MESSAGE("Testing pricing of foreign payouts under domestic "
                       "measure in LGM3F model...");

    SavedSettings backup;

    Date referenceDate(30, July, 2015);

    Settings::instance().evaluationDate() = referenceDate;

    Handle<YieldTermStructure> eurYts(
        boost::make_shared<FlatForward>(referenceDate, 0.02, Actual365Fixed()));

    Handle<YieldTermStructure> usdYts(
        boost::make_shared<FlatForward>(referenceDate, 0.05, Actual365Fixed()));

    // use different grids for the EUR and USD  models and the FX volatility
    // process to test the piecewise numerical integration ...

    std::vector<Date> volstepdatesEur, volstepdatesUsd, volstepdatesFx;

    volstepdatesEur.push_back(Date(15, July, 2016));
    volstepdatesEur.push_back(Date(15, July, 2017));
    volstepdatesEur.push_back(Date(15, July, 2018));
    volstepdatesEur.push_back(Date(15, July, 2019));
    volstepdatesEur.push_back(Date(15, July, 2020));

    volstepdatesUsd.push_back(Date(13, April, 2016));
    volstepdatesUsd.push_back(Date(13, September, 2016));
    volstepdatesUsd.push_back(Date(13, April, 2017));
    volstepdatesUsd.push_back(Date(13, September, 2017));
    volstepdatesUsd.push_back(Date(13, April, 2018));
    volstepdatesUsd.push_back(Date(15, July, 2018)); // shared with EUR
    volstepdatesUsd.push_back(Date(13, April, 2019));
    volstepdatesUsd.push_back(Date(13, September, 2019));

    volstepdatesFx.push_back(Date(15, July, 2016)); // shared with EUR
    volstepdatesFx.push_back(Date(15, October, 2016));
    volstepdatesFx.push_back(Date(15, May, 2017));
    volstepdatesFx.push_back(Date(13, September, 2017)); // shared with USD
    volstepdatesFx.push_back(Date(15, July, 2018)); //  shared with EUR and USD

    std::vector<Real> eurVols, usdVols, fxSigmas;

    for (Size i = 0; i < volstepdatesEur.size() + 1; ++i) {
        eurVols.push_back(0.0050 +
                          (0.0080 - 0.0050) *
                              std::exp(-0.3 * static_cast<double>(i)));
    }
    for (Size i = 0; i < volstepdatesUsd.size() + 1; ++i) {
        usdVols.push_back(0.0030 +
                          (0.0110 - 0.0030) *
                              std::exp(-0.3 * static_cast<double>(i)));
    }
    for (Size i = 0; i < volstepdatesFx.size() + 1; ++i) {
        fxSigmas.push_back(
            0.15 + (0.20 - 0.15) * std::exp(-0.3 * static_cast<double>(i)));
    }

    boost::shared_ptr<Lgm1::model_type> eurLgm =
        boost::make_shared<Lgm1>(eurYts, volstepdatesEur, eurVols, 0.02);
    boost::shared_ptr<Lgm1::model_type> usdLgm =
        boost::make_shared<Lgm1>(usdYts, volstepdatesUsd, usdVols, 0.04);

    std::vector<boost::shared_ptr<Lgm1::model_type> > singleModels;
    singleModels.push_back(eurLgm);
    singleModels.push_back(usdLgm);

    std::vector<Handle<YieldTermStructure> > curves;
    curves.push_back(eurYts);
    curves.push_back(usdYts);

    std::vector<Handle<Quote> > fxSpots;
    fxSpots.push_back(Handle<Quote>(boost::make_shared<SimpleQuote>(
        std::log(0.90)))); // USD per EUR in log scale

    std::vector<std::vector<Real> > fxVolatilities;
    fxVolatilities.push_back(fxSigmas);

    Matrix c(3, 3);
    //  FX             EUR         USD
    c[0][0] = 1.0; c[0][1] = 0.8; c[0][2] = -0.5; // FX
    c[1][0] = 0.8; c[1][1] = 1.0; c[1][2] = -0.2; // EUR
    c[2][0] = -0.5; c[2][1] = -0.2; c[2][2] = 1.0; // USD

    boost::shared_ptr<CcLgm1> ccLgm = boost::make_shared<CcLgm1>(
        singleModels, fxSpots, volstepdatesFx, fxVolatilities, c, curves);

    boost::shared_ptr<StochasticProcess> process = ccLgm->stateProcess();

    boost::shared_ptr<StochasticProcess> usdProcess = usdLgm->stateProcess();

    // path generation

    Size n = 500000; // number of paths
    Size seed = 121; // seed
    // maturity of test payoffs
    Time T = 5.0;
    // take large steps, but not only one (since we are testing)
    Size steps = static_cast<Size>(T * 2.0);
    TimeGrid grid(T, steps);
    PseudoRandom::rsg_type sg =
        PseudoRandom::make_sequence_generator(3 * steps, seed);
    PseudoRandom::rsg_type sg2 =
        PseudoRandom::make_sequence_generator(steps, seed);

    MultiPathGenerator<PseudoRandom::rsg_type> pg(process, grid, sg, false);
    PathGenerator<PseudoRandom::rsg_type> pg2(usdProcess, grid, sg2, false);

    // test
    // 1 deterministic USD cashflow under EUR numeraire vs. price on USD curve
    // 2 zero bond option USD under EUR numeraire vs. USD numeraire
    // 3 fx option USD-EUR under EUR numeraire vs. analytical price

    accumulator_set<double, stats<tag::mean, tag::error_of<tag::mean> > > stat1,
        stat2a, stat2b, stat3;

    // same for paths2 since shared time grid
    for (Size j = 0; j < n; ++j) {
        Sample<MultiPath> path = pg.next();
        Sample<Path> path2 = pg2.next();
        Size l = path.value[0].length() - 1;
        Real fx = std::exp(path.value[0][l]);
        Real zeur = path.value[1][l];
        Real zusd = path.value[2][l];
        Real zusd2 = path2.value[l];
        Real yeur = (zeur - eurLgm->stateProcess()->expectation(0.0, 0.0, T)) /
                    eurLgm->stateProcess()->stdDeviation(0.0, 0.0, T);
        Real yusd = (zusd - usdLgm->stateProcess()->expectation(0.0, 0.0, T)) /
                    usdLgm->stateProcess()->stdDeviation(0.0, 0.0, T);
        Real yusd2 =
            (zusd2 - usdLgm->stateProcess()->expectation(0.0, 0.0, T)) /
            usdLgm->stateProcess()->stdDeviation(0.0, 0.0, T);

        // 1 USD paid at T deflated with EUR numeraire
        stat1(1.0 * fx / eurLgm->numeraire(T, yeur));

        // 2 USD zero bond option at T on P(T,T+10) strike 0.5 ...
        // ... under EUR numeraire ...
        Real zbOpt = std::max(usdLgm->zerobond(T + 10.0, T, yusd) - 0.5, 0.0);
        stat2a(zbOpt * fx / eurLgm->numeraire(T, yeur));
        // ... and under USD numeraire ...
        Real zbOpt2 = std::max(usdLgm->zerobond(T + 10.0, T, yusd2) - 0.5, 0.0);
        stat2b(zbOpt2 / usdLgm->numeraire(T, yusd2));

        // 3 USD-EUR fx option @0.9
        stat3(std::max(fx - 0.9, 0.0) / eurLgm->numeraire(T, yeur));
    }

    boost::shared_ptr<VanillaOption> fxOption =
        boost::make_shared<VanillaOption>(
            boost::make_shared<PlainVanillaPayoff>(Option::Call, 0.9),
            boost::make_shared<EuropeanExercise>(referenceDate + 5 * 365));

    boost::shared_ptr<PricingEngine> ccLgmFxOptionEngine =
        boost::make_shared<CcLgmAnalyticFxOptionEngine<
            CcLgm1::cclgm_model_type, CcLgm1::lgmfx_model_type,
            CcLgm1::lgm_model_type> >(ccLgm, 0);

    fxOption->setPricingEngine(ccLgmFxOptionEngine);

    Real npv1 = mean(stat1);
    Real error1 = error_of<tag::mean>(stat1);
    Real expected1 = usdYts->discount(5.0) * std::exp(fxSpots[0]->value());
    Real npv2a = mean(stat2a);
    Real error2a = error_of<tag::mean>(stat2a);
    Real npv2b = mean(stat2b) * std::exp(fxSpots[0]->value());
    Real error2b = error_of<tag::mean>(stat2b) * std::exp(fxSpots[0]->value());
    Real npv3 = mean(stat3);
    Real error3 = error_of<tag::mean>(stat3);

    // accept this relative difference in error estimates
    Real tolError = 0.2;
    // accept tolErrEst*errorEstimate as absolute difference
    Real tolErrEst = 1.0;

    if (std::fabs((error1 - 4E-4) / 4E-4) > tolError)
        BOOST_ERROR("error estimate deterministic "
                    "cashflow pricing can not be "
                    "reproduced, is "
                    << error1
                    << ", expected 4E-4, relative tolerance "
                    << tolError);
    if (std::fabs((error2a - 1E-4) / 1E-4) > tolError)
        BOOST_ERROR("error estimate zero bond "
                    "option pricing (foreign measure) can "
                    "not be reproduced, is "
                    << error2a
                    << ", expected 1E-4, relative tolerance "
                    << tolError);
    if (std::fabs((error2b - 7E-5) / 7E-5) > tolError)
        BOOST_ERROR("error estimate zero bond "
                    "option pricing (domestic measure) can "
                    "not be reproduced, is "
                    << error2b
                    << ", expected 7E-5, relative tolerance "
                    << tolError);
    if (std::fabs((error3 - 2.7E-4) / 2.7E-4) > tolError)
        BOOST_ERROR(
            "error estimate fx option pricing can not be reproduced, is "
            << error3 << ", expected 2.7E-4, relative tolerance " << tolError);

    if (std::fabs(npv1 - expected1) > tolErrEst * error1)
        BOOST_ERROR("can no reproduce deterministic cashflow pricing, is "
                    << npv1 << ", expected " << expected1 << ", tolerance "
                    << tolErrEst << "*" << error1);

    if (std::fabs(npv2a - npv2b) >
        tolErrEst * std::sqrt(error2a * error2a + error2b * error2b))
        BOOST_ERROR("can no reproduce zero bond option pricing, domestic "
                    "measure result is "
                    << npv2a
                    << ", foreign measure result is "
                    << npv2b
                    << ", tolerance "
                    << tolErrEst
                    << "*"
                    << std::sqrt(error2a * error2a + error2b * error2b));

    if (std::fabs(npv3 - fxOption->NPV()) > tolErrEst * std::sqrt(error3))
        BOOST_ERROR("can no reproduce fx option pricing, monte carlo result is "
                    << npv3 << ", analytical pricing result is "
                    << fxOption->NPV() << ", tolerance is " << tolErrEst << "*"
                    << error3);

} // testLgm3fForeignPayouts

void LgmTest::testLgm4fAndFxCalibration() {
    BOOST_TEST_MESSAGE("Testing LGM4F model and fx calibration...");

    SavedSettings backup;

    Date referenceDate(30, July, 2015);

    Settings::instance().evaluationDate() = referenceDate;

    Handle<YieldTermStructure> eurYts(
        boost::make_shared<FlatForward>(referenceDate, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> usdYts(
        boost::make_shared<FlatForward>(referenceDate, 0.05, Actual365Fixed()));
    Handle<YieldTermStructure> gbpYts(
        boost::make_shared<FlatForward>(referenceDate, 0.04, Actual365Fixed()));

    std::vector<Date> volstepdates, volstepdatesFx;

    volstepdates.push_back(Date(15, July, 2016));
    volstepdates.push_back(Date(15, July, 2017));
    volstepdates.push_back(Date(15, July, 2018));
    volstepdates.push_back(Date(15, July, 2019));
    volstepdates.push_back(Date(15, July, 2020));

    volstepdatesFx.push_back(Date(15, July, 2016));
    volstepdatesFx.push_back(Date(15, October, 2016));
    volstepdatesFx.push_back(Date(15, May, 2017));
    volstepdatesFx.push_back(Date(13, September, 2017));
    volstepdatesFx.push_back(Date(15, July, 2018));

    std::vector<Real> eurVols, usdVols, gbpVols, fxSigmasUsd, fxSigmasGbp;

    for (Size i = 0; i < volstepdates.size() + 1; ++i) {
        eurVols.push_back(0.0050 +
                          (0.0080 - 0.0050) *
                              std::exp(-0.3 * static_cast<double>(i)));
    }
    for (Size i = 0; i < volstepdates.size() + 1; ++i) {
        usdVols.push_back(0.0030 +
                          (0.0110 - 0.0030) *
                              std::exp(-0.3 * static_cast<double>(i)));
    }
    for (Size i = 0; i < volstepdates.size() + 1; ++i) {
        gbpVols.push_back(0.0070 +
                          (0.0095 - 0.0070) *
                              std::exp(-0.3 * static_cast<double>(i)));
    }
    for (Size i = 0; i < volstepdatesFx.size() + 1; ++i) {
        fxSigmasUsd.push_back(
            0.15 + (0.20 - 0.15) * std::exp(-0.3 * static_cast<double>(i)));
    }
    for (Size i = 0; i < volstepdatesFx.size() + 1; ++i) {
        fxSigmasGbp.push_back(
            0.10 + (0.15 - 0.10) * std::exp(-0.3 * static_cast<double>(i)));
    }

    boost::shared_ptr<Lgm1::model_type> eurLgm =
        boost::make_shared<Lgm1>(eurYts, volstepdates, eurVols, 0.02);
    boost::shared_ptr<Lgm1::model_type> usdLgm =
        boost::make_shared<Lgm1>(usdYts, volstepdates, usdVols, 0.03);
    boost::shared_ptr<Lgm1::model_type> gbpLgm =
        boost::make_shared<Lgm1>(usdYts, volstepdates, gbpVols, 0.04);

    std::vector<boost::shared_ptr<Lgm1::model_type> > singleModels;
    singleModels.push_back(eurLgm);
    singleModels.push_back(usdLgm);
    singleModels.push_back(gbpLgm);

    // we test the 4f model against the 3f model eur-gbp
    std::vector<boost::shared_ptr<Lgm1::model_type> > singleModelsProjected;
    singleModelsProjected.push_back(eurLgm);
    singleModelsProjected.push_back(gbpLgm);

    std::vector<Handle<YieldTermStructure> > curves;
    curves.push_back(eurYts);
    curves.push_back(usdYts);
    curves.push_back(gbpYts);

    std::vector<Handle<YieldTermStructure> > curvesProjected;
    curvesProjected.push_back(eurYts);
    curvesProjected.push_back(gbpYts);

    std::vector<Handle<Quote> > fxSpots;
    fxSpots.push_back(Handle<Quote>(boost::make_shared<SimpleQuote>(
        std::log(0.90)))); // EUR per one unit of USD in log scale
    fxSpots.push_back(Handle<Quote>(boost::make_shared<SimpleQuote>(
        std::log(1.35)))); // EUR per one unit of GBP in log scale

    std::vector<Handle<Quote> > fxSpotsProjected;
    fxSpotsProjected.push_back(fxSpots[1]);

    std::vector<std::vector<Real> > fxVolatilities;
    fxVolatilities.push_back(fxSigmasUsd);
    fxVolatilities.push_back(fxSigmasGbp);

    std::vector<std::vector<Real> > fxVolatilitiesProjected;
    fxVolatilitiesProjected.push_back(fxSigmasGbp);

    Matrix c(5, 5);
    //  FX USD-EUR      FX GBP-EUR     EUR           USD             GBP
    c[0][0] = 1.0;  c[0][1]= 0.3;   c[0][2] = 0.2; c[0][3] = -0.2; c[0][4]=0.0;  // FX USD-EUR
    c[1][0] = 0.3;  c[1][1]= 1.0;   c[1][2] = 0.3; c[1][3] = -0.1; c[1][4]=0.1;  // FX GBP-EUR
    c[2][0] = 0.2;  c[2][1]= 0.3;   c[2][2] = 1.0; c[2][3] = 0.6;  c[2][4]=0.3;  // EUR
    c[3][0] = -0.2; c[3][1]=-0.1;   c[3][2] = 0.6; c[3][3] = 1.0;  c[3][4]=0.1;  // USD
    c[4][0] = 0.0;  c[4][1]= 0.1;   c[4][2] = 0.3; c[4][3] = 0.1;  c[4][4]=1.0;  // GBP

    Matrix cProjected(3, 3);
    for (Size i = 0, ii = 0; i < 5; ++i) {
        if (i != 0 && i != 3) {
            for (Size j = 0, jj = 0; j < 5; ++j) {
                if (j != 0 && j != 3)
                    cProjected[ii][jj++] = c[i][j];
            }
            ++ii;
        }
    }

    boost::shared_ptr<CcLgm1> ccLgm = boost::make_shared<CcLgm1>(
        singleModels, fxSpots, volstepdatesFx, fxVolatilities, c, curves);

    boost::shared_ptr<CcLgm1> ccLgmProjected = boost::make_shared<CcLgm1>(
        singleModelsProjected, fxSpotsProjected, volstepdatesFx,
        fxVolatilitiesProjected, cProjected, curvesProjected);

    boost::shared_ptr<PricingEngine> ccLgmFxOptionEngineUsd =
        boost::make_shared<CcLgmAnalyticFxOptionEngine<
            CcLgm1::cclgm_model_type, CcLgm1::lgmfx_model_type,
            CcLgm1::lgm_model_type> >(ccLgm, 0);

    boost::shared_ptr<PricingEngine> ccLgmFxOptionEngineGbp =
        boost::make_shared<CcLgmAnalyticFxOptionEngine<
            CcLgm1::cclgm_model_type, CcLgm1::lgmfx_model_type,
            CcLgm1::lgm_model_type> >(ccLgm, 1);

    boost::shared_ptr<PricingEngine> ccLgmProjectedFxOptionEngineGbp =
        boost::make_shared<CcLgmAnalyticFxOptionEngine<
            CcLgm1::cclgm_model_type, CcLgm1::lgmfx_model_type,
            CcLgm1::lgm_model_type> >(ccLgmProjected, 0);

    // while the initial fx vol starts at 0.2 for usd and 0.15 for gbp
    // we calibrate to helpers with 0.15 and 0.2 target implied vol
    std::vector<boost::shared_ptr<CalibrationHelper> > helpersUsd, helpersGbp;
    for (Size i = 0; i <= volstepdatesFx.size(); ++i) {
        boost::shared_ptr<CalibrationHelper> tmpUsd =
            boost::make_shared<FxOptionHelper>(
                i < volstepdatesFx.size() ? volstepdatesFx[i]
                                          : volstepdatesFx.back() + 365,
                0.90, Handle<Quote>(boost::make_shared<SimpleQuote>(
                          std::exp(fxSpots[0]->value()))),
                Handle<Quote>(boost::make_shared<SimpleQuote>(0.15)),
                ccLgm->termStructure(0), ccLgm->termStructure(1));
        boost::shared_ptr<CalibrationHelper> tmpGbp =
            boost::make_shared<FxOptionHelper>(
                i < volstepdatesFx.size() ? volstepdatesFx[i]
                                          : volstepdatesFx.back() + 365,
                1.35, Handle<Quote>(boost::make_shared<SimpleQuote>(
                          std::exp(fxSpots[1]->value()))),
                Handle<Quote>(boost::make_shared<SimpleQuote>(0.20)),
                ccLgm->termStructure(0), ccLgm->termStructure(2));
        tmpUsd->setPricingEngine(ccLgmFxOptionEngineUsd);
        tmpGbp->setPricingEngine(ccLgmFxOptionEngineGbp);
        helpersUsd.push_back(tmpUsd);
        helpersGbp.push_back(tmpGbp);
    }

    LevenbergMarquardt lm(1E-8, 1E-8, 1E-8);
    EndCriteria ec(1000, 500, 1E-8, 1E-8, 1E-8);

    // calibrate USD-EUR FX volatility
    ccLgm->calibrateFxVolatilitiesIterative(0, helpersUsd, lm, ec);
    // calibrate GBP-EUR FX volatility
    ccLgm->calibrateFxVolatilitiesIterative(1, helpersGbp, lm, ec);

    Real tol = 1E-6;
    for (Size i = 0; i < helpersUsd.size(); ++i) {
        Real market = helpersUsd[i]->marketValue();
        Real model = helpersUsd[i]->modelValue();
        Real calibratedVol = ccLgm->fxVolatility(0)[i];
        if (std::fabs(market - model) > tol)
            BOOST_ERROR("calibration for fx option helper #"
                        << i << " (USD) failed, market premium is " << market
                        << " while model premium is " << model);
        // the stochastic rates produce some noise, but do not have a huge
        // impact on the effective volatility, so we check that they are
        // in line with a cached example
        if (std::fabs(calibratedVol - 0.143) > 0.01)
            BOOST_ERROR(
                "calibrated fx volatility #"
                << i
                << " (USD) seems off, expected to be 0.15 +- 0.0030, but is "
                << calibratedVol);
    }
    for (Size i = 0; i < helpersGbp.size(); ++i) {
        Real market = helpersGbp[i]->marketValue();
        Real model = helpersGbp[i]->modelValue();
        Real calibratedVol = ccLgm->fxVolatility(1)[i];
        if (std::fabs(market - model) > tol)
            BOOST_ERROR("calibration for fx option helper #"
                        << i << " (GBP) failed, market premium is " << market
                        << " while model premium is " << model);
        // see above
        if (std::fabs(calibratedVol - 0.193) > 0.01)
            BOOST_ERROR(
                "calibrated fx volatility #"
                << i << " (USD) seems off, expected to be 0.20 +- 0.01, but is "
                << calibratedVol);
    }

    // calibrate the projected model

    for (Size i = 0; i < helpersGbp.size(); ++i) {
        helpersGbp[i]->setPricingEngine(ccLgmProjectedFxOptionEngineGbp);
    }

    ccLgmProjected->calibrateFxVolatilitiesIterative(0, helpersGbp, lm, ec);

    for (Size i = 0; i < helpersGbp.size(); ++i) {
        Real fullModelVol = ccLgm->fxVolatility(1)[i];
        Real projectedModelVol = ccLgmProjected->fxVolatility(0)[i];
        if (std::fabs(fullModelVol - projectedModelVol) > tol)
            BOOST_ERROR(
                "calibrated fx volatility of full model @"
                << i << " (" << fullModelVol
                << ") is inconsistent with that of the projected model ("
                << projectedModelVol << ")");
    }

} // testLgm4fAndFxCalibration

test_suite *LgmTest::suite() {
    test_suite *suite = BOOST_TEST_SUITE("LGM model tests");
    suite->add(QUANTLIB_TEST_CASE(&LgmTest::testBermudanLgm1fGsr));
    suite->add(QUANTLIB_TEST_CASE(&LgmTest::testLgm1fCalibration));
    suite->add(QUANTLIB_TEST_CASE(&LgmTest::testLgm3fForeignPayouts));
    suite->add(QUANTLIB_TEST_CASE(&LgmTest::testLgm4fAndFxCalibration));
    return suite;
}
