/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Peter Caspers

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

#include "markovfunctional.hpp"
#include "utilities.hpp"
#include <ql/processes/mfstateprocess.hpp>
#include <ql/models/shortrate/onefactormodels/markovfunctional.hpp>
#include <ql/pricingengines/swaption/gaussian1dswaptionengine.hpp>
#include <ql/pricingengines/capfloor/gaussian1dcapfloorengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube1.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolsurface.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper1.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <ql/termstructures/volatility/interpolatedsmilesection.hpp>
#include <ql/termstructures/volatility/kahalesmilesection.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/instruments/makeswaption.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/models/shortrate/calibrationhelpers/caphelper.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::fabs;

void MarkovFunctionalTest::testMfStateProcess() {

    const Real tolerance = 1E-10;
    BOOST_TEST_MESSAGE("Testing Markov functional state process...");

    Array times1(0), vols1(1, 1.0);
    MfStateProcess sp1(0.00, times1, vols1);
    Real var11 = sp1.variance(0.0, 0.0, 1.0);
    Real var12 = sp1.variance(0.0, 0.0, 2.0);
    if (std::fabs(var11 - 1.0) > tolerance)
        BOOST_ERROR("process 1 has not variance 1.0 for dt = 1.0 but "
                    << var11);
    if (std::fabs(var12 - 2.0) > tolerance)
        BOOST_ERROR("process 1 has not variance 1.0 for dt = 1.0 but "
                    << var12);

    Array times2(2), vols2(3);
    times2[0] = 1.0;
    times2[1] = 2.0;
    vols2[0] = 1.0;
    vols2[1] = 2.0;
    vols2[2] = 3.0;
    MfStateProcess sp2(0.00, times2, vols2);
    Real dif21 = sp2.diffusion(0.0, 0.0);
    Real dif22 = sp2.diffusion(0.99, 0.0);
    Real dif23 = sp2.diffusion(1.0, 0.0);
    Real dif24 = sp2.diffusion(1.9, 0.0);
    Real dif25 = sp2.diffusion(2.0, 0.0);
    Real dif26 = sp2.diffusion(3.0, 0.0);
    Real dif27 = sp2.diffusion(5.0, 0.0);
    if (std::fabs(dif21 - 1.0) > tolerance)
        BOOST_ERROR("process 2 has wrong drift at 0.0, should be 1.0 but is "
                    << dif21);
    if (std::fabs(dif22 - 1.0) > tolerance)
        BOOST_ERROR("process 2 has wrong drift at 0.99, should be 1.0 but is "
                    << dif22);
    if (std::fabs(dif23 - 2.0) > tolerance)
        BOOST_ERROR("process 2 has wrong drift at 1.0, should be 2.0 but is "
                    << dif23);
    if (std::fabs(dif24 - 2.0) > tolerance)
        BOOST_ERROR("process 2 has wrong drift at 1.9, should be 2.0 but is "
                    << dif24);
    if (std::fabs(dif25 - 3.0) > tolerance)
        BOOST_ERROR("process 2 has wrong drift at 2.0, should be 3.0 but is "
                    << dif25);
    if (std::fabs(dif26 - 3.0) > tolerance)
        BOOST_ERROR("process 2 has wrong drift at 3.0, should be 3.0 but is "
                    << dif26);
    if (std::fabs(dif27 - 3.0) > tolerance)
        BOOST_ERROR("process 2 has wrong drift at 5.0, should be 3.0 but is "
                    << dif27);
    Real var21 = sp2.variance(0.0, 0.0, 0.0);
    Real var22 = sp2.variance(0.0, 0.0, 0.5);
    Real var23 = sp2.variance(0.0, 0.0, 1.0);
    Real var24 = sp2.variance(0.0, 0.0, 1.5);
    Real var25 = sp2.variance(0.0, 0.0, 3.0);
    Real var26 = sp2.variance(0.0, 0.0, 5.0);
    Real var27 = sp2.variance(1.2, 0.0, 1.0);
    if (std::fabs(var21 - 0.0) > tolerance)
        BOOST_ERROR("process 2 has wrong variance at 0.0, should be 0.0 but is "
                    << var21);
    if (std::fabs(var22 - 0.5) > tolerance)
        BOOST_ERROR("process 2 has wrong variance at 0.5, should be 0.5 but is "
                    << var22);
    if (std::fabs(var23 - 1.0) > tolerance)
        BOOST_ERROR("process 2 has wrong variance at 1.0, should be 1.0 but is "
                    << var23);
    if (std::fabs(var24 - 3.0) > tolerance)
        BOOST_ERROR("process 2 has wrong variance at 1.5, should be 3.0 but is "
                    << var24);
    if (std::fabs(var25 - 14.0) > tolerance)
        BOOST_ERROR(
            "process 2 has wrong variance at 3.0, should be 14.0 but is "
            << var25);
    if (std::fabs(var26 - 32.0) > tolerance)
        BOOST_ERROR(
            "process 2 has wrong variance at 5.0, should be 32.0 but is "
            << var26);
    if (std::fabs(var27 - 5.0) > tolerance)
        BOOST_ERROR("process 2 has wrong variance between 1.2 and 2.2, should "
                    "be 5.0 but is "
                    << var27);

    MfStateProcess sp3(0.01, times2, vols2);
    Real var31 = sp3.variance(0.0, 0.0, 0.0);
    Real var32 = sp3.variance(0.0, 0.0, 0.5);
    Real var33 = sp3.variance(0.0, 0.0, 1.0);
    Real var34 = sp3.variance(0.0, 0.0, 1.5);
    Real var35 = sp3.variance(0.0, 0.0, 3.0);
    Real var36 = sp3.variance(0.0, 0.0, 5.0);
    Real var37 = sp3.variance(1.2, 0.0, 1.0);
    if (std::fabs(var31 - 0.0) > tolerance)
        BOOST_ERROR("process 3 has wrong variance at 0.0, should be 0.0 but is "
                    << std::setprecision(12) << var31);
    if (std::fabs(var32 - 0.502508354208) > tolerance)
        BOOST_ERROR("process 3 has wrong variance at 0.5, should be 0.5 but it "
                    << std::setprecision(12) << var32);
    if (std::fabs(var33 - 1.01006700134) > tolerance)
        BOOST_ERROR("process 3 has wrong variance at 1.0, should be 1.0 but it "
                    << std::setprecision(12) << var33);
    if (std::fabs(var34 - 3.06070578669) > tolerance)
        BOOST_ERROR("process 3 has wrong variance at 1.5, should be 3.0 but it "
                    << std::setprecision(12) << var34);
    if (std::fabs(var35 - 14.5935513933) > tolerance)
        BOOST_ERROR(
            "process 3 has wrong variance at 3.0, should be 14.0 but it "
            << std::setprecision(12) << var35);
    if (std::fabs(var36 - 34.0940185819) > tolerance)
        BOOST_ERROR(
            "process 3 has wrong variance at 5.0, should be 32.0 but it "
            << std::setprecision(12) << var36);
    if (std::fabs(var37 - 5.18130257358) > tolerance)
        BOOST_ERROR("process 3 has wrong variance between 1.2 and 2.2, should "
                    "be 5.0 but it "
                    << std::setprecision(12) << var37);
}

namespace {

    // Flat yield term structure at 3%

    Handle<YieldTermStructure> flatYts() {

        return Handle<YieldTermStructure>(ext::shared_ptr<YieldTermStructure>(
            new FlatForward(0, TARGET(), 0.03, Actual365Fixed())));
    }

    // Flat swaption volatility structure at 20%

    Handle<SwaptionVolatilityStructure> flatSwaptionVts() {

        return Handle<SwaptionVolatilityStructure>(
            ext::shared_ptr<SwaptionVolatilityStructure>(
                new ConstantSwaptionVolatility(0, TARGET(), ModifiedFollowing,
                                               0.20, Actual365Fixed())));
    }

    // Flat cap volatility structure at 20%

    Handle<OptionletVolatilityStructure> flatOptionletVts() {

        return Handle<OptionletVolatilityStructure>(
            ext::shared_ptr<OptionletVolatilityStructure>(
                new ConstantOptionletVolatility(0, TARGET(), ModifiedFollowing,
                                                0.20, Actual365Fixed())));
    }

    // Yield term structure as of 14.11.2012 (6m discounting)

    Handle<YieldTermStructure> md0Yts() {

        ext::shared_ptr<IborIndex> euribor6mEmpty(new Euribor(6 * Months));

        std::vector<ext::shared_ptr<Quote> > q6m;
        std::vector<ext::shared_ptr<RateHelper> > r6m;

        double q6mh[] = { 0.0001,  0.0001,  0.0001,  0.0003,  0.00055, 0.0009,
                          0.0014,  0.0019,  0.0025,  0.0031,  0.00325, 0.00313,
                          0.0031,  0.00307, 0.00309, 0.00339, 0.00316, 0.00326,
                          0.00335, 0.00343, 0.00358, 0.00351, 0.00388, 0.00404,
                          0.00425, 0.00442, 0.00462, 0.00386, 0.00491, 0.00647,
                          0.00837, 0.01033, 0.01218, 0.01382, 0.01527, 0.01654,
                          0.0177,  0.01872, 0.01959, 0.0203,  0.02088, 0.02132,
                          0.02164, 0.02186, 0.02202, 0.02213, 0.02222, 0.02229,
                          0.02234, 0.02238, 0.02241, 0.02243, 0.02244, 0.02245,
                          0.02247, 0.0225,  0.02284, 0.02336, 0.02407, 0.0245 };

        Period q6mh1[] = { 1 * Days,   1 * Days,   1 * Days,   1 * Weeks,
                           1 * Months, 2 * Months, 3 * Months, 4 * Months,
                           5 * Months, 6 * Months };

        Period q6mh2[] = {
            7 * Months,  8 * Months,  9 * Months,  10 * Months, 11 * Months,
            1 * Years,   13 * Months, 14 * Months, 15 * Months, 16 * Months,
            17 * Months, 18 * Months, 19 * Months, 20 * Months, 21 * Months,
            22 * Months, 23 * Months, 2 * Years,   3 * Years,   4 * Years,
            5 * Years,   6 * Years,   7 * Years,   8 * Years,   9 * Years,
            10 * Years,  11 * Years,  12 * Years,  13 * Years,  14 * Years,
            15 * Years,  16 * Years,  17 * Years,  18 * Years,  19 * Years,
            20 * Years,  21 * Years,  22 * Years,  23 * Years,  24 * Years,
            25 * Years,  26 * Years,  27 * Years,  28 * Years,  29 * Years,
            30 * Years,  35 * Years,  40 * Years,  50 * Years,  60 * Years
        };

        q6m.reserve(10 + 15 + 35);
        for (double i : q6mh) {
            q6m.push_back(ext::shared_ptr<Quote>(new SimpleQuote(i)));
        }

        r6m.reserve(10);
        for (int i = 0; i < 10; i++) {
            r6m.push_back(ext::make_shared<DepositRateHelper>(
                Handle<Quote>(q6m[i]), q6mh1[i],
                                      i < 2 ? i : 2, TARGET(),
                                      ModifiedFollowing, false, Actual360()));
        }

        for (int i = 0; i < 18; i++) {
            if (i + 1 != 6 && i + 1 != 12 && i + 1 != 18) {
                r6m.push_back(ext::make_shared<FraRateHelper>(
                            Handle<Quote>(q6m[10 + i]), i + 1,
                                          i + 7, 2, TARGET(), ModifiedFollowing,
                                          false, Actual360()));
            }
        }

        for (int i = 0; i < 15 + 35; i++) {
            if (i + 7 == 12 || i + 7 == 18 || i + 7 >= 24) {
                r6m.push_back(ext::make_shared<SwapRateHelper>(
                    Handle<Quote>(q6m[10 + i]), q6mh2[i],
                                       TARGET(), Annual, ModifiedFollowing,
                                       Actual360(), euribor6mEmpty));
            }
        }

        Handle<YieldTermStructure> res(ext::shared_ptr<YieldTermStructure>(
            new PiecewiseYieldCurve<Discount, LogLinear>(0, TARGET(), r6m,
                                                         Actual365Fixed())));
        res->enableExtrapolation();

        return res;
    }

