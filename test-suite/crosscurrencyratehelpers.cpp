/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2021 Marcin Rybacki

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

#include "crosscurrencyratehelpers.hpp"
#include "utilities.hpp"
#include <ql/experimental/termstructures/crosscurrencyratehelpers.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace crosscurrencyratehelpers_test {

    struct XccyTestDatum {
        Integer n;
        TimeUnit units;
        Spread basis;

        XccyTestDatum(Integer n, TimeUnit units, Spread basis) : n(n), units(units), basis(basis) {}
    };

    struct CommonVars {
        Real basisPoint;
        Real fxSpot;

        Date today, settlement;
        Calendar calendar;
        Natural settlementDays;
        Currency ccy;
        BusinessDayConvention businessConvention;
        DayCounter dayCount;
        bool endOfMonth;

        ext::shared_ptr<IborIndex> baseCcyIdx;
        ext::shared_ptr<IborIndex> quoteCcyIdx;

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
                quoteHandle, tenor, settlementDays, calendar, businessConvention, endOfMonth,
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
                                bool isFxBaseCurrencyLegResettable) const {
            Handle<Quote> quoteHandle(ext::make_shared<SimpleQuote>(q.basis * basisPoint));
            Period tenor(q.n, q.units);
            return ext::shared_ptr<RateHelper>(new MtMCrossCurrencyBasisSwapRateHelper(
                quoteHandle, tenor, settlementDays, calendar, businessConvention, endOfMonth,
                baseCcyIdx, quoteCcyIdx, collateralHandle, isFxBaseCurrencyCollateralCurrency,
                isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable));
        }

        std::vector<ext::shared_ptr<RateHelper> >
        buildResettingXccyRateHelpers(const std::vector<XccyTestDatum>& xccyData,
                                       const Handle<YieldTermStructure>& collateralHandle,
                                       bool isFxBaseCurrencyCollateralCurrency,
                                       bool isBasisOnFxBaseCurrencyLeg,
                                       bool isFxBaseCurrencyLegResettable) const {
            std::vector<ext::shared_ptr<RateHelper> > instruments;
            instruments.reserve(xccyData.size());
            for (const auto& i : xccyData) {
                instruments.push_back(resettingXccyRateHelper(
                    i, collateralHandle, isFxBaseCurrencyCollateralCurrency,
                    isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable));
            }

            return instruments;
        }

        Schedule legSchedule(const Period& tenor, 
                             const ext::shared_ptr<IborIndex>& idx) const {
            return MakeSchedule()
                .from(settlement)
                .to(settlement + tenor)
                .withTenor(idx->tenor())
                .withCalendar(calendar)
                .withConvention(businessConvention)
                .endOfMonth(endOfMonth)
                .backwards();
        }

        Leg constantNotionalLeg(const Schedule& schedule,
                                const ext::shared_ptr<IborIndex>& idx,
                                Real notional,
                                Spread basis) const {
            Leg leg = IborLeg(schedule, idx).withNotionals(notional).withSpreads(basis);
            Date lastPaymentDate = leg.back()->date();
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
            settlementDays = 2;
            businessConvention = Following;
            calendar = TARGET();
            dayCount = Actual365Fixed();
            endOfMonth = false;

            basisPoint = 1.0e-4;
            fxSpot = 1.25;

            baseCcyIdx = ext::shared_ptr<IborIndex>(new Euribor3M(baseCcyIdxHandle));
            quoteCcyIdx = ext::shared_ptr<IborIndex>(new USDLibor(3 * Months, quoteCcyIdxHandle));

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
            settlement = calendar.advance(today, settlementDays, Days);

            baseCcyIdxHandle.linkTo(flatRate(settlement, 0.007, dayCount));
            quoteCcyIdxHandle.linkTo(flatRate(settlement, 0.015, dayCount));
        }
    };
}

