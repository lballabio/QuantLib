/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Quaternion Risk Management Ltd
 All rights reserved.

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

#include "toplevelfixture.hpp"
#include <ql/currencies/all.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/time/calendars/all.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/types.hpp>
#include <ql/instruments/crossccyfixfloatswap.hpp>
#include <ql/pricingengines/swap/crossccyswapengine.hpp>

using namespace std;
using namespace boost::unit_test_framework;
using namespace QuantLib;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CrossCurrencyFixFloatSwapTest)

#define CHECK_XCCY_SWAP_RESULT(what, calculated, expected, tolerance)   \
    if (std::fabs(calculated-expected) > tolerance) { \
        BOOST_ERROR("Failed to reproduce " what ":" \
                    << "\n    expected:   " << std::setprecision(12) << expected \
                    << "\n    calculated: " << std::setprecision(12) << calculated \
                    << "\n    error:      " << std::setprecision(12) << std::fabs(calculated-expected)); \
    }

Handle<YieldTermStructure> usdDiscountCurve() {

    vector<Date> dates(27);
    vector<DiscountFactor> dfs(27);
    Actual365Fixed dayCounter;

    dates[0] = Date(11, Sep, 2018);
    dfs[0] = 1;
    dates[1] = Date(14, Sep, 2018);
    dfs[1] = 0.99994666951096;
    dates[2] = Date(20, Sep, 2018);
    dfs[2] = 0.999627719221066;
    dates[3] = Date(27, Sep, 2018);
    dfs[3] = 0.999254084816959;
    dates[4] = Date(04, Oct, 2018);
    dfs[4] = 0.998837020905631;
    dates[5] = Date(15, Oct, 2018);
    dfs[5] = 0.998176132423265;
    dates[6] = Date(13, Nov, 2018);
    dfs[6] = 0.99644587210048;
    dates[7] = Date(13, Dec, 2018);
    dfs[7] = 0.994644668243218;
    dates[8] = Date(14, Jan, 2019);
    dfs[8] = 0.992596634984033;
    dates[9] = Date(13, Feb, 2019);
    dfs[9] = 0.990636503861861;
    dates[10] = Date(13, Mar, 2019);
    dfs[10] = 0.988809127958345;
    dates[11] = Date(13, Jun, 2019);
    dfs[11] = 0.982417991680868;
    dates[12] = Date(13, Sep, 2019);
    dfs[12] = 0.975723193871552;
    dates[13] = Date(13, Mar, 2020);
    dfs[13] = 0.96219213956104;
    dates[14] = Date(14, Sep, 2020);
    dfs[14] = 0.948588232418325;
    dates[15] = Date(13, Sep, 2021);
    dfs[15] = 0.92279636773464;
    dates[16] = Date(13, Sep, 2022);
    dfs[16] = 0.898345201557914;
    dates[17] = Date(13, Sep, 2023);
    dfs[17] = 0.874715322269088;
    dates[18] = Date(15, Sep, 2025);
    dfs[18] = 0.828658611114833;
    dates[19] = Date(13, Sep, 2028);
    dfs[19] = 0.763030152740947;
    dates[20] = Date(13, Sep, 2030);
    dfs[20] = 0.722238847877756;
    dates[21] = Date(13, Sep, 2033);
    dfs[21] = 0.664460629674362;
    dates[22] = Date(13, Sep, 2038);
    dfs[22] = 0.580288693473926;
    dates[23] = Date(14, Sep, 2043);
    dfs[23] = 0.510857007600479;
    dates[24] = Date(14, Sep, 2048);
    dfs[24] = 0.44941525649436;
    dates[25] = Date(13, Sep, 2058);
    dfs[25] = 0.352389176933952;
    dates[26] = Date(13, Sep, 2068);
    dfs[26] = 0.28183300653329;

    return Handle<YieldTermStructure>(ext::make_shared<DiscountCurve>(dates, dfs, dayCounter));
}

