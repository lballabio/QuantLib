/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2021 Marcin Rybacki
 Copyright (C) 2025 Uzair Beg
 Copyright (C) 2026 Kyrylo Protsenko

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
#include <ql/indexes/ibor/eonia.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/pricingengines/vanilla/all.hpp>
#include <ql/experimental/fx/discountingmtmcrosscurrencybasisswapengine.hpp>
#include <ql/experimental/fx/fxresetcashflows.hpp>
#include <ql/experimental/termstructures/crosscurrencyratehelpers.hpp>
#include <ql/optional.hpp>
#include <ql/indexes/ibor/bkbm.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/all.hpp>


using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CrossCurrencyRateHelpersTests)

namespace {

    struct IborCouponSettingsRestorer { // NOLINT(cppcoreguidelines-special-member-functions)
        bool initiallyUsingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();
        ~IborCouponSettingsRestorer() {
            if (initiallyUsingAtParCoupons)
                IborCoupon::Settings::instance().createAtParCoupons();
            else
                IborCoupon::Settings::instance().createIndexedCoupons();
        }
    };

    ext::shared_ptr<IborCoupon> firstIborCoupon(const Leg& leg) {
        for (const auto& cashflow : leg) {
            if (auto coupon = ext::dynamic_pointer_cast<IborCoupon>(cashflow))
                return coupon;
            if (auto fxResetCoupon = ext::dynamic_pointer_cast<FxResetCoupon>(cashflow))
                if (auto coupon =
                        ext::dynamic_pointer_cast<IborCoupon>(fxResetCoupon->underlying()))
                    return coupon;
        }
        return {};
    }

    void checkIndexedCouponMode(const std::vector<Leg>& legs, bool expectedIndexed) {
        Size checkedLegs = 0;
        for (const auto& leg : legs) {
            auto coupon = firstIborCoupon(leg);
            if (coupon) {
                ++checkedLegs;
                auto pricer = ext::dynamic_pointer_cast<IborCouponPricer>(coupon->pricer());
                BOOST_REQUIRE(pricer);
                BOOST_CHECK_EQUAL(pricer->useIndexedCoupon(), expectedIndexed);
            }
        }
        BOOST_REQUIRE(checkedLegs > 0);
    }
}

struct XccyTestDatum {
    Integer n;
    TimeUnit units;
    Spread basis;

    XccyTestDatum(Integer n, TimeUnit units, Spread basis) : n(n), units(units), basis(basis) {}
};

struct CommonVars {
    Real basisPoint;
    Real fxSpot;

    Natural instrumentSettlementDays, curveSettlementDays;
    Date today, instrumentSettlementDt, curveSettlementDt;
    Calendar calendar;
    Currency ccy;
    BusinessDayConvention businessConvention;
    DayCounter dayCount;
    bool endOfMonth;

    ext::shared_ptr<IborIndex> baseCcyIdx;
    ext::shared_ptr<IborIndex> quoteCcyIdx;
    ext::shared_ptr<IborIndex> quoteOvernightIndex;
    ext::shared_ptr<IborIndex> baseOvernightIndex;

    RelinkableHandle<YieldTermStructure> baseCcyIdxHandle;
    RelinkableHandle<YieldTermStructure> quoteCcyIdxHandle;

    std::vector<XccyTestDatum> basisData;

    // utilities

    ext::shared_ptr<RateHelper>
    constantNotionalXccyRateHelper(const XccyTestDatum& q,
                                   const Handle<YieldTermStructure>& collateralHandle,
                                   bool isFxBaseCurrencyCollateralCurrency,
                                   bool isBasisOnFxBaseCurrencyLeg) const {
        Handle<Quote> quoteHandle(ext::make_shared<SimpleQuote>(q.basis * basisPoint));
        Period tenor(q.n, q.units);
        return ext::shared_ptr<RateHelper>(new ConstNotionalCrossCurrencyBasisSwapRateHelper(
                quoteHandle, tenor, instrumentSettlementDays, calendar, businessConvention, endOfMonth,
            baseCcyIdx, quoteCcyIdx, collateralHandle, isFxBaseCurrencyCollateralCurrency,
                isBasisOnFxBaseCurrencyLeg));
    }

    std::vector<ext::shared_ptr<RateHelper> >
    buildConstantNotionalXccyRateHelpers(const std::vector<XccyTestDatum>& xccyData,
                                         const Handle<YieldTermStructure>& collateralHandle,
                                         bool isFxBaseCurrencyCollateralCurrency,
                                         bool isBasisOnFxBaseCurrencyLeg) const {
        std::vector<ext::shared_ptr<RateHelper> > instruments;
        instruments.reserve(xccyData.size());
        for (const auto& i : xccyData) {
            instruments.push_back(constantNotionalXccyRateHelper(
                    i, collateralHandle, isFxBaseCurrencyCollateralCurrency,
                    isBasisOnFxBaseCurrencyLeg));
        }

        return instruments;
    }

    ext::shared_ptr<RateHelper>
    resettingXccyRateHelper(const XccyTestDatum& q,
                            const Handle<YieldTermStructure>& collateralHandle,
                            bool isFxBaseCurrencyCollateralCurrency,
                            bool isBasisOnFxBaseCurrencyLeg,
                            bool isFxBaseCurrencyLegResettable,
                            std::optional<Frequency> paymentFrequency = std::nullopt,
                            Integer paymentLag = 0,
                            bool useOvernightIndex = false,
                            std::optional<Frequency> quoteCcyPaymentFrequency = std::nullopt) const {
        Handle<Quote> quoteHandle(ext::make_shared<SimpleQuote>(q.basis * basisPoint));
        Period tenor(q.n, q.units);
        ext::shared_ptr<IborIndex> baseIndex, quoteIndex;
        if (useOvernightIndex) {
            baseIndex = baseOvernightIndex;
            quoteIndex = quoteOvernightIndex;
        } else {
            baseIndex = baseCcyIdx;
            quoteIndex = quoteCcyIdx;
        }

        return ext::shared_ptr<RateHelper>(new MtMCrossCurrencyBasisSwapRateHelper(
                quoteHandle, tenor, instrumentSettlementDays, calendar, businessConvention, endOfMonth,
            baseIndex, quoteIndex, collateralHandle, isFxBaseCurrencyCollateralCurrency,
                isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, paymentFrequency, paymentLag,
                quoteCcyPaymentFrequency));
    }

    std::vector<ext::shared_ptr<RateHelper> >
    buildResettingXccyRateHelpers(const std::vector<XccyTestDatum>& xccyData,
                                  const Handle<YieldTermStructure>& collateralHandle,
                                  bool isFxBaseCurrencyCollateralCurrency,
                                  bool isBasisOnFxBaseCurrencyLeg,
                                  bool isFxBaseCurrencyLegResettable,
                                  std::optional<Frequency> paymentFrequency = std::nullopt,
                                  Integer paymentLag = 0,
                                  bool useOvernightQuoteIndex = false,
                                  std::optional<Frequency> quoteCcyPaymentFrequency = std::nullopt) const {
        std::vector<ext::shared_ptr<RateHelper> > instruments;
        instruments.reserve(xccyData.size());
        for (const auto& i : xccyData) {
            instruments.push_back(resettingXccyRateHelper(
                    i, collateralHandle, isFxBaseCurrencyCollateralCurrency,
                    isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable,
                    paymentFrequency, paymentLag, useOvernightQuoteIndex,
                    quoteCcyPaymentFrequency));
        }

        return instruments;
    }

