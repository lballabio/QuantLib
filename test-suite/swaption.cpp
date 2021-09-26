/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2007, 2008 StatPro Italia srl
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

#include "swaption.hpp"
#include "utilities.hpp"
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace swaption_test {

    Period exercises[] = { 1*Years, 2*Years, 3*Years,
                           5*Years, 7*Years, 10*Years };
    Period lengths[] = { 1*Years, 2*Years, 3*Years,
                         5*Years, 7*Years, 10*Years,
                         15*Years, 20*Years };
    Swap::Type type[] = { Swap::Receiver, Swap::Payer };

    struct CommonVars {
        // global data
        Date today, settlement;
        Real nominal;
        Calendar calendar;

        BusinessDayConvention fixedConvention;
        Frequency fixedFrequency;
        DayCounter fixedDayCount;

        BusinessDayConvention floatingConvention;
        Period floatingTenor;
        ext::shared_ptr<IborIndex> index;

        Natural settlementDays;
        RelinkableHandle<YieldTermStructure> termStructure;

        // cleanup
        SavedSettings backup;

        // utilities
        ext::shared_ptr<Swaption> makeSwaption(
            const ext::shared_ptr<VanillaSwap>& swap,
            const Date& exercise,
            Volatility volatility,
            Settlement::Type settlementType = Settlement::Physical,
            Settlement::Method settlementMethod = Settlement::PhysicalOTC,
            BlackSwaptionEngine::CashAnnuityModel model = BlackSwaptionEngine::SwapRate) const {
            Handle<Quote> vol(ext::shared_ptr<Quote>(
                                                new SimpleQuote(volatility)));
            ext::shared_ptr<PricingEngine> engine(new BlackSwaptionEngine(
                termStructure, vol, Actual365Fixed(), 0.0, model));

            ext::shared_ptr<Swaption> result(new
                Swaption(swap,
                         ext::shared_ptr<Exercise>(
                                              new EuropeanExercise(exercise)),
                         settlementType, settlementMethod));
            result->setPricingEngine(engine);
            return result;
        }

        ext::shared_ptr<PricingEngine> makeEngine(
            Volatility volatility,
            BlackSwaptionEngine::CashAnnuityModel model = BlackSwaptionEngine::SwapRate) const {
            Handle<Quote> h(ext::shared_ptr<Quote>(new SimpleQuote(volatility)));
            return ext::shared_ptr<PricingEngine>(
                new BlackSwaptionEngine(termStructure, h, Actual365Fixed(), 0.0, model));
        }

        CommonVars() {
            settlementDays = 2;
            nominal = 1000000.0;
            fixedConvention = Unadjusted;
            fixedFrequency = Annual;
            fixedDayCount = Thirty360(Thirty360::BondBasis);

            index = ext::shared_ptr<IborIndex>(new Euribor6M(termStructure));
            floatingConvention = index->businessDayConvention();
            floatingTenor = index->tenor();
            calendar = index->fixingCalendar();
            today = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today,settlementDays,Days);
            termStructure.linkTo(flatRate(settlement,0.05,Actual365Fixed()));
        }
    };

}


void SwaptionTest::testStrikeDependency() {

    BOOST_TEST_MESSAGE("Testing swaption dependency on strike...");

    using namespace swaption_test;

    CommonVars vars;

    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    for (auto& exercise : exercises) {
        for (auto& length : lengths) {
            for (auto& k : type) {
                Date exerciseDate = vars.calendar.advance(vars.today, exercise);
                Date startDate =
                    vars.calendar.advance(exerciseDate,
                                          vars.settlementDays,Days);
                // store the results for different rates...
                std::vector<Real> values;
                std::vector<Real> values_cash;
                Volatility vol = 0.20;
                for (double strike : strikes) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(length, vars.index, strike)
                            .withEffectiveDate(startDate)
                            .withFixedLegTenor(1 * Years)
                            .withFixedLegDayCount(vars.fixedDayCount)
                            .withFloatingLegSpread(0.0)
                            .withType(k);
                    ext::shared_ptr<Swaption> swaption =
                        vars.makeSwaption(swap,exerciseDate,vol);
                    // FLOATING_POINT_EXCEPTION
                    values.push_back(swaption->NPV());
                    ext::shared_ptr<Swaption> swaption_cash =
                        vars.makeSwaption(swap,exerciseDate,vol,
                                          Settlement::Cash, Settlement::ParYieldCurve);
                    values_cash.push_back(swaption_cash->NPV());
                }
                // and check that they go the right way
                if (k == Swap::Payer) {
                    auto it = std::adjacent_find(values.begin(), values.end(), std::less<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR("NPV of Payer swaption with delivery settlement"
                                    "is increasing with the strike:"
                                    << "\noption tenor: " << exercise << "\noption date:  "
                                    << exerciseDate << "\nvolatility:   " << io::rate(vol)
                                    << "\nswap tenor:   " << length << "\nvalue:        "
                                    << values[n] << " at strike: " << io::rate(strikes[n])
                                    << "\nvalue:        " << values[n + 1]
                                    << " at strike: " << io::rate(strikes[n + 1]));
                    }
                    auto it_cash = std::adjacent_find(values_cash.begin(), values_cash.end(),
                                                      std::less<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR("NPV of Payer swaption with cash settlement"
                                    "is increasing with the strike:"
                                    << "\noption tenor: " << exercise << "\noption date:  "
                                    << exerciseDate << "\nvolatility:   " << io::rate(vol)
                                    << "\nswap tenor:   " << length << "\nvalue:        "
                                    << values_cash[n] << " at strike: " << io::rate(strikes[n])
                                    << "\nvalue:        " << values_cash[n + 1]
                                    << " at strike: " << io::rate(strikes[n + 1]));
                    }
                } else {
                    auto it =
                        std::adjacent_find(values.begin(), values.end(), std::greater<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR("NPV of Receiver swaption with delivery settlement"
                                    "is increasing with the strike:"
                                    << "\noption tenor: " << exercise << "\noption date:  "
                                    << exerciseDate << "\nvolatility:   " << io::rate(vol)
                                    << "\nswap tenor:   " << length << "\nvalue:        "
                                    << values[n] << " at strike: " << io::rate(strikes[n])
                                    << "\nvalue:        " << values[n + 1]
                                    << " at strike: " << io::rate(strikes[n + 1]));
                    }
                    auto it_cash = std::adjacent_find(values_cash.begin(), values_cash.end(),
                                                      std::greater<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR("NPV of Receiver swaption with cash settlement"
                                    "is increasing with the strike:"
                                    << "\noption tenor: " << exercise << "\noption date:  "
                                    << exerciseDate << "\nvolatility:   " << io::rate(vol)
                                    << "\nswap tenor:   " << length << "\nvalue:        "
                                    << values_cash[n] << " at strike: " << io::rate(strikes[n])
                                    << "\nvalue:        " << values_cash[n + 1]
                                    << " at strike: " << io::rate(strikes[n + 1]));
                    }
                }
            }
        }
    }
}

