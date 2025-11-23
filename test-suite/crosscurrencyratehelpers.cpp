/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2025 Uzair Beg 
Copyright (C) 2021 Marcin Rybacki

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
#include <ql/experimental/termstructures/crosscurrencyratehelpers.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/all.hpp>


using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CrossCurrencyRateHelpersTests)

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
                            Frequency paymentFrequency = NoFrequency,
                            Integer paymentLag = 0,
                            bool useOvernightIndex = false) const {
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
                isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, paymentFrequency, paymentLag));
    }

    std::vector<ext::shared_ptr<RateHelper> >
    buildResettingXccyRateHelpers(const std::vector<XccyTestDatum>& xccyData,
                                  const Handle<YieldTermStructure>& collateralHandle,
                                  bool isFxBaseCurrencyCollateralCurrency,
                                  bool isBasisOnFxBaseCurrencyLeg,
                                  bool isFxBaseCurrencyLegResettable,
                                  Frequency paymentFrequency = NoFrequency,
                                  Integer paymentLag = 0,
                                  bool useOvernightQuoteIndex = false) const {
        std::vector<ext::shared_ptr<RateHelper> > instruments;
        instruments.reserve(xccyData.size());
        for (const auto& i : xccyData) {
            instruments.push_back(resettingXccyRateHelper(
                    i, collateralHandle, isFxBaseCurrencyCollateralCurrency,
                    isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable,
                    paymentFrequency, paymentLag, useOvernightQuoteIndex));
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
                       bool isFxBaseCurrencyCollateralCurrency,
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
            quote, vars.fxSpot, isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg);

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
                                     Frequency paymentFrequency = NoFrequency,
                                     Integer paymentLag = 0,
                                     bool useOvernightIndex = false) {

    CommonVars vars;

    Handle<YieldTermStructure> collateralHandle =
        isFxBaseCurrencyCollateralCurrency ? vars.baseCcyIdxHandle : vars.quoteCcyIdxHandle;

    std::vector<ext::shared_ptr<RateHelper> > resettingInstruments =
        vars.buildResettingXccyRateHelpers(
            vars.basisData, collateralHandle, isFxBaseCurrencyCollateralCurrency,
            isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, paymentFrequency, paymentLag,
            useOvernightIndex);

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

BOOST_AUTO_TEST_CASE(testResettingBasisSwapsWithPaymentLag) {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in quote ccy and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                    isFxBaseCurrencyLegResettable, NoFrequency, 2);
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
                          isFxBaseCurrencyLegResettable, NoFrequency, 0, true),
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
    BOOST_TEST_MESSAGE("Testing ConstNotionalCrossCurrencySwapRateHelper reaction to relinked curves");

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
        Thirty360(Thirty360::BondBasis), USDCurrency(), euribor3m, EURCurrency(),
        usdCollat, true);

    RelinkableHandle<YieldTermStructure> bootstrapCurve;
    bootstrapCurve.linkTo(ext::make_shared<FlatForward>(today, 0.02, Actual360()));
    h.setTermStructure(bootstrapCurve.currentLink().get());

    Real oldQuote = h.impliedQuote();

    usdCollat.linkTo(ext::make_shared<FlatForward>(today, 0.03, Actual365Fixed())); // 3%
    Real newQuote = h.impliedQuote();

    BOOST_CHECK(oldQuote != newQuote);
}