    Schedule legSchedule(const Period& tenor,
                         const ext::shared_ptr<IborIndex>& idx) const {
        return MakeSchedule()
            .from(instrumentSettlementDt)
            .to(instrumentSettlementDt + tenor)
            .withTenor(idx->tenor())
            .withCalendar(calendar)
            .withConvention(businessConvention)
            .endOfMonth(endOfMonth)
            .backwards();
    }

    Leg constantNotionalLeg(Schedule schedule,
                            const ext::shared_ptr<IborIndex>& idx,
                            Real notional,
                            Spread basis) const {
        Leg leg = IborLeg(std::move(schedule), idx).withNotionals(notional).withSpreads(basis);

        Date initialPaymentDate = CashFlows::startDate(leg);
        leg.push_back(ext::make_shared<SimpleCashFlow>(-notional, initialPaymentDate));

        Date lastPaymentDate = CashFlows::maturityDate(leg);
        leg.push_back(ext::make_shared<SimpleCashFlow>(notional, lastPaymentDate));
        return leg;
    }

    std::vector<ext::shared_ptr<Swap> >
    buildXccyBasisSwap(const XccyTestDatum& q,
                       Real fxSpot,
                       bool isBasisOnFxBaseCurrencyLeg) const {
        const Real baseCcyLegNotional = 1.0;
        Real quoteCcyLegNotional = baseCcyLegNotional * fxSpot;

        Spread baseCcyLegBasis = isBasisOnFxBaseCurrencyLeg ? Real(q.basis * basisPoint) : 0.0;
        Spread quoteCcyLegBasis = isBasisOnFxBaseCurrencyLeg ? 0.0 : Real(q.basis * basisPoint);

        std::vector<ext::shared_ptr<Swap> > legs;
        bool payer = true;

        Leg baseCcyLeg = constantNotionalLeg(legSchedule(Period(q.n, q.units), baseCcyIdx),
                                             baseCcyIdx, baseCcyLegNotional, baseCcyLegBasis);
        legs.push_back(ext::make_shared<Swap>(std::vector<Leg>(1, baseCcyLeg),
                                              std::vector<bool>(1, !payer)));

        Leg quoteCcyLeg =
            constantNotionalLeg(legSchedule(Period(q.n, q.units), quoteCcyIdx), quoteCcyIdx,
                                quoteCcyLegNotional, quoteCcyLegBasis);
        legs.push_back(ext::make_shared<Swap>(std::vector<Leg>(1, quoteCcyLeg),
                                              std::vector<bool>(1, payer)));
        return legs;
    }

    CommonVars() {
        curveSettlementDays = 0;
        instrumentSettlementDays = 2;
        businessConvention = Following;
        calendar = TARGET();
        dayCount = Actual365Fixed();
        endOfMonth = false;

        basisPoint = 1.0e-4;
        fxSpot = 1.25;

        baseCcyIdx = ext::shared_ptr<IborIndex>(new Euribor3M(baseCcyIdxHandle));
        quoteCcyIdx = ext::shared_ptr<IborIndex>(new USDLibor(3 * Months, quoteCcyIdxHandle));
        baseOvernightIndex = ext::shared_ptr<IborIndex>(new Eonia(baseCcyIdxHandle));
        quoteOvernightIndex = ext::shared_ptr<IborIndex>(new Sofr(quoteCcyIdxHandle));

        /* Data source:
           N. Moreni, A. Pallavicini (2015)
           FX Modelling in Collateralized Markets: foreign measures, basis curves
           and pricing formulae.

           section 4.2.1, Table 2.
        */
        basisData.emplace_back(1, Years, -14.5);
        basisData.emplace_back(18, Months, -18.5);
        basisData.emplace_back(2, Years, -20.5);
        basisData.emplace_back(3, Years, -23.75);
        basisData.emplace_back(4, Years, -25.5);
        basisData.emplace_back(5, Years, -26.5);
        basisData.emplace_back(7, Years, -26.75);
        basisData.emplace_back(10, Years, -26.25);
        basisData.emplace_back(15, Years, -24.75);
        basisData.emplace_back(20, Years, -23.25);
        basisData.emplace_back(30, Years, -20.50);

        today = calendar.adjust(Date(6, September, 2013));
        Settings::instance().evaluationDate() = today;

        instrumentSettlementDt = calendar.advance(today, instrumentSettlementDays, Days);
        curveSettlementDt = calendar.advance(today, curveSettlementDays, Days);

        baseCcyIdxHandle.linkTo(flatRate(curveSettlementDt, 0.007, dayCount));
        quoteCcyIdxHandle.linkTo(flatRate(curveSettlementDt, 0.015, dayCount));
    }
};

void checkCrossCurrencyHelperCouponModes(std::optional<bool> useIndexedCoupons,
                                         bool expectedIndexed) {
    CommonVars vars;
    Handle<YieldTermStructure> collateralCurve = vars.quoteCcyIdxHandle;
    Handle<Quote> quote = makeQuoteHandle(-20.0 * vars.basisPoint);

    ConstNotionalCrossCurrencyBasisSwapRateHelper constantHelper(
        quote, 2 * Years, vars.instrumentSettlementDays, vars.calendar,
        vars.businessConvention, vars.endOfMonth, vars.baseCcyIdx, vars.quoteCcyIdx,
        collateralCurve, false, true, Semiannual, 0, Semiannual, useIndexedCoupons);
    checkIndexedCouponMode(constantHelper.swap()->legs(), expectedIndexed);

    MtMCrossCurrencyBasisSwapRateHelper mtmHelper(
        quote, 2 * Years, vars.instrumentSettlementDays, vars.calendar,
        vars.businessConvention, vars.endOfMonth, vars.baseCcyIdx, vars.quoteCcyIdx,
        collateralCurve, false, true, false, Semiannual, 0, Semiannual, 0, Calendar(),
        useIndexedCoupons);
    checkIndexedCouponMode(mtmHelper.swap()->legs(), expectedIndexed);

    ConstNotionalCrossCurrencySwapRateHelper fixedVsFloatingHelper(
        makeQuoteHandle(0.01), 2 * Years, vars.instrumentSettlementDays, vars.calendar,
        vars.businessConvention, vars.endOfMonth, Annual, Thirty360(Thirty360::BondBasis),
        vars.quoteCcyIdx, collateralCurve, false, 0, useIndexedCoupons);
    checkIndexedCouponMode(fixedVsFloatingHelper.swap()->legs(), expectedIndexed);
}