void SwaptionTest::testSpreadDependency() {

    BOOST_TEST_MESSAGE("Testing swaption dependency on spread...");

    using namespace swaption_test;

    CommonVars vars;

    Spread spreads[] = { -0.002, -0.001, 0.0, 0.001, 0.002 };

    for (auto exercise : exercises) {
        for (auto& length : lengths) {
            for (auto& k : type) {
                Date exerciseDate = vars.calendar.advance(vars.today, exercise);
                Date startDate =
                    vars.calendar.advance(exerciseDate,
                                          vars.settlementDays,Days);
                // store the results for different rates...
                std::vector<Real> values;
                std::vector<Real> values_cash;
                for (double spread : spreads) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(length, vars.index, 0.06)
                            .withFixedLegTenor(1 * Years)
                            .withFixedLegDayCount(vars.fixedDayCount)
                            .withEffectiveDate(startDate)
                            .withFloatingLegSpread(spread)
                            .withType(k);
                    ext::shared_ptr<Swaption> swaption =
                        vars.makeSwaption(swap,exerciseDate,0.20);
                    // FLOATING_POINT_EXCEPTION
                    values.push_back(swaption->NPV());
                    ext::shared_ptr<Swaption> swaption_cash =
                        vars.makeSwaption(swap,exerciseDate,0.20,
                                          Settlement::Cash, Settlement::ParYieldCurve);
                    values_cash.push_back(swaption_cash->NPV());
                }
                // and check that they go the right way
                if (k == Swap::Payer) {
                    auto it =
                        std::adjacent_find(values.begin(), values.end(), std::greater<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR("NPV is decreasing with the spread "
                                    << "in a payer swaption (physical delivered):"
                                    << "\nexercise date: " << exerciseDate << "\nlength:        "
                                    << length << "\nvalue:         " << values[n] << " for spread: "
                                    << io::rate(spreads[n]) << "\nvalue:         " << values[n + 1]
                                    << " for spread: " << io::rate(spreads[n + 1]));
                    }
                    auto it_cash = std::adjacent_find(values_cash.begin(), values_cash.end(),
                                                      std::greater<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR("NPV is decreasing with the spread "
                                    << "in a payer swaption (cash delivered):"
                                    << "\nexercise date: " << exerciseDate << "\nlength: " << length
                                    << "\nvalue:  " << values_cash[n] << " for spread: "
                                    << io::rate(spreads[n]) << "\nvalue:  " << values_cash[n + 1]
                                    << " for spread: " << io::rate(spreads[n + 1]));
                    }
                } else {
                    auto it = std::adjacent_find(values.begin(), values.end(), std::less<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR("NPV is increasing with the spread "
                                    << "in a receiver swaption (physical delivered):"
                                       "\nexercise date: "
                                    << exerciseDate << "\nlength: " << length << "\nvalue:  "
                                    << values[n] << " for spread: " << io::rate(spreads[n])
                                    << "\nvalue:  " << values[n + 1]
                                    << " for spread: " << io::rate(spreads[n + 1]));
                    }
                    auto it_cash = std::adjacent_find(values_cash.begin(), values_cash.end(),
                                                      std::less<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR("NPV is increasing with the spread "
                                    << "in a receiver swaption (cash delivered):"
                                       "\nexercise date: "
                                    << exerciseDate << "\nlength: " << length << "\nvalue:  "
                                    << values_cash[n] << " for spread: " << io::rate(spreads[n])
                                    << "\nvalue:  " << values_cash[n + 1]
                                    << " for spread: " << io::rate(spreads[n + 1]));
                    }
                }
            }
        }
    }
}

