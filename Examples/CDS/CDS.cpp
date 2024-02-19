/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2014 Peter Caspers

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

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/pricingengines/credit/isdacdsengine.hpp>
#include <ql/termstructures/credit/piecewisedefaultcurve.hpp>
#include <ql/termstructures/credit/defaultprobabilityhelpers.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/quotes/simplequote.hpp>

#include <iostream>
#include <iomanip>

using namespace std;
using namespace QuantLib;

void example01() {

    std::cout << std::endl;

    /*********************
     ***  MARKET DATA  ***
     *********************/

    Calendar calendar = TARGET();
    Date todaysDate(15, May, 2007);
    // must be a business day
    todaysDate = calendar.adjust(todaysDate);

    Settings::instance().evaluationDate() = todaysDate;

    // dummy curve
    auto flatRate = ext::make_shared<SimpleQuote>(0.01);
    Handle<YieldTermStructure> tsCurve(
        ext::make_shared<FlatForward>(
            todaysDate, Handle<Quote>(flatRate), Actual365Fixed()));

    /*
      In Lehmans Brothers "guide to exotic credit derivatives"
      p. 32 there's a simple case, zero flat curve with a flat CDS
      curve with constant market spreads of 150 bp and RR = 50%
      corresponds to a flat 3% hazard rate. The implied 1-year
      survival probability is 97.04% and the 2-years is 94.18%
    */

    // market
    Natural settlementDays = 1;
    Real recovery_rate = 0.5;
    Real quoted_spreads[] = { 0.0150, 0.0150, 0.0150, 0.0150 };
    vector<Period> tenors;
    tenors.push_back(3 * Months);
    tenors.push_back(6 * Months);
    tenors.push_back(1 * Years);
    tenors.push_back(2 * Years);

    Date settlementDate = calendar.advance(todaysDate, settlementDays, Days);
    vector<Date> maturities;
    for (Size i = 0; i < 4; i++) {
        maturities.push_back(
            calendar.adjust(settlementDate + tenors[i], Following));
    }

    std::vector<ext::shared_ptr<DefaultProbabilityHelper>> instruments;
    for (Size i = 0; i < 4; i++) {
        instruments.push_back(ext::make_shared<SpreadCdsHelper>(
                                makeQuoteHandle(quoted_spreads[i]),
                                tenors[i], settlementDays, calendar, Quarterly, Following,
                                DateGeneration::TwentiethIMM, Actual365Fixed(),
                                recovery_rate, tsCurve));
    }

    // Bootstrap hazard rates
    auto hazardRateStructure = ext::make_shared<PiecewiseDefaultCurve<HazardRate, BackwardFlat>>(
        todaysDate, instruments, Actual365Fixed());
    vector<pair<Date, Real>> hr_curve_data = hazardRateStructure->nodes();

    cout << "Calibrated hazard rate values: " << endl;
    for (auto& i : hr_curve_data) {
        cout << "hazard rate on " << i.first << " is " << i.second << endl;
    }
    cout << endl;

    cout << "Some survival probability values: " << endl;
    cout << "1Y survival probability: "
         << io::percent(hazardRateStructure->survivalProbability(todaysDate +
                                                                 1 * Years))
         << endl << "               expected: " << io::percent(0.9704) << endl;
    cout << "2Y survival probability: "
         << io::percent(hazardRateStructure->survivalProbability(todaysDate +
                                                                 2 * Years))
         << endl << "               expected: " << io::percent(0.9418) << endl;

    cout << endl << endl;

    // reprice instruments
    Real nominal = 1000000.0;
    Handle<DefaultProbabilityTermStructure> probability(hazardRateStructure);
    auto engine = ext::make_shared<MidPointCdsEngine>(probability, recovery_rate, tsCurve);

    Schedule cdsSchedule = MakeSchedule()
                               .from(settlementDate)
                               .to(maturities[0])
                               .withFrequency(Quarterly)
                               .withCalendar(calendar)
                               .withTerminationDateConvention(Unadjusted)
                               .withRule(DateGeneration::TwentiethIMM);
    CreditDefaultSwap cds_3m(Protection::Seller, nominal, quoted_spreads[0],
                             cdsSchedule, Following, Actual365Fixed());

    cdsSchedule = MakeSchedule()
                      .from(settlementDate)
                      .to(maturities[1])
                      .withFrequency(Quarterly)
                      .withCalendar(calendar)
                      .withTerminationDateConvention(Unadjusted)
                      .withRule(DateGeneration::TwentiethIMM);
    CreditDefaultSwap cds_6m(Protection::Seller, nominal, quoted_spreads[1],
                             cdsSchedule, Following, Actual365Fixed());

    cdsSchedule = MakeSchedule()
                      .from(settlementDate)
                      .to(maturities[2])
                      .withFrequency(Quarterly)
                      .withCalendar(calendar)
                      .withTerminationDateConvention(Unadjusted)
                      .withRule(DateGeneration::TwentiethIMM);
    CreditDefaultSwap cds_1y(Protection::Seller, nominal, quoted_spreads[2],
                             cdsSchedule, Following, Actual365Fixed());

    cdsSchedule = MakeSchedule()
                      .from(settlementDate)
                      .to(maturities[3])
                      .withFrequency(Quarterly)
                      .withCalendar(calendar)
                      .withTerminationDateConvention(Unadjusted)
                      .withRule(DateGeneration::TwentiethIMM);
    CreditDefaultSwap cds_2y(Protection::Seller, nominal, quoted_spreads[3],
                             cdsSchedule, Following, Actual365Fixed());

    cds_3m.setPricingEngine(engine);
    cds_6m.setPricingEngine(engine);
    cds_1y.setPricingEngine(engine);
    cds_2y.setPricingEngine(engine);

    cout << "Repricing of quoted CDSs employed for calibration: " << endl;
    cout << "3M fair spread: " << io::rate(cds_3m.fairSpread()) << endl
         << "   NPV:         " << cds_3m.NPV() << endl
         << "   default leg: " << cds_3m.defaultLegNPV() << endl
         << "   coupon leg:  " << cds_3m.couponLegNPV() << endl << endl;

    cout << "6M fair spread: " << io::rate(cds_6m.fairSpread()) << endl
         << "   NPV:         " << cds_6m.NPV() << endl
         << "   default leg: " << cds_6m.defaultLegNPV() << endl
         << "   coupon leg:  " << cds_6m.couponLegNPV() << endl << endl;

    cout << "1Y fair spread: " << io::rate(cds_1y.fairSpread()) << endl
         << "   NPV:         " << cds_1y.NPV() << endl
         << "   default leg: " << cds_1y.defaultLegNPV() << endl
         << "   coupon leg:  " << cds_1y.couponLegNPV() << endl << endl;

    cout << "2Y fair spread: " << io::rate(cds_2y.fairSpread()) << endl
         << "   NPV:         " << cds_2y.NPV() << endl
         << "   default leg: " << cds_2y.defaultLegNPV() << endl
         << "   coupon leg:  " << cds_2y.couponLegNPV() << endl << endl;

    cout << endl << endl;

}