void testConstantNotionalCrossCurrencySwapsNPV(bool isFxBaseCurrencyCollateralCurrency,
                                               bool isBasisOnFxBaseCurrencyLeg) {

    CommonVars vars;

    Handle<YieldTermStructure> collateralHandle =
        isFxBaseCurrencyCollateralCurrency ? vars.baseCcyIdxHandle : vars.quoteCcyIdxHandle;

    ext::shared_ptr<DiscountingSwapEngine> collateralCcyLegEngine(
        new DiscountingSwapEngine(collateralHandle));

    std::vector<ext::shared_ptr<RateHelper> > instruments =
        vars.buildConstantNotionalXccyRateHelpers(vars.basisData, collateralHandle,
                                                  isFxBaseCurrencyCollateralCurrency,
                                                  isBasisOnFxBaseCurrencyLeg);
    ext::shared_ptr<YieldTermStructure> foreignCcyCurve(
        new PiecewiseYieldCurve<Discount, LogLinear>(vars.curveSettlementDt, instruments, vars.dayCount));
    foreignCcyCurve->enableExtrapolation();
    Handle<YieldTermStructure> foreignCcyHandle(foreignCcyCurve);
    ext::shared_ptr<DiscountingSwapEngine> foreignCcyLegEngine(
        new DiscountingSwapEngine(foreignCcyHandle));

    Real tolerance = 1.0e-12;

    for (Size i = 0; i < vars.basisData.size(); ++i) {

        XccyTestDatum quote = vars.basisData[i];
        std::vector<ext::shared_ptr<Swap> > xccySwapProxy = vars.buildXccyBasisSwap(
            quote, vars.fxSpot, isBasisOnFxBaseCurrencyLeg);

        if (isFxBaseCurrencyCollateralCurrency) {
            xccySwapProxy[0]->setPricingEngine(collateralCcyLegEngine);
            xccySwapProxy[1]->setPricingEngine(foreignCcyLegEngine);
        } else {
            xccySwapProxy[0]->setPricingEngine(foreignCcyLegEngine);
            xccySwapProxy[1]->setPricingEngine(collateralCcyLegEngine);
        }

        Period p = quote.n * quote.units;

        Real baseCcyLegNpv = vars.fxSpot * xccySwapProxy[0]->NPV();
        Real quoteCcyLegNpv = xccySwapProxy[1]->NPV();
        Real npv = baseCcyLegNpv + quoteCcyLegNpv;

        if (std::fabs(npv) > tolerance)
            BOOST_ERROR("unable to price the cross currency basis swap to par\n"
                        << std::setprecision(5) << "    calculated NPV:    " << npv << "\n"
                        << "    expected:    " << 0.0 << "\n"
                        << "    implied basis:    " << quote.basis << "\n"
                        << "    tenor:    " << p << "\n");
    }
}

void testResettingCrossCurrencySwaps(bool isFxBaseCurrencyCollateralCurrency,
                                     bool isBasisOnFxBaseCurrencyLeg,
                                     bool isFxBaseCurrencyLegResettable,
                                     std::optional<Frequency> paymentFrequency = std::nullopt,
                                     Integer paymentLag = 0,
                                     bool useOvernightIndex = false,
                                     std::optional<Frequency> quoteCcyPaymentFrequency = std::nullopt) {

    CommonVars vars;

    Handle<YieldTermStructure> collateralHandle =
        isFxBaseCurrencyCollateralCurrency ? vars.baseCcyIdxHandle : vars.quoteCcyIdxHandle;

    std::vector<ext::shared_ptr<RateHelper> > resettingInstruments =
        vars.buildResettingXccyRateHelpers(
            vars.basisData, collateralHandle, isFxBaseCurrencyCollateralCurrency,
            isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, paymentFrequency, paymentLag,
            useOvernightIndex, quoteCcyPaymentFrequency);

    std::vector<ext::shared_ptr<RateHelper> > constNotionalInstruments =
        vars.buildConstantNotionalXccyRateHelpers(vars.basisData, collateralHandle,
                                                  isFxBaseCurrencyCollateralCurrency,
                                                  isBasisOnFxBaseCurrencyLeg);

    ext::shared_ptr<YieldTermStructure> resettingCurve(
        new PiecewiseYieldCurve<Discount, LogLinear>(vars.curveSettlementDt, resettingInstruments, vars.dayCount));
    resettingCurve->enableExtrapolation();

    ext::shared_ptr<YieldTermStructure> constNotionalCurve(
        new PiecewiseYieldCurve<Discount, LogLinear>(vars.curveSettlementDt,
                                                     constNotionalInstruments,
                                                     vars.dayCount));
    constNotionalCurve->enableExtrapolation();

    Real tolerance = 1.0e-4 * 5;
    Size numberOfInstruments = vars.basisData.size();

    for (Size i = 0; i < numberOfInstruments; ++i) {

        Date maturity = resettingInstruments[i]->maturityDate();
        Rate resettingZero = resettingCurve->zeroRate(maturity, vars.dayCount, Continuous);
        Rate constNotionalZero = constNotionalCurve->zeroRate(maturity, vars.dayCount, Continuous);

        // The difference between resetting and constant notional curves
        // is not expected to be substantial. With the current setup it should
        // amount to only a few basis points - hence the tolerance level was
        // set at 5 bps.
        if (std::fabs(resettingZero - constNotionalZero) > tolerance)
            BOOST_ERROR("too large difference between resetting and constant notional curve \n"
                        << std::setprecision(5)
                        << "    zero from resetting curve:    " << resettingZero << "\n"
                        << "    zero from const notional curve:    " << constNotionalZero << "\n"
                        << "    maturity:    " << maturity << "\n");
    }
}

BOOST_AUTO_TEST_CASE(testIndexedCouponOverrides) {
    BOOST_TEST_MESSAGE(
        "Testing indexed-coupon overrides for cross-currency rate helpers...");

    IborCouponSettingsRestorer settingsRestorer;

    IborCoupon::Settings::instance().createAtParCoupons();
    checkCrossCurrencyHelperCouponModes(true, true);

    IborCoupon::Settings::instance().createIndexedCoupons();
    checkCrossCurrencyHelperCouponModes(false, false);

    IborCoupon::Settings::instance().createAtParCoupons();
    checkCrossCurrencyHelperCouponModes(std::nullopt, false);

    IborCoupon::Settings::instance().createIndexedCoupons();
    checkCrossCurrencyHelperCouponModes(std::nullopt, true);
}

BOOST_AUTO_TEST_CASE(testConstNotionalBasisSwapsWithCollateralInQuoteAndBasisInBaseCcy) {
    BOOST_TEST_MESSAGE("Testing constant notional basis swaps with collateral in quote ccy and "
                       "basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testConstantNotionalCrossCurrencySwapsNPV(isFxBaseCurrencyCollateralCurrency,
                                              isBasisOnFxBaseCurrencyLeg);
}

BOOST_AUTO_TEST_CASE(testConstNotionalBasisSwapsWithCollateralInBaseAndBasisInQuoteCcy) {
    BOOST_TEST_MESSAGE(
        "Testing constant notional basis swaps with collateral in base ccy and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testConstantNotionalCrossCurrencySwapsNPV(isFxBaseCurrencyCollateralCurrency,
                                              isBasisOnFxBaseCurrencyLeg);
}

BOOST_AUTO_TEST_CASE(testConstNotionalBasisSwapsWithCollateralAndBasisInBaseCcy) {
    BOOST_TEST_MESSAGE(
        "Testing constant notional basis swaps with collateral and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testConstantNotionalCrossCurrencySwapsNPV(isFxBaseCurrencyCollateralCurrency,
                                              isBasisOnFxBaseCurrencyLeg);
}