void SwaptionTest::testSpreadTreatment() {

    BOOST_TEST_MESSAGE("Testing swaption treatment of spread...");

    using namespace swaption_test;

    CommonVars vars;

    Spread spreads[] = { -0.002, -0.001, 0.0, 0.001, 0.002 };

    for (auto exercise : exercises) {
        for (auto& length : lengths) {
            for (auto& k : type) {
                Date exerciseDate = vars.calendar.advance(vars.today, exercise);
                Date startDate =
                    vars.calendar.advance(exerciseDate,
                                          vars.settlementDays,Days);
                for (double spread : spreads) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(length, vars.index, 0.06)
                            .withFixedLegTenor(1 * Years)
                            .withFixedLegDayCount(vars.fixedDayCount)
                            .withEffectiveDate(startDate)
                            .withFloatingLegSpread(spread)
                            .withType(k);
                    // FLOATING_POINT_EXCEPTION
                    Spread correction = spread * swap->floatingLegBPS() / swap->fixedLegBPS();
                    ext::shared_ptr<VanillaSwap> equivalentSwap =
                        MakeVanillaSwap(length, vars.index, 0.06 + correction)
                            .withFixedLegTenor(1 * Years)
                            .withFixedLegDayCount(vars.fixedDayCount)
                            .withEffectiveDate(startDate)
                            .withFloatingLegSpread(0.0)
                            .withType(k);
                    ext::shared_ptr<Swaption> swaption1 =
                        vars.makeSwaption(swap,exerciseDate,0.20);
                    ext::shared_ptr<Swaption> swaption2 =
                        vars.makeSwaption(equivalentSwap,exerciseDate,0.20);
                    ext::shared_ptr<Swaption> swaption1_cash =
                        vars.makeSwaption(swap,exerciseDate,0.20,
                                          Settlement::Cash, Settlement::ParYieldCurve);
                    ext::shared_ptr<Swaption> swaption2_cash =
                        vars.makeSwaption(equivalentSwap,exerciseDate,0.20,
                                          Settlement::Cash, Settlement::ParYieldCurve);
                    if (std::fabs(swaption1->NPV()-swaption2->NPV()) > 1.0e-6)
                        BOOST_ERROR("wrong spread treatment:"
                                    << "\nexercise: " << exerciseDate << "\nlength:   " << length
                                    << "\ntype      " << k << "\nspread:   " << io::rate(spread)
                                    << "\noriginal swaption value:   " << swaption1->NPV()
                                    << "\nequivalent swaption value: " << swaption2->NPV());

                    if (std::fabs(swaption1_cash->NPV()-swaption2_cash->NPV()) > 1.0e-6)
                        BOOST_ERROR("wrong spread treatment:"
                                    << "\nexercise date: " << exerciseDate << "\nlength: " << length
                                    << "\npay " << (k ? "fixed" : "floating")
                                    << "\nspread: " << io::rate(spread)
                                    << "\nvalue of original swaption:   " << swaption1_cash->NPV()
                                    << "\nvalue of equivalent swaption: " << swaption2_cash->NPV());
                }
            }
        }
    }
}

void SwaptionTest::testCachedValue() {

    BOOST_TEST_MESSAGE("Testing swaption value against cached value...");

    using namespace swaption_test;

    CommonVars vars;

    vars.today = Date(13, March, 2002);
    vars.settlement = Date(15, March, 2002);
    Settings::instance().evaluationDate() = vars.today;
    vars.termStructure.linkTo(flatRate(vars.settlement, 0.05, Actual365Fixed()));
    Date exerciseDate = vars.calendar.advance(vars.settlement, 5*Years);
    Date startDate = vars.calendar.advance(exerciseDate,
                                           vars.settlementDays, Days);
    ext::shared_ptr<VanillaSwap> swap =
        MakeVanillaSwap(10*Years, vars.index, 0.06)
        .withEffectiveDate(startDate)
        .withFixedLegTenor(1*Years)
        .withFixedLegDayCount(vars.fixedDayCount);

    ext::shared_ptr<Swaption> swaption =
        vars.makeSwaption(swap, exerciseDate, 0.20);

    Real cachedNPV =
#ifdef QL_USE_INDEXED_COUPON
        0.036421429684;
#else
        0.036418158579;
#endif

    // FLOATING_POINT_EXCEPTION
    if (std::fabs(swaption->NPV()-cachedNPV) > 1.0e-12)
        BOOST_ERROR("failed to reproduce cached swaption value:\n" <<
                    std::fixed << std::setprecision(12) <<
                    "\ncalculated: " << swaption->NPV() <<
                    "\nexpected:   " << cachedNPV);
}