    // Swaption volatility cube as of 14.11.2012, 1y underlying vols are not
    // converted here from 3m to 6m

    Handle<SwaptionVolatilityStructure> md0SwaptionVts() {

        std::vector<Period> optionTenors = {
                            1 * Months, 2 * Months, 3 * Months,  6 * Months,
                            9 * Months, 1 * Years,  18 * Months, 2 * Years,
                            3 * Years,  4 * Years,  5 * Years,   6 * Years,
                            7 * Years,  8 * Years,  9 * Years,   10 * Years,
                            15 * Years, 20 * Years, 25 * Years,  30 * Years };

        std::vector<Period> swapTenors = {
                            1 * Years,  2 * Years,  3 * Years,  4 * Years,
                            5 * Years,  6 * Years,  7 * Years,  8 * Years,
                            9 * Years,  10 * Years, 15 * Years, 20 * Years,
                            25 * Years, 30 * Years };

        double qSwAtmh[] = {
            1.81,  0.897, 0.819, 0.692, 0.551, 0.47,  0.416, 0.379, 0.357,
            0.335, 0.283, 0.279, 0.283, 0.287, 1.717, 0.886, 0.79,  0.69,
            0.562, 0.481, 0.425, 0.386, 0.359, 0.339, 0.29,  0.287, 0.292,
            0.296, 1.762, 0.903, 0.804, 0.693, 0.582, 0.5,   0.448, 0.411,
            0.387, 0.365, 0.31,  0.307, 0.312, 0.317, 1.662, 0.882, 0.764,
            0.67,  0.586, 0.513, 0.468, 0.432, 0.408, 0.388, 0.331, 0.325,
            0.33,  0.334, 1.53,  0.854, 0.728, 0.643, 0.565, 0.503, 0.464,
            0.435, 0.415, 0.393, 0.337, 0.33,  0.333, 0.338, 1.344, 0.786,
            0.683, 0.609, 0.54,  0.488, 0.453, 0.429, 0.411, 0.39,  0.335,
            0.329, 0.332, 0.336, 1.1,   0.711, 0.617, 0.548, 0.497, 0.456,
            0.43,  0.408, 0.392, 0.374, 0.328, 0.323, 0.326, 0.33,  0.956,
            0.638, 0.553, 0.496, 0.459, 0.427, 0.403, 0.385, 0.371, 0.359,
            0.321, 0.318, 0.323, 0.327, 0.671, 0.505, 0.45,  0.42,  0.397,
            0.375, 0.36,  0.347, 0.337, 0.329, 0.305, 0.303, 0.309, 0.313,
            0.497, 0.406, 0.378, 0.36,  0.348, 0.334, 0.323, 0.315, 0.309,
            0.304, 0.289, 0.289, 0.294, 0.297, 0.404, 0.352, 0.334, 0.322,
            0.313, 0.304, 0.296, 0.291, 0.288, 0.286, 0.278, 0.277, 0.281,
            0.282, 0.345, 0.312, 0.302, 0.294, 0.286, 0.28,  0.276, 0.274,
            0.273, 0.273, 0.267, 0.265, 0.268, 0.269, 0.305, 0.285, 0.277,
            0.271, 0.266, 0.262, 0.26,  0.259, 0.26,  0.262, 0.259, 0.256,
            0.257, 0.256, 0.282, 0.265, 0.259, 0.254, 0.251, 0.25,  0.25,
            0.251, 0.253, 0.256, 0.253, 0.25,  0.249, 0.246, 0.263, 0.248,
            0.244, 0.241, 0.24,  0.24,  0.242, 0.245, 0.249, 0.252, 0.249,
            0.245, 0.243, 0.238, 0.242, 0.234, 0.232, 0.232, 0.233, 0.235,
            0.239, 0.243, 0.247, 0.249, 0.246, 0.242, 0.237, 0.231, 0.233,
            0.234, 0.241, 0.246, 0.249, 0.253, 0.257, 0.261, 0.263, 0.264,
            0.251, 0.236, 0.222, 0.214, 0.262, 0.26,  0.262, 0.263, 0.263,
            0.266, 0.268, 0.269, 0.269, 0.265, 0.237, 0.214, 0.202, 0.196,
            0.26,  0.26,  0.261, 0.261, 0.258, 0.255, 0.252, 0.248, 0.245,
            0.24,  0.207, 0.187, 0.182, 0.176, 0.236, 0.223, 0.221, 0.218,
            0.214, 0.21,  0.207, 0.204, 0.202, 0.2,   0.175, 0.167, 0.163,
            0.158
        };

        std::vector<std::vector<Handle<Quote> > > qSwAtm;
        for (int i = 0; i < 20; i++) {
            std::vector<Handle<Quote> > qSwAtmTmp;
            qSwAtmTmp.reserve(14);
            for (int j = 0; j < 14; j++) {
                qSwAtmTmp.emplace_back(
                    ext::shared_ptr<Quote>(new SimpleQuote(qSwAtmh[i * 14 + j])));
            }
            qSwAtm.push_back(qSwAtmTmp);
        }

        Handle<SwaptionVolatilityStructure> swaptionVolAtm(
            ext::shared_ptr<SwaptionVolatilityStructure>(
                new SwaptionVolatilityMatrix(TARGET(), ModifiedFollowing,
                                             optionTenors, swapTenors, qSwAtm,
                                             Actual365Fixed())));

        std::vector<Period> optionTenorsSmile = { 3 * Months, 1 * Years,  5 * Years,
                                                  10 * Years, 20 * Years, 30 * Years };
        std::vector<Period> swapTenorsSmile = { 2 * Years,  5 * Years, 10 * Years,
                                                20 * Years, 30 * Years };
        std::vector<Real> strikeSpreads = { -0.02,  -0.01,  -0.0050, -0.0025, 0.0,
                                            0.0025, 0.0050, 0.01,    0.02 };

        std::vector<std::vector<Handle<Quote> > > qSwSmile;

        double qSwSmileh[] = {
            2.2562,  2.2562,  2.2562,  0.1851,  0.0,     -0.0389, -0.0507,
            -0.0571, -0.06,   14.9619, 14.9619, 0.1249,  0.0328,  0.0,
            -0.0075, -0.005,  0.0078,  0.0328,  0.2296,  0.2296,  0.0717,
            0.0267,  0.0,     -0.0115, -0.0126, -0.0002, 0.0345,  0.6665,
            0.1607,  0.0593,  0.0245,  0.0,     -0.0145, -0.0204, -0.0164,
            0.0102,  0.6509,  0.1649,  0.0632,  0.027,   0.0,     -0.018,
            -0.0278, -0.0303, -0.0105, 0.6303,  0.6303,  0.6303,  0.1169,
            0.0,     -0.0469, -0.0699, -0.091,  -0.1065, 0.4437,  0.4437,
            0.0944,  0.0348,  0.0,     -0.0206, -0.0327, -0.0439, -0.0472,
            2.1557,  0.1501,  0.0531,  0.0225,  0.0,     -0.0161, -0.0272,
            -0.0391, -0.0429, 0.4365,  0.1077,  0.0414,  0.0181,  0.0,
            -0.0137, -0.0237, -0.0354, -0.0401, 0.4415,  0.1117,  0.0437,
            0.0193,  0.0,     -0.015,  -0.0264, -0.0407, -0.0491, 0.4301,
            0.0776,  0.0283,  0.0122,  0.0,     -0.0094, -0.0165, -0.0262,
            -0.035,  0.2496,  0.0637,  0.0246,  0.0109,  0.0,     -0.0086,
            -0.0153, -0.0247, -0.0334, 0.1912,  0.0569,  0.023,   0.0104,
            0.0,     -0.0085, -0.0155, -0.0256, -0.0361, 0.2095,  0.06,
            0.0239,  0.0107,  0.0,     -0.0087, -0.0156, -0.0254, -0.0348,
            0.2357,  0.0669,  0.0267,  0.012,   0.0,     -0.0097, -0.0174,
            -0.0282, -0.0383, 0.1291,  0.0397,  0.0158,  0.007,   0.0,
            -0.0056, -0.01,   -0.0158, -0.0203, 0.1281,  0.0397,  0.0159,
            0.0071,  0.0,     -0.0057, -0.0102, -0.0164, -0.0215, 0.1547,
            0.0468,  0.0189,  0.0085,  0.0,     -0.0069, -0.0125, -0.0205,
            -0.0283, 0.1851,  0.0522,  0.0208,  0.0093,  0.0,     -0.0075,
            -0.0135, -0.0221, -0.0304, 0.1782,  0.0506,  0.02,    0.0089,
            0.0,     -0.0071, -0.0128, -0.0206, -0.0276, 0.2665,  0.0654,
            0.0255,  0.0113,  0.0,     -0.0091, -0.0163, -0.0265, -0.0367,
            0.2873,  0.0686,  0.0269,  0.0121,  0.0,     -0.0098, -0.0179,
            -0.0298, -0.043,  0.2993,  0.0688,  0.0273,  0.0123,  0.0,
            -0.0103, -0.0189, -0.0324, -0.0494, 0.1869,  0.0501,  0.0202,
            0.0091,  0.0,     -0.0076, -0.014,  -0.0239, -0.0358, 0.1573,
            0.0441,  0.0178,  0.008,   0.0,     -0.0066, -0.0121, -0.0202,
            -0.0294, 0.196,   0.0525,  0.0204,  0.009,   0.0,     -0.0071,
            -0.0125, -0.0197, -0.0253, 0.1795,  0.0497,  0.0197,  0.0088,
            0.0,     -0.0071, -0.0128, -0.0208, -0.0286, 0.1401,  0.0415,
            0.0171,  0.0078,  0.0,     -0.0066, -0.0122, -0.0209, -0.0318,
            0.112,   0.0344,  0.0142,  0.0065,  0.0,     -0.0055, -0.01,
            -0.0171, -0.0256, 0.1077,  0.0328,  0.0134,  0.0061,  0.0,
            -0.005,  -0.0091, -0.0152, -0.0216,
        };

        for (int i = 0; i < 30; i++) {
            std::vector<Handle<Quote> > qSwSmileTmp;
            qSwSmileTmp.reserve(9);
            for (int j = 0; j < 9; j++) {
                qSwSmileTmp.emplace_back(
                    ext::shared_ptr<Quote>(new SimpleQuote(qSwSmileh[i * 9 + j])));
            }
            qSwSmile.push_back(qSwSmileTmp);
        }

        double qSwSmileh1[] = {
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2,
            0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2, 0.01, 0.2, 0.8, -0.2
        };

        std::vector<bool> parameterFixed = {false, false, false, false};

        std::vector<std::vector<Handle<Quote> > > parameterGuess;
        for (int i = 0; i < 30; i++) {
            std::vector<Handle<Quote> > parameterGuessTmp;
            parameterGuessTmp.reserve(4);
            for (int j = 0; j < 4; j++) {
                parameterGuessTmp.emplace_back(
                    ext::shared_ptr<Quote>(new SimpleQuote(qSwSmileh1[i * 4 + j])));
            }
            parameterGuess.push_back(parameterGuessTmp);
        }

        ext::shared_ptr<EndCriteria> ec(
            new EndCriteria(50000, 250, 1E-6, 1E-6, 1E-6));

        ext::shared_ptr<SwapIndex> swapIndex(new EuriborSwapIsdaFixA(
            30 * Years, Handle<YieldTermStructure>(md0Yts())));
        ext::shared_ptr<SwapIndex> shortSwapIndex(new EuriborSwapIsdaFixA(
            1 * Years,
            Handle<YieldTermStructure>(md0Yts()))); // We assume that we have 6m
                                                    // vols (which we actually
                                                    // don't have for 1y
                                                    // underlying, but this is
                                                    // just a test...)

        // return Handle<SwaptionVolatilityStructure>(new
        // SwaptionVolCube2(swaptionVolAtm,optionTenorsSmile,swapTenorsSmile,strikeSpreads,qSwSmile,swapIndex,shortSwapIndex,false));
        // // bilinear interpolation gives nasty digitals
        Handle<SwaptionVolatilityStructure> res(
            ext::shared_ptr<SwaptionVolatilityStructure>(new SwaptionVolCube1(
                swaptionVolAtm, optionTenorsSmile, swapTenorsSmile,
                strikeSpreads, qSwSmile, swapIndex, shortSwapIndex, true,
                parameterGuess, parameterFixed, true, ec,
                0.0050))); // put a big error tolerance here ... we just want a
                           // smooth cube for testing
        res->enableExtrapolation();
        return res;
    }