BOOST_AUTO_TEST_CASE(testConstNotionalBasisSwapsWithCollateralAndBasisInQuoteCcy) {
    BOOST_TEST_MESSAGE("Testing constant notional basis swaps with collateral and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testConstantNotionalCrossCurrencySwapsNPV(isFxBaseCurrencyCollateralCurrency,
                                              isBasisOnFxBaseCurrencyLeg);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithCollateralInQuoteAndBasisInBaseCcy) {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in quote ccy and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                     isFxBaseCurrencyLegResettable);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithCollateralInBaseAndBasisInQuoteCcy) {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in base ccy and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isFxBaseCurrencyLegResettable = true;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                     isFxBaseCurrencyLegResettable);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithCollateralAndBasisInBaseCcy) {
    BOOST_TEST_MESSAGE("Testing resetting basis swaps with collateral and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isFxBaseCurrencyLegResettable = true;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                     isFxBaseCurrencyLegResettable);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithCollateralAndBasisInQuoteCcy) {
    BOOST_TEST_MESSAGE("Testing resetting basis swaps with collateral and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                     isFxBaseCurrencyLegResettable);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithArbitraryFreq) {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in quote ccy and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                    isFxBaseCurrencyLegResettable,
                                    Weekly);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithAsymmetricFrequencies) {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with different payment frequencies per leg...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                    isFxBaseCurrencyLegResettable, Semiannual, 0, false,
                                    Quarterly);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsTreatNoFrequencyAsUnset) {
    BOOST_TEST_MESSAGE(
        "Testing that an explicit NoFrequency reproduces the default payment frequency...");

    CommonVars vars;

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isBasisOnFxBaseCurrencyLeg = true;
    bool isFxBaseCurrencyLegResettable = false;

    Handle<YieldTermStructure> collateralHandle = vars.quoteCcyIdxHandle;

    // Default (unset) payment frequencies: the schedule is derived from the
    // index tenor on both legs.
    std::vector<ext::shared_ptr<RateHelper> > defaultInstruments =
        vars.buildResettingXccyRateHelpers(
            vars.basisData, collateralHandle, isFxBaseCurrencyCollateralCurrency,
            isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, std::nullopt, 0, false,
            std::nullopt);

    // An explicit NoFrequency on both legs must behave identically.  Before
    // NoFrequency was normalized to nullopt it built a single-period schedule
    // (Period(NoFrequency) has zero length, which Schedule maps to the Zero
    // date-generation rule), silently changing the result instead of using the
    // index tenor.
    std::vector<ext::shared_ptr<RateHelper> > noFrequencyInstruments =
        vars.buildResettingXccyRateHelpers(
            vars.basisData, collateralHandle, isFxBaseCurrencyCollateralCurrency,
            isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, NoFrequency, 0, false,
            NoFrequency);

    ext::shared_ptr<YieldTermStructure> defaultCurve(new PiecewiseYieldCurve<Discount, LogLinear>(
        vars.curveSettlementDt, defaultInstruments, vars.dayCount));
    defaultCurve->enableExtrapolation();

    ext::shared_ptr<YieldTermStructure> noFrequencyCurve(new PiecewiseYieldCurve<Discount, LogLinear>(
        vars.curveSettlementDt, noFrequencyInstruments, vars.dayCount));
    noFrequencyCurve->enableExtrapolation();

    Real tolerance = 1.0e-12;
    for (Size i = 0; i < vars.basisData.size(); ++i) {
        Date maturity = defaultInstruments[i]->maturityDate();
        Rate defaultZero = defaultCurve->zeroRate(maturity, vars.dayCount, Continuous);
        Rate noFrequencyZero = noFrequencyCurve->zeroRate(maturity, vars.dayCount, Continuous);

        if (std::fabs(defaultZero - noFrequencyZero) > tolerance)
            BOOST_ERROR("explicit NoFrequency does not match the default payment frequency\n"
                        << std::setprecision(16)
                        << "    zero with default frequency:    " << defaultZero << "\n"
                        << "    zero with explicit NoFrequency:    " << noFrequencyZero << "\n"
                        << "    maturity:    " << maturity << "\n");
    }
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsQuoteFrequencyDefaultsToBase) {
    BOOST_TEST_MESSAGE(
        "Testing that an unset quote-currency payment frequency defaults to the base frequency...");

    CommonVars vars;

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isBasisOnFxBaseCurrencyLeg = true;
    bool isFxBaseCurrencyLegResettable = false;

    Handle<YieldTermStructure> collateralHandle = vars.quoteCcyIdxHandle;

    Frequency baseFrequency = Semiannual;

    // Quote-currency frequency left unset: it must fall back to the base frequency.
    std::vector<ext::shared_ptr<RateHelper> > fallbackInstruments =
        vars.buildResettingXccyRateHelpers(
            vars.basisData, collateralHandle, isFxBaseCurrencyCollateralCurrency,
            isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, baseFrequency, 0, false,
            std::nullopt);

    // Quote-currency frequency set explicitly to the base frequency: same result.
    std::vector<ext::shared_ptr<RateHelper> > explicitInstruments =
        vars.buildResettingXccyRateHelpers(
            vars.basisData, collateralHandle, isFxBaseCurrencyCollateralCurrency,
            isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, baseFrequency, 0, false,
            baseFrequency);

    ext::shared_ptr<YieldTermStructure> fallbackCurve(new PiecewiseYieldCurve<Discount, LogLinear>(
        vars.curveSettlementDt, fallbackInstruments, vars.dayCount));
    fallbackCurve->enableExtrapolation();

    ext::shared_ptr<YieldTermStructure> explicitCurve(new PiecewiseYieldCurve<Discount, LogLinear>(
        vars.curveSettlementDt, explicitInstruments, vars.dayCount));
    explicitCurve->enableExtrapolation();

    Real tolerance = 1.0e-12;
    for (Size i = 0; i < vars.basisData.size(); ++i) {
        Date maturity = fallbackInstruments[i]->maturityDate();
        Rate fallbackZero = fallbackCurve->zeroRate(maturity, vars.dayCount, Continuous);
        Rate explicitZero = explicitCurve->zeroRate(maturity, vars.dayCount, Continuous);

        if (std::fabs(fallbackZero - explicitZero) > tolerance)
            BOOST_ERROR("unset quote-currency frequency does not default to the base frequency\n"
                        << std::setprecision(16)
                        << "    zero with quote frequency unset:    " << fallbackZero << "\n"
                        << "    zero with quote frequency = base:    " << explicitZero << "\n"
                        << "    maturity:    " << maturity << "\n");
    }
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithPaymentLag) {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in quote ccy and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                    isFxBaseCurrencyLegResettable, std::nullopt, 2);
}