void SwaptionTest::testVega() {

    BOOST_TEST_MESSAGE("Testing swaption vega...");

    using namespace swaption_test;

    CommonVars vars;

    Settlement::Type types[] = { Settlement::Physical, Settlement::Cash };
    Settlement::Method methods[] = { Settlement::PhysicalOTC, Settlement::ParYieldCurve };
    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.20, 0.30, 0.70, 0.90 };
    Volatility shift = 1e-8;
    for (auto& exercise : exercises) {
        Date exerciseDate = vars.calendar.advance(vars.today, exercise);
        Date startDate = vars.calendar.advance(exerciseDate,
                                           vars.settlementDays*Days);
        for (auto& length : lengths) {
            for (double strike : strikes) {
                for (Size h=0; h<LENGTH(type); h++) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(length, vars.index, strike)
                            .withEffectiveDate(startDate)
                            .withFixedLegTenor(1 * Years)
                            .withFixedLegDayCount(vars.fixedDayCount)
                            .withFloatingLegSpread(0.0)
                            .withType(type[h]);
                    for (double vol : vols) {
                        ext::shared_ptr<Swaption> swaption =
                            vars.makeSwaption(swap, exerciseDate, vol, types[h], methods[h]);
                        // FLOATING_POINT_EXCEPTION
                        ext::shared_ptr<Swaption> swaption1 = vars.makeSwaption(
                            swap, exerciseDate, vol - shift, types[h], methods[h]);
                        ext::shared_ptr<Swaption> swaption2 = vars.makeSwaption(
                            swap, exerciseDate, vol + shift, types[h], methods[h]);

                        Real swaptionNPV = swaption->NPV();
                        Real numericalVegaPerPoint =
                            (swaption2->NPV()-swaption1->NPV())/(200.0*shift);
                        // check only relevant vega
                        if (numericalVegaPerPoint/swaptionNPV>1.0e-7) {
                            Real analyticalVegaPerPoint =
                                swaption->result<Real>("vega")/100.0;
                            Real discrepancy = std::fabs(analyticalVegaPerPoint
                                - numericalVegaPerPoint);
                            discrepancy /= numericalVegaPerPoint;
                            Real tolerance = 0.015;
                            if (discrepancy > tolerance)
                                BOOST_FAIL("failed to compute swaption vega:"
                                           << "\n  option tenor:    " << exercise
                                           << "\n  volatility:      " << io::rate(vol)
                                           << "\n  option type:     " << swaption->type()
                                           << "\n  swap tenor:      " << length
                                           << "\n  strike:          " << io::rate(strike)
                                           << "\n  settlement:      " << types[h]
                                           << "\n  nominal:         "
                                           << swaption->underlyingSwap()->nominal()
                                           << "\n  npv:             " << swaptionNPV
                                           << "\n  calculated vega: " << analyticalVegaPerPoint
                                           << "\n  expected vega:   " << numericalVegaPerPoint
                                           << "\n  discrepancy:     " << io::rate(discrepancy)
                                           << "\n  tolerance:       " << io::rate(tolerance));
                        }
                    }
                }
            }
        }
    }
}



