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

#include "xccyratehelpers.hpp"
#include "utilities.hpp"
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/termstructures/yield/xccyratehelpers.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace xccyratehelpers_test {

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

        ext::shared_ptr<IborIndex> baseCcyIdx;
        ext::shared_ptr<IborIndex> quoteCcyIdx;

        RelinkableHandle<YieldTermStructure> baseCcyHandle;
        RelinkableHandle<YieldTermStructure> quoteCcyHandle;
        RelinkableHandle<YieldTermStructure> foreignCcyHandle;

        std::vector<Datum> basisData;

        // cleanup
        SavedSettings backup;
        // utilities

        ext::shared_ptr<RateHelper> xccyRateHelper(const Datum &q, 
                                                   const Handle<YieldTermStructure> &collateralHandle,
                                                   bool isFxBaseCurrencyCollateralCurrency,
                                                   bool isBasisOnFxBaseCurrencyLeg) {
            return ext::make_shared<XCCYBasisSwapRateHelper>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(q.basis * basisPoint)),
                Period(q.n, q.units), 
                settlementDays, 
                calendar,
                businessConvention, 
                false, 
                baseCcyIdx, 
                quoteCcyIdx, 
                collateralHandle, 
                isFxBaseCurrencyCollateralCurrency, 
                isBasisOnFxBaseCurrencyLeg);
        }

        std::vector<ext::shared_ptr<Swap> > proxyXccyBasisSwap(const Date& start,
                                                               const Datum &q,
                                                               Real fxSpot,
                                                               bool isFxBaseCurrencyCollateralCurrency, 
                                                               bool isBasisOnFxBaseCurrencyLeg) {
            const Real notional = 1.0;
            std::vector<ext::shared_ptr<Swap> > legs;
            
            ext::shared_ptr<Swap> baseCcyLegProxy = XCCYBasisSwapRateHelper::proxyXCCYLeg(
                start, Period(q.n, q.units), settlementDays, calendar, businessConvention, false,
                baseCcyIdx, VanillaSwap::Receiver, notional * fxSpot, q.basis * basisPoint);
            legs.push_back(baseCcyLegProxy);

            ext::shared_ptr<Swap> quoteCcyLegProxy = XCCYBasisSwapRateHelper::proxyXCCYLeg(
                start, Period(q.n, q.units), settlementDays, calendar, businessConvention, false,
                quoteCcyIdx, VanillaSwap::Payer, notional);
            legs.push_back(quoteCcyLegProxy);

            return legs;
        }

        CommonVars() {
            settlementDays = 2;
            businessConvention = Following;
            calendar = TARGET();
            dayCount = Actual365Fixed();

            baseCcyIdx = ext::shared_ptr<IborIndex>(new Euribor3M(baseCcyHandle));
            quoteCcyIdx = ext::shared_ptr<IborIndex>(new USDLibor(3 * Months, quoteCcyHandle));

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

            today = calendar.adjust(Date(6, December, 2013));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            baseCcyHandle.linkTo(flatRate(settlement, 0.007, dayCount));
            quoteCcyHandle.linkTo(flatRate(settlement, 0.015, dayCount));

            std::vector<ext::shared_ptr<RateHelper> > instruments;
            for (Size i = 0; i < basisData.size(); i++) {
                instruments.push_back(xccyRateHelper(basisData[i], quoteCcyHandle, false, true));
            }

            ext::shared_ptr<YieldTermStructure> foreignCcyCurve(
                new PiecewiseYieldCurve<Discount, LogLinear>(settlement, instruments, dayCount));
            foreignCcyCurve->enableExtrapolation();
            foreignCcyHandle.linkTo(foreignCcyCurve);
        }
    };
}

void XCCYRateHelpersTest::test() {
    BOOST_TEST_MESSAGE("TBD...");

    using namespace xccyratehelpers_test;

    CommonVars vars;

    Handle<YieldTermStructure> collateralHandle = vars.quoteCcyHandle;
    ext::shared_ptr<DiscountingSwapEngine> quoteCcyLegEngine(
        new DiscountingSwapEngine(collateralHandle));
    ext::shared_ptr<DiscountingSwapEngine> baseCcyLegEngine(
        new DiscountingSwapEngine(vars.foreignCcyHandle));

    Real tolerance = 1.0e-15;

    for (Size i = 0; i < vars.basisData.size(); ++i) {
        
        Datum quote = vars.basisData[i];
        std::vector<ext::shared_ptr<Swap> > xccySwapProxy = vars.proxyXccyBasisSwap(
            vars.today, quote, vars.fxSpot, false, true);
        xccySwapProxy[0]->setPricingEngine(baseCcyLegEngine);
        xccySwapProxy[1]->setPricingEngine(quoteCcyLegEngine);
        
        Period p = quote.n * quote.units;

        Real baseCcyLegNpv = vars.fxSpot * xccySwapProxy[0]->NPV();
        Real quoteCcyLegNpv = xccySwapProxy[1]->NPV();
        Real npv = baseCcyLegNpv + quoteCcyLegNpv;

        if (std::fabs(npv) > tolerance)
            BOOST_ERROR("unable to price the cross currency basis swap to par\n"
                        << std::setprecision(5) << "    calculated NPV:    " << npv << "\n"
                        << "    expected:    " << 0.0 << "\n"
                        << "    basis:    " << quote.basis << "\n"
                        << "    tenor:    " << p << "\n");
    }
}

test_suite* XCCYRateHelpersTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("XCCY rate helpers tests");

    suite->add(QUANTLIB_TEST_CASE(&XCCYRateHelpersTest::test));
    return suite;
}