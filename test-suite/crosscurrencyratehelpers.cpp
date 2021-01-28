/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2020 Marcin Rybacki

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
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/crosscurrencyratehelpers.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace crosscurrencyratehelpers_test {

    struct Datum {
        Integer n;
        TimeUnit units;
        Spread basis;
    };

    struct CommonVars {
        const Real basisPoint = 1.0e-4;
        const Real fxSpot = 1.25;

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

        std::vector<Datum> basisData;

        // cleanup
        SavedSettings backup;
        // utilities

        ext::shared_ptr<RateHelper>
        xccyRateHelper(const Datum& q,
                       const Handle<YieldTermStructure>& collateralHandle,
                       bool isFxBaseCurrencyCollateralCurrency,
                       bool isBasisOnFxBaseCurrencyLeg) const {
            return ext::make_shared<CrossCurrencyBasisSwapRateHelper>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(q.basis * basisPoint)),
                Period(q.n, q.units), settlementDays, calendar, businessConvention, endOfMonth,
                baseCcyIdx, quoteCcyIdx, collateralHandle, isFxBaseCurrencyCollateralCurrency,
                isBasisOnFxBaseCurrencyLeg);
        }

        std::vector<ext::shared_ptr<Swap> >
        proxyXccyBasisSwap(const Date& start,
                           const Datum& q,
                           Real fxSpot,
                           bool isFxBaseCurrencyCollateralCurrency,
                           bool isBasisOnFxBaseCurrencyLeg) const {
            const Real baseCcyLegNotional = 1.0;
            Real quoteCcyLegNotional = baseCcyLegNotional * fxSpot;

            Spread baseCcyLegBasis = isBasisOnFxBaseCurrencyLeg ? q.basis * basisPoint : 0.0;
            Spread quoteCcyLegBasis = isBasisOnFxBaseCurrencyLeg ? 0.0 : q.basis * basisPoint;
            
            std::vector<ext::shared_ptr<Swap> > legs;
            ext::shared_ptr<Swap> baseCcyLegProxy =
                CrossCurrencyHelperFunctions::proxyCrossCurrencyLeg(
                    start, Period(q.n, q.units), settlementDays, calendar, businessConvention,
                    endOfMonth, baseCcyIdx, VanillaSwap::Receiver, baseCcyLegNotional,
                    baseCcyLegBasis);
            legs.push_back(baseCcyLegProxy);

            ext::shared_ptr<Swap> quoteCcyLegProxy =
                CrossCurrencyHelperFunctions::proxyCrossCurrencyLeg(
                    start, Period(q.n, q.units), settlementDays, calendar, businessConvention,
                    endOfMonth, quoteCcyIdx, VanillaSwap::Payer, quoteCcyLegNotional,
                    quoteCcyLegBasis);
            legs.push_back(quoteCcyLegProxy);

            return legs;
        }

        Handle<YieldTermStructure>
        buildForeignCcyCurveHandle(const Handle<YieldTermStructure>& collateralHandle,
                                   bool isFxBaseCurrencyCollateralCurrency,
                                   bool isBasisOnFxBaseCurrencyLeg) const {
            std::vector<ext::shared_ptr<RateHelper> > instruments;
            for (Size i = 0; i < basisData.size(); i++) {
                instruments.push_back(xccyRateHelper(basisData[i], collateralHandle,
                                                     isFxBaseCurrencyCollateralCurrency,
                                                     isBasisOnFxBaseCurrencyLeg));
            }

            ext::shared_ptr<YieldTermStructure> foreignCcyCurve(
                new PiecewiseYieldCurve<Discount, LogLinear>(settlement, instruments, dayCount));
            foreignCcyCurve->enableExtrapolation();
            return Handle<YieldTermStructure>(foreignCcyCurve);
        }

        CommonVars() {
            settlementDays = 2;
            businessConvention = Following;
            calendar = TARGET();
            dayCount = Actual365Fixed();
            endOfMonth = false;

            baseCcyIdx = ext::shared_ptr<IborIndex>(new Euribor3M(baseCcyIdxHandle));
            quoteCcyIdx = ext::shared_ptr<IborIndex>(new USDLibor(3 * Months, quoteCcyIdxHandle));

            /* Data source:
               N. Moreni, A. Pallavicini (2015)
               FX Modelling in Collateralized Markets: foreign measures, basis curves
               and pricing formulae.

               section 4.2.1, Table 2.
            */
            basisData.push_back({1, Years, -14.5});
            basisData.push_back({18, Months, -18.5});
            basisData.push_back({2, Years, -20.5});
            basisData.push_back({3, Years, -23.75});
            basisData.push_back({4, Years, -25.5});
            basisData.push_back({5, Years, -26.5});
            basisData.push_back({7, Years, -26.75});
            basisData.push_back({10, Years, -26.25});
            basisData.push_back({15, Years, -24.75});
            basisData.push_back({20, Years, -23.25});
            basisData.push_back({30, Years, -20.50});

            today = calendar.adjust(Date(6, September, 2013));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            baseCcyIdxHandle.linkTo(flatRate(settlement, 0.007, dayCount));
            quoteCcyIdxHandle.linkTo(flatRate(settlement, 0.015, dayCount));
        }
    };
}