BOOST_AUTO_TEST_CASE(testConstNotionalHelperCollateralOnFixedLeg) {

 BOOST_TEST_MESSAGE("Testing const-notional CCS helper with collateral on fixed leg");


    SavedSettings backup;
    Date today(20, March, 2030); 
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCollat(
        ext::make_shared<FlatForward>(today, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> eurFwd(
        ext::make_shared<FlatForward>(today, 0.017, Actual365Fixed()));

    ext::shared_ptr<IborIndex> euribor3m =
        ext::make_shared<Euribor3M>(eurFwd);

    Handle<Quote> q(ext::make_shared<SimpleQuote>(0.018));
    Period tenor(5, Years);
    Natural fixingDays = 5; 
    Calendar cal = TARGET();
    BusinessDayConvention bdc = Following;
    bool endOfMonth = true;
    Frequency fixedFreq = Annual;
    DayCounter fixedDC = Thirty360(Thirty360::BondBasis);

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    helpers.push_back(ext::make_shared<ConstNotionalCrossCurrencySwapRateHelper>(
        q, tenor, fixingDays, cal, bdc, endOfMonth,
        fixedFreq, fixedDC,
        USDCurrency(), euribor3m, EURCurrency(),
        usdCollat, true));

    typedef PiecewiseYieldCurve<Discount, LogLinear> Curve;
    ext::shared_ptr<YieldTermStructure> curve(
        new Curve(today, helpers, Actual365Fixed()));
    curve->enableExtrapolation();
    Handle<YieldTermStructure> curveHandle(curve);

    Date settlement = cal.advance(today, fixingDays, Days);
    Date maturity   = cal.advance(settlement, tenor, bdc, endOfMonth);

    Schedule fixedSched(settlement, maturity,
        Period(fixedFreq),
        cal, bdc, bdc,
        DateGeneration::Forward, endOfMonth);


    Schedule floatSched(settlement, maturity,
                        euribor3m->tenor(),
                        euribor3m->fixingCalendar(),
                        euribor3m->businessDayConvention(),
                        euribor3m->businessDayConvention(),
                        DateGeneration::Forward, false);

    Leg fixedLeg = FixedRateLeg(fixedSched)
                       .withNotionals(1.0)
                       .withCouponRates(q->value(), fixedDC);

    Leg floatLeg = IborLeg(floatSched, euribor3m)
                       .withNotionals(1.0)
                       .withSpreads(0.0);

    Date initialPaymentDate = CashFlows::startDate(fixedLeg);
    fixedLeg.push_back(ext::make_shared<SimpleCashFlow>(-1.0, initialPaymentDate));
    floatLeg.push_back(ext::make_shared<SimpleCashFlow>(-1.0, initialPaymentDate));

    Date finalPaymentDate = CashFlows::maturityDate(fixedLeg);
    fixedLeg.push_back(ext::make_shared<SimpleCashFlow>(1.0, finalPaymentDate));
    floatLeg.push_back(ext::make_shared<SimpleCashFlow>(1.0, finalPaymentDate));

    ext::shared_ptr<PricingEngine> fixedEngine(
        new DiscountingSwapEngine(usdCollat));
    ext::shared_ptr<PricingEngine> floatEngine(
        new DiscountingSwapEngine(curveHandle));

    Swap fixedProxy(std::vector<Leg>(1, fixedLeg),
                    std::vector<bool>(1, true));
    Swap floatProxy(std::vector<Leg>(1, floatLeg),
                    std::vector<bool>(1, false));

    fixedProxy.setPricingEngine(fixedEngine);
    floatProxy.setPricingEngine(floatEngine);

    Real npv = fixedProxy.NPV() + floatProxy.NPV();
    Real tolerance = 1e-10;

    BOOST_CHECK_SMALL(npv, tolerance);
}




BOOST_AUTO_TEST_CASE(testConstNotionalHelperCollateralOnFloatingLeg) {
    BOOST_TEST_MESSAGE("Testing const-notional CCS helper with collateral on floating leg");

    SavedSettings backup;
    Date today(20, March, 2030);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCollat(
        ext::make_shared<FlatForward>(today, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> eurFwd(
        ext::make_shared<FlatForward>(today, 0.017, Actual365Fixed()));

    ext::shared_ptr<IborIndex> euribor3m =
        ext::make_shared<Euribor3M>(eurFwd);

    Handle<Quote> q(ext::make_shared<SimpleQuote>(0.018));
    Period tenor(5, Years);
    Natural fixingDays = 5;
    Calendar cal = TARGET();
    BusinessDayConvention bdc = Following;
    bool endOfMonth = true;
    Frequency fixedFreq = Annual;
    DayCounter fixedDC = Thirty360(Thirty360::BondBasis);

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    helpers.push_back(ext::make_shared<ConstNotionalCrossCurrencySwapRateHelper>(
        q, tenor, fixingDays, cal, bdc, endOfMonth,
        fixedFreq, fixedDC,
        USDCurrency(), euribor3m, EURCurrency(),
        usdCollat, false));  

    typedef PiecewiseYieldCurve<Discount, LogLinear> Curve;
    ext::shared_ptr<YieldTermStructure> curve(
        new Curve(today, helpers, Actual365Fixed()));
    curve->enableExtrapolation();
    Handle<YieldTermStructure> curveHandle(curve);

    Date settlement = cal.advance(today, fixingDays, Days);
    Date maturity   = cal.advance(settlement, tenor, bdc, endOfMonth);

    Schedule fixedSched(settlement, maturity,
        Period(fixedFreq),
        cal, bdc, bdc,
        DateGeneration::Forward, endOfMonth);

    Schedule floatSched(settlement, maturity,
                        euribor3m->tenor(),
                        euribor3m->fixingCalendar(),
                        euribor3m->businessDayConvention(),
                        euribor3m->businessDayConvention(),
                        DateGeneration::Forward, false);

    Leg fixedLeg = FixedRateLeg(fixedSched)
                       .withNotionals(1.0)
                       .withCouponRates(q->value(), fixedDC);

    Leg floatLeg = IborLeg(floatSched, euribor3m)
                       .withNotionals(1.0)
                       .withSpreads(0.0);

    Date initialPaymentDate = CashFlows::startDate(fixedLeg);
    fixedLeg.push_back(ext::make_shared<SimpleCashFlow>(-1.0, initialPaymentDate));
    floatLeg.push_back(ext::make_shared<SimpleCashFlow>(-1.0, initialPaymentDate));

    Date finalPaymentDate = CashFlows::maturityDate(fixedLeg);
    fixedLeg.push_back(ext::make_shared<SimpleCashFlow>(1.0, finalPaymentDate));
    floatLeg.push_back(ext::make_shared<SimpleCashFlow>(1.0, finalPaymentDate));

    ext::shared_ptr<PricingEngine> fixedEngine(
        new DiscountingSwapEngine(curveHandle)); 
    ext::shared_ptr<PricingEngine> floatEngine(
        new DiscountingSwapEngine(usdCollat)); 

    Swap fixedProxy(std::vector<Leg>(1, fixedLeg),
                    std::vector<bool>(1, true));
    Swap floatProxy(std::vector<Leg>(1, floatLeg),
                    std::vector<bool>(1, false));

    fixedProxy.setPricingEngine(fixedEngine);
    floatProxy.setPricingEngine(floatEngine);

    Real npv = fixedProxy.NPV() + floatProxy.NPV();
    Real tolerance = 1e-10;

    BOOST_CHECK_SMALL(npv, tolerance);
}



BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
