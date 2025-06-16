/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Paolo D'Elia

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
#include "utilities.hpp"
#include <ql/instruments/crossccyswap.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/indexes/ibor/gbplibor.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/pricingengines/swap/crossccyswapengine.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/all.hpp>
#include <ql/currencies/all.hpp>

using namespace std;
using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CrossCcySwapTests)

#define CHECK_XCCY_SWAP_RESULT(what, calculated, expected, tolerance)   \
    if (std::fabs(calculated-expected) > tolerance) { \
        BOOST_ERROR("Failed to reproduce " what ":" \
                    << "\n    expected:   " << std::setprecision(12) << expected \
                    << "\n    calculated: " << std::setprecision(12) << calculated \
                    << "\n    error:      " << std::setprecision(12) << std::fabs(calculated-expected)); \
    }

// Data for the tests
struct CommonVars {
    Date today, startDate, endDate;
    Calendar payCalendar;
    DateGeneration::Rule rule;
    BusinessDayConvention convention;
    bool endOfMonth;
    DayCounter dc;

    CommonVars(Calendar c, BusinessDayConvention conv, DateGeneration::Rule r) {
        payCalendar = c;
        today = Date(11, Sep, 2018);
        Settings::instance().evaluationDate() = today;
        startDate = payCalendar.advance(today, Period(2, Days));
        endDate = payCalendar.advance(today, Period(5, Years));
        rule = r;
        convention = conv;
        endOfMonth = false;
        dc = Actual365Fixed();
    }
};

Handle<YieldTermStructure> CHFDiscountCurve() {
    std::vector<Date> dates(27);
    std::vector<DiscountFactor> dfs(27);
    Actual365Fixed dayCounter;

    dates[0] = Date(11, Sep, 2018);
    dfs[0] = 1.0;
    dates[1] = Date(14, Sep, 2018);
    dfs[1] = 0.99998;
    dates[2] = Date(20, Sep, 2018);
    dfs[2] = 0.99975;
    dates[3] = Date(27, Sep, 2018);
    dfs[3] = 0.99945;
    dates[4] = Date(04, Oct, 2018);
    dfs[4] = 0.99910;
    dates[5] = Date(15, Oct, 2018);
    dfs[5] = 0.99855;
    dates[6] = Date(13, Nov, 2018);
    dfs[6] = 0.99700;
    dates[7] = Date(13, Dec, 2018);
    dfs[7] = 0.99540;
    dates[8] = Date(14, Jan, 2019);
    dfs[8] = 0.99360;
    dates[9] = Date(13, Feb, 2019);
    dfs[9] = 0.99190;
    dates[10] = Date(13, Mar, 2019);
    dfs[10] = 0.99030;
    dates[11] = Date(13, Jun, 2019);
    dfs[11] = 0.98430;
    dates[12] = Date(13, Sep, 2019);
    dfs[12] = 0.97800;
    dates[13] = Date(13, Mar, 2020);
    dfs[13] = 0.96500;
    dates[14] = Date(14, Sep, 2020);
    dfs[14] = 0.95200;
    dates[15] = Date(13, Sep, 2021);
    dfs[15] = 0.92700;
    dates[16] = Date(13, Sep, 2022);
    dfs[16] = 0.90300;
    dates[17] = Date(13, Sep, 2023);
    dfs[17] = 0.88000;
    dates[18] = Date(15, Sep, 2025);
    dfs[18] = 0.83600;
    dates[19] = Date(13, Sep, 2028);
    dfs[19] = 0.77300;
    dates[20] = Date(13, Sep, 2030);
    dfs[20] = 0.73400;
    dates[21] = Date(13, Sep, 2033);
    dfs[21] = 0.67800;
    dates[22] = Date(13, Sep, 2038);
    dfs[22] = 0.59600;
    dates[23] = Date(14, Sep, 2043);
    dfs[23] = 0.52800;
    dates[24] = Date(14, Sep, 2048);
    dfs[24] = 0.46800;
    dates[25] = Date(13, Sep, 2058);
    dfs[25] = 0.36700;
    dates[26] = Date(13, Sep, 2068);
    dfs[26] = 0.29700;

    return Handle<YieldTermStructure>(
        ext::make_shared<DiscountCurve>(dates, dfs, dayCounter)
    );
}

