/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2025  Uzair Beg

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


#include <ql/termstructures/yield/crosscurrencyswapratehelper.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/currencies/all.hpp>


using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)
BOOST_AUTO_TEST_SUITE(CrossCurrencySwapRateHelperTests)

// Helpers
namespace {

struct Env {
    Date today;
    Calendar calFixed = TARGET();
    Calendar calFloat = TARGET();
    DayCounter dcFixed = Actual360();
    DayCounter dcFloat = Actual360();
    RelinkableHandle<YieldTermStructure> fixedCurve, floatFwdCurve, floatDiscCurve;

    Env() {
        today = Settings::instance().evaluationDate();
    }

    void linkFlat(Real fixedR, Real floatR) {
        fixedCurve.linkTo(ext::make_shared<FlatForward>(today, fixedR, dcFixed));
        floatFwdCurve.linkTo(ext::make_shared<FlatForward>(today, floatR, dcFloat));
        floatDiscCurve.linkTo(ext::make_shared<FlatForward>(today, floatR, dcFloat));
    }
};

// Build a helper with knobs.
static CrossCurrencySwapRateHelper makeHelper(
    const Handle<Quote>& qFixed,
    const Period& tenor,
    const Calendar& fixedCal,
    Frequency fixedFreq,
    BusinessDayConvention fixedBdc,
    const DayCounter& fixedDc,
    const Currency& fixedCcy,
    const ext::shared_ptr<IborIndex>& floatIndex,
    const Currency& floatCcy,
    Real fx,
    const Handle<YieldTermStructure>& fixedDiscount,
    const Handle<YieldTermStructure>& floatDiscount,
    Natural settlementDays = 2) {

    return CrossCurrencySwapRateHelper(
        qFixed, tenor, fixedCal, fixedFreq, fixedBdc, fixedDc,
        fixedCcy, floatIndex, floatCcy,
        Handle<Quote>(ext::make_shared<SimpleQuote>(fx)),
        fixedDiscount, floatDiscount, settlementDays);
}

static void checkClose(const char* label, Real implied, Real expected, Real tol) {
    if (std::fabs(implied - expected) > tol) {
        BOOST_ERROR(std::string(label)
        << "\n  implied:  " << io::rate(implied)
        << "\n  expected: " << io::rate(expected)
        << "\n  tol:      " << tol);
    
    }
}

} 

// 1) Par-rate sanity under flat identical curves and fx=1
BOOST_AUTO_TEST_CASE(testFlatFxOneParRate) {
    BOOST_TEST_MESSAGE("CrossCurrencySwapRateHelper: flat curves, fx=1 → par ≈ flat");

    Env E; E.linkFlat(0.0200, 0.0200); // 2% both sides
    ext::shared_ptr<IborIndex> euribor6m = ext::make_shared<Euribor6M>(E.floatFwdCurve);

    Handle<Quote> qFixed(ext::make_shared<SimpleQuote>(0.02));

    auto H = makeHelper(qFixed,
                        Period(2, Years),
                        E.calFixed,
                        Semiannual,
                        ModifiedFollowing,
                        E.dcFixed,
                        EURCurrency(),
                        euribor6m,
                        EURCurrency(),   
                        1.0,
                        E.fixedCurve,
                        E.floatDiscCurve,
                        2);

    Real implied = H.impliedQuote();
    checkClose("Flat fx=1", implied, 0.02, 5e-4); 
}

// 2) FX sensitivity direction with float richer than fixed
BOOST_AUTO_TEST_CASE(testFxSensitivityDirection) {
    BOOST_TEST_MESSAGE("CrossCurrencySwapRateHelper: FX↑ should not decrease fair fixed if float side is richer");

    Env E; E.linkFlat(0.0200, 0.0250); // float richer
    ext::shared_ptr<IborIndex> euribor6m = ext::make_shared<Euribor6M>(E.floatFwdCurve);
    Handle<Quote> qFixed(ext::make_shared<SimpleQuote>(0.02));

    auto h1 = makeHelper(qFixed, Period(3, Years), E.calFixed, Semiannual, ModifiedFollowing,
                         E.dcFixed, EURCurrency(), euribor6m, EURCurrency(),
                         1.0, E.fixedCurve, E.floatDiscCurve, 2);
    auto h2 = makeHelper(qFixed, Period(3, Years), E.calFixed, Semiannual, ModifiedFollowing,
                         E.dcFixed, EURCurrency(), euribor6m, EURCurrency(),
                         1.2, E.fixedCurve, E.floatDiscCurve, 2);

    Real r1 = h1.impliedQuote();
    Real r2 = h2.impliedQuote();
    if (r2 + 1e-12 < r1) {
        BOOST_ERROR("FX increased but fair fixed decreased"
                    << "\n  r1(fx=1.0): " << io::rate(r1)
                    << "\n  r2(fx=1.2): " << io::rate(r2));
    }
}