    // Cap volatility surface as of 14.11.2012. Par vols up to 2y are converted
    // to 6m to get a consistent caplet surface.

    Handle<OptionletVolatilityStructure> md0OptionletVts() {

        // with the thread safe observer it takes very long to destruct
        // the cap floor instruments created in OptionletStripper1
#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
        return flatOptionletVts();
#endif

        Size nOptTen = 16;
        Size nStrikes = 12; // leave out last strike 10% because it causes an
                            // exception in bootstrapping

        std::vector<Period> optionTenors = {
                            1 * Years,  18 * Months, 2 * Years,  3 * Years,
                            4 * Years,  5 * Years,   6 * Years,  7 * Years,
                            8 * Years,  9 * Years,   10 * Years, 12 * Years,
                            15 * Years, 20 * Years,  25 * Years, 30 * Years };

        std::vector<Real> strikes = {
                        0.0025, 0.0050, 0.0100, 0.0150, 0.0200, 0.0225,
                        0.0250, 0.0300, 0.0350, 0.0400, 0.0500, 0.0600 };

        Matrix vols(nOptTen, nStrikes);
        Real volsa[13][16] = { { 1.3378, 1.3032, 1.2514, 1.081, 1.019, 0.961,
                                 0.907,  0.862,  0.822,  0.788, 0.758, 0.709,
                                 0.66,   0.619,  0.597,  0.579 }, // strike1
                               { 1.1882, 1.1057, 0.9823, 0.879, 0.828, 0.779,
                                 0.736,  0.7,    0.67,   0.644, 0.621, 0.582,
                                 0.544,  0.513,  0.496,  0.482 }, // strike2
                               { 1.1646, 1.0356, 0.857, 0.742, 0.682, 0.626,
                                 0.585,  0.553,  0.527, 0.506, 0.488, 0.459,
                                 0.43,   0.408,  0.396, 0.386 }, // ...
                               { 1.1932, 1.0364, 0.8291, 0.691, 0.618, 0.553,
                                 0.509,  0.477,  0.452,  0.433, 0.417, 0.391,
                                 0.367,  0.35,   0.342,  0.335 },
                               { 1.2233, 1.0489, 0.8268, 0.666, 0.582, 0.51,
                                 0.463,  0.43,   0.405,  0.387, 0.372, 0.348,
                                 0.326,  0.312,  0.306,  0.301 },
                               { 1.2369, 1.0555, 0.8283, 0.659, 0.57,  0.495,
                                 0.447,  0.414,  0.388,  0.37,  0.355, 0.331,
                                 0.31,   0.298,  0.293,  0.289 },
                               { 1.2498, 1.0622, 0.8307, 0.653, 0.56,  0.483,
                                 0.434,  0.4,    0.374,  0.356, 0.341, 0.318,
                                 0.297,  0.286,  0.282,  0.279 },
                               { 1.2719, 1.0747, 0.8368, 0.646, 0.546, 0.465,
                                 0.415,  0.38,   0.353,  0.335, 0.32,  0.296,
                                 0.277,  0.268,  0.265,  0.263 },
                               { 1.2905, 1.0858, 0.8438, 0.643, 0.536, 0.453,
                                 0.403,  0.367,  0.339,  0.32,  0.305, 0.281,
                                 0.262,  0.255,  0.254,  0.252 },
                               { 1.3063, 1.0953, 0.8508, 0.642, 0.53,  0.445,
                                 0.395,  0.358,  0.329,  0.31,  0.294, 0.271,
                                 0.252,  0.246,  0.246,  0.244 },
                               { 1.332, 1.1108, 0.8631, 0.642, 0.521, 0.436,
                                 0.386, 0.348,  0.319,  0.298, 0.282, 0.258,
                                 0.24,  0.237,  0.237,  0.236 },
                               { 1.3513, 1.1226, 0.8732, 0.645, 0.517, 0.43,
                                 0.381,  0.344,  0.314,  0.293, 0.277, 0.252,
                                 0.235,  0.233,  0.234,  0.233 },
                               { 1.395, 1.1491, 0.9003, 0.661, 0.511, 0.425,
                                 0.38,  0.344,  0.314,  0.292, 0.275, 0.251,
                                 0.236, 0.236,  0.238,  0.235 } };

        for (Size i = 0; i < nStrikes; i++) {
            for (Size j = 0; j < nOptTen; j++) {
                vols[j][i] = volsa[i][j];
            }
        }

        ext::shared_ptr<IborIndex> iborIndex(
            new Euribor(6 * Months, md0Yts()));
        ext::shared_ptr<CapFloorTermVolSurface> cf(new CapFloorTermVolSurface(
            0, TARGET(), ModifiedFollowing, optionTenors, strikes, vols));
        ext::shared_ptr<OptionletStripper> stripper(
            new OptionletStripper1(cf, iborIndex));

        return Handle<OptionletVolatilityStructure>(
            ext::shared_ptr<OptionletVolatilityStructure>(
                new StrippedOptionletAdapter(stripper)));
    }

    // Calibration Basket 1: CMS10y Swaptions, 5 yearly fixings

    Disposable<std::vector<Date> > expiriesCalBasket1() {

        std::vector<Date> res;
        Date referenceDate_ = Settings::instance().evaluationDate();

        for (int i = 1; i <= 5; i++)
            res.push_back(TARGET().advance(referenceDate_, i * Years));

        return res;
    }

    Disposable<std::vector<Period> > tenorsCalBasket1() {

        std::vector<Period> res(5, 10 * Years);

        return res;
    }

    // Calibration Basket 2: 6m caplets, 5 years

    Disposable<std::vector<Date> > expiriesCalBasket2() {

        Date referenceDate_ = Settings::instance().evaluationDate();

        std::vector<Date> res = {
            TARGET().advance(referenceDate_, 6 * Months),
            TARGET().advance(referenceDate_, 12 * Months),
            TARGET().advance(referenceDate_, 18 * Months),
            TARGET().advance(referenceDate_, 24 * Months),
            TARGET().advance(referenceDate_, 30 * Months),
            TARGET().advance(referenceDate_, 36 * Months),
            TARGET().advance(referenceDate_, 42 * Months),
            TARGET().advance(referenceDate_, 48 * Months),
            TARGET().advance(referenceDate_, 54 * Months),
            TARGET().advance(referenceDate_, 60 * Months)
        };

        return res;
    }