Handle<YieldTermStructure> USDDiscountCurve() {

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

Handle<YieldTermStructure> USDProjectionCurve() {

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

Handle<YieldTermStructure> GBPDiscountCurve() {

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

Handle<YieldTermStructure> GBPProjectionCurve() {

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

Handle<YieldTermStructure> TRYDiscountCurve() {

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

    return Handle<YieldTermStructure>(QuantLib::ext::make_shared<DiscountCurve>(dates, dfs, dayCounter));
}

// Helper functions

ext::shared_ptr<CrossCcySwap> makeFixFixXCCYSwap(Real leg1Nominal, Rate spotFx) {
	Calendar payCalendar = JointCalendar(UnitedStates(UnitedStates::Settlement), Switzerland());
	
    CommonVars vars(payCalendar, Following, DateGeneration::Forward);

    Schedule schedule(
        vars.startDate,
        vars.endDate,
        Period(3, Months),
        payCalendar,
        vars.convention,
        vars.convention,
        vars.rule,
        vars.endOfMonth
    );

	Rate usdRate = 0.0575;
	Rate chfRate = 0.0201;

	// USD Leg
    Leg usdLeg = FixedRateLeg(schedule)
                       .withNotionals(leg1Nominal)
                       .withCouponRates(usdRate, vars.dc)
                       .withPaymentAdjustment(vars.convention)
                       .withPaymentCalendar(vars.payCalendar);
	Date aDate = payCalendar.adjust(schedule.dates().front(), vars.convention);
    auto initialCapitalFlow = ext::make_shared<SimpleCashFlow>(-leg1Nominal, aDate);
    auto finalCapitalFlow = ext::make_shared<SimpleCashFlow>(leg1Nominal, usdLeg.back()->date());
	usdLeg.insert(usdLeg.begin(), initialCapitalFlow);
    usdLeg.push_back(finalCapitalFlow);

	// CHF Leg
    Leg chfLeg = FixedRateLeg(schedule)
                        .withNotionals(leg1Nominal * spotFx)
                        .withCouponRates(chfRate, vars.dc)
                        .withPaymentAdjustment(vars.convention)
                        .withPaymentCalendar(vars.payCalendar);
    auto initialCHFCapitalFlow = ext::make_shared<SimpleCashFlow>(-leg1Nominal * spotFx, aDate);
    auto finalCHFCapitalFlow = ext::make_shared<SimpleCashFlow>(leg1Nominal * spotFx, chfLeg.back()->date());
	chfLeg.insert(chfLeg.begin(), initialCHFCapitalFlow);
    chfLeg.push_back(finalCHFCapitalFlow);
	
	// Create swap
	return ext::shared_ptr<CrossCcySwap> (new CrossCcySwap(
		usdLeg, USDCurrency(), chfLeg, CHFCurrency()));
}

ext::shared_ptr<CrossCcySwap> makeFixFloatXCCYSwap(Real leg1Nominal, Rate spotFx) {
	Calendar payCalendar = JointCalendar(UnitedStates(UnitedStates::Settlement), UnitedKingdom(), Turkey());
	
    CommonVars vars(payCalendar, ModifiedFollowing, DateGeneration::Backward);
    BusinessDayConvention payConvention = Following;

    Schedule floatSchedule(
        vars.startDate,
        vars.endDate,
        Period(3, Months),
        payCalendar,
        vars.convention,
        vars.convention,
        vars.rule,
        vars.endOfMonth
    );
	
	Schedule fixSchedule(
        vars.startDate,
        vars.endDate,
        Period(1, Years),
        payCalendar,
        vars.convention,
        vars.convention,
        vars.rule,
        vars.endOfMonth
    );

	// TRY Leg
	Rate tryRate = 0.249;
	Leg tryLeg = FixedRateLeg(fixSchedule)
                    .withNotionals(leg1Nominal * spotFx)
                    .withCouponRates(tryRate, vars.dc)
                    .withPaymentAdjustment(payConvention)
                    .withPaymentCalendar(payCalendar);
	Date aDate = payCalendar.adjust(fixSchedule.dates().front(), vars.convention);
    auto initialTRYCapitalFlow = ext::make_shared<SimpleCashFlow>(-leg1Nominal * spotFx, aDate);
    auto finalTRYCapitalFlow = ext::make_shared<SimpleCashFlow>(leg1Nominal * spotFx, tryLeg.back()->date());
	tryLeg.insert(tryLeg.begin(), initialTRYCapitalFlow);
    tryLeg.push_back(finalTRYCapitalFlow);

	// USD Leg
    auto usdlibor3M = ext::make_shared<USDLibor>(Period(3, Months), USDProjectionCurve());
    Leg usdLeg = IborLeg(floatSchedule, usdlibor3M)
				.withNotionals(leg1Nominal)
				.withPaymentAdjustment(payConvention)
                .withPaymentCalendar(payCalendar);
	aDate = payCalendar.adjust(floatSchedule.dates().front(), vars.convention);
    auto initialUSDNotionalExchange = ext::make_shared<SimpleCashFlow>(-leg1Nominal, aDate);
	usdLeg.insert(usdLeg.begin(), initialUSDNotionalExchange);
	auto finalUSDNotionalExchange = ext::make_shared<SimpleCashFlow>(leg1Nominal, usdLeg.back()->date());
	usdLeg.push_back(finalUSDNotionalExchange);
	
	// Create swap
	return ext::shared_ptr<CrossCcySwap>(new CrossCcySwap(
		tryLeg, TRYCurrency(), usdLeg, USDCurrency()));
}

ext::shared_ptr<CrossCcySwap> makeFloatFloatXCCYSwap(Real leg1Nominal, Rate spotFx) {
	Calendar payCalendar = JointCalendar(UnitedStates(UnitedStates::Settlement), UnitedKingdom());
	
    CommonVars vars(payCalendar, Following, DateGeneration::Forward);

    Schedule schedule(
        vars.startDate,
        vars.endDate,
        Period(3, Months),
        payCalendar,
        vars.convention,
        vars.convention,
        vars.rule,
        vars.endOfMonth
    );

	// USD Leg
    auto usdlibor3M = ext::make_shared<USDLibor>(Period(3, Months), USDProjectionCurve());
	Leg usdLeg = IborLeg(schedule, usdlibor3M)
					.withNotionals(leg1Nominal)
					.withPaymentAdjustment(vars.convention)
					.withPaymentCalendar(payCalendar);
	Date aDate = payCalendar.adjust(schedule.dates().front());
	auto initialUSDNotionalExchange = ext::make_shared<SimpleCashFlow>(-leg1Nominal, aDate);
	usdLeg.insert(usdLeg.begin(), initialUSDNotionalExchange);
	auto finalUSDNotionalExchange = ext::make_shared<SimpleCashFlow>(leg1Nominal, usdLeg.back()->date());
	usdLeg.push_back(finalUSDNotionalExchange);
	
	// GBP Leg
    auto gbpLibor3M = ext::make_shared<GBPLibor>(Period(3, Months), GBPProjectionCurve());
    Leg gbpLeg = IborLeg(schedule, gbpLibor3M)
					.withNotionals(leg1Nominal * spotFx)
					.withPaymentAdjustment(vars.convention)
					.withPaymentCalendar(payCalendar);
	auto initialGBPNotionalExchange = ext::make_shared<SimpleCashFlow>(-leg1Nominal * spotFx, aDate);
	gbpLeg.insert(gbpLeg.begin(), initialGBPNotionalExchange);
	auto finalGBPNotionalExchange = ext::make_shared<SimpleCashFlow>(leg1Nominal * spotFx, gbpLeg.back()->date());
	gbpLeg.push_back(finalGBPNotionalExchange);
	
	// Create swap
	return ext::shared_ptr<CrossCcySwap> (new CrossCcySwap(
		usdLeg, USDCurrency(), gbpLeg, GBPCurrency()));
}


BOOST_AUTO_TEST_CASE(testFixFixXCCYSwapPricing) {
    BOOST_TEST_MESSAGE("Test Fix-Fix cross currency swap pricing against known results");

    // Create swap
    Real USDNominal = 125'000'000;
    Real spotFx = 1.22;
    auto xccySwap = makeFixFixXCCYSwap(USDNominal, spotFx);

    // Attach pricing engine
	auto fxSpotQuote = makeQuoteHandle(1.0 / spotFx);
    auto engine = ext::make_shared<CrossCcySwapEngine>(
        USDCurrency(), USDDiscountCurve(), CHFCurrency(), CHFDiscountCurve(), fxSpotQuote);

    xccySwap->setPricingEngine(engine);

    // Check values
    Real tolerance = 0.01;
	Real expNpv = -21108172.67;
	
	CHECK_XCCY_SWAP_RESULT("NPV", xccySwap->NPV(), expNpv, tolerance);
	
	Real expPayLegNpv = -17892458.36;
	Real expPayLegBps = -58317.61;
	CHECK_XCCY_SWAP_RESULT("Leg 0 NPV", xccySwap->legNPV(0), expPayLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 0 BPS", xccySwap->legBPS(0), expPayLegBps, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 0 inCcyNPV", xccySwap->inCcyLegNPV(0), expPayLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 0 inCcyBPS", xccySwap->inCcyLegBPS(0), expPayLegBps, tolerance);

	Real expRecLegNpv = -3215714.30;
	Real expRecLegBps = 58542.62;
	CHECK_XCCY_SWAP_RESULT("Leg 1 NPV", xccySwap->legNPV(1), expRecLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 1 BPS", xccySwap->legBPS(1), expRecLegBps, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 1 inCcyNPV", xccySwap->inCcyLegNPV(1), expRecLegNpv * spotFx, tolerance * spotFx);
	CHECK_XCCY_SWAP_RESULT("Leg 1 inCcyBPS", xccySwap->inCcyLegBPS(1), expRecLegBps * spotFx, tolerance * spotFx);
}

BOOST_AUTO_TEST_CASE(testFloatFixXCCYSwapPricing) {
    BOOST_TEST_MESSAGE("Test Float-Fix cross currency pricing against known results");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    // Create swap
	Real USDNominal = 10'000'000;
    Rate spotFx = 6.4304;
    auto xccySwap = makeFixFloatXCCYSwap(USDNominal, spotFx);

    // Attach pricing engine
	auto fxSpotQuote = makeQuoteHandle(1.0 / spotFx);
    auto engine = ext::make_shared<CrossCcySwapEngine>(
        USDCurrency(), USDDiscountCurve(), TRYCurrency(), TRYDiscountCurve(), fxSpotQuote);
    xccySwap->setPricingEngine(engine);

    // Check values
    Real tolerance = 0.01;

	Real expNpv = usingAtParCoupons ? 218961.99 : 218981.99;
    Real npv = xccySwap->NPV();
	CHECK_XCCY_SWAP_RESULT("NPV", npv, expNpv, tolerance);

	Real expPayLegNpv = 77054.99;
	Real expPayLegBps = -2591.34;
	CHECK_XCCY_SWAP_RESULT("Leg 0 NPV", xccySwap->legNPV(0), expPayLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 0 BPS", xccySwap->legBPS(0), expPayLegBps, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 0 inCcyNPV", xccySwap->inCcyLegNPV(0), expPayLegNpv  * spotFx, tolerance  * spotFx);
	CHECK_XCCY_SWAP_RESULT("Leg 0 inCcyBPS", xccySwap->inCcyLegBPS(0), expPayLegBps * spotFx, tolerance * spotFx);

	Real expRecLegNpv = usingAtParCoupons ? 141906.99 : 141926.99;
	Real expRecLegBps = 4730.19;
	CHECK_XCCY_SWAP_RESULT("Leg 1 NPV", xccySwap->legNPV(1), expRecLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 1 BPS", xccySwap->legBPS(1), expRecLegBps, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 1 inCcyNPV", xccySwap->inCcyLegNPV(1), expRecLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 1 inCcyBPS", xccySwap->inCcyLegBPS(1), expRecLegBps, tolerance);
}

BOOST_AUTO_TEST_CASE(testFloatFloatXCCYSwapPricing) {
    BOOST_TEST_MESSAGE("Test Float-Float cross currency pricing against known results");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    // Create swap
	Real USDNominal = 125'000'000;
    Rate spotFx = 1.35;
    auto xccySwap = makeFloatFloatXCCYSwap(USDNominal, spotFx);

    // Attach pricing engine
	auto fxSpotQuote = makeQuoteHandle(1.0 / spotFx);
    auto engine = ext::make_shared<CrossCcySwapEngine>(
        USDCurrency(), USDDiscountCurve(), GBPCurrency(), GBPDiscountCurve(), fxSpotQuote);

    xccySwap->setPricingEngine(engine);

    // Check values
    Real tolerance = 0.01;

    Real expNpv = 0.00;
	CHECK_XCCY_SWAP_RESULT("NPV", xccySwap->NPV(), expNpv, tolerance);

	Real expPayLegNpv = usingAtParCoupons ? -1773829.64 : -1773772.22;
	Real expPayLegBps = -59127.58;
	CHECK_XCCY_SWAP_RESULT("Leg 0 NPV", xccySwap->legNPV(0), expPayLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 0 BPS", xccySwap->legBPS(0), expPayLegBps, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 0 inCcyNPV", xccySwap->inCcyLegNPV(0), expPayLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 0 inCcyBPS", xccySwap->inCcyLegBPS(0), expPayLegBps, tolerance);

	Real expRecLegNpv = usingAtParCoupons ? 1773829.64 : 1773772.22;
	Real expRecLegBps = 58317.61;
	CHECK_XCCY_SWAP_RESULT("Leg 1 NPV", xccySwap->legNPV(1), expRecLegNpv, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 1 BPS", xccySwap->legBPS(1), expRecLegBps, tolerance);
	CHECK_XCCY_SWAP_RESULT("Leg 1 inCcyNPV", xccySwap->inCcyLegNPV(1), expRecLegNpv * spotFx, tolerance * spotFx);
	CHECK_XCCY_SWAP_RESULT("Leg 1 inCcyBPS", xccySwap->inCcyLegBPS(1), expRecLegBps * spotFx, tolerance * spotFx);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()