// 3) Tenor / frequency / daycount grid under matched curves
BOOST_AUTO_TEST_CASE(testMiniGridMatchedCurves) {
    BOOST_TEST_MESSAGE("CrossCurrencySwapRateHelper: mini-grid on tenor × freq × dc, matched curves");

    Env E; E.linkFlat(0.0150, 0.0150);
    ext::shared_ptr<IborIndex> euribor6m = ext::make_shared<Euribor6M>(E.floatFwdCurve);
    Handle<Quote> qFixed(ext::make_shared<SimpleQuote>(0.0150));

    const Period tenors[] = { Period(1, Years), Period(2, Years), Period(5, Years) };
    const Frequency freqs[] = { Annual, Semiannual, Quarterly };
    const DayCounter dcs[] = { Actual360(), Actual365Fixed() };

    for (const auto& T : tenors) {
        for (auto F : freqs) {
            for (const auto& DC : dcs) {
                auto H = makeHelper(qFixed, T, E.calFixed, F, ModifiedFollowing,
                                    DC, EURCurrency(), euribor6m, EURCurrency(),
                                    1.0, E.fixedCurve, E.floatDiscCurve, 2);
                Real implied = H.impliedQuote();
                checkClose("Grid test", implied, 0.0150, 5e-4);  // 5bp tolerance

            }
        }
    }
}

// 4) Calendar/BDC variations (TARGET vs UnitedStates, Following vs ModifiedFollowing)
BOOST_AUTO_TEST_CASE(testCalendarAndBdcVariations) {
    BOOST_TEST_MESSAGE("CrossCurrencySwapRateHelper: calendar/BDC variations");

    Env E; E.linkFlat(0.01, 0.01);
    ext::shared_ptr<IborIndex> usdLibor3m = ext::make_shared<USDLibor>(Period(3, Months), E.floatFwdCurve);

    struct S { Calendar cal; BusinessDayConvention bdc; } cases[] = {
        { TARGET(), Following }, { TARGET(), ModifiedFollowing },
        { UnitedStates(UnitedStates::GovernmentBond), Following },
        { UnitedStates(UnitedStates::GovernmentBond), ModifiedFollowing }
    };

    for (const auto& C : cases) {
        auto H = makeHelper(Handle<Quote>(ext::make_shared<SimpleQuote>(0.01)),
                            Period(2, Years),
                            C.cal,
                            Semiannual,
                            C.bdc,
                            Actual360(),
                            USDCurrency(),
                            usdLibor3m,
                            USDCurrency(),
                            1.0,
                            E.fixedCurve, E.floatDiscCurve, 2);
        Real implied = H.impliedQuote();
        checkClose("Calendar/BDC", implied, 0.01, 5e-4);  // 5bp tolerance
    }
}

// 5) Relinking behavior: fixed side uses the curve under construction when fixedDiscount is empty
BOOST_AUTO_TEST_CASE(testRelinkingFixedSide) {
    BOOST_TEST_MESSAGE("CrossCurrencySwapRateHelper: relinking fixed side to bootstrap curve when fixedDiscount empty");

    Env E;
    E.floatFwdCurve.linkTo(ext::make_shared<FlatForward>(E.today, 0.02, E.dcFloat));
    E.floatDiscCurve.linkTo(ext::make_shared<FlatForward>(E.today, 0.02, E.dcFloat));

    ext::shared_ptr<IborIndex> euribor6m = ext::make_shared<Euribor6M>(E.floatFwdCurve);
    Handle<Quote> qFixed(ext::make_shared<SimpleQuote>(0.02));

    CrossCurrencySwapRateHelper H(
        qFixed, Period(2, Years),
        E.calFixed, Semiannual, ModifiedFollowing, E.dcFixed,
        EURCurrency(), euribor6m, EURCurrency(),
        Handle<Quote>(ext::make_shared<SimpleQuote>(1.0)),
        Handle<YieldTermStructure>(), // empty fixed discount
        E.floatDiscCurve,
        2);

    RelinkableHandle<YieldTermStructure> boot;
    boot.linkTo(ext::make_shared<FlatForward>(E.today, 0.021, E.dcFixed)); // 2.1% on fixed side
    H.setTermStructure(boot.currentLink().get());

    Real implied = H.impliedQuote();
    checkClose("Relinking fixed side", implied, 0.02, 5e-4); 
}