    // Calibration Basket 3: Coterminal Swaptions 10y

    Disposable<std::vector<Date> > expiriesCalBasket3() {

        Date referenceDate_ = Settings::instance().evaluationDate();

        std::vector<Date> res = {
            TARGET().advance(referenceDate_, 1 * Years),
            TARGET().advance(referenceDate_, 2 * Years),
            TARGET().advance(referenceDate_, 3 * Years),
            TARGET().advance(referenceDate_, 4 * Years),
            TARGET().advance(referenceDate_, 5 * Years),
            TARGET().advance(referenceDate_, 6 * Years),
            TARGET().advance(referenceDate_, 7 * Years),
            TARGET().advance(referenceDate_, 8 * Years),
            TARGET().advance(referenceDate_, 9 * Years)
        };

        return res;
    }

    Disposable<std::vector<Period> > tenorsCalBasket3() {

        std::vector<Period> res = {9 * Years, 8 * Years, 7 * Years, 6 * Years, 5 * Years,
                                   4 * Years, 3 * Years, 2 * Years, 1 * Years};
        return res;
    }

    Disposable<std::vector<Real> >
    impliedStdDevs(const Real atm, const std::vector<Real> &strikes,
                   const std::vector<Real> &prices) {

        std::vector<Real> result;

        for (Size i = 0; i < prices.size(); i++) {
            result.push_back(blackFormulaImpliedStdDev(Option::Call, strikes[i],
                                                       atm, prices[i], 1.0, 0.0,
                                                       0.2, 1E-8, 1000));
        }

        return result;
    }
}

void MarkovFunctionalTest::testKahaleSmileSection() {

    BOOST_TEST_MESSAGE("Testing Kahale smile section...");

    const Real tol = 1E-8;

    // arbitrage free sample smile data

    const Real atm = 0.05;
    const Real t = 1.0;

    const Real strikes0[] = { 0.01, 0.02, 0.03, 0.04, 0.05,
                              0.06, 0.07, 0.08, 0.09, 0.10 };

    const std::vector<Real> strikes(strikes0, strikes0 + 10);
    std::vector<Real> money;
    std::vector<Real> calls0;

    for (double strike : strikes) {
        money.push_back(strike / atm);
        calls0.push_back(blackFormula(Option::Call, strike, atm, 0.50 * std::sqrt(t), 1.0, 0.0));
    }

    std::vector<Real> stdDevs0 = impliedStdDevs(atm, strikes, calls0);
    ext::shared_ptr<SmileSection> sec1(
        new InterpolatedSmileSection<Linear>(t, strikes, stdDevs0, atm));

    // test arbitrage free smile reproduction

    ext::shared_ptr<KahaleSmileSection> ksec11(
        new KahaleSmileSection(sec1, atm, false, false, false, money));

    if (std::fabs(ksec11->leftCoreStrike() - 0.01) > tol)
        BOOST_ERROR("smile11 left af strike is " << ksec11->leftCoreStrike()
                                                 << " expected 0.01");

    if (std::fabs(ksec11->rightCoreStrike() - 0.10) > tol)
        BOOST_ERROR("smile11 right af strike is " << ksec11->rightCoreStrike()
                                                  << " expected 0.10");

    Real k = strikes[0];
    while (k <= strikes.back() + tol) {
        Real pric0 = sec1->optionPrice(k);
        Real pric1 = ksec11->optionPrice(k);
        if (std::fabs(pric0 - pric1) > tol)
            BOOST_ERROR("smile11 is not reprocduced at strike "
                        << k << " input smile call price is  " << pric0
                        << " kahale smile call price is " << pric1);
        k += 0.0001;
    }

    // test interpolation

    ext::shared_ptr<KahaleSmileSection> ksec12(
        new KahaleSmileSection(sec1, atm, true, false, false, money));

    // sanity check for left point extrapolation may mark 0.01 as bad as well as
    // good depending
    // on platform and compiler due to numerical differences, so we have to
    // admit two possible results
    if (std::fabs(ksec12->leftCoreStrike() - 0.02) > tol &&
        std::fabs(ksec12->leftCoreStrike() - 0.01) > tol)
        BOOST_ERROR("smile12 left af strike is " << ksec12->leftCoreStrike()
                                                 << "expected 0.01 or 0.02");

    if (std::fabs(ksec12->rightCoreStrike() - 0.10) > tol)
        BOOST_ERROR("smile12 right af strike is " << ksec12->rightCoreStrike()
                                                  << "expected 0.10");

    for (Size i = 1; i < strikes.size(); i++) {
        Real pric0 = sec1->optionPrice(strikes[i]);
        Real pric1 = ksec12->optionPrice(strikes[i]);
        if (std::fabs(pric0 - pric1) > tol)
            BOOST_ERROR("smile12 is not reproduced on grid at strike "
                        << strikes[i] << " input smile call price is " << pric0
                        << " kahale smile call price is " << pric1);
    }

    // test global no arbitrageability

    k = 0.0010;
    Real dig00 = 1.0, dig10 = 1.0;
    while (k <= 2.0 * strikes.back() + tol) {
        Real dig0 = ksec11->digitalOptionPrice(k);
        Real dig1 = ksec12->digitalOptionPrice(k);
        if (!(dig0 <= dig00 + tol && dig0 >= 0.0))
            BOOST_ERROR("arbitrage in digitals11 (" << dig00 << "," << dig0
                                                    << ") at strike " << k);
        if (!(dig1 <= dig10 + tol && dig1 >= 0.0))
            BOOST_ERROR("arbitrage in digitals12 (" << dig10 << "," << dig1
                                                    << ") at strike " << k);
        dig00 = dig0;
        dig10 = dig1;
        k += 0.0001;
    }

    // test exponential extrapolation

    ext::shared_ptr<KahaleSmileSection> ksec13(
        new KahaleSmileSection(sec1, atm, false, true, false, money));

    k = strikes.back();
    Real dig0 = ksec13->digitalOptionPrice(k - 0.0010);
    while (k <= 10.0 * strikes.back() + tol) {
        Real dig = ksec13->digitalOptionPrice(k);
        if (!(dig <= dig0 + tol && dig >= 0.0))
            BOOST_ERROR("arbitrage in digitals13 (" << dig0 << "," << dig
                                                    << ") at strike " << k);
        k += 0.0001;
    }

    // test arbitrageable smile (leftmost point)

    std::vector<Real> calls1(calls0);
    calls1[0] = (atm - strikes[0]) +
                0.0010; // introduce arbitrage by changing call price
    std::vector<Real> stdDevs1 = impliedStdDevs(atm, strikes, calls1);
    ext::shared_ptr<SmileSection> sec2(
        new InterpolatedSmileSection<Linear>(t, strikes, stdDevs1, atm));

    ext::shared_ptr<KahaleSmileSection> ksec21(
        new KahaleSmileSection(sec2, atm, false, false, false, money));
    ext::shared_ptr<KahaleSmileSection> ksec22(
        new KahaleSmileSection(sec2, atm, true, false, true, money));

    if (std::fabs(ksec21->leftCoreStrike() - 0.02) > tol)
        BOOST_ERROR("smile21 left af strike is " << ksec21->leftCoreStrike()
                                                 << " expected 0.02");
    if (std::fabs(ksec22->leftCoreStrike() - 0.02) > tol)
        BOOST_ERROR("smile22 left af strike is " << ksec22->leftCoreStrike()
                                                 << " expected 0.02");

    if (std::fabs(ksec21->rightCoreStrike() - 0.10) > tol)
        BOOST_ERROR("smile21 right af strike is " << ksec21->rightCoreStrike()
                                                  << " expected 0.10");
    if (std::fabs(ksec22->rightCoreStrike() - 0.10) > tol)
        BOOST_ERROR("smile22 right af strike is " << ksec22->rightCoreStrike()
                                                  << " expected 0.10");

    k = 0.0010;
    dig00 = dig10 = 1.0;
    while (k <= 2.0 * strikes.back() + tol) {
        Real dig0 = ksec21->digitalOptionPrice(k);
        Real dig1 = ksec22->digitalOptionPrice(k);
        if (!(dig0 <= dig00 + tol && dig0 >= 0.0))
            BOOST_ERROR("arbitrage in digitals21 (" << dig00 << "," << dig0
                                                    << ") at strike " << k);
        if (!(dig1 <= dig10 + tol && dig1 >= 0.0))
            BOOST_ERROR("arbitrage in digitals22 (" << dig10 << "," << dig1
                                                    << ") at strike " << k);
        dig00 = dig0;
        dig10 = dig1;
        k += 0.0001;
    }

    // test arbitrageable smile (second but rightmost point)

    std::vector<Real> calls2(calls0);
    calls2[8] = 0.9 * calls2[9] +
                0.1 * calls2[8]; // introduce arbitrage by changing call price
    std::vector<Real> stdDevs2 = impliedStdDevs(atm, strikes, calls2);
    ext::shared_ptr<SmileSection> sec3(
        new InterpolatedSmileSection<Linear>(t, strikes, stdDevs2, atm));

    ext::shared_ptr<KahaleSmileSection> ksec31(
        new KahaleSmileSection(sec3, atm, false, false, false, money));
    ext::shared_ptr<KahaleSmileSection> ksec32(
        new KahaleSmileSection(sec3, atm, true, false, true, money));

    if (std::fabs(ksec31->leftCoreStrike() - 0.01) > tol)
        BOOST_ERROR("smile31 left af strike is " << ksec31->leftCoreStrike()
                                                 << " expected 0.01");

    // sanity check for left point extrapolation may mark 0.01 as bad as well as
    // good depending
    // on platform and compiler due to numerical differences, so we have to
    // admit two possible results
    if (std::fabs(ksec32->leftCoreStrike() - 0.02) > tol &&
        std::fabs(ksec32->leftCoreStrike() - 0.01) > tol)
        BOOST_ERROR("smile32 left af strike is " << ksec32->leftCoreStrike()
                                                 << " expected 0.01 or 0.02");

    if (std::fabs(ksec31->rightCoreStrike() - 0.08) > tol)
        BOOST_ERROR("smile31 right af strike is " << ksec31->rightCoreStrike()
                                                  << " expected 0.08");
    if (std::fabs(ksec32->rightCoreStrike() - 0.10) > tol)
        BOOST_ERROR("smile32 right af strike is " << ksec32->rightCoreStrike()
                                                  << " expected 0.10");
    k = 0.0010;
    dig00 = dig10 = 1.0;
    while (k <= 2.0 * strikes.back() + tol) {
        Real dig0 = ksec31->digitalOptionPrice(k);
        Real dig1 = ksec32->digitalOptionPrice(k);
        if (!(dig0 <= dig00 + tol && dig0 >= 0.0))
            BOOST_ERROR("arbitrage in digitals31 (" << dig00 << "," << dig0
                                                    << ") at strike " << k);
        if (!(dig1 <= dig10 + tol && dig1 >= 0.0))
            BOOST_ERROR("arbitrage in digitals32 (" << dig10 << "," << dig1
                                                    << ") at strike " << k);
        dig00 = dig0;
        dig10 = dig1;
        k += 0.0001;
    }
}