BOOST_AUTO_TEST_CASE(testMtMHelperMatchesStandaloneWithAsymmetricFxHolidays) {
    BOOST_TEST_MESSAGE(
        "Testing helper/instrument equivalence across asymmetric FX holidays...");

    SavedSettings backup;
    Date today(3, July, 2024);
    Settings::instance().evaluationDate() = today;

    Calendar legCalendar = TARGET();
    Calendar fxCalendar = JointCalendar(
        TARGET(), UnitedStates(UnitedStates::Settlement), JoinHolidays);
    Natural settlementDays = 3;
    Natural fxResetFixingDays = 2;
    Integer paymentLag = 2;
    Date start = legCalendar.advance(today, settlementDays, Days);
    Date expectedFxFixingDate = fxCalendar.advance(
        start, -static_cast<Integer>(fxResetFixingDays), Days);

    BOOST_REQUIRE_EQUAL(start, Date(8, July, 2024));
    BOOST_REQUIRE_EQUAL(expectedFxFixingDate, today);
    // July 4th is a TARGET business day but a US holiday.  This proves that
    // the separate FX calendar, rather than the leg calendar, drives the reset.
    BOOST_REQUIRE_EQUAL(
        legCalendar.advance(start, -static_cast<Integer>(fxResetFixingDays), Days),
        Date(4, July, 2024));

    Handle<YieldTermStructure> eurForecast(
        ext::make_shared<FlatForward>(today, 0.015, Actual365Fixed()));
    Handle<YieldTermStructure> usdCurve(
        ext::make_shared<FlatForward>(today, 0.030, Actual365Fixed()));
    auto eurIndex = ext::make_shared<Euribor3M>(eurForecast);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);

    Real observedFx = 1.25;
    ExchangeRateManager::instance().add(
        ExchangeRate(EURCurrency(), USDCurrency(), observedFx),
        expectedFxFixingDate, expectedFxFixingDate);

    Spread basis = 10.0e-4;
    auto helper = ext::make_shared<MtMCrossCurrencyBasisSwapRateHelper>(
        makeQuoteHandle(basis), 1 * Years, settlementDays, legCalendar, Following, false,
        eurIndex, usdIndex, usdCurve,
        /*isFxBaseCurrencyCollateralCurrency=*/false,
        /*isBasisOnFxBaseCurrencyLeg=*/false,
        /*isFxBaseCurrencyLegResettable=*/false,
        Quarterly, paymentLag, Semiannual, fxResetFixingDays, fxCalendar);

    std::vector<ext::shared_ptr<RateHelper> > helpers(1, helper);
    auto eurDiscount = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear> >(
        today, helpers, Actual365Fixed());
    eurDiscount->enableExtrapolation();
    eurDiscount->discount(helper->maturityDate());
    Handle<YieldTermStructure> eurDiscountHandle(eurDiscount);

    auto helperSwap = helper->swap();
    auto standalone = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency,
        1.0, EURCurrency(), helperSwap->fxBaseSchedule(), eurIndex, 0.0, 1.0,
        1.0, USDCurrency(), helperSwap->fxQuoteSchedule(), usdIndex, 0.0, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false,
        fxResetFixingDays, fxCalendar,
        paymentLag, paymentLag, Following, Following);
    standalone->setPricingEngine(
        ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
            USDCurrency(), usdCurve, EURCurrency(), eurDiscountHandle,
            makeQuoteHandle(1.0), true));

    const Leg& helperResettingLeg = helperSwap->resettingLeg();
    const Leg& standaloneResettingLeg = standalone->resettingLeg();
    BOOST_REQUIRE_EQUAL(helperResettingLeg.size(), standaloneResettingLeg.size());

    ext::shared_ptr<FxResetCoupon> firstHelperCoupon;
    for (Size i = 0; i < helperResettingLeg.size(); ++i) {
        BOOST_CHECK_EQUAL(helperResettingLeg[i]->date(), standaloneResettingLeg[i]->date());
        auto helperCoupon = ext::dynamic_pointer_cast<FxResetCoupon>(helperResettingLeg[i]);
        auto standaloneCoupon =
            ext::dynamic_pointer_cast<FxResetCoupon>(standaloneResettingLeg[i]);
        BOOST_REQUIRE_EQUAL(helperCoupon != nullptr, standaloneCoupon != nullptr);
        if (helperCoupon) {
            if (!firstHelperCoupon)
                firstHelperCoupon = helperCoupon;
            BOOST_CHECK_EQUAL(helperCoupon->fxResetDate(), standaloneCoupon->fxResetDate());
            BOOST_CHECK_EQUAL(helperCoupon->fxResetValueDate(),
                              standaloneCoupon->fxResetValueDate());
        }
    }

    BOOST_REQUIRE(firstHelperCoupon != nullptr);
    BOOST_CHECK_EQUAL(firstHelperCoupon->fxResetDate(), expectedFxFixingDate);
    BOOST_CHECK_EQUAL(firstHelperCoupon->fxResetValueDate(), start);
    BOOST_CHECK_SMALL(helperSwap->NPV() - standalone->NPV(), 1.0e-12);
    BOOST_CHECK_SMALL(helperSwap->fairFxQuoteSpread() - standalone->fairFxQuoteSpread(),
                      1.0e-12);
    BOOST_CHECK_SMALL(helperSwap->fairFxQuoteSpread() - basis, 1.0e-10);

    // Under the previous zero-day convention the first reset is still in the
    // future and is projected instead of using today's observed fixing.
    auto zeroLagReset = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency,
        1.0, EURCurrency(), helperSwap->fxBaseSchedule(), eurIndex, 0.0, 1.0,
        1.0, USDCurrency(), helperSwap->fxQuoteSchedule(), usdIndex, 0.0, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false, /*fxResetFixingDays=*/0, /*fxResetFixingCalendar=*/Calendar(),
        paymentLag, paymentLag, Following, Following);
    zeroLagReset->setPricingEngine(
        ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
            USDCurrency(), usdCurve, EURCurrency(), eurDiscountHandle,
            makeQuoteHandle(1.0), true));

    ext::shared_ptr<FxResetCoupon> firstZeroLagCoupon;
    for (const auto& cashflow : zeroLagReset->resettingLeg()) {
        firstZeroLagCoupon = ext::dynamic_pointer_cast<FxResetCoupon>(cashflow);
        if (firstZeroLagCoupon)
            break;
    }
    BOOST_REQUIRE(firstZeroLagCoupon != nullptr);
    BOOST_CHECK_EQUAL(firstZeroLagCoupon->fxResetDate(), start);
    Spread zeroLagFairSpreadDifference =
        std::fabs(zeroLagReset->fairFxQuoteSpread() - standalone->fairFxQuoteSpread());
    BOOST_CHECK_MESSAGE(zeroLagFairSpreadDifference > 1.0e-8,
                        "zero-lag fair-spread difference was "
                            << zeroLagFairSpreadDifference);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithOvernightIndex) {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in quote ccy and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                    isFxBaseCurrencyLegResettable, Quarterly, 0, true);
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithOvernightIndexException) {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in quote ccy and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    BOOST_CHECK_THROW(testResettingCrossCurrencySwaps(
                          isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                          isFxBaseCurrencyLegResettable, std::nullopt, 0, true),
        Error);
}

BOOST_AUTO_TEST_CASE(testExceptionWhenInstrumentTenorShorterThanIndexFrequency) {
    BOOST_TEST_MESSAGE(
        "Testing exception when instrument tenor is shorter than index frequency...");

    CommonVars vars;

    std::vector<XccyTestDatum> data{{1, Months, 10.0}};
    Handle<YieldTermStructure> collateralHandle;

    BOOST_CHECK_THROW(
        std::vector<ext::shared_ptr<RateHelper> > resettingInstruments =
            vars.buildConstantNotionalXccyRateHelpers(data, collateralHandle, true, true),
        Error);
}

// -----------------------------------------------------------------------------
// ConstNotionalCrossCurrencySwapRateHelper Tests
// -----------------------------------------------------------------------------