Handle<YieldTermStructure> usdProjectionCurve() {

    vector<Date> dates(25);
    vector<DiscountFactor> dfs(25);
    Actual365Fixed dayCounter;

    dates[0] = Date(11, Sep, 2018);
    dfs[0] = 1;
    dates[1] = Date(13, Dec, 2018);
    dfs[1] = 0.994134145990132;
    dates[2] = Date(19, Dec, 2018);
    dfs[2] = 0.993695776146116;
    dates[3] = Date(20, Mar, 2019);
    dfs[3] = 0.987047992958673;
    dates[4] = Date(19, Jun, 2019);
    dfs[4] = 0.980016364694049;
    dates[5] = Date(18, Sep, 2019);
    dfs[5] = 0.972708376777628;
    dates[6] = Date(18, Dec, 2019);
    dfs[6] = 0.965277162951128;
    dates[7] = Date(18, Mar, 2020);
    dfs[7] = 0.957799302363697;
    dates[8] = Date(14, Sep, 2020);
    dfs[8] = 0.943264331984248;
    dates[9] = Date(13, Sep, 2021);
    dfs[9] = 0.914816470778467;
    dates[10] = Date(13, Sep, 2022);
    dfs[10] = 0.88764714641623;
    dates[11] = Date(13, Sep, 2023);
    dfs[11] = 0.861475671008934;
    dates[12] = Date(13, Sep, 2024);
    dfs[12] = 0.835944798717806;
    dates[13] = Date(15, Sep, 2025);
    dfs[13] = 0.810833947617338;
    dates[14] = Date(14, Sep, 2026);
    dfs[14] = 0.78631849267276;
    dates[15] = Date(13, Sep, 2027);
    dfs[15] = 0.762267648509673;
    dates[16] = Date(13, Sep, 2028);
    dfs[16] = 0.738613627359076;
    dates[17] = Date(13, Sep, 2029);
    dfs[17] = 0.715502378943932;
    dates[18] = Date(13, Sep, 2030);
    dfs[18] = 0.693380472578176;
    dates[19] = Date(13, Sep, 2033);
    dfs[19] = 0.631097994110912;
    dates[20] = Date(13, Sep, 2038);
    dfs[20] = 0.540797634630251;
    dates[21] = Date(14, Sep, 2043);
    dfs[21] = 0.465599237331079;
    dates[22] = Date(14, Sep, 2048);
    dfs[22] = 0.402119473746341;
    dates[23] = Date(13, Sep, 2058);
    dfs[23] = 0.303129773289934;
    dates[24] = Date(13, Sep, 2068);
    dfs[24] = 0.23210070222569;

    return Handle<YieldTermStructure>(ext::make_shared<DiscountCurve>(dates, dfs, dayCounter));
}

Handle<YieldTermStructure> tryDiscountCurve() {

    vector<Date> dates(18);
    vector<DiscountFactor> dfs(18);
    Actual365Fixed dayCounter;

    dates[0] = Date(11, Sep, 2018);
    dfs[0] = 1;
    dates[1] = Date(15, Oct, 2018);
    dfs[1] = 0.979316826759248;
    dates[2] = Date(13, Nov, 2018);
    dfs[2] = 0.959997676372812;
    dates[3] = Date(13, Dec, 2018);
    dfs[3] = 0.939987819768341;
    dates[4] = Date(14, Jan, 2019);
    dfs[4] = 0.917879348095857;
    dates[5] = Date(13, Feb, 2019);
    dfs[5] = 0.897309447005875;
    dates[6] = Date(13, Mar, 2019);
    dfs[6] = 0.878377243062539;
    dates[7] = Date(13, Sep, 2019);
    dfs[7] = 0.76374502801031;
    dates[8] = Date(14, Sep, 2020);
    dfs[8] = 0.595566112318217;
    dates[9] = Date(13, Sep, 2021);
    dfs[9] = 0.483132147134316;
    dates[10] = Date(13, Sep, 2022);
    dfs[10] = 0.402466076327945;
    dates[11] = Date(13, Sep, 2023);
    dfs[11] = 0.345531820837392;
    dates[12] = Date(13, Sep, 2024);
    dfs[12] = 0.298070398810781;
    dates[13] = Date(13, Sep, 2025);
    dfs[13] = 0.264039803303106;
    dates[14] = Date(13, Sep, 2026);
    dfs[14] = 0.237813130821584;
    dates[15] = Date(13, Sep, 2027);
    dfs[15] = 0.216456097559999;
    dates[16] = Date(13, Sep, 2028);
    dfs[16] = 0.200289181912326;
    dates[17] = Date(13, Sep, 2033);
    dfs[17] = 0.122659501286113;

    return Handle<YieldTermStructure>(ext::make_shared<DiscountCurve>(dates, dfs, dayCounter));
}