void MarkovFunctionalTest::testCalibrationOneInstrumentSet() {

    const Real tol0 = 0.0001; //  1bp tolerance for model zero rates vs. market
                              // zero rates (note that model zero rates are
                              // implied by the calibration of the numeraire to
                              // the smile)
    const Real tol1 =
        0.0001; // 1bp tolerance for model call put premia vs. market premia

    BOOST_TEST_MESSAGE(
        "Testing Markov functional calibration to one instrument set...");

    Date savedEvalDate = Settings::instance().evaluationDate();
    Date referenceDate(14, November, 2012);
    Settings::instance().evaluationDate() = referenceDate;

    Handle<YieldTermStructure> flatYts_ = flatYts();
    Handle<YieldTermStructure> md0Yts_ = md0Yts();
    Handle<SwaptionVolatilityStructure> flatSwaptionVts_ = flatSwaptionVts();
    Handle<SwaptionVolatilityStructure> md0SwaptionVts_ = md0SwaptionVts();
    Handle<OptionletVolatilityStructure> flatOptionletVts_ = flatOptionletVts();
    Handle<OptionletVolatilityStructure> md0OptionletVts_ = md0OptionletVts();

    ext::shared_ptr<SwapIndex> swapIndexBase(
        new EuriborSwapIsdaFixA(1 * Years));
    ext::shared_ptr<IborIndex> iborIndex(new Euribor(6 * Months));

    std::vector<Date> volStepDates;
    std::vector<Real> vols = {1.0};

    // use a grid with fewer points for smile arbitrage
    // testing and model outputs than the default grid
    // for the testing here
    std::vector<Real> money = {0.1, 0.25, 0.50, 0.75, 1.0, 1.25, 1.50, 2.0, 5.0};

    // Calibration Basket 1 / flat yts, vts

    ext::shared_ptr<MarkovFunctional> mf1(new MarkovFunctional(
        flatYts_, 0.01, volStepDates, vols, flatSwaptionVts_,
        expiriesCalBasket1(), tenorsCalBasket1(), swapIndexBase,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(64) // we use the default values more or less, this
                                 // is just to demonstrate how to set the model
                                 // parameters
            .withYStdDevs(7.0)
            .withGaussHermitePoints(32)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withAdjustments(
                 MarkovFunctional::ModelSettings::KahaleSmile |
                 MarkovFunctional::ModelSettings::SmileExponentialExtrapolation)
            .withSmileMoneynessCheckpoints(money)));

    MarkovFunctional::ModelOutputs outputs1 =
        mf1->modelOutputs(); // this costs a lot of time, so only use it if you
                             // want to check the calibration
    // BOOST_TEST_MESSAGE(outputs1);

    for (Size i = 0; i < outputs1.expiries_.size(); i++) {
        if (fabs(outputs1.marketZerorate_[i] - outputs1.modelZerorate_[i]) >
            tol0)
            BOOST_ERROR("Basket 1 / flat termstructures : Market zero rate ("
                        << outputs1.marketZerorate_[i]
                        << ") and model zero rate ("
                        << outputs1.modelZerorate_[i] << ") do not agree.");
    }

    for (Size i = 0; i < outputs1.expiries_.size(); i++) {
        for (Size j = 0; j < outputs1.smileStrikes_[i].size(); j++) {
            if (fabs(outputs1.marketCallPremium_[i][j] -
                     outputs1.modelCallPremium_[i][j]) > tol1)
                BOOST_ERROR(
                    "Basket 1 / flat termstructures : Market call premium ("
                    << outputs1.marketCallPremium_[i][j]
                    << ") does not match model premium ("
                    << outputs1.modelCallPremium_[i][j] << ")");
            if (fabs(outputs1.marketPutPremium_[i][j] -
                     outputs1.modelPutPremium_[i][j]) > tol1)
                BOOST_ERROR(
                    "Basket 1 / flat termstructures : Market put premium ("
                    << outputs1.marketPutPremium_[i][j]
                    << ") does not match model premium ("
                    << outputs1.modelPutPremium_[i][j] << ")");
        }
    }

    // Calibration Basket 2 / flat yts, vts

    ext::shared_ptr<MarkovFunctional> mf2(new MarkovFunctional(
        flatYts_, 0.01, volStepDates, vols, flatOptionletVts_,
        expiriesCalBasket2(), iborIndex,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(64)
            .withYStdDevs(7.0)
            .withGaussHermitePoints(32)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withAdjustments(MarkovFunctional::ModelSettings::AdjustNone)
            .withSmileMoneynessCheckpoints(money)));

    MarkovFunctional::ModelOutputs outputs2 = mf2->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs2);

    for (Size i = 0; i < outputs2.expiries_.size(); i++) {
        if (fabs(outputs2.marketZerorate_[i] - outputs2.modelZerorate_[i]) >
            tol0)
            BOOST_ERROR("Basket 2 / flat termstructures : Market zero rate ("
                        << outputs2.marketZerorate_[i]
                        << ") and model zero rate ("
                        << outputs2.modelZerorate_[i] << ") do not agree.");
    }

    for (Size i = 0; i < outputs2.expiries_.size(); i++) {
        for (Size j = 0; j < outputs2.smileStrikes_[i].size(); j++) {
            if (fabs(outputs2.marketCallPremium_[i][j] -
                     outputs2.modelCallPremium_[i][j]) > tol1)
                BOOST_ERROR(
                    "Basket 2 / flat termstructures : Market call premium ("
                    << outputs2.marketCallPremium_[i][j]
                    << ") does not match model premium ("
                    << outputs2.modelCallPremium_[i][j] << ")");
            if (fabs(outputs2.marketPutPremium_[i][j] -
                     outputs2.modelPutPremium_[i][j]) > tol1)
                BOOST_ERROR(
                    "Basket 2/ flat termstructures : Market put premium ("
                    << outputs2.marketPutPremium_[i][j]
                    << ") does not match model premium ("
                    << outputs2.modelPutPremium_[i][j] << ")");
        }
    }

    // Calibration Basket 1 / real yts, vts

    ext::shared_ptr<MarkovFunctional> mf3(new MarkovFunctional(
        md0Yts_, 0.01, volStepDates, vols, md0SwaptionVts_,
        expiriesCalBasket1(), tenorsCalBasket1(), swapIndexBase,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(128) // use more points to increase accuracy
            .withYStdDevs(7.0)
            .withGaussHermitePoints(64)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withSmileMoneynessCheckpoints(money)));

    MarkovFunctional::ModelOutputs outputs3 = mf3->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs3);
    // outputSurfaces(mf3,md0Yts_);

    for (Size i = 0; i < outputs3.expiries_.size(); i++) {
        if (fabs(outputs3.marketZerorate_[i] - outputs3.modelZerorate_[i]) >
            tol0)
            BOOST_ERROR("Basket 1 / real termstructures: Market zero rate ("
                        << outputs3.marketZerorate_[i]
                        << ") and model zero rate ("
                        << outputs3.modelZerorate_[i] << ") do not agree.");
    }

    for (Size i = 0; i < outputs3.expiries_.size(); i++) {
        for (Size j = 0; j < outputs3.smileStrikes_[i].size(); j++) {
            if (fabs(outputs3.marketCallPremium_[i][j] -
                     outputs3.modelCallPremium_[i][j]) > tol1)
                BOOST_ERROR(
                    "Basket 1 / real termstructures: Market call premium ("
                    << outputs3.marketCallPremium_[i][j]
                    << ") does not match model premium ("
                    << outputs3.modelCallPremium_[i][j] << ")");
            if (fabs(outputs3.marketPutPremium_[i][j] -
                     outputs3.modelPutPremium_[i][j]) > tol1)
                BOOST_ERROR(
                    "Basket 1 /  real termstructures: Market put premium ("
                    << outputs3.marketPutPremium_[i][j]
                    << ") does not match model premium ("
                    << outputs3.modelPutPremium_[i][j] << ")");
        }
    }

    // Calibration Basket 2 / real yts, vts

    ext::shared_ptr<MarkovFunctional> mf4(
        new MarkovFunctional(md0Yts_, 0.01, volStepDates, vols,
                             md0OptionletVts_, expiriesCalBasket2(), iborIndex,
                             MarkovFunctional::ModelSettings()
                                 .withYGridPoints(64)
                                 .withYStdDevs(7.0)
                                 .withGaussHermitePoints(32)
                                 .withDigitalGap(1e-5)
                                 .withMarketRateAccuracy(1e-7)
                                 .withLowerRateBound(0.0)
                                 .withUpperRateBound(2.0)
                                 .withSmileMoneynessCheckpoints(money)));

    MarkovFunctional::ModelOutputs outputs4 = mf4->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs4);

    for (Size i = 0; i < outputs4.expiries_.size(); i++) {
        if (fabs(outputs4.marketZerorate_[i] - outputs4.modelZerorate_[i]) >
            tol0)
            BOOST_ERROR("Basket 2 / real termstructures : Market zero rate ("
                        << outputs4.marketZerorate_[i]
                        << ") and model zero rate ("
                        << outputs4.modelZerorate_[i] << ") do not agree.");
    }

    for (Size i = 0; i < outputs4.expiries_.size(); i++) {
        for (Size j = 0; j < outputs4.smileStrikes_[i].size(); j++) {
            if (fabs(outputs4.marketCallPremium_[i][j] -
                     outputs4.modelCallPremium_[i][j]) > tol1)
                BOOST_ERROR(
                    "Basket 2 / real termstructures : Market call premium ("
                    << outputs4.marketCallPremium_[i][j]
                    << ") does not match model premium ("
                    << outputs4.modelCallPremium_[i][j] << ")");
            if (fabs(outputs4.marketPutPremium_[i][j] -
                     outputs4.modelPutPremium_[i][j]) > tol1)
                BOOST_ERROR(
                    "Basket 2/ real termstructures : Market put premium ("
                    << outputs4.marketPutPremium_[i][j]
                    << ") does not match model premium ("
                    << outputs4.modelPutPremium_[i][j] << ")");
        }
    }

    Settings::instance().evaluationDate() = savedEvalDate;
}