BOOST_AUTO_TEST_CASE(testConstNotionalCrossCurrencySwapRateHelperRelinking) {
    BOOST_TEST_MESSAGE("Testing ConstNotionalCrossCurrencySwapRateHelper reaction to relinked curves...");

    SavedSettings backup;
    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;

    RelinkableHandle<YieldTermStructure> usdCollat;
    usdCollat.linkTo(ext::make_shared<FlatForward>(today, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> eurFwd(
        ext::make_shared<FlatForward>(today, 0.017, Actual365Fixed()));

    ext::shared_ptr<IborIndex> euribor3m = ext::make_shared<Euribor3M>(eurFwd);
    Handle<Quote> q(ext::make_shared<SimpleQuote>(0.018));

    ConstNotionalCrossCurrencySwapRateHelper h(
        q, Period(5, Years), 2, TARGET(), Following, true, Annual,
        Thirty360(Thirty360::BondBasis), euribor3m,
        usdCollat, true);

    RelinkableHandle<YieldTermStructure> bootstrapCurve;
    bootstrapCurve.linkTo(ext::make_shared<FlatForward>(today, 0.02, Actual360()));
    h.setTermStructure(bootstrapCurve.currentLink().get());

    Real oldQuote = h.impliedQuote();

    usdCollat.linkTo(ext::make_shared<FlatForward>(today, 0.03, Actual365Fixed())); // 3%
    Real newQuote = h.impliedQuote();

    BOOST_CHECK(oldQuote != newQuote);
}

BOOST_AUTO_TEST_CASE(testConstNotionalHelperFloatingLegConvention) {
    BOOST_TEST_MESSAGE("Testing the floating-leg convention of fixed-vs-floating helpers...");

    SavedSettings backup;
    Date today(29, July, 2026);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = WeekendsOnly();
    Handle<YieldTermStructure> collateralCurve(
        ext::make_shared<FlatForward>(today, 0.04, Actual360()));
    // An overnight index reports Following regardless of the market it models,
    // so the helper convention has to drive both legs.
    auto overnightIndex = ext::make_shared<OvernightIndex>(
        "ON", 2, USDCurrency(), calendar, Actual360(), collateralCurve);
    BOOST_CHECK_EQUAL(overnightIndex->businessDayConvention(), Following);

    // The helper convention alone, as passed by an ordinary caller, must reach
    // the floating leg: 31st July 2027 is a Saturday, so ModifiedFollowing
    // rolls back to the 30th instead of forward into August.
    ConstNotionalCrossCurrencySwapRateHelper helper(
        makeQuoteHandle(0.0), 1 * Years, 2, calendar, ModifiedFollowing, false,
        Semiannual, Actual365Fixed(), overnightIndex, collateralCurve, false, 0,
        std::nullopt, Semiannual);

    Date expectedMaturity(30, July, 2027);
    BOOST_CHECK_EQUAL(helper.maturityDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(helper.swap()->fixedSchedule().endDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(helper.swap()->floatSchedule().endDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(helper.swap()->fixedPaymentBdc(), ModifiedFollowing);
    BOOST_CHECK_EQUAL(helper.swap()->floatPaymentBdc(), ModifiedFollowing);
}

BOOST_AUTO_TEST_CASE(testConstNotionalHelperIgnoresIndexFixingCalendar) {
    BOOST_TEST_MESSAGE("Testing that fixed-vs-floating helpers roll both legs on the helper calendar...");

    SavedSettings backup;
    Date today(1, July, 2026);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = WeekendsOnly();
    Handle<YieldTermStructure> collateralCurve(
        ext::make_shared<FlatForward>(today, 0.04, Actual360()));

    // Same index in every respect but the fixing calendar.  A fixing calendar
    // governs where the rate fixes, not where the accrual schedule rolls, so
    // neither the schedules nor the pillar may depend on it: 3rd July 2027 is a
    // Saturday and the 5th is the observed US Independence Day, so leaking the
    // index calendar into the schedule would push the floating leg to the 6th.
    auto weekendIndex = ext::make_shared<OvernightIndex>(
        "ON weekend", 2, USDCurrency(), calendar, Actual360(), collateralCurve);
    auto usIndex = ext::make_shared<OvernightIndex>(
        "ON US", 2, USDCurrency(), UnitedStates(UnitedStates::GovernmentBond), Actual360(),
        collateralCurve);
    BOOST_CHECK(usIndex->fixingCalendar().isHoliday(Date(5, July, 2027)));

    auto makeHelper = [&](const ext::shared_ptr<IborIndex>& index) {
        return ConstNotionalCrossCurrencySwapRateHelper(
            makeQuoteHandle(0.0), 1 * Years, 2, calendar, ModifiedFollowing, false,
            Semiannual, Actual365Fixed(), index, collateralCurve, false, 0,
            std::nullopt, Semiannual);
    };
    auto weekendHelper = makeHelper(weekendIndex);
    auto usHelper = makeHelper(usIndex);

    Date expectedMaturity(5, July, 2027);
    BOOST_CHECK_EQUAL(weekendHelper.swap()->floatSchedule().endDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(usHelper.swap()->floatSchedule().endDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(usHelper.swap()->fixedSchedule().endDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(usHelper.maturityDate(), weekendHelper.maturityDate());
    // The index keeps its own calendar for its fixings.
    BOOST_CHECK(usIndex->fixingCalendar() != weekendIndex->fixingCalendar());
}

BOOST_AUTO_TEST_CASE(testConstNotionalHelperFloatingPaymentFrequency) {
    BOOST_TEST_MESSAGE("Testing the floating-leg payment frequency of fixed-vs-floating helpers...");

    SavedSettings backup;
    Date today(15, July, 2026);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = TARGET();
    Handle<YieldTermStructure> collateralCurve(
        ext::make_shared<FlatForward>(today, 0.04, Actual360()));
    auto overnightIndex = ext::make_shared<OvernightIndex>(
        "ON", 0, EURCurrency(), calendar, Actual360(), collateralCurve);
    auto euribor3m = ext::make_shared<Euribor3M>(collateralCurve);

    auto makeHelper = [&](const ext::shared_ptr<IborIndex>& index,
                          std::optional<Frequency> floatFrequency) {
        return ConstNotionalCrossCurrencySwapRateHelper(
            makeQuoteHandle(0.01), 1 * Years, 2, calendar, Following, false,
            Annual, Actual365Fixed(), index, collateralCurve, false, 0,
            std::nullopt, floatFrequency);
    };

    // An overnight index has a one-day tenor, which is not a payment frequency;
    // asking it for one would silently produce a daily-paying leg.
    BOOST_CHECK_THROW(makeHelper(overnightIndex, std::nullopt), Error);

    // Given a frequency, the overnight leg pays on that schedule.
    auto overnightHelper = makeHelper(overnightIndex, Semiannual);
    BOOST_CHECK_EQUAL(overnightHelper.swap()->floatSchedule().size(), 3);
    BOOST_CHECK_EQUAL(overnightHelper.swap()->fixedSchedule().size(), 2);

    // An ibor index still falls back to its own tenor.
    auto iborHelper = makeHelper(euribor3m, std::nullopt);
    BOOST_CHECK_EQUAL(iborHelper.swap()->floatSchedule().size(), 5);

    // An explicit frequency overrides the ibor tenor.
    auto iborSemiannualHelper = makeHelper(euribor3m, Semiannual);
    BOOST_CHECK_EQUAL(iborSemiannualHelper.swap()->floatSchedule().size(), 3);

    // NoFrequency is a synonym for an unset value, as in the basis helpers.
    auto iborNoFrequencyHelper = makeHelper(euribor3m, NoFrequency);
    BOOST_CHECK_EQUAL(iborNoFrequencyHelper.swap()->floatSchedule().size(), 5);
}

BOOST_AUTO_TEST_CASE(testPaymentLagDoesNotDelayNotionalExchanges) {
    BOOST_TEST_MESSAGE("Testing that payment lag applies to coupons but not notional exchanges...");

    SavedSettings backup;
    Date today(15, July, 2026);
    Settings::instance().evaluationDate() = today;

    Calendar cal = TARGET();
    Handle<YieldTermStructure> collateralCurve(
        ext::make_shared<FlatForward>(today, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> forwardingCurve(
        ext::make_shared<FlatForward>(today, 0.017, Actual365Fixed()));
    auto euribor3m = ext::make_shared<Euribor3M>(forwardingCurve);

    ConstNotionalCrossCurrencySwapRateHelper helper(
        makeQuoteHandle(0.02), 3 * Months, 2, cal, Following, false,
        Quarterly, Actual360(), euribor3m, collateralCurve, false, 2);

    Date effectiveDate(17, July, 2026);
    Date maturityDate(19, October, 2026);
    Date couponPaymentDate(21, October, 2026);

    const auto& swap = helper.swap();
    for (const Leg& leg : swap->legs()) {
        BOOST_REQUIRE_EQUAL(leg.size(), 3U);
        BOOST_CHECK_EQUAL(leg[0]->date(), effectiveDate);
        BOOST_CHECK_EQUAL(leg[1]->date(), maturityDate);
        BOOST_CHECK_EQUAL(leg[2]->date(), couponPaymentDate);
    }

    BOOST_CHECK_EQUAL(helper.earliestDate(), effectiveDate);
    BOOST_CHECK_EQUAL(helper.maturityDate(), maturityDate);
    BOOST_CHECK_EQUAL(helper.latestDate(), couponPaymentDate);
}

BOOST_AUTO_TEST_CASE(testBasisHelpersWithPaymentLagMatchUnderlyingSwaps) {
    BOOST_TEST_MESSAGE("Testing payment-lagged basis helpers against their underlying swaps...");

    CommonVars vars;
    Handle<YieldTermStructure> collateralCurve = vars.quoteCcyIdxHandle;
    auto bootstrappedCurve = vars.baseCcyIdxHandle.currentLink();
    Handle<Quote> quote = makeQuoteHandle(-20.0 * vars.basisPoint);

    ConstNotionalCrossCurrencyBasisSwapRateHelper constantHelper(
        quote, 5 * Years, vars.instrumentSettlementDays, vars.calendar,
        vars.businessConvention, vars.endOfMonth, vars.baseCcyIdx, vars.quoteCcyIdx,
        collateralCurve, false, true, std::nullopt, 2);
    constantHelper.setTermStructure(bootstrappedCurve.get());

    BOOST_CHECK_SMALL(constantHelper.impliedQuote() - constantHelper.swap()->fairPaySpread(),
                      1.0e-12);

    MtMCrossCurrencyBasisSwapRateHelper resettingHelper(
        quote, 5 * Years, vars.instrumentSettlementDays, vars.calendar,
        vars.businessConvention, vars.endOfMonth, vars.baseCcyIdx, vars.quoteCcyIdx,
        collateralCurve, false, true, false, std::nullopt, 2);
    resettingHelper.setTermStructure(bootstrappedCurve.get());

    BOOST_CHECK_SMALL(resettingHelper.impliedQuote() -
                          resettingHelper.swap()->fairFxBaseSpread(),
                      1.0e-12);
}

BOOST_AUTO_TEST_CASE(testConstNotionalHelperCollateralOnFixedLeg) {
    BOOST_TEST_MESSAGE("Testing const-notional CCS helper with collateral on fixed leg...");

    SavedSettings backup;
    Date today(20, March, 2030);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCollat(
        ext::make_shared<FlatForward>(today, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> eurFwd(
        ext::make_shared<FlatForward>(today, 0.017, Actual365Fixed()));

    ext::shared_ptr<IborIndex> euribor3m =
        ext::make_shared<Euribor3M>(eurFwd);

    Natural fixingDays = 5;
    Calendar cal = TARGET();
    BusinessDayConvention bdc = Following;
    bool endOfMonth = true;
    Frequency fixedFreq = Annual;
    DayCounter fixedDC = Thirty360(Thirty360::BondBasis);

    std::vector<std::pair<Period, Real>> quotes = {
        {Period(5, Years), 0.018},
        {Period(7, Years), 0.019},
        {Period(10, Years), 0.022},
        {Period(15, Years), 0.024},
        {Period(20, Years), 0.028},
    };

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    helpers.reserve(quotes.size());
    for (auto [tenor, q]: quotes) {
        helpers.push_back(ext::make_shared<ConstNotionalCrossCurrencySwapRateHelper>(
            makeQuoteHandle(q), tenor, fixingDays, cal, bdc, endOfMonth,
            fixedFreq, fixedDC, euribor3m,
            usdCollat, true));
    }

    typedef PiecewiseYieldCurve<Discount, LogLinear> Curve;
    ext::shared_ptr<YieldTermStructure> curve(
        new Curve(today, helpers, Actual365Fixed()));
    curve->enableExtrapolation();
    Handle<YieldTermStructure> curveHandle(curve);

    auto fixedEngine = ext::make_shared<DiscountingSwapEngine>(usdCollat);
    auto floatEngine = ext::make_shared<DiscountingSwapEngine>(curveHandle);

    for (auto [tenor, q]: quotes) {

        Date settlement = cal.advance(today, fixingDays, Days);
        Date maturity   = cal.advance(settlement, tenor, bdc, endOfMonth);

        Schedule fixedSched(settlement, maturity,
                            Period(fixedFreq),
                            cal, bdc, bdc,
                            DateGeneration::Forward, endOfMonth);

        // The helper rolls both legs on its own calendar and convention; the
        // index only supplies its fixing calendar for fixings.
        Schedule floatSched(settlement, maturity,
                            euribor3m->tenor(),
                            cal, bdc, bdc,
                            DateGeneration::Forward, false);

        Leg fixedLeg = FixedRateLeg(fixedSched)
                       .withNotionals(1.0)
                       .withCouponRates(q, fixedDC);

        Leg floatLeg = IborLeg(floatSched, euribor3m)
                       .withNotionals(1.0)
                       .withSpreads(0.0);

        Date initialPaymentDate = CashFlows::startDate(fixedLeg);
        fixedLeg.insert(fixedLeg.begin(), ext::make_shared<SimpleCashFlow>(-1.0, initialPaymentDate));
        floatLeg.insert(floatLeg.begin(), ext::make_shared<SimpleCashFlow>(-1.0, initialPaymentDate));

        Date finalPaymentDate = CashFlows::maturityDate(fixedLeg);
        fixedLeg.push_back(ext::make_shared<SimpleCashFlow>(1.0, finalPaymentDate));
        floatLeg.push_back(ext::make_shared<SimpleCashFlow>(1.0, finalPaymentDate));

        Swap fixedProxy(std::vector<Leg>(1, fixedLeg),
                        std::vector<bool>(1, true));
        Swap floatProxy(std::vector<Leg>(1, floatLeg),
                        std::vector<bool>(1, false));

        fixedProxy.setPricingEngine(fixedEngine);
        floatProxy.setPricingEngine(floatEngine);

        Real npv = fixedProxy.NPV() + floatProxy.NPV();
        Real tolerance = 1e-10;

        QL_CHECK_SMALL(npv, tolerance);
    }
}


BOOST_AUTO_TEST_CASE(testConstNotionalHelperCollateralOnFloatingLeg) {
    BOOST_TEST_MESSAGE("Testing const-notional CCS helper with collateral on floating leg...");

    SavedSettings backup;
    Date today(20, March, 2030);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCollat(
        ext::make_shared<FlatForward>(today, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> eurFwd(
        ext::make_shared<FlatForward>(today, 0.017, Actual365Fixed()));

    ext::shared_ptr<IborIndex> euribor3m =
        ext::make_shared<Euribor3M>(eurFwd);

    Natural fixingDays = 5;
    Calendar cal = TARGET();
    BusinessDayConvention bdc = Following;
    bool endOfMonth = true;
    Frequency fixedFreq = Annual;
    DayCounter fixedDC = Thirty360(Thirty360::BondBasis);
    Integer paymentLag = 5;

    std::vector<std::pair<Period, Real>> quotes = {
        {Period(5, Years), 0.018},
        {Period(7, Years), 0.019},
        {Period(10, Years), 0.022},
        {Period(15, Years), 0.024},
        {Period(20, Years), 0.028},
    };

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    helpers.reserve(quotes.size());
    for (auto [tenor, q]: quotes) {
        helpers.push_back(ext::make_shared<ConstNotionalCrossCurrencySwapRateHelper>(
            makeQuoteHandle(q), tenor, fixingDays, cal, bdc, endOfMonth,
            fixedFreq, fixedDC, euribor3m,
            usdCollat, false, paymentLag));
    }

    typedef PiecewiseYieldCurve<Discount, LogLinear> Curve;
    ext::shared_ptr<YieldTermStructure> curve(
        new Curve(today, helpers, Actual365Fixed()));
    curve->enableExtrapolation();
    Handle<YieldTermStructure> curveHandle(curve);

    auto fixedEngine = ext::make_shared<DiscountingSwapEngine>(curveHandle);
    auto floatEngine = ext::make_shared<DiscountingSwapEngine>(usdCollat);

    for (auto [tenor, q]: quotes) {
        Date settlement = cal.advance(today, fixingDays, Days);
        Date maturity   = cal.advance(settlement, tenor, bdc, endOfMonth);

        Schedule fixedSched(settlement, maturity,
                            Period(fixedFreq),
                            cal, bdc, bdc,
                            DateGeneration::Forward, endOfMonth);

        // The helper rolls both legs on its own calendar and convention; the
        // index only supplies its fixing calendar for fixings.
        Schedule floatSched(settlement, maturity,
                            euribor3m->tenor(),
                            cal, bdc, bdc,
                            DateGeneration::Forward, false);

        Leg fixedLeg = FixedRateLeg(fixedSched)
                       .withNotionals(1.0)
                       .withCouponRates(q, fixedDC)
                       .withPaymentLag(paymentLag)
                       .withPaymentAdjustment(bdc)
                       .withPaymentCalendar(cal);

        Leg floatLeg = IborLeg(floatSched, euribor3m)
                       .withNotionals(1.0)
                       .withSpreads(0.0)
                       .withPaymentLag(paymentLag)
                       .withPaymentAdjustment(bdc)
                       .withPaymentCalendar(cal);

        Date initialPaymentDate = CashFlows::startDate(fixedLeg);
        fixedLeg.insert(fixedLeg.begin(), ext::make_shared<SimpleCashFlow>(-1.0, initialPaymentDate));
        floatLeg.insert(floatLeg.begin(), ext::make_shared<SimpleCashFlow>(-1.0, initialPaymentDate));

        Date finalPaymentDate = CashFlows::maturityDate(fixedLeg);
        fixedLeg.push_back(ext::make_shared<SimpleCashFlow>(1.0, finalPaymentDate));
        floatLeg.push_back(ext::make_shared<SimpleCashFlow>(1.0, finalPaymentDate));

        Swap fixedProxy(std::vector<Leg>(1, fixedLeg),
                        std::vector<bool>(1, true));
        Swap floatProxy(std::vector<Leg>(1, floatLeg),
                        std::vector<bool>(1, false));

        fixedProxy.setPricingEngine(fixedEngine);
        floatProxy.setPricingEngine(floatEngine);

        Real npv = fixedProxy.NPV() + floatProxy.NPV();
        Real tolerance = 1e-10;

        QL_CHECK_SMALL(npv, tolerance);
    }
}

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithInterpolatedBrokenIndex) {
    BOOST_TEST_MESSAGE(
        "Testing MtM cross-currency bootstrap with an interpolated broken-index stub...");

    SavedSettings backup;
    Date today(27, May, 2026);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = NewZealand();
    DayCounter dayCount = Actual365Fixed();

    // Distinct projection curves so the interpolated stub fixing differs
    // measurably from the leg index's own fixing.
    RelinkableHandle<YieldTermStructure> shortProjection, baseProjection, usdProjection;
    shortProjection.linkTo(flatRate(today, 0.02, dayCount));
    baseProjection.linkTo(flatRate(today, 0.04, dayCount));
    usdProjection.linkTo(flatRate(today, 0.015, Actual360()));

    auto bkbm2m = ext::make_shared<Bkbm2M>(shortProjection);
    auto bkbm3m = ext::make_shared<Bkbm3M>(baseProjection);
    auto sofr = ext::make_shared<Sofr>(usdProjection);

    BrokenIndexConfig brokenIndexConfig{BrokenIndexConvention::Interpolated,
                                        {bkbm2m, bkbm3m}};

    Handle<YieldTermStructure> collateralHandle = usdProjection;

    auto makeHelper = [&](Spread basis, const Period& tenor,
                          const BrokenIndexConfig& config,
                          bool isFxBaseCurrencyLegResettable) {
        return ext::make_shared<MtMCrossCurrencyBasisSwapRateHelper>(
            makeQuoteHandle(basis), tenor, 2, calendar, ModifiedFollowing, false,
            bkbm3m, sofr, collateralHandle,
            false,  // collateral in quote currency
            true,   // basis on the base-currency leg
            isFxBaseCurrencyLegResettable, Quarterly, 0, Quarterly, 0, Calendar(),
            std::nullopt, config);
    };

    for (bool isFxBaseCurrencyLegResettable : {false, true}) {
        // The 9M swap's spot start (29 May 2026) plus 9M lands on 28 Feb 2027,
        // so the backward quarterly schedule leaves a broken first period.
        auto stubHelper =
            makeHelper(-7e-4, 9 * Months, brokenIndexConfig, isFxBaseCurrencyLegResettable);

        auto stubCoupon = ext::dynamic_pointer_cast<StubIborCoupon>(
            firstIborCoupon(stubHelper->swap()->legs()[0]));
        BOOST_REQUIRE(stubCoupon);

        const Date fixingDate = stubCoupon->fixingDate();
        const Date valueDate = stubCoupon->accrualStartDate();
        const Date target = stubCoupon->accrualEndDate();
        const Date shortMaturity = bkbm2m->maturityDate(valueDate);
        const Date longMaturity = bkbm3m->maturityDate(valueDate);
        BOOST_REQUIRE(shortMaturity < target && target < longMaturity);

        const Rate shortRate = bkbm2m->fixing(fixingDate);
        const Rate longRate = bkbm3m->fixing(fixingDate);
        const Real weight =
            Real(target - shortMaturity) / Real(longMaturity - shortMaturity);
        const Rate expected = shortRate + (longRate - shortRate) * weight;
        QL_CHECK_SMALL(stubCoupon->indexFixing() - expected, 1e-14);
        BOOST_CHECK(std::fabs(stubCoupon->indexFixing() - bkbm3m->fixing(fixingDate)) > 1e-4);

        // Regular periods are untouched: only the broken 9M front period gets a
        // stub coupon, so the 6M and 1Y helpers price identically either way.
        std::vector<ext::shared_ptr<RateHelper> > instruments = {
            makeHelper(-5e-4, 6 * Months, brokenIndexConfig, isFxBaseCurrencyLegResettable),
            stubHelper,
            makeHelper(-9e-4, 1 * Years, brokenIndexConfig, isFxBaseCurrencyLegResettable)};

        auto stubCurve = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear> >(
            today, instruments, dayCount);
        stubCurve->enableExtrapolation();
        stubCurve->discount(1.0);

        for (const auto& instrument : instruments)
            QL_CHECK_SMALL(instrument->impliedQuote() - instrument->quote()->value(), 1e-10);

        // The same market priced with the default current-index convention must
        // bootstrap to a measurably different curve at the broken pillar.
        std::vector<ext::shared_ptr<RateHelper> > plainInstruments = {
            makeHelper(-5e-4, 6 * Months, {}, isFxBaseCurrencyLegResettable),
            makeHelper(-7e-4, 9 * Months, {}, isFxBaseCurrencyLegResettable),
            makeHelper(-9e-4, 1 * Years, {}, isFxBaseCurrencyLegResettable)};

        auto plainCurve = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear> >(
            today, plainInstruments, dayCount);
        plainCurve->enableExtrapolation();

        const Date brokenPillar = stubHelper->maturityDate();
        BOOST_CHECK(std::fabs(stubCurve->discount(brokenPillar) -
                              plainCurve->discount(brokenPillar)) > 1e-6);
    }
}


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