void SwaptionTest::testCashSettledSwaptions() {

    BOOST_TEST_MESSAGE("Testing cash settled swaptions modified annuity...");

    using namespace swaption_test;

    CommonVars vars;

    Rate strike = 0.05;

    for (auto exercise : exercises) {
        for (auto length : lengths) {

            Date exerciseDate = vars.calendar.advance(vars.today, exercise);
            Date startDate = vars.calendar.advance(exerciseDate,
                                                   vars.settlementDays,Days);
            Date maturity = vars.calendar.advance(startDate, length, vars.floatingConvention);
            Schedule floatSchedule(startDate, maturity, vars.floatingTenor,
                                   vars.calendar,vars.floatingConvention,
                                   vars.floatingConvention,
                                   DateGeneration::Forward, false);
            // Swap with fixed leg conventions: Business Days = Unadjusted, DayCount = 30/360
            Schedule fixedSchedule_u(startDate, maturity,
                                     Period(vars.fixedFrequency),
                                     vars.calendar, Unadjusted, Unadjusted,
                                     DateGeneration::Forward, true);
            ext::shared_ptr<VanillaSwap> swap_u360(
                new VanillaSwap(type[0], vars.nominal,
                                fixedSchedule_u,strike,Thirty360(Thirty360::BondBasis),
                                floatSchedule,vars.index,0.0,
                                vars.index->dayCounter()));

            // Swap with fixed leg conventions: Business Days = Unadjusted, DayCount = Act/365
            ext::shared_ptr<VanillaSwap> swap_u365(
                new VanillaSwap(type[0],vars.nominal,
                                fixedSchedule_u,strike,Actual365Fixed(),
                                floatSchedule,vars.index,0.0,
                                vars.index->dayCounter()));

            // Swap with fixed leg conventions: Business Days = Modified Following, DayCount = 30/360
            Schedule fixedSchedule_a(startDate,maturity,
                                     Period(vars.fixedFrequency),
                                     vars.calendar,ModifiedFollowing,
                                     ModifiedFollowing,
                                     DateGeneration::Forward, true);
            ext::shared_ptr<VanillaSwap> swap_a360(
                new VanillaSwap(type[0],vars.nominal,
                                fixedSchedule_a,strike,Thirty360(Thirty360::BondBasis),
                                floatSchedule,vars.index,0.0,
                                vars.index->dayCounter()));

            // Swap with fixed leg conventions: Business Days = Modified Following, DayCount = Act/365
            ext::shared_ptr<VanillaSwap> swap_a365(
                new VanillaSwap(type[0],vars.nominal,
                                fixedSchedule_a,strike,Actual365Fixed(),
                                floatSchedule,vars.index,0.0,
                                vars.index->dayCounter()));

            ext::shared_ptr<PricingEngine> swapEngine(
                               new DiscountingSwapEngine(vars.termStructure));

            swap_u360->setPricingEngine(swapEngine);
            swap_a360->setPricingEngine(swapEngine);
            swap_u365->setPricingEngine(swapEngine);
            swap_a365->setPricingEngine(swapEngine);

            const Leg& swapFixedLeg_u360 = swap_u360->fixedLeg();
            const Leg& swapFixedLeg_a360 = swap_a360->fixedLeg();
            const Leg& swapFixedLeg_u365 = swap_u365->fixedLeg();
            const Leg& swapFixedLeg_a365 = swap_a365->fixedLeg();

            // FlatForward curves
            // FLOATING_POINT_EXCEPTION
            Handle<YieldTermStructure> termStructure_u360(
                ext::shared_ptr<YieldTermStructure>(
                    new FlatForward(vars.settlement,swap_u360->fairRate(),
                                    Thirty360(Thirty360::BondBasis),Compounded,
                                    vars.fixedFrequency)));
            Handle<YieldTermStructure> termStructure_a360(
                ext::shared_ptr<YieldTermStructure>(
                    new FlatForward(vars.settlement,swap_a360->fairRate(),
                                    Thirty360(Thirty360::BondBasis),Compounded,
                                    vars.fixedFrequency)));
            Handle<YieldTermStructure> termStructure_u365(
                ext::shared_ptr<YieldTermStructure>(
                    new FlatForward(vars.settlement,swap_u365->fairRate(),
                                    Actual365Fixed(),Compounded,
                                    vars.fixedFrequency)));
            Handle<YieldTermStructure> termStructure_a365(
                ext::shared_ptr<YieldTermStructure>(
                    new FlatForward(vars.settlement,swap_a365->fairRate(),
                                    Actual365Fixed(),Compounded,
                                    vars.fixedFrequency)));

            // Annuity calculated by swap method fixedLegBPS().
            // Fixed leg conventions: Unadjusted, 30/360
            Real annuity_u360 = swap_u360->fixedLegBPS() / 0.0001;
            annuity_u360 = swap_u360->type()==Swap::Payer ?
                -annuity_u360 : annuity_u360;
            // Fixed leg conventions: ModifiedFollowing, act/365
            Real annuity_a365 = swap_a365->fixedLegBPS() / 0.0001;
            annuity_a365 = swap_a365->type()==Swap::Payer ?
                -annuity_a365 : annuity_a365;
            // Fixed leg conventions: ModifiedFollowing, 30/360
            Real annuity_a360 = swap_a360->fixedLegBPS() / 0.0001;
            annuity_a360 = swap_a360->type()==Swap::Payer ?
                -annuity_a360 : annuity_a360;
            // Fixed leg conventions: Unadjusted, act/365
            Real annuity_u365 = swap_u365->fixedLegBPS() / 0.0001;
            annuity_u365 = swap_u365->type()==Swap::Payer ?
                -annuity_u365 : annuity_u365;

            // Calculation of Modified Annuity (cash settlement)
            // Fixed leg conventions of swap: unadjusted, 30/360
            Real cashannuity_u360 = 0.;
            Size i;
            for (i=0; i<swapFixedLeg_u360.size(); i++) {
                cashannuity_u360 += swapFixedLeg_u360[i]->amount()/strike
                                  * termStructure_u360->discount(
                                    swapFixedLeg_u360[i]->date());
            }
            // Fixed leg conventions of swap: unadjusted, act/365
            Real cashannuity_u365 = 0.;
            for (i=0; i<swapFixedLeg_u365.size(); i++) {
                cashannuity_u365 += swapFixedLeg_u365[i]->amount()/strike
                                  * termStructure_u365->discount(
                                    swapFixedLeg_u365[i]->date());
            }
            // Fixed leg conventions of swap: modified following, 30/360
            Real cashannuity_a360 = 0.;
            for (i=0; i<swapFixedLeg_a360.size(); i++) {
                cashannuity_a360 += swapFixedLeg_a360[i]->amount()/strike
                                  * termStructure_a360->discount(
                                    swapFixedLeg_a360[i]->date());
            }
            // Fixed leg conventions of swap: modified following, act/365
            Real cashannuity_a365 = 0.;
            for (i=0; i<swapFixedLeg_a365.size(); i++) {
                cashannuity_a365 += swapFixedLeg_a365[i]->amount()/strike
                                  * termStructure_a365->discount(
                                    swapFixedLeg_a365[i]->date());
            }

            // Swaptions: underlying swap fixed leg conventions:
            // unadjusted, 30/360

            // Physical settled swaption
            ext::shared_ptr<Swaption> swaption_p_u360 =
                vars.makeSwaption(swap_u360,exerciseDate,0.20);
            Real value_p_u360 = swaption_p_u360->NPV();
            // Cash settled swaption
            ext::shared_ptr<Swaption> swaption_c_u360 =
                vars.makeSwaption(swap_u360,exerciseDate,0.20,
                                  Settlement::Cash, Settlement::ParYieldCurve);
            Real value_c_u360 = swaption_c_u360->NPV();
            // the NPV's ratio must be equal to annuities ratio
            Real npv_ratio_u360 = value_c_u360 / value_p_u360;
            Real annuity_ratio_u360 = cashannuity_u360 / annuity_u360;

            // Swaptions: underlying swap fixed leg conventions:
            // modified following, act/365

            // Physical settled swaption
            ext::shared_ptr<Swaption> swaption_p_a365 =
                vars.makeSwaption(swap_a365,exerciseDate,0.20);
            Real value_p_a365 = swaption_p_a365->NPV();
            // Cash settled swaption
            ext::shared_ptr<Swaption> swaption_c_a365 =
                vars.makeSwaption(swap_a365,exerciseDate,0.20,
                                  Settlement::Cash, Settlement::ParYieldCurve);
            Real value_c_a365 = swaption_c_a365->NPV();
            // the NPV's ratio must be equal to annuities ratio
            Real npv_ratio_a365 = value_c_a365 / value_p_a365;
            Real annuity_ratio_a365 =  cashannuity_a365 / annuity_a365;

            // Swaptions: underlying swap fixed leg conventions:
            // modified following, 30/360

            // Physical settled swaption
            ext::shared_ptr<Swaption> swaption_p_a360 =
                vars.makeSwaption(swap_a360,exerciseDate,0.20);
            Real value_p_a360 = swaption_p_a360->NPV();
            // Cash settled swaption
            ext::shared_ptr<Swaption> swaption_c_a360 =
                vars.makeSwaption(swap_a360,exerciseDate,0.20,
                                  Settlement::Cash, Settlement::ParYieldCurve);
            Real value_c_a360 = swaption_c_a360->NPV();
            // the NPV's ratio must be equal to annuities ratio
            Real npv_ratio_a360 = value_c_a360 / value_p_a360;
            Real annuity_ratio_a360 =  cashannuity_a360 / annuity_a360;

            // Swaptions: underlying swap fixed leg conventions:
            // unadjusted, act/365

            // Physical settled swaption
            ext::shared_ptr<Swaption> swaption_p_u365 =
                vars.makeSwaption(swap_u365,exerciseDate,0.20);
            Real value_p_u365 = swaption_p_u365->NPV();
            // Cash settled swaption
            ext::shared_ptr<Swaption> swaption_c_u365 =
                vars.makeSwaption(swap_u365,exerciseDate,0.20,
                                  Settlement::Cash, Settlement::ParYieldCurve);
            Real value_c_u365 = swaption_c_u365->NPV();
            // the NPV's ratio must be equal to annuities ratio
            Real npv_ratio_u365 = value_c_u365 / value_p_u365;
            Real annuity_ratio_u365 =  cashannuity_u365 / annuity_u365;

            if (std::fabs(annuity_ratio_u360-npv_ratio_u360)>1e-10 ) {
                BOOST_ERROR("\n"
                            << "    The npv's ratio must be equal to "
                            << " annuities ratio"
                            << "\n"
                               "    Swaption "
                            << exercises[i].units() << "y x " << length.units() << "y"
                            << " (underlying swap fixed leg Unadjusted, 30/360)"
                            << "\n"
                            << "    Today           : " << vars.today << "\n"
                            << "    Settlement date : " << vars.settlement << "\n"
                            << "    Exercise date   : " << exerciseDate << "\n"
                            << "    Swap start date : " << startDate << "\n"
                            << "    Swap end date   : " << maturity << "\n"
                            << "    physical delivered swaption npv : " << value_p_u360 << "\t\t\t"
                            << "    annuity : " << annuity_u360 << "\n"
                            << "    cash delivered swaption npv :     " << value_c_u360 << "\t\t\t"
                            << "    annuity : " << cashannuity_u360 << "\n"
                            << "    npv ratio : " << npv_ratio_u360 << "\n"
                            << "    annuity ratio : " << annuity_ratio_u360 << "\n"
                            << "    difference : " << (annuity_ratio_u360 - npv_ratio_u360));
            }
            if (std::fabs(annuity_ratio_a365-npv_ratio_a365)>1e-10) {
                BOOST_ERROR("\n"
                            << "    The npv's ratio must be equal to "
                            << " annuities ratio"
                            << "\n"
                               "    Swaption "
                            << exercises[i].units() << "y x " << length.units() << "y"
                            << " (underlying swap fixed leg Modified Following, act/365"
                            << "\n"
                            << "    Today           : " << vars.today << "\n"
                            << "    Settlement date : " << vars.settlement << "\n"
                            << "    Exercise date   : " << exerciseDate << "\n"
                            << "    Swap start date : " << startDate << "\n"
                            << "    Swap end date   : " << maturity << "\n"
                            << "    physical delivered swaption npv : " << value_p_a365 << "\t\t\t"
                            << "    annuity : " << annuity_a365 << "\n"
                            << "    cash delivered swaption npv :     " << value_c_a365 << "\t\t\t"
                            << "    annuity : " << cashannuity_a365 << "\n"
                            << "    npv ratio : " << npv_ratio_a365 << "\n"
                            << "    annuity ratio : " << annuity_ratio_a365 << "\n"
                            << "    difference : " << (annuity_ratio_a365 - npv_ratio_a365));
                }
            if (std::fabs(annuity_ratio_a360-npv_ratio_a360)>1e-10) {
                BOOST_ERROR("\n"
                            << "    The npv's ratio must be equal to "
                            << " annuities ratio"
                            << "\n"
                               "    Swaption "
                            << exercises[i].units() << "y x " << length.units() << "y"
                            << " (underlying swap fixed leg Unadjusted, 30/360)"
                            << "\n"
                            << "    Today           : " << vars.today << "\n"
                            << "    Settlement date : " << vars.settlement << "\n"
                            << "    Exercise date   : " << exerciseDate << "\n"
                            << "    Swap start date : " << startDate << "\n"
                            << "    Swap end date   : " << maturity << "\n"
                            << "    physical delivered swaption npv : " << value_p_a360 << "\t\t\t"
                            << "    annuity : " << annuity_a360 << "\n"
                            << "    cash delivered swaption npv :     " << value_c_a360 << "\t\t\t"
                            << "    annuity : " << cashannuity_a360 << "\n"
                            << "    npv ratio : " << npv_ratio_a360 << "\n"
                            << "    annuity ratio : " << annuity_ratio_a360 << "\n"
                            << "    difference : " << (annuity_ratio_a360 - npv_ratio_a360));
            }
            if (std::fabs(annuity_ratio_u365-npv_ratio_u365)>1e-10) {
                BOOST_ERROR("\n"
                            << "    The npv's ratio must be equal to "
                            << " annuities ratio"
                            << "\n"
                               "    Swaption "
                            << exercises[i].units() << "y x " << length.units() << "y"
                            << " (underlying swap fixed leg Unadjusted, act/365)"
                            << "\n"
                            << "    Today           : " << vars.today << "\n"
                            << "    Settlement date : " << vars.settlement << "\n"
                            << "    Exercise date   : " << exerciseDate << "\n"
                            << "    Swap start date : " << startDate << "\n"
                            << "    Swap end date   : " << maturity << "\n"
                            << "    physical delivered swaption npv : " << value_p_u365 << "\t\t\t"
                            << "    annuity : " << annuity_u365 << "\n"
                            << "    cash delivered swaption npv :     " << value_c_u365 << "\t\t\t"
                            << "    annuity : " << cashannuity_u365 << "\n"
                            << "    npv ratio : " << npv_ratio_u365 << "\n"
                            << "    annuity ratio : " << annuity_ratio_u365 << "\n"
                            << "    difference : " << (annuity_ratio_u365 - npv_ratio_u365));
            }
        }
    }
}