void MarkovFunctionalTest::testVanillaEngines() {

    const Real tol1 = 0.0001; // 1bp tolerance for model engine call put premia
                              // vs. black premia
    // note that we use the real market conventions here (i.e. 2 fixing days),
    // different from the calibration approach where 0 fixing days must be used.
    // therefore higher errors compared to the calibration results are expected.

    BOOST_TEST_MESSAGE("Testing Markov functional vanilla engines...");

    Date savedEvalDate = Settings::instance().evaluationDate();
    Date referenceDate(14, November, 2012);
    Settings::instance().evaluationDate() = referenceDate;

    Handle<YieldTermStructure> flatYts_ = flatYts();
    Handle<YieldTermStructure> md0Yts_ = md0Yts();
    Handle<SwaptionVolatilityStructure> flatSwaptionVts_ = flatSwaptionVts();
    Handle<SwaptionVolatilityStructure> md0SwaptionVts_ = md0SwaptionVts();
    Handle<OptionletVolatilityStructure> flatOptionletVts_ = flatOptionletVts();
    Handle<OptionletVolatilityStructure> md0OptionletVts_ = md0OptionletVts();

    ext::shared_ptr<SwapIndex> swapIndexBase(
        new EuriborSwapIsdaFixA(1 * Years));

    std::vector<Date> volStepDates;
    std::vector<Real> vols = {1.0};

    // use a grid with few points for the check here
    std::vector<Real> money = {0.1, 0.25, 0.50, 0.75, 1.0, 1.25, 1.50, 2.0, 5.0};

    // Calibration Basket 1 / flat yts, vts

    ext::shared_ptr<IborIndex> iborIndex1(new Euribor(6 * Months, flatYts_));

    ext::shared_ptr<MarkovFunctional> mf1(new MarkovFunctional(
        flatYts_, 0.01, volStepDates, vols, flatSwaptionVts_,
        expiriesCalBasket1(), tenorsCalBasket1(), swapIndexBase,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(64)
            .withYStdDevs(7.0)
            .withGaussHermitePoints(32)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withSmileMoneynessCheckpoints(money)));

    MarkovFunctional::ModelOutputs outputs1 = mf1->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs1);

    ext::shared_ptr<Gaussian1dSwaptionEngine> mfSwaptionEngine1(
        new Gaussian1dSwaptionEngine(mf1, 64, 7.0));
    ext::shared_ptr<BlackSwaptionEngine> blackSwaptionEngine1(
        new BlackSwaptionEngine(flatYts_, flatSwaptionVts_));

    for (Size i = 0; i < outputs1.expiries_.size(); i++) {
        for (Size j = 0; j < outputs1.smileStrikes_[0].size(); j++) {
            ext::shared_ptr<VanillaSwap> underlyingCall =
                MakeVanillaSwap(outputs1.tenors_[i], iborIndex1,
                                outputs1.smileStrikes_[i][j])
                    .withEffectiveDate(
                         TARGET().advance(outputs1.expiries_[i], 2, Days))
                    .receiveFixed(false);
            ext::shared_ptr<VanillaSwap> underlyingPut =
                MakeVanillaSwap(outputs1.tenors_[i], iborIndex1,
                                outputs1.smileStrikes_[i][j])
                    .withEffectiveDate(
                         TARGET().advance(outputs1.expiries_[i], 2, Days))
                    .receiveFixed(true);
            ext::shared_ptr<Exercise> exercise(
                new EuropeanExercise(outputs1.expiries_[i]));
            Swaption swaptionC(underlyingCall, exercise);
            Swaption swaptionP(underlyingPut, exercise);
            swaptionC.setPricingEngine(blackSwaptionEngine1);
            swaptionP.setPricingEngine(blackSwaptionEngine1);
            Real blackPriceCall = swaptionC.NPV();
            Real blackPricePut = swaptionP.NPV();
            swaptionC.setPricingEngine(mfSwaptionEngine1);
            swaptionP.setPricingEngine(mfSwaptionEngine1);
            Real mfPriceCall = swaptionC.NPV();
            Real mfPricePut = swaptionP.NPV();
            if (fabs(blackPriceCall - mfPriceCall) > tol1)
                BOOST_ERROR(
                    "Basket 1 / flat termstructures: Call premium market ("
                    << blackPriceCall << ") does not match model premium ("
                    << mfPriceCall << ")");
            if (fabs(blackPricePut - mfPricePut) > tol1)
                BOOST_ERROR(
                    "Basket 1 / flat termstructures: Put premium market ("
                    << blackPricePut << ") does not match model premium ("
                    << mfPricePut << ")");
        }
    }

    // Calibration Basket 2 / flat yts, vts

    ext::shared_ptr<IborIndex> iborIndex2(new Euribor(6 * Months, flatYts_));

    ext::shared_ptr<MarkovFunctional> mf2(new MarkovFunctional(
        flatYts_, 0.01, volStepDates, vols, flatOptionletVts_,
        expiriesCalBasket2(), iborIndex2,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(64)
            .withYStdDevs(7.0)
            .withGaussHermitePoints(16)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withSmileMoneynessCheckpoints(money)));

    MarkovFunctional::ModelOutputs outputs2 = mf2->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs2);

    ext::shared_ptr<BlackCapFloorEngine> blackCapFloorEngine2(
        new BlackCapFloorEngine(flatYts_, flatOptionletVts_));
    ext::shared_ptr<Gaussian1dCapFloorEngine> mfCapFloorEngine2(
        new Gaussian1dCapFloorEngine(mf2, 64, 7.0));
    std::vector<CapFloor> c2 = {
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex2, 0.01),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex2, 0.02),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex2, 0.03),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex2, 0.04),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex2, 0.05),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex2, 0.07),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex2, 0.10),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex2, 0.01),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex2, 0.02),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex2, 0.03),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex2, 0.04),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex2, 0.05),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex2, 0.07),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex2, 0.10)
    };

    for (auto& i : c2) {
        i.setPricingEngine(blackCapFloorEngine2);
        Real blackPrice = i.NPV();
        i.setPricingEngine(mfCapFloorEngine2);
        Real mfPrice = i.NPV();
        if (fabs(blackPrice - mfPrice) > tol1)
            BOOST_ERROR(
                "Basket 2 / flat termstructures: Cap/Floor premium market ("
                << blackPrice << ") does not match model premium (" << mfPrice
                << ")");
    }

    // Calibration Basket 1 / real yts, vts

    ext::shared_ptr<IborIndex> iborIndex3(new Euribor(6 * Months, md0Yts_));

    ext::shared_ptr<MarkovFunctional> mf3(new MarkovFunctional(
        md0Yts_, 0.01, volStepDates, vols, md0SwaptionVts_,
        expiriesCalBasket1(), tenorsCalBasket1(), swapIndexBase,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(64)
            .withYStdDevs(7.0)
            .withGaussHermitePoints(32)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withSmileMoneynessCheckpoints(money)));

    ext::shared_ptr<Gaussian1dSwaptionEngine> mfSwaptionEngine3(
        new Gaussian1dSwaptionEngine(mf3, 64, 7.0));
    ext::shared_ptr<BlackSwaptionEngine> blackSwaptionEngine3(
        new BlackSwaptionEngine(md0Yts_, md0SwaptionVts_));

    MarkovFunctional::ModelOutputs outputs3 = mf3->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs3);

    for (Size i = 0; i < outputs3.expiries_.size(); i++) {
        for (Size j = 0; j < outputs3.smileStrikes_[0].size(); j++) {
            ext::shared_ptr<VanillaSwap> underlyingCall =
                MakeVanillaSwap(outputs3.tenors_[i], iborIndex3,
                                outputs3.smileStrikes_[i][j])
                    .withEffectiveDate(
                         TARGET().advance(outputs3.expiries_[i], 2, Days))
                    .receiveFixed(false);
            ext::shared_ptr<VanillaSwap> underlyingPut =
                MakeVanillaSwap(outputs3.tenors_[i], iborIndex3,
                                outputs3.smileStrikes_[i][j])
                    .withEffectiveDate(
                         TARGET().advance(outputs3.expiries_[i], 2, Days))
                    .receiveFixed(true);
            ext::shared_ptr<Exercise> exercise(
                new EuropeanExercise(outputs3.expiries_[i]));
            Swaption swaptionC(underlyingCall, exercise);
            Swaption swaptionP(underlyingPut, exercise);
            swaptionC.setPricingEngine(blackSwaptionEngine3);
            swaptionP.setPricingEngine(blackSwaptionEngine3);
            Real blackPriceCall = swaptionC.NPV();
            Real blackPricePut = swaptionP.NPV();
            swaptionC.setPricingEngine(mfSwaptionEngine3);
            swaptionP.setPricingEngine(mfSwaptionEngine3);
            Real mfPriceCall = swaptionC.NPV();
            Real mfPricePut = swaptionP.NPV();
            Real smileCorrectionCall =
                (outputs3.marketCallPremium_[i][j] -
                 outputs3.marketRawCallPremium_[i][j]); // we can not expect to
                                                        // match the black
                                                        // scholes price where
                                                        // the smile is adjusted
            Real smileCorrectionPut = (outputs3.marketPutPremium_[i][j] -
                                       outputs3.marketRawPutPremium_[i][j]);
            if (fabs(blackPriceCall - mfPriceCall + smileCorrectionCall) > tol1)
                BOOST_ERROR(
                    "Basket 1 / real termstructures: Call premium market ("
                    << blackPriceCall << ") does not match model premium ("
                    << mfPriceCall << ")");
            if (fabs(blackPricePut - mfPricePut + smileCorrectionPut) > tol1)
                BOOST_ERROR(
                    "Basket 1 / real termstructures: Put premium market ("
                    << blackPricePut << ") does not match model premium ("
                    << mfPricePut << ")");
        }
    }

    // Calibration Basket 2 / real yts, vts

    ext::shared_ptr<IborIndex> iborIndex4(new Euribor(6 * Months, md0Yts_));

    ext::shared_ptr<MarkovFunctional> mf4(new MarkovFunctional(
        md0Yts_, 0.01, volStepDates, vols, md0OptionletVts_,
        expiriesCalBasket2(), iborIndex4,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(64)
            .withYStdDevs(7.0)
            .withGaussHermitePoints(32)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withSmileMoneynessCheckpoints(money)
        //.withAdjustments(MarkovFunctional::ModelSettings::KahaleSmile
        //                                              |
        // MarkovFunctional::ModelSettings::KahaleInterpolation
        //| MarkovFunctional::ModelSettings::KahaleExponentialExtrapolation
        //)
        ));

    MarkovFunctional::ModelOutputs outputs4 = mf4->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs4);

    ext::shared_ptr<BlackCapFloorEngine> blackCapFloorEngine4(
        new BlackCapFloorEngine(md0Yts_, md0OptionletVts_));
    ext::shared_ptr<Gaussian1dCapFloorEngine> mfCapFloorEngine4(
        new Gaussian1dCapFloorEngine(mf4, 64, 7.0));

    std::vector<CapFloor> c4 = {
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex4, 0.01),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex4, 0.02),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex4, 0.03),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex4, 0.04),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex4, 0.05),
        MakeCapFloor(CapFloor::Cap, 5 * Years, iborIndex4, 0.06),
        // //exclude because caplet stripper fails for this strike
        // MakeCapFloor(CapFloor::Cap,5*Years,iborIndex4,0.10),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex4, 0.01),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex4, 0.02),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex4, 0.03),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex4, 0.04),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex4, 0.05),
        MakeCapFloor(CapFloor::Floor, 5 * Years, iborIndex4, 0.06)
        // //exclude because caplet stripper fails for this strike
        // MakeCapFloor(CapFloor::Floor,5*Years,iborIndex4,0.10)
    };

    for (auto& i : c4) {
        i.setPricingEngine(blackCapFloorEngine4);
        Real blackPrice = i.NPV();
        i.setPricingEngine(mfCapFloorEngine4);
        Real mfPrice = i.NPV();
        if (fabs(blackPrice - mfPrice) > tol1)
            BOOST_ERROR(
                "Basket 2 / real termstructures: Cap/Floor premium market ("
                << blackPrice << ") does not match model premium (" << mfPrice
                << ")");
    }

    Settings::instance().evaluationDate() = savedEvalDate;
}