void testConstantNotionalCrossCurrencySwaps(bool isFxBaseCurrencyCollateralCurrency,
                                            bool isBasisOnFxBaseCurrencyLeg) {

    using namespace crosscurrencyratehelpers_test;

    CommonVars vars;

    Handle<YieldTermStructure> collateralHandle =
        isFxBaseCurrencyCollateralCurrency ? vars.baseCcyIdxHandle : vars.quoteCcyIdxHandle;

    ext::shared_ptr<DiscountingSwapEngine> collateralCcyLegEngine(
        new DiscountingSwapEngine(collateralHandle));

    Handle<YieldTermStructure> foreignCcyHandle = vars.buildForeignCcyCurveHandle(
        collateralHandle, isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg);
    ext::shared_ptr<DiscountingSwapEngine> foreignCcyLegEngine(
        new DiscountingSwapEngine(foreignCcyHandle));

    Real tolerance = 1.0e-12;

    for (Size i = 0; i < vars.basisData.size(); ++i) {

        Datum quote = vars.basisData[i];
        std::vector<ext::shared_ptr<Swap> > xccySwapProxy =
            vars.proxyXccyBasisSwap(vars.today, quote, vars.fxSpot,
                                    isFxBaseCurrencyCollateralCurrency, isBasisOnFxBaseCurrencyLeg);

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

void CrossCurrencyRateHelpersTest::testBasisSwapsWithCollateralInQuoteAndBasisInBaseCcy() {
    BOOST_TEST_MESSAGE("Testing basis swaps instruments with collateral in quote ccy and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testConstantNotionalCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency,
                                           isBasisOnFxBaseCurrencyLeg);
}

void CrossCurrencyRateHelpersTest::testBasisSwapsWithCollateralInBaseAndBasisInQuoteCcy() {
    BOOST_TEST_MESSAGE(
        "Testing basis swaps instruments with collateral in base ccy and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testConstantNotionalCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency,
                                           isBasisOnFxBaseCurrencyLeg);
}

void CrossCurrencyRateHelpersTest::testBasisSwapsWithCollateralAndBasisInBaseCcy() {
    BOOST_TEST_MESSAGE(
        "Testing basis swaps instruments with collateral and basis in base ccy...");

    bool isFxBaseCurrencyCollateralCurrency = true;
    bool isBasisOnFxBaseCurrencyLeg = true;

    testConstantNotionalCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency,
                                           isBasisOnFxBaseCurrencyLeg);
}

void CrossCurrencyRateHelpersTest::testBasisSwapsWithCollateralAndBasisInQuoteCcy() {
    BOOST_TEST_MESSAGE("Testing basis swaps instruments with collateral and basis in quote ccy...");

    bool isFxBaseCurrencyCollateralCurrency = false;
    bool isBasisOnFxBaseCurrencyLeg = false;

    testConstantNotionalCrossCurrencySwaps(isFxBaseCurrencyCollateralCurrency,
                                           isBasisOnFxBaseCurrencyLeg);
}

test_suite* CrossCurrencyRateHelpersTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cross currency rate helpers tests");

    suite->add(QUANTLIB_TEST_CASE(
        &CrossCurrencyRateHelpersTest::testBasisSwapsWithCollateralInQuoteAndBasisInBaseCcy));
    suite->add(QUANTLIB_TEST_CASE(
        &CrossCurrencyRateHelpersTest::testBasisSwapsWithCollateralInBaseAndBasisInQuoteCcy));
    suite->add(QUANTLIB_TEST_CASE(
        &CrossCurrencyRateHelpersTest::testBasisSwapsWithCollateralAndBasisInBaseCcy));
    suite->add(QUANTLIB_TEST_CASE(
        &CrossCurrencyRateHelpersTest::testBasisSwapsWithCollateralAndBasisInQuoteCcy));
    return suite;
}