// 6) Different float discount vs forward curves
BOOST_AUTO_TEST_CASE(testSeparateFloatDiscountAndForward) {
    BOOST_TEST_MESSAGE("CrossCurrencySwapRateHelper: separate float discount vs forward curves");

    Env E;
    E.fixedCurve.linkTo(ext::make_shared<FlatForward>(E.today, 0.019, E.dcFixed));   
    E.floatFwdCurve.linkTo(ext::make_shared<FlatForward>(E.today, 0.022, E.dcFloat)); 
    E.floatDiscCurve.linkTo(ext::make_shared<FlatForward>(E.today, 0.020, E.dcFloat)); 

    ext::shared_ptr<IborIndex> euribor6m = ext::make_shared<Euribor6M>(E.floatFwdCurve);
    Handle<Quote> qFixed(ext::make_shared<SimpleQuote>(0.02));

    auto H = makeHelper(qFixed, Period(4, Years),
                        E.calFixed, Semiannual, ModifiedFollowing, E.dcFixed,
                        EURCurrency(), euribor6m, EURCurrency(),
                        1.0, E.fixedCurve, E.floatDiscCurve, 2);

    Real r1 = H.impliedQuote();
    auto H2 = makeHelper(qFixed, Period(4, Years),
                         E.calFixed, Semiannual, ModifiedFollowing, E.dcFixed,
                         EURCurrency(), euribor6m, EURCurrency(),
                         1.1, E.fixedCurve, E.floatDiscCurve, 2);
    Real r2 = H2.impliedQuote();
    if (r2 + 1e-12 < r1)
        BOOST_ERROR("FX up but implied fixed down under distinct disc/fwd (unexpected monotonicity)");
}

// 7) Exception: missing float curves entirely
BOOST_AUTO_TEST_CASE(testThrowsWhenFloatCurvesMissing) {
    BOOST_TEST_MESSAGE("CrossCurrencySwapRateHelper: throws if float side has no forwarding/discount");

    Calendar cal = TARGET();
    DayCounter dc = Actual360();
    Handle<Quote> qFixed(ext::make_shared<SimpleQuote>(0.02));
    Handle<Quote> fx(ext::make_shared<SimpleQuote>(1.0));

    ext::shared_ptr<IborIndex> euribor6m = ext::make_shared<Euribor6M>();

    CrossCurrencySwapRateHelper H(
        qFixed, Period(2, Years),
        cal, Semiannual, ModifiedFollowing, dc,
        EURCurrency(), euribor6m, EURCurrency(),
        fx,
        Handle<YieldTermStructure>(ext::make_shared<FlatForward>(Settings::instance().evaluationDate(), 0.02, dc)),
        Handle<YieldTermStructure>() /* float discount empty */,
        2);

    BOOST_CHECK_THROW(H.impliedQuote(), Error);
}

// 8) Settlement days & end-of-month sanity
BOOST_AUTO_TEST_CASE(testSettlementDaysAndDates) {
    BOOST_TEST_MESSAGE("CrossCurrencySwapRateHelper: settlementDays/date generation sanity");

    Env E; E.linkFlat(0.02, 0.02);
    ext::shared_ptr<IborIndex> euribor6m = ext::make_shared<Euribor6M>(E.floatFwdCurve);
    Handle<Quote> qFixed(ext::make_shared<SimpleQuote>(0.02));

    for (Natural sd : { 0u, 2u, 5u }) {
        CrossCurrencySwapRateHelper H(
            qFixed, Period(1, Years),
            TARGET(), 
            Annual, ModifiedFollowing, E.dcFixed,
            EURCurrency(), euribor6m, EURCurrency(),
            Handle<Quote>(ext::make_shared<SimpleQuote>(1.0)),
            E.fixedCurve, E.floatDiscCurve, sd);

        BOOST_CHECK(H.latestDate() > H.earliestDate());
    }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