void testConstantNotionalCrossCurrencySwapsNPV(bool isFxBaseCurrencyCollateralCurrency,
                                               bool isBasisOnFxBaseCurrencyLeg) {

    using namespace crosscurrencyratehelpers_test;

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
        new PiecewiseYieldCurve<Discount, LogLinear>(vars.settlement, instruments, vars.dayCount));
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
                                     bool isFxBaseCurrencyLegResettable) {

    using namespace crosscurrencyratehelpers_test;

    CommonVars vars;

    Handle<YieldTermStructure> collateralHandle =
        isFxBaseCurrencyCollateralCurrency ? vars.baseCcyIdxHandle : vars.quoteCcyIdxHandle;

    std::vector<ext::shared_ptr<RateHelper> > resettingInstruments =
        vars.buildResettingXccyRateHelpers(
            vars.basisData, collateralHandle, isFxBaseCurrencyCollateralCurrency,
            isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable);
    
    std::vector<ext::shared_ptr<RateHelper> > constNotionalInstruments =
        vars.buildConstantNotionalXccyRateHelpers(vars.basisData, collateralHandle,
                                                  isFxBaseCurrencyCollateralCurrency,
                                                  isBasisOnFxBaseCurrencyLeg);

    ext::shared_ptr<YieldTermStructure> resettingCurve(
        new PiecewiseYieldCurve<Discount, LogLinear>(vars.settlement, resettingInstruments, vars.dayCount));
    resettingCurve->enableExtrapolation();

    ext::shared_ptr<YieldTermStructure> constNotionalCurve(
        new PiecewiseYieldCurve<Discount, LogLinear>(vars.settlement, constNotionalInstruments,
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

void CrossCurrencyRateHelpersTest::
    testConstNotionalBasisSwapsWithCollateralInQuoteAndBasisInBaseCcy() {
    BOOST_TEST_MESSAGE("Testing constant notional basis swaps with collateral in quote ccy and "
                       "basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testConstantNotionalCrossCurrencySwapsNPV(isFxBaseCurrencyCollateralCurrency,
                                              isBasisOnFxBaseCurrencyLeg);
}

void CrossCurrencyRateHelpersTest::testConstNotionalBasisSwapsWithCollateralInBaseAndBasisInQuoteCcy() {
    BOOST_TEST_MESSAGE(
        "Testing constant notional basis swaps with collateral in base ccy and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testConstantNotionalCrossCurrencySwapsNPV(isFxBaseCurrencyCollateralCurrency,
                                              isBasisOnFxBaseCurrencyLeg);
}

void CrossCurrencyRateHelpersTest::testConstNotionalBasisSwapsWithCollateralAndBasisInBaseCcy() {
    BOOST_TEST_MESSAGE(
        "Testing constant notional basis swaps with collateral and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testConstantNotionalCrossCurrencySwapsNPV(isFxBaseCurrencyCollateralCurrency,
                                              isBasisOnFxBaseCurrencyLeg);
}

void CrossCurrencyRateHelpersTest::testConstNotionalBasisSwapsWithCollateralAndBasisInQuoteCcy() {
    BOOST_TEST_MESSAGE("Testing constant notional basis swaps with collateral and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testConstantNotionalCrossCurrencySwapsNPV(isFxBaseCurrencyCollateralCurrency,
                                              isBasisOnFxBaseCurrencyLeg);
}

void CrossCurrencyRateHelpersTest::
    testResettingBasisSwapsWithCollateralInQuoteAndBasisInBaseCcy() {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in quote ccy and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                     isFxBaseCurrencyLegResettable);
}

void CrossCurrencyRateHelpersTest::
    testResettingBasisSwapsWithCollateralInBaseAndBasisInQuoteCcy() {
    BOOST_TEST_MESSAGE(
        "Testing resetting basis swaps with collateral in base ccy and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isFxBaseCurrencyLegResettable = true;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                     isFxBaseCurrencyLegResettable);
}

void CrossCurrencyRateHelpersTest::testResettingBasisSwapsWithCollateralAndBasisInBaseCcy() {
    BOOST_TEST_MESSAGE("Testing resetting basis swaps with collateral and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isFxBaseCurrencyLegResettable = true;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                     isFxBaseCurrencyLegResettable);
}

void CrossCurrencyRateHelpersTest::testResettingBasisSwapsWithCollateralAndBasisInQuoteCcy() {
    BOOST_TEST_MESSAGE("Testing resetting basis swaps with collateral and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isFxBaseCurrencyLegResettable = false;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testResettingCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg,
                                     isFxBaseCurrencyLegResettable);
}

void CrossCurrencyRateHelpersTest::testExceptionWhenInstrumentTenorShorterThanIndexFrequency() {
    BOOST_TEST_MESSAGE(
        "Testing exception when instrument tenor is shorter than index frequency...");

    using namespace crosscurrencyratehelpers_test;

    CommonVars vars;

    std::vector<XccyTestDatum> data{{1, Months, 10.0}};
    Handle<YieldTermStructure> collateralHandle;
    
    BOOST_CHECK_THROW(
        std::vector<ext::shared_ptr<RateHelper> > resettingInstruments =
            vars.buildConstantNotionalXccyRateHelpers(data, collateralHandle, true, true),
        Error);
}

test_suite* CrossCurrencyRateHelpersTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Cross currency rate helpers tests");

    suite->add(
        QUANTLIB_TEST_CASE(&CrossCurrencyRateHelpersTest::
                               testConstNotionalBasisSwapsWithCollateralInQuoteAndBasisInBaseCcy));
    suite->add(
        QUANTLIB_TEST_CASE(&CrossCurrencyRateHelpersTest::
                               testConstNotionalBasisSwapsWithCollateralInBaseAndBasisInQuoteCcy));
    suite->add(QUANTLIB_TEST_CASE(
        &CrossCurrencyRateHelpersTest::testConstNotionalBasisSwapsWithCollateralAndBasisInBaseCcy));
    suite->add(QUANTLIB_TEST_CASE(&CrossCurrencyRateHelpersTest::
                                      testConstNotionalBasisSwapsWithCollateralAndBasisInQuoteCcy));

    suite->add(
        QUANTLIB_TEST_CASE(&CrossCurrencyRateHelpersTest::
                               testResettingBasisSwapsWithCollateralInQuoteAndBasisInBaseCcy));
    suite->add(
        QUANTLIB_TEST_CASE(&CrossCurrencyRateHelpersTest::
                               testResettingBasisSwapsWithCollateralInBaseAndBasisInQuoteCcy));
    suite->add(QUANTLIB_TEST_CASE(
        &CrossCurrencyRateHelpersTest::testResettingBasisSwapsWithCollateralAndBasisInBaseCcy));
    suite->add(QUANTLIB_TEST_CASE(
        &CrossCurrencyRateHelpersTest::testResettingBasisSwapsWithCollateralAndBasisInQuoteCcy));

    suite->add(QUANTLIB_TEST_CASE(
        &CrossCurrencyRateHelpersTest::testExceptionWhenInstrumentTenorShorterThanIndexFrequency));
    return suite;
}