void MarkovFunctionalTest::testCalibrationTwoInstrumentSets() {

    const Real tol1 = 0.1; // 0.1 times vega tolerance for model vs. market in
                           // second instrument set
    BOOST_TEST_MESSAGE(
        "Testing Markov functional calibration to two instrument sets...");

    Date savedEvalDate = Settings::instance().evaluationDate();
    Date referenceDate(14, November, 2012);
    Settings::instance().evaluationDate() = referenceDate;

    Handle<YieldTermStructure> flatYts_ = flatYts();
    Handle<YieldTermStructure> md0Yts_ = md0Yts();
    Handle<SwaptionVolatilityStructure> flatSwaptionVts_ = flatSwaptionVts();
    Handle<SwaptionVolatilityStructure> md0SwaptionVts_ = md0SwaptionVts();
    Handle<OptionletVolatilityStructure> flatOptionletVts_ = flatOptionletVts();
    Handle<OptionletVolatilityStructure> md0OptionletVts_ = md0OptionletVts();

    ext::shared_ptr<SwapIndex> swapIndexBase(
        new EuriborSwapIsdaFixA(1 * Years));

    std::vector<Date> volStepDates = {
        TARGET().advance(referenceDate, 1 * Years),
        TARGET().advance(referenceDate, 2 * Years),
        TARGET().advance(referenceDate, 3 * Years),
        TARGET().advance(referenceDate, 4 * Years)
    };
    std::vector<Real> vols = {1.0, 1.0, 1.0, 1.0, 1.0};
    std::vector<Real> money = {0.1, 0.25, 0.50, 0.75, 1.0, 1.25, 1.50, 2.0, 5.0};

    LevenbergMarquardt om;
    // ConjugateGradient om;
    EndCriteria ec(1000, 500, 1e-2, 1e-2, 1e-2);

    // Calibration Basket 1 / flat yts, vts / Secondary calibration set consists
    // of coterminal swaptions

    ext::shared_ptr<IborIndex> iborIndex1(new Euribor(6 * Months, flatYts_));

    std::vector<ext::shared_ptr<BlackCalibrationHelper> > calibrationHelper1;
    std::vector<Real> calibrationHelperVols1 = {0.20, 0.20, 0.20, 0.20};

    calibrationHelper1.push_back(ext::shared_ptr<BlackCalibrationHelper>(
        new SwaptionHelper(1 * Years, 4 * Years,
                           Handle<Quote>(ext::shared_ptr<Quote>(
                               new SimpleQuote(calibrationHelperVols1[0]))),
                           iborIndex1, 1 * Years, Thirty360(Thirty360::BondBasis), Actual360(),
                           flatYts_)));
    calibrationHelper1.push_back(ext::shared_ptr<BlackCalibrationHelper>(
        new SwaptionHelper(2 * Years, 3 * Years,
                           Handle<Quote>(ext::shared_ptr<Quote>(
                               new SimpleQuote(calibrationHelperVols1[1]))),
                           iborIndex1, 1 * Years, Thirty360(Thirty360::BondBasis), Actual360(),
                           flatYts_)));
    calibrationHelper1.push_back(ext::shared_ptr<BlackCalibrationHelper>(
        new SwaptionHelper(3 * Years, 2 * Years,
                           Handle<Quote>(ext::shared_ptr<Quote>(
                               new SimpleQuote(calibrationHelperVols1[2]))),
                           iborIndex1, 1 * Years, Thirty360(Thirty360::BondBasis), Actual360(),
                           flatYts_)));
    calibrationHelper1.push_back(ext::shared_ptr<BlackCalibrationHelper>(
        new SwaptionHelper(4 * Years, 1 * Years,
                           Handle<Quote>(ext::shared_ptr<Quote>(
                               new SimpleQuote(calibrationHelperVols1[3]))),
                           iborIndex1, 1 * Years, Thirty360(Thirty360::BondBasis), Actual360(),
                           flatYts_)));

    ext::shared_ptr<MarkovFunctional> mf1(new MarkovFunctional(
        flatYts_, 0.01, volStepDates, vols, flatSwaptionVts_,
        expiriesCalBasket1(), tenorsCalBasket1(), swapIndexBase,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(64)
            .withYStdDevs(7.0)
            .withGaussHermitePoints(32)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withSmileMoneynessCheckpoints(money)));

    ext::shared_ptr<Gaussian1dSwaptionEngine> mfSwaptionEngine1(
        new Gaussian1dSwaptionEngine(mf1, 64, 7.0));
    calibrationHelper1[0]->setPricingEngine(mfSwaptionEngine1);
    calibrationHelper1[1]->setPricingEngine(mfSwaptionEngine1);
    calibrationHelper1[2]->setPricingEngine(mfSwaptionEngine1);
    calibrationHelper1[3]->setPricingEngine(mfSwaptionEngine1);

    mf1->calibrate(calibrationHelper1, om, ec);

    // std::cout << "Calibrated parameters 1: ";
    // Array params1 = mf1->params();
    // for(Size i=0;i<params1.size();i++) std::cout << params1[i] << ";";
    // std::cout << std::endl;

    std::vector<Swaption> ch1;
    ch1.push_back(
        MakeSwaption(ext::shared_ptr<SwapIndex>(
                         new EuriborSwapIsdaFixA(4 * Years, flatYts_)),
                     1 * Years));
    ch1.push_back(
        MakeSwaption(ext::shared_ptr<SwapIndex>(
                         new EuriborSwapIsdaFixA(3 * Years, flatYts_)),
                     2 * Years));
    ch1.push_back(
        MakeSwaption(ext::shared_ptr<SwapIndex>(
                         new EuriborSwapIsdaFixA(2 * Years, flatYts_)),
                     3 * Years));
    ch1.push_back(
        MakeSwaption(ext::shared_ptr<SwapIndex>(
                         new EuriborSwapIsdaFixA(1 * Years, flatYts_)),
                     4 * Years));

    for (Size i = 0; i < ch1.size(); i++) {
        ext::shared_ptr<BlackSwaptionEngine> blackEngine(
            new BlackSwaptionEngine(flatYts_, calibrationHelperVols1[i]));
        ch1[i].setPricingEngine(blackEngine);
        Real blackPrice = ch1[i].NPV();
        Real blackVega = ch1[i].result<Real>("vega");
        ch1[i].setPricingEngine(mfSwaptionEngine1);
        Real mfPrice = ch1[i].NPV();
        if (fabs(blackPrice - mfPrice) / blackVega > tol1)
            BOOST_TEST_MESSAGE("Basket 1 / flat yts, vts: Secondary instrument set "
                          "calibration failed for instrument #"
                          << i << " black premium is " << blackPrice
                          << " while model premium is " << mfPrice
                          << " (market vega is " << blackVega << ")");
    }

    // MarkovFunctional::ModelOutputs outputs1 = mf1->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs1);

    // Calibration Basket 1 / real yts, vts / Secondary calibration set consists
    // of coterminal swaptions

    ext::shared_ptr<IborIndex> iborIndex2(new Euribor(6 * Months, md0Yts_));

    ext::shared_ptr<MarkovFunctional> mf2(new MarkovFunctional(
        md0Yts_, 0.01, volStepDates, vols, md0SwaptionVts_,
        expiriesCalBasket1(), tenorsCalBasket1(), swapIndexBase,
        MarkovFunctional::ModelSettings()
            .withYGridPoints(64)
            .withYStdDevs(7.0)
            .withGaussHermitePoints(32)
            .withDigitalGap(1e-5)
            .withMarketRateAccuracy(1e-7)
            .withLowerRateBound(0.0)
            .withUpperRateBound(2.0)
            .withSmileMoneynessCheckpoints(money)));

    std::vector<ext::shared_ptr<BlackCalibrationHelper> > calibrationHelper2;
    std::vector<Real> calibrationHelperVols2;
    calibrationHelperVols2.push_back(md0SwaptionVts_->volatility(
        1 * Years, 4 * Years,
        ext::dynamic_pointer_cast<SwaptionVolatilityCube>(
            md0SwaptionVts_.currentLink())->atmStrike(1 * Years, 4 * Years)));
    calibrationHelperVols2.push_back(md0SwaptionVts_->volatility(
        2 * Years, 3 * Years,
        ext::dynamic_pointer_cast<SwaptionVolatilityCube>(
            md0SwaptionVts_.currentLink())->atmStrike(2 * Years, 3 * Years)));
    calibrationHelperVols2.push_back(md0SwaptionVts_->volatility(
        3 * Years, 2 * Years,
        ext::dynamic_pointer_cast<SwaptionVolatilityCube>(
            md0SwaptionVts_.currentLink())->atmStrike(3 * Years, 2 * Years)));
    calibrationHelperVols2.push_back(md0SwaptionVts_->volatility(
        4 * Years, 1 * Years,
        ext::dynamic_pointer_cast<SwaptionVolatilityCube>(
            md0SwaptionVts_.currentLink())->atmStrike(4 * Years, 1 * Years)));

    calibrationHelper2.push_back(ext::shared_ptr<BlackCalibrationHelper>(
        new SwaptionHelper(1 * Years, 4 * Years,
                           Handle<Quote>(ext::shared_ptr<Quote>(
                               new SimpleQuote(calibrationHelperVols2[0]))),
                           iborIndex2, 1 * Years, Thirty360(Thirty360::BondBasis), Actual360(),
                           md0Yts_)));
    calibrationHelper2.push_back(ext::shared_ptr<BlackCalibrationHelper>(
        new SwaptionHelper(2 * Years, 3 * Years,
                           Handle<Quote>(ext::shared_ptr<Quote>(
                               new SimpleQuote(calibrationHelperVols2[1]))),
                           iborIndex2, 1 * Years, Thirty360(Thirty360::BondBasis), Actual360(),
                           md0Yts_)));
    calibrationHelper2.push_back(ext::shared_ptr<BlackCalibrationHelper>(
        new SwaptionHelper(3 * Years, 2 * Years,
                           Handle<Quote>(ext::shared_ptr<Quote>(
                               new SimpleQuote(calibrationHelperVols2[2]))),
                           iborIndex2, 1 * Years, Thirty360(Thirty360::BondBasis), Actual360(),
                           md0Yts_)));
    calibrationHelper2.push_back(ext::shared_ptr<BlackCalibrationHelper>(
        new SwaptionHelper(4 * Years, 1 * Years,
                           Handle<Quote>(ext::shared_ptr<Quote>(
                               new SimpleQuote(calibrationHelperVols2[3]))),
                           iborIndex2, 1 * Years, Thirty360(Thirty360::BondBasis), Actual360(),
                           md0Yts_)));

    ext::shared_ptr<Gaussian1dSwaptionEngine> mfSwaptionEngine2(
        new Gaussian1dSwaptionEngine(mf2, 64, 7.0));
    calibrationHelper2[0]->setPricingEngine(mfSwaptionEngine2);
    calibrationHelper2[1]->setPricingEngine(mfSwaptionEngine2);
    calibrationHelper2[2]->setPricingEngine(mfSwaptionEngine2);
    calibrationHelper2[3]->setPricingEngine(mfSwaptionEngine2);

    mf2->calibrate(calibrationHelper2, om, ec);

    // std::cout << "Calibrated parameters 2: ";
    // Array params2 = mf2->params();
    // for(Size i=0;i<params2.size();i++) std::cout << params2[i] << ";";
    // std::cout << std::endl;

    std::vector<Swaption> ch2;
    ch2.push_back(MakeSwaption(ext::shared_ptr<SwapIndex>(
                                   new EuriborSwapIsdaFixA(4 * Years, md0Yts_)),
                               1 * Years));
    ch2.push_back(MakeSwaption(ext::shared_ptr<SwapIndex>(
                                   new EuriborSwapIsdaFixA(3 * Years, md0Yts_)),
                               2 * Years));
    ch2.push_back(MakeSwaption(ext::shared_ptr<SwapIndex>(
                                   new EuriborSwapIsdaFixA(2 * Years, md0Yts_)),
                               3 * Years));
    ch2.push_back(MakeSwaption(ext::shared_ptr<SwapIndex>(
                                   new EuriborSwapIsdaFixA(1 * Years, md0Yts_)),
                               4 * Years));

    for (Size i = 0; i < ch2.size(); i++) {
        ext::shared_ptr<BlackSwaptionEngine> blackEngine(
            new BlackSwaptionEngine(md0Yts_, calibrationHelperVols2[i]));
        ch2[i].setPricingEngine(blackEngine);
        Real blackPrice = ch2[i].NPV();
        Real blackVega = ch2[i].result<Real>("vega");
        ch2[i].setPricingEngine(mfSwaptionEngine2);
        Real mfPrice = ch2[i].NPV();
        if (fabs(blackPrice - mfPrice) / blackVega > tol1)
            BOOST_TEST_MESSAGE("Basket 1 / real yts, vts: Secondary instrument set "
                          "calibration failed for instrument #"
                          << i << " black premium is " << blackPrice
                          << " while model premium is " << mfPrice
                          << " (market vega is " << blackVega << ")");
    }

    // MarkovFunctional::ModelOutputs outputs2 = mf2->modelOutputs();
    // BOOST_TEST_MESSAGE(outputs2);

    Settings::instance().evaluationDate() = savedEvalDate;
}