void example02() {

Date todaysDate(25, September, 2014);
Settings::instance().evaluationDate() = todaysDate;

Date termDate = TARGET().adjust(todaysDate+Period(2*Years), Following);

Schedule cdsSchedule =
    MakeSchedule().from(todaysDate).to(termDate)
                  .withFrequency(Quarterly)
                  .withCalendar(WeekendsOnly())
                  .withConvention(ModifiedFollowing)
                  .withTerminationDateConvention(ModifiedFollowing)
                  .withRule(DateGeneration::CDS);

std::copy(cdsSchedule.begin(), cdsSchedule.end(),
    std::ostream_iterator<Date>(cout, "\n"));

    Date evaluationDate = Date(21, October, 2014);

    Settings::instance().evaluationDate() = evaluationDate;

    IborCoupon::Settings::instance().createAtParCoupons();

    // set up ISDA IR curve helpers

    auto dp1m = ext::make_shared<DepositRateHelper>(0.000060, 1 * Months, 2,
                                              TARGET(), ModifiedFollowing,
                                              false, Actual360());
    auto dp2m = ext::make_shared<DepositRateHelper>(0.000450, 2 * Months, 2,
                                              TARGET(), ModifiedFollowing,
                                              false, Actual360());
    auto dp3m = ext::make_shared<DepositRateHelper>(0.000810, 3 * Months, 2,
                                              TARGET(), ModifiedFollowing,
                                              false, Actual360());
    auto dp6m = ext::make_shared<DepositRateHelper>(0.001840, 6 * Months, 2,
                                              TARGET(), ModifiedFollowing,
                                              false, Actual360());
    auto dp9m = ext::make_shared<DepositRateHelper>(0.002560, 9 * Months, 2,
                                              TARGET(), ModifiedFollowing,
                                              false, Actual360());
    auto dp12m = ext::make_shared<DepositRateHelper>(0.003370, 12 * Months, 2,
                                              TARGET(), ModifiedFollowing,
                                              false, Actual360());

    // intentionally we do not provide a fixing for the euribor index used for
    // bootstrapping in order to be compliant with the ISDA specification

    auto euribor6m = ext::make_shared<Euribor>(Euribor(6 * Months));

    DayCounter thirty360 = Thirty360(Thirty360::BondBasis);

    auto sw2y = ext::make_shared<SwapRateHelper>(
        0.002230, 2 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw3y = ext::make_shared<SwapRateHelper>(
        0.002760, 3 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw4y = ext::make_shared<SwapRateHelper>(
        0.003530, 4 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw5y = ext::make_shared<SwapRateHelper>(
        0.004520, 5 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw6y = ext::make_shared<SwapRateHelper>(
        0.005720, 6 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw7y = ext::make_shared<SwapRateHelper>(
        0.007050, 7 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw8y = ext::make_shared<SwapRateHelper>(
        0.008420, 8 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw9y = ext::make_shared<SwapRateHelper>(
        0.009720, 9 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw10y = ext::make_shared<SwapRateHelper>(
        0.010900, 10 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw12y = ext::make_shared<SwapRateHelper>(
        0.012870, 12 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw15y = ext::make_shared<SwapRateHelper>(
        0.014970, 15 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw20y = ext::make_shared<SwapRateHelper>(
        0.017000, 20 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw30y = ext::make_shared<SwapRateHelper>(
        0.018210, 30 * Years, TARGET(), Annual, ModifiedFollowing, thirty360,
        euribor6m);

    std::vector<ext::shared_ptr<RateHelper>> isdaRateHelper;

    isdaRateHelper.push_back(dp1m);
    isdaRateHelper.push_back(dp2m);
    isdaRateHelper.push_back(dp3m);
    isdaRateHelper.push_back(dp6m);
    isdaRateHelper.push_back(dp9m);
    isdaRateHelper.push_back(dp12m);
    isdaRateHelper.push_back(sw2y);
    isdaRateHelper.push_back(sw3y);
    isdaRateHelper.push_back(sw4y);
    isdaRateHelper.push_back(sw5y);
    isdaRateHelper.push_back(sw6y);
    isdaRateHelper.push_back(sw7y);
    isdaRateHelper.push_back(sw8y);
    isdaRateHelper.push_back(sw9y);
    isdaRateHelper.push_back(sw10y);
    isdaRateHelper.push_back(sw12y);
    isdaRateHelper.push_back(sw15y);
    isdaRateHelper.push_back(sw20y);
    isdaRateHelper.push_back(sw30y);

    Handle<YieldTermStructure> rateTs(
        ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(
            0, WeekendsOnly(), isdaRateHelper, Actual365Fixed()));
    rateTs->enableExtrapolation();

    // output rate curve
    std::cout << "ISDA rate curve: " << std::endl;
    for (auto& i : isdaRateHelper) {
        Date d = i->latestDate();
        std::cout << d << "\t" << setprecision(6) <<
            rateTs->zeroRate(d,Actual365Fixed(),Continuous).rate() << "\t" <<
            rateTs->discount(d) << std::endl;
    }

    // build reference credit curve (flat)
    auto defaultTs0 = ext::make_shared<FlatHazardRate>(0, WeekendsOnly(), 0.016739207493630, Actual365Fixed());

    // reference CDS
    Schedule sched( Date(22,September,2014), Date(20,December,2019), 3*Months,
            WeekendsOnly(), Following, Unadjusted, DateGeneration::CDS, false, Date(), Date() );
    auto trade = ext::make_shared<CreditDefaultSwap>(
            Protection::Buyer, 100000000.0, 0.01, sched,
                                  Following, Actual360(), true, true,
                                  Date(22,October,2014), ext::shared_ptr<Claim>(),
                                  Actual360(true), true);

    auto cp = ext::dynamic_pointer_cast<FixedRateCoupon>(trade->coupons()[0]);
    std::cout << "first period = " << cp->accrualStartDate() << " to " << cp->accrualEndDate() <<
        " accrued amount = " << cp->accruedAmount(Date(24,October,2014)) << std::endl;

    // price with isda engine
    auto engine = ext::make_shared<IsdaCdsEngine>(
            Handle<DefaultProbabilityTermStructure>(defaultTs0), 0.4, rateTs,
            false, IsdaCdsEngine::Taylor, IsdaCdsEngine::NoBias, IsdaCdsEngine::Piecewise);

    trade->setPricingEngine(engine);

    std::cout << "reference trade NPV = " << trade->NPV() << std::endl;


    // build credit curve with one cds
    std::vector<ext::shared_ptr<DefaultProbabilityHelper>> isdaCdsHelper;

    auto cds5y = ext::make_shared<SpreadCdsHelper>(
        0.00672658551, 4 * Years + 6 * Months, 1, WeekendsOnly(), Quarterly,
        Following, DateGeneration::CDS, Actual360(), 0.4, rateTs, true, true,
        Date(), Actual360(true), true, CreditDefaultSwap::ISDA);

    isdaCdsHelper.push_back(cds5y);

    Handle<DefaultProbabilityTermStructure> defaultTs(ext::make_shared<
        PiecewiseDefaultCurve<SurvivalProbability, LogLinear>>(
        0, WeekendsOnly(), isdaCdsHelper, Actual365Fixed()));

    std::cout << "ISDA credit curve: " << std::endl;
    for (auto& i : isdaCdsHelper) {
        Date d = i->latestDate();
        Real pd = defaultTs->defaultProbability(d);
        Real t = defaultTs->timeFromReference(d);
        std::cout << d << ";" << pd << ";" << 1.0 - pd << ";" <<
            -std::log(1.0-pd)/t << std::endl;
    }


    // // set up sample CDS trade

    // auto trade =
    //     MakeCreditDefaultSwap(5 * Years, 0.03);

    // // set up isda engine

    // // auto isdaPricer =
    // //     ext::make_shared<IsdaCdsEngine>(
    // //         isdaCdsHelper, 0.4, isdaRateHelper);
    // auto isdaPricer =
    //     ext::make_shared<IsdaCdsEngine>(defaultTs,0.40,rateTs);

    // check the curves built by the engine

    // Handle<YieldTermStructure> isdaYts = isdaPricer->isdaRateCurve();
    // Handle<DefaultProbabilityTermStructure> isdaCts = isdaPricer->isdaCreditCurve();

    // std::cout << "isda rate 1m " << dp1m->latestDate() << " "
    //           << isdaYts->zeroRate(dp1m->latestDate(), Actual365Fixed(),
    //                                   Continuous) << std::endl;
    // std::cout << "isda rate 3m " << dp3m->latestDate() << " "
    //           << isdaYts->zeroRate(dp3m->latestDate(), Actual365Fixed(),
    //                                   Continuous) << std::endl;
    // std::cout << "isda rate 6m " << dp6m->latestDate() << " "
    //           << isdaYts->zeroRate(dp6m->latestDate(), Actual365Fixed(),
    //                                   Continuous) << std::endl;

    // std::cout << "isda hazard 5y " << cds5y->latestDate() << " "
    //           << isdaCts->hazardRate(cds5y->latestDate()) << std::endl;

    // price the trade

    // trade->setPricingEngine(isdaPricer);

    // Real npv = trade->NPV();

    // std::cout << "Pricing of example trade with ISDA engine:" << std::endl;
    // std::cout << "NPV = " << npv << std::endl;

}

void example03() {

    // this is the example from Apdx E in pricing and risk management of CDS, OpenGamma

    Date tradeDate(13,June,2011);

    Settings::instance().evaluationDate() = tradeDate;

    IborCoupon::Settings::instance().createAtParCoupons();

    DayCounter actual360 = Actual360();
    DayCounter thirty360 = Thirty360(Thirty360::BondBasis);

    auto dp1m = ext::make_shared<DepositRateHelper>(0.00445, 1 * Months, 2,
                                              WeekendsOnly(), ModifiedFollowing,
                                              false, actual360);
    auto dp2m = ext::make_shared<DepositRateHelper>(0.00949, 2 * Months, 2,
                                              WeekendsOnly(), ModifiedFollowing,
                                              false, actual360);
    auto dp3m = ext::make_shared<DepositRateHelper>(0.01234, 3 * Months, 2,
                                              WeekendsOnly(), ModifiedFollowing,
                                              false, actual360);
    auto dp6m = ext::make_shared<DepositRateHelper>(0.01776, 6 * Months, 2,
                                              WeekendsOnly(), ModifiedFollowing,
                                              false, actual360);
    auto dp9m = ext::make_shared<DepositRateHelper>(0.01935, 9 * Months, 2,
                                              WeekendsOnly(), ModifiedFollowing,
                                              false, actual360);
    auto dp1y = ext::make_shared<DepositRateHelper>(0.02084, 12 * Months, 2,
                                              WeekendsOnly(), ModifiedFollowing,
                                              false, actual360);

    // this index is probably not important since we are not using
    // IborCoupon::Settings::instance().usingAtParCoupons() == false
    // - define it "isda compliant" anyway
    auto euribor6m = ext::make_shared<IborIndex>(
        "IsdaIbor", 6 * Months, 2, EURCurrency(), WeekendsOnly(),
        ModifiedFollowing, false, actual360);

    auto sw2y = ext::make_shared<SwapRateHelper>(
        0.01652, 2 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw3y = ext::make_shared<SwapRateHelper>(
        0.02018, 3 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw4y = ext::make_shared<SwapRateHelper>(
        0.02303, 4 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw5y = ext::make_shared<SwapRateHelper>(
        0.02525, 5 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw6y = ext::make_shared<SwapRateHelper>(
        0.02696, 6 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw7y = ext::make_shared<SwapRateHelper>(
        0.02825, 7 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw8y = ext::make_shared<SwapRateHelper>(
        0.02931, 8 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw9y = ext::make_shared<SwapRateHelper>(
        0.03017, 9 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw10y = ext::make_shared<SwapRateHelper>(
        0.03092, 10 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw11y = ext::make_shared<SwapRateHelper>(
        0.03160, 11 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw12y = ext::make_shared<SwapRateHelper>(
        0.03231, 12 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw15y = ext::make_shared<SwapRateHelper>(
        0.03367, 15 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw20y = ext::make_shared<SwapRateHelper>(
        0.03419, 20 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw25y = ext::make_shared<SwapRateHelper>(
        0.03411, 25 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);
    auto sw30y = ext::make_shared<SwapRateHelper>(
        0.03412, 30 * Years, WeekendsOnly(), Annual, ModifiedFollowing, thirty360,
        euribor6m);

    std::vector<ext::shared_ptr<RateHelper>> isdaYieldHelpers;

    isdaYieldHelpers.push_back(dp1m);
    isdaYieldHelpers.push_back(dp2m);
    isdaYieldHelpers.push_back(dp3m);
    isdaYieldHelpers.push_back(dp6m);
    isdaYieldHelpers.push_back(dp9m);
    isdaYieldHelpers.push_back(dp1y);
    isdaYieldHelpers.push_back(sw2y);
    isdaYieldHelpers.push_back(sw3y);
    isdaYieldHelpers.push_back(sw4y);
    isdaYieldHelpers.push_back(sw5y);
    isdaYieldHelpers.push_back(sw6y);
    isdaYieldHelpers.push_back(sw7y);
    isdaYieldHelpers.push_back(sw8y);
    isdaYieldHelpers.push_back(sw9y);
    isdaYieldHelpers.push_back(sw10y);
    isdaYieldHelpers.push_back(sw11y);
    isdaYieldHelpers.push_back(sw12y);
    isdaYieldHelpers.push_back(sw15y);
    isdaYieldHelpers.push_back(sw20y);
    isdaYieldHelpers.push_back(sw25y);
    isdaYieldHelpers.push_back(sw30y);

    // build yield curve
    Handle<YieldTermStructure> isdaYts = Handle<YieldTermStructure>(
            ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(
                0, WeekendsOnly(), isdaYieldHelpers, Actual365Fixed()));
    isdaYts->enableExtrapolation();


    CreditDefaultSwap::PricingModel model = CreditDefaultSwap::ISDA;
    auto cds6m = ext::make_shared<SpreadCdsHelper>(
        0.007927, 6 * Months, 1, WeekendsOnly(), Quarterly, Following,
        DateGeneration::CDS, Actual360(), 0.4, isdaYts, true, true, Date(),
        Actual360(true), true, model);
    auto cds1y = ext::make_shared<SpreadCdsHelper>(
        0.007927, 1 * Years, 1, WeekendsOnly(), Quarterly, Following,
        DateGeneration::CDS, Actual360(), 0.4, isdaYts, true, true, Date(),
        Actual360(true), true, model);
    auto cds3y = ext::make_shared<SpreadCdsHelper>(
        0.012239, 3 * Years, 1, WeekendsOnly(), Quarterly, Following,
        DateGeneration::CDS, Actual360(), 0.4, isdaYts, true, true, Date(),
        Actual360(true), true, model);
    auto cds5y = ext::make_shared<SpreadCdsHelper>(
        0.016979, 5 * Years, 1, WeekendsOnly(), Quarterly, Following,
        DateGeneration::CDS, Actual360(), 0.4, isdaYts, true, true, Date(),
        Actual360(true), true, model);
    auto cds7y = ext::make_shared<SpreadCdsHelper>(
        0.019271, 7 * Years, 1, WeekendsOnly(), Quarterly, Following,
        DateGeneration::CDS, Actual360(), 0.4, isdaYts, true, true, Date(),
        Actual360(true), true, model);
    auto cds10y = ext::make_shared<SpreadCdsHelper>(
        0.020860, 10 * Years, 1, WeekendsOnly(), Quarterly, Following,
        DateGeneration::CDS, Actual360(), 0.4, isdaYts, true, true, Date(),
        Actual360(true), true, model);

    std::vector<ext::shared_ptr<DefaultProbabilityHelper>> isdaCdsHelpers;

    isdaCdsHelpers.push_back(cds6m);
    isdaCdsHelpers.push_back(cds1y);
    isdaCdsHelpers.push_back(cds3y);
    isdaCdsHelpers.push_back(cds5y);
    isdaCdsHelpers.push_back(cds7y);
    isdaCdsHelpers.push_back(cds10y);

    // build credit curve
    auto isdaCts =
    Handle<DefaultProbabilityTermStructure>(ext::make_shared<
        PiecewiseDefaultCurve<SurvivalProbability, LogLinear>>(
            0, WeekendsOnly(), isdaCdsHelpers, Actual365Fixed()));

    // set up isda engine
    auto isdaPricer = ext::make_shared<IsdaCdsEngine>(isdaCts, 0.4, isdaYts);

    // check the curves
    std::cout << "ISDA yield curve:" << std::endl;
    std::cout << "date;time;zeroyield" << std::endl;
    for (auto& isdaYieldHelper : isdaYieldHelpers) {
        Date d = isdaYieldHelper->latestDate();
        Real t = isdaYts->timeFromReference(d);
        std::cout << d << ";" << t << ";"
                  << isdaYts->zeroRate(d, Actual365Fixed(), Continuous).rate()
                  << std::endl;
    }

    std::cout << "ISDA credit curve:" << std::endl;
    std::cout << "date;time;survivalprob" << std::endl;
    for (auto& isdaCdsHelper : isdaCdsHelpers) {
        Date d = isdaCdsHelper->latestDate();
        Real t = isdaCts->timeFromReference(d);
        std::cout << d << ";" << t << ";" << isdaCts->survivalProbability(d)
                  << std::endl;
    }
}


int main(int argc, char *argv[]) {

    try {
        Size example = 0;
        if (argc == 2)
            example = std::atoi(argv[1]);

        if (example == 0 || example == 1) {
            std::cout << "***** Running example #1 *****" << std::endl;
            example01();
        }

        if (example == 0 || example == 2) {
            std::cout << "***** Running example #2 *****" << std::endl;
            example02();
        }

        if (example == 0 || example == 3) {
            std::cout << "***** Running example #3 *****" << std::endl;
            example03();
        }

        return 0;
    }
    catch (exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
    catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}