void SwaptionTest::testImpliedVolatility() {

    BOOST_TEST_MESSAGE("Testing implied volatility for swaptions...");

    using namespace swaption_test;

    CommonVars vars;

    Size maxEvaluations = 100;
    Real tolerance = 1.0e-08;

    Settlement::Type types[] = { Settlement::Physical, Settlement::Cash };
    Settlement::Method methods[] = { Settlement::PhysicalOTC, Settlement::ParYieldCurve };
    // test data
    Rate strikes[] = { 0.02, 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.20, 0.30, 0.70, 0.90 };

    for (auto& exercise : exercises) {
        for (auto& length : lengths) {
            Date exerciseDate = vars.calendar.advance(vars.today, exercise);
            Date startDate = vars.calendar.advance(exerciseDate,
                                                   vars.settlementDays, Days);

            for (double& strike : strikes) {
                for (auto& k : type) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(length, vars.index, strike)
                            .withEffectiveDate(startDate)
                            .withFixedLegTenor(1 * Years)
                            .withFixedLegDayCount(vars.fixedDayCount)
                            .withFloatingLegSpread(0.0)
                            .withType(k);
                    for (Size h=0; h<LENGTH(types); h++) {
                        for (double vol : vols) {
                            ext::shared_ptr<Swaption> swaption =
                                vars.makeSwaption(swap, exerciseDate, vol, types[h], methods[h],
                                                  BlackSwaptionEngine::DiscountCurve);
                            // Black price
                            Real value = swaption->NPV();
                            Volatility implVol = 0.0;
                            try {
                                implVol =
                                  swaption->impliedVolatility(value,
                                                              vars.termStructure,
                                                              0.10,
                                                              tolerance,
                                                              maxEvaluations,
                                                              1.0e-7,
                                                              4.0,
                                                              ShiftedLognormal,
                                                              0.0);
                            } catch (std::exception& e) {
                                // couldn't bracket?
                                swaption->setPricingEngine(vars.makeEngine(0.0, BlackSwaptionEngine::DiscountCurve));
                                Real value2 = swaption->NPV();
                                if (std::fabs(value-value2) < tolerance) {
                                    // ok, just skip:
                                    continue;
                                }
                                // otherwise, report error
                                BOOST_ERROR("implied vol failure: "
                                            << exercise << "x" << length << " " << k
                                            << "\nsettlement: " << types[h] << "\nstrike      "
                                            << strike
                                            << "\natm level:  " << io::rate(swap->fairRate())
                                            << "\nvol:        " << io::volatility(vol)
                                            << "\nprice:      " << value << "\n"
                                            << e.what());
                            }
                            if (std::fabs(implVol - vol) > tolerance) {
                                // the difference might not matter
                                swaption->setPricingEngine(vars.makeEngine(implVol, BlackSwaptionEngine::DiscountCurve));
                                Real value2 = swaption->NPV();
                                if (std::fabs(value-value2) > tolerance) {
                                    BOOST_ERROR("implied vol failure: "
                                                << exercise << "x" << length << " " << k
                                                << "\nsettlement:    " << types[h]
                                                << "\nstrike         " << strike
                                                << "\natm level:     " << io::rate(swap->fairRate())
                                                << "\nvol:           " << io::volatility(vol)
                                                << "\nprice:         " << value
                                                << "\nimplied vol:   " << io::volatility(implVol)
                                                << "\nimplied price: " << value2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

template <typename Engine>
ext::shared_ptr<Engine> makeConstVolEngine(
    const Handle<YieldTermStructure> &discountCurve,
    Volatility volatility)
{
    Handle<Quote> h(ext::make_shared<SimpleQuote>(volatility));
    return ext::make_shared<Engine>(discountCurve, h);
}

template <typename Engine>
void checkSwaptionDelta(bool useBachelierVol)
{
    using namespace swaption_test;

    CommonVars vars;
    Date today = vars.today;
    Calendar calendar = vars.calendar;

    const Real bump = 1.e-4;
    const Real epsilon = 1.e-10;
    
    RelinkableHandle<YieldTermStructure> projectionCurveHandle;
    
    const Real projectionRate = 0.01;
    RelinkableHandle<Quote> projectionQuoteHandle;

    ext::shared_ptr<YieldTermStructure> projectionCurve = ext::make_shared<FlatForward>(
        today, projectionQuoteHandle, Actual365Fixed());
    projectionCurveHandle.linkTo(projectionCurve);

    Handle<YieldTermStructure> discountHandle(ext::make_shared<FlatForward>(
            today, 
            Handle<Quote>(ext::make_shared<SimpleQuote>(0.0085)), 
            Actual365Fixed()));
    ext::shared_ptr<DiscountingSwapEngine> swapEngine = ext::make_shared<DiscountingSwapEngine>(
        discountHandle);
    
    ext::shared_ptr<IborIndex> idx = ext::make_shared<Euribor6M>(projectionCurveHandle);
    
    Settlement::Type types[] = { Settlement::Physical, Settlement::Cash };
    Settlement::Method methods[] = { Settlement::PhysicalOTC, Settlement::CollateralizedCashPrice};
    
    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.0, 0.10, 0.20, 0.30, 0.70, 0.90 };

    for (double vol : vols) {
        for (auto exercise : exercises) {
            for (auto& length : lengths) {
                for (double& strike : strikes) {
                    for (Size h=0; h<LENGTH(type); h++) {
                        Volatility volatility = useBachelierVol ? vol / 100.0 : vol;
                        ext::shared_ptr<Engine> swaptionEngine = makeConstVolEngine<Engine>(
                            discountHandle, volatility);

                        Date exerciseDate = calendar.advance(today, exercise);
                        Date startDate = calendar.advance(exerciseDate, 2*Days);
                        projectionQuoteHandle.linkTo(ext::make_shared<SimpleQuote>(projectionRate));

                        ext::shared_ptr<VanillaSwap> underlying =
                            MakeVanillaSwap(length, idx, strike)
                                .withEffectiveDate(startDate)
                                .withFixedLegTenor(1 * Years)
                                .withFixedLegDayCount(Thirty360(Thirty360::BondBasis))
                                .withFloatingLegSpread(0.0)
                                .withType(type[h]);
                        underlying->setPricingEngine(swapEngine);
                        
                        Real fairRate = underlying->fairRate();

                        ext::shared_ptr<Swaption> swaption = ext::make_shared<Swaption>(
                            underlying, 
                            ext::make_shared<EuropeanExercise>(exerciseDate), 
                            types[h], 
                            methods[h]);
                        swaption->setPricingEngine(swaptionEngine);

                        Real value = swaption->NPV();
                        Real delta = swaption->result<Real>("delta") * bump;

                        projectionQuoteHandle.linkTo(ext::make_shared<SimpleQuote>(
                            projectionRate + bump));
    
                        Real bumpedFairRate = underlying->fairRate();
                        Real bumpedValue = swaption->NPV();
                        Real bumpedDelta = swaption->result<Real>("delta") * bump;

                        Real deltaBump = bumpedFairRate - fairRate;
                        Real approxDelta = (bumpedValue - value) / deltaBump * bump;
    
                        Real lowerBound = std::min(delta, bumpedDelta) - epsilon;
                        Real upperBound = std::max(delta, bumpedDelta) + epsilon;

                        /*! Based on the Mean Value Theorem, the below inequality
                            should hold for any function that is monotonic in the
                            area of the bump.
                        */
                        bool checkIsCorrect = (lowerBound < approxDelta) && (approxDelta < upperBound);
                        
                        if (!checkIsCorrect)
                            BOOST_FAIL(
                                "failed to compute swaption delta:"
                                << "\n  option tenor:     " << exerciseDate
                                << "\n  volatility:       " << io::rate(volatility)
                                << "\n  option type:      " << swaption->type()
                                << "\n  swap tenor:       " << length << "\n  strike:           "
                                << strike << "\n  settlement:       " << types[h]
                                << "\n  method:           " << methods[h]
                                << "\n  nominal:          " << swaption->underlyingSwap()->nominal()
                                << "\n  npv:              " << value << "\n  calculated delta: "
                                << delta << "\n  expected delta:   " << approxDelta);
                    }
                }
            }
        }
    }
}

void SwaptionTest::testSwaptionDeltaInBlackModel() {

    BOOST_TEST_MESSAGE("Testing swaption delta in Black model...");

    checkSwaptionDelta<BlackSwaptionEngine>(false);
}

void SwaptionTest::testSwaptionDeltaInBachelierModel() {

    BOOST_TEST_MESSAGE("Testing swaption delta in Bachelier model...");

    checkSwaptionDelta<BachelierSwaptionEngine>(true);
}

test_suite* SwaptionTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("Swaption tests");

    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testCashSettledSwaptions));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testStrikeDependency));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testSpreadDependency));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testSpreadTreatment));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testCachedValue));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testVega));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testSwaptionDeltaInBlackModel));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testSwaptionDeltaInBachelierModel));   

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testImpliedVolatility));
    };

    return suite;
}