void MarkovFunctionalTest::testBermudanSwaption() {

    Real tol0 = 0.0001; // 1bp tolerance against cached values

    BOOST_TEST_MESSAGE("Testing Markov functional Bermudan swaption engine...");

    Date savedEvalDate = Settings::instance().evaluationDate();
    Date referenceDate(14, November, 2012);
    Settings::instance().evaluationDate() = referenceDate;

    Handle<YieldTermStructure> flatYts_ = flatYts();
    Handle<YieldTermStructure> md0Yts_ = md0Yts();
    Handle<SwaptionVolatilityStructure> flatSwaptionVts_ = flatSwaptionVts();
    Handle<SwaptionVolatilityStructure> md0SwaptionVts_ = md0SwaptionVts();
    Handle<OptionletVolatilityStructure> flatOptionletVts_ = flatOptionletVts();
    Handle<OptionletVolatilityStructure> md0OptionletVts_ = md0OptionletVts();

    ext::shared_ptr<SwapIndex> swapIndexBase(
        new EuriborSwapIsdaFixA(1 * Years));

    std::vector<Date> volStepDates;
    std::vector<Real> vols = {1.0};

    ext::shared_ptr<IborIndex> iborIndex1(new Euribor(6 * Months, md0Yts_));

    ext::shared_ptr<MarkovFunctional> mf1(
        new MarkovFunctional(md0Yts_, 0.01, volStepDates, vols, md0SwaptionVts_,
                             expiriesCalBasket3(), tenorsCalBasket3(),
                             swapIndexBase, MarkovFunctional::ModelSettings()
                                                .withYGridPoints(32)
                                                .withYStdDevs(7.0)
                                                .withGaussHermitePoints(16)
                                                .withMarketRateAccuracy(1e-7)
                                                .withDigitalGap(1e-5)
                                                .withLowerRateBound(0.0)
                                                .withUpperRateBound(2.0)));

    ext::shared_ptr<PricingEngine> mfSwaptionEngine1(
        new Gaussian1dSwaptionEngine(mf1, 64, 7.0));

    ext::shared_ptr<VanillaSwap> underlyingCall =
        MakeVanillaSwap(10 * Years, iborIndex1, 0.03)
            .withEffectiveDate(TARGET().advance(referenceDate, 2, Days))
        //.withNominal(100000000.0)
            .receiveFixed(false);

    std::vector<ext::shared_ptr<Exercise> > europeanExercises;
    std::vector<Date> expiries = expiriesCalBasket3();
    std::vector<Swaption> europeanSwaptions;
    for (Size i = 0; i < expiries.size(); i++) {
        europeanExercises.push_back(
            ext::shared_ptr<Exercise>(new EuropeanExercise(expiries[i])));
        europeanSwaptions.emplace_back(underlyingCall, europeanExercises[i]);
        europeanSwaptions.back().setPricingEngine(mfSwaptionEngine1);
    }

    ext::shared_ptr<Exercise> bermudanExercise(
        new BermudanExercise(expiries));
    Swaption bermudanSwaption(underlyingCall, bermudanExercise);
    bermudanSwaption.setPricingEngine(mfSwaptionEngine1);

    Real cachedValues[] = { 0.0030757, 0.0107344, 0.0179862,
                            0.0225881, 0.0243215, 0.0229148,
                            0.0191415, 0.0139035, 0.0076354 };
    Real cachedValue = 0.0327776;

    for (Size i = 0; i < expiries.size(); i++) {
        Real npv = europeanSwaptions[i].NPV();
        if (fabs(npv - cachedValues[i]) > tol0)
            BOOST_ERROR("European swaption value ("
                        << npv << ") deviates from cached value ("
                        << cachedValues[i] << ")");
    }

    Real npv = bermudanSwaption.NPV();
    if (fabs(npv - cachedValue) > tol0)
        BOOST_ERROR("Bermudan swaption value ("
                    << npv << ") deviates from cached value (" << cachedValue
                    << ")");

    Settings::instance().evaluationDate() = savedEvalDate;
}

test_suite *MarkovFunctionalTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("Markov functional model tests");

    suite->add(QUANTLIB_TEST_CASE(&MarkovFunctionalTest::testMfStateProcess));
    suite->add(QUANTLIB_TEST_CASE(&MarkovFunctionalTest::testKahaleSmileSection));
    suite->add(QUANTLIB_TEST_CASE(&MarkovFunctionalTest::testBermudanSwaption));

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&MarkovFunctionalTest::testCalibrationTwoInstrumentSets));
    }

    if (speed == Slow) {
        suite->add(QUANTLIB_TEST_CASE(&MarkovFunctionalTest::testCalibrationOneInstrumentSet));
        suite->add(QUANTLIB_TEST_CASE(&MarkovFunctionalTest::testVanillaEngines));
    }

    return suite;
}