ext::shared_ptr<CrossCcyFixFloatSwap> makeFixFloatXCCYSwap(Rate spotFx, Rate rate, Spread spread) {

    // USD nominal
    Real usdNominal = 10000000.0;

    // Shared settlement conventions
    BusinessDayConvention payConvention = Following;
    Natural payLag = 0;
    JointCalendar payCalendar = JointCalendar(UnitedStates(UnitedStates::Settlement), UnitedKingdom(), Turkey());

    // Swap start and end date
    Date referenceDate = Settings::instance().evaluationDate();
    referenceDate = payCalendar.adjust(referenceDate);
    Date start = payCalendar.advance(referenceDate, 2 * Days);
    Date end = start + 5 * Years;

    // Fixed TRY schedule
    Schedule fixedSchedule(start, end, 1 * Years, payCalendar, ModifiedFollowing, ModifiedFollowing,
                           DateGeneration::Backward, false);

    // Float USD schedule
    Schedule floatSchedule(start, end, 3 * Months, payCalendar, ModifiedFollowing, ModifiedFollowing,
                           DateGeneration::Backward, false);

    auto index = ext::make_shared<USDLibor>(3 * Months, usdProjectionCurve());

    // Create swap
    return ext::shared_ptr<CrossCcyFixFloatSwap>(
        new CrossCcyFixFloatSwap(CrossCcyFixFloatSwap::Payer, usdNominal * spotFx, TRYCurrency(), fixedSchedule, rate,
                                 Actual360(), payConvention, payLag, payCalendar, usdNominal, USDCurrency(),
                                 floatSchedule, index, spread, payConvention, payLag, payCalendar));
}


BOOST_AUTO_TEST_CASE(testFixFloatXCCYSwapPricing) {
    BOOST_TEST_MESSAGE("Test cross currency fix float swap pricing against known results");

    SavedSettings backup;
    Settings::instance().evaluationDate() = Date(11, Sep, 2018);
	bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    // Create swap, USD 3M Libor vs TRY annual fixed
    Rate spotFx = 6.4304;
    Rate rate = 0.249;
    Spread spread = 0.0;
    auto xccy = makeFixFloatXCCYSwap(spotFx, rate, spread);

    // Attach pricing engine
    auto fxSpotQuote = makeQuoteHandle(1.0 / spotFx);
    auto engine = ext::make_shared<CrossCcySwapEngine>(
        USDCurrency(), usdDiscountCurve(), TRYCurrency(), tryDiscountCurve(), fxSpotQuote);
    xccy->setPricingEngine(engine);

    // Check values
    Real usdTolerance = 0.01;
	
    Real expNpv = usingAtParCoupons ? 129777.91 : 129767.99;
    CHECK_XCCY_SWAP_RESULT("NPV", xccy->NPV(), expNpv, usdTolerance);

    Real expPayLegNpv = -12286.45;
    Real expPayLegBps = -2628.39;
    CHECK_XCCY_SWAP_RESULT("Leg 0 NPV", xccy->legNPV(0), expPayLegNpv, usdTolerance);
    CHECK_XCCY_SWAP_RESULT("Leg 0 BPS", xccy->legBPS(0), expPayLegBps, usdTolerance);
    CHECK_XCCY_SWAP_RESULT("Leg 0 inCcyNPV", xccy->inCcyLegNPV(0), expPayLegNpv * spotFx, usdTolerance * spotFx);
    CHECK_XCCY_SWAP_RESULT("Leg 0 inCcyBPS", xccy->inCcyLegBPS(0), expPayLegBps * spotFx, usdTolerance * spotFx);

    Real expRecLegNpv = usingAtParCoupons ? 142064.36 : 142054.44;
    Real expRecLegBps = 4735.03;
    CHECK_XCCY_SWAP_RESULT("Leg 1 NPV", xccy->legNPV(1), expRecLegNpv, usdTolerance);
    CHECK_XCCY_SWAP_RESULT("Leg 1 BPS", xccy->legBPS(1), expRecLegBps, usdTolerance);
    CHECK_XCCY_SWAP_RESULT("Leg 1 inCcyNPV", xccy->inCcyLegNPV(1), expRecLegNpv, usdTolerance);
    CHECK_XCCY_SWAP_RESULT("Leg 1 inCcyBPS", xccy->inCcyLegBPS(1), expRecLegBps, usdTolerance);

    Real expectedFairRate = usingAtParCoupons ? 0.253937551076 : 0.253937173908;
    Real expectedFairSpread = usingAtParCoupons ? -0.002740802104 : -0.002740592739;

    CHECK_XCCY_SWAP_RESULT("Fair Fixed Rate", xccy->fairFixedRate(), expectedFairRate, 1e-10);
    CHECK_XCCY_SWAP_RESULT("Fair Spread", xccy->fairSpread(), expectedFairSpread, 1e-10);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
