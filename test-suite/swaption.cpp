/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2007, 2008 StatPro Italia srl

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
    VanillaSwap::Type type[] = { VanillaSwap::Receiver, VanillaSwap::Payer };

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
            fixedDayCount = Thirty360();

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

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(type); k++) {
                Date exerciseDate = vars.calendar.advance(vars.today,
                                                          exercises[i]);
                Date startDate =
                    vars.calendar.advance(exerciseDate,
                                          vars.settlementDays,Days);
                // store the results for different rates...
                std::vector<Real> values;
                std::vector<Real> values_cash;
                Volatility vol = 0.20;
                for (Size l=0; l<LENGTH(strikes); l++) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(lengths[j], vars.index, strikes[l])
                                .withEffectiveDate(startDate)
                                .withFixedLegTenor(1*Years)
                                .withFixedLegDayCount(vars.fixedDayCount)
                                .withFloatingLegSpread(0.0)
                                .withType(type[k]);
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
                if (type[k]==VanillaSwap::Payer) {
                    std::vector<Real>::iterator it =
                        std::adjacent_find(values.begin(), values.end(),
                                           std::less<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR("NPV of Payer swaption with delivery settlement"
                                    "is increasing with the strike:" <<
                                    "\noption tenor: " << exercises[i] <<
                                    "\noption date:  " << exerciseDate <<
                                    "\nvolatility:   " << io::rate(vol) <<
                                    "\nswap tenor:   " << lengths[j] <<
                                    "\nvalue:        " << values[n  ] <<" at strike: " << io::rate(strikes[n  ]) <<
                                    "\nvalue:        " << values[n+1] << " at strike: " << io::rate(strikes[n+1]));
                    }
                    std::vector<Real>::iterator it_cash =
                        std::adjacent_find(values_cash.begin(), values_cash.end(),
                                           std::less<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR("NPV of Payer swaption with cash settlement"
                                    "is increasing with the strike:" <<
                                    "\noption tenor: " << exercises[i] <<
                                    "\noption date:  " << exerciseDate <<
                                    "\nvolatility:   " << io::rate(vol) <<
                                    "\nswap tenor:   " << lengths[j] <<
                                    "\nvalue:        " << values_cash[n  ] << " at strike: " << io::rate(strikes[n  ]) <<
                                    "\nvalue:        " << values_cash[n+1] << " at strike: " << io::rate(strikes[n+1]));
                    }
                } else {
                    std::vector<Real>::iterator it =
                        std::adjacent_find(values.begin(), values.end(),
                                           std::greater<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR("NPV of Receiver swaption with delivery settlement"
                                    "is increasing with the strike:" <<
                                    "\noption tenor: " << exercises[i] <<
                                    "\noption date:  " << exerciseDate <<
                                    "\nvolatility:   " << io::rate(vol) <<
                                    "\nswap tenor:   " << lengths[j] <<
                                    "\nvalue:        " << values[n  ] << " at strike: " << io::rate(strikes[n  ]) <<
                                    "\nvalue:        " << values[n+1] << " at strike: " << io::rate(strikes[n+1]));
                    }
                    std::vector<Real>::iterator it_cash =
                        std::adjacent_find(values_cash.begin(), values_cash.end(),
                                           std::greater<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR("NPV of Receiver swaption with cash settlement"
                                    "is increasing with the strike:" <<
                                    "\noption tenor: " << exercises[i] <<
                                    "\noption date:  " << exerciseDate <<
                                    "\nvolatility:   " << io::rate(vol) <<
                                    "\nswap tenor:   " << lengths[j] <<
                                    "\nvalue:        " << values_cash[n  ] << " at strike: " << io::rate(strikes[n  ]) <<
                                    "\nvalue:        " << values_cash[n+1] << " at strike: " << io::rate(strikes[n+1]));
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

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(type); k++) {
                Date exerciseDate = vars.calendar.advance(vars.today,
                                                          exercises[i]);
                Date startDate =
                    vars.calendar.advance(exerciseDate,
                                          vars.settlementDays,Days);
                // store the results for different rates...
                std::vector<Real> values;
                std::vector<Real> values_cash;
                for (Size l=0; l<LENGTH(spreads); l++) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(lengths[j], vars.index, 0.06)
                                .withFixedLegTenor(1*Years)
                                .withFixedLegDayCount(vars.fixedDayCount)
                                .withEffectiveDate(startDate)
                                .withFloatingLegSpread(spreads[l])
                                .withType(type[k]);
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
                if (type[k]==VanillaSwap::Payer) {
                    std::vector<Real>::iterator it =
                        std::adjacent_find(values.begin(), values.end(),
                                           std::greater<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR("NPV is decreasing with the spread " <<
                            "in a payer swaption (physical delivered):" <<
                            "\nexercise date: " << exerciseDate <<
                            "\nlength:        " << lengths[j] <<
                            "\nvalue:         " << values[n  ] << " for spread: " << io::rate(spreads[n]) <<
                            "\nvalue:         " << values[n+1] << " for spread: " << io::rate(spreads[n+1]));
                    }
                    std::vector<Real>::iterator it_cash =
                        std::adjacent_find(values_cash.begin(), values_cash.end(),
                                           std::greater<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR("NPV is decreasing with the spread " <<
                            "in a payer swaption (cash delivered):" <<
                            "\nexercise date: " << exerciseDate <<
                            "\nlength: " << lengths[j] <<
                            "\nvalue:  " << values_cash[n  ] << " for spread: " << io::rate(spreads[n]) <<
                            "\nvalue:  " << values_cash[n+1] << " for spread: " << io::rate(spreads[n+1]));
                    }
                } else {
                    std::vector<Real>::iterator it =
                        std::adjacent_find(values.begin(), values.end(),
                                           std::less<Real>());
                    if (it != values.end()) {
                        Size n = it - values.begin();
                        BOOST_ERROR("NPV is increasing with the spread " <<
                            "in a receiver swaption (physical delivered):"
                            "\nexercise date: " << exerciseDate <<
                            "\nlength: " << lengths[j] <<
                            "\nvalue:  " << values[n  ] << " for spread: " << io::rate(spreads[n]) <<
                            "\nvalue:  " << values[n+1] << " for spread: " << io::rate(spreads[n+1]));
                    }
                    std::vector<Real>::iterator it_cash =
                        std::adjacent_find(values_cash.begin(), values_cash.end(),
                                           std::less<Real>());
                    if (it_cash != values_cash.end()) {
                        Size n = it_cash - values_cash.begin();
                        BOOST_ERROR("NPV is increasing with the spread " <<
                            "in a receiver swaption (cash delivered):"
                            "\nexercise date: " << exerciseDate <<
                            "\nlength: " << lengths[j] <<
                            "\nvalue:  " << values_cash[n  ] << " for spread: " << io::rate(spreads[n]) <<
                            "\nvalue:  " << values_cash[n+1] << " for spread: " << io::rate(spreads[n+1]));
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

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size k=0; k<LENGTH(type); k++) {
                Date exerciseDate = vars.calendar.advance(vars.today,
                                                          exercises[i]);
                Date startDate =
                    vars.calendar.advance(exerciseDate,
                                          vars.settlementDays,Days);
                for (Size l=0; l<LENGTH(spreads); l++) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(lengths[j], vars.index, 0.06)
                                .withFixedLegTenor(1*Years)
                                .withFixedLegDayCount(vars.fixedDayCount)
                                .withEffectiveDate(startDate)
                                .withFloatingLegSpread(spreads[l])
                                .withType(type[k]);
                    // FLOATING_POINT_EXCEPTION
                    Spread correction = spreads[l] *
                                        swap->floatingLegBPS() /
                                        swap->fixedLegBPS();
                    ext::shared_ptr<VanillaSwap> equivalentSwap =
                        MakeVanillaSwap(lengths[j], vars.index, 0.06+correction)
                                .withFixedLegTenor(1*Years)
                                .withFixedLegDayCount(vars.fixedDayCount)
                                .withEffectiveDate(startDate)
                                .withFloatingLegSpread(0.0)
                                .withType(type[k]);
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
                        BOOST_ERROR("wrong spread treatment:" <<
                            "\nexercise: " << exerciseDate <<
                            "\nlength:   " << lengths[j] <<
                            "\ntype      " << type[k] <<
                            "\nspread:   " << io::rate(spreads[l]) <<
                            "\noriginal swaption value:   " << swaption1->NPV() <<
                            "\nequivalent swaption value: " << swaption2->NPV());

                    if (std::fabs(swaption1_cash->NPV()-swaption2_cash->NPV()) > 1.0e-6)
                        BOOST_ERROR("wrong spread treatment:" <<
                            "\nexercise date: " << exerciseDate <<
                            "\nlength: " << lengths[j] <<
                            "\npay " << (type[k] ? "fixed" : "floating") <<
                            "\nspread: " << io::rate(spreads[l]) <<
                            "\nvalue of original swaption:   "  << swaption1_cash->NPV() <<
                            "\nvalue of equivalent swaption: "  << swaption2_cash->NPV());
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

    Real cachedNPV;
    if (IborCoupon::usingAtParCoupons())
        cachedNPV = 0.036418158579;
    else
        cachedNPV = 0.036421429684;

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
    for (Size i=0; i<LENGTH(exercises); i++) {
        Date exerciseDate = vars.calendar.advance(vars.today, exercises[i]);
        Date startDate = vars.calendar.advance(exerciseDate,
                                           vars.settlementDays*Days);
        for (Size j=0; j<LENGTH(lengths); j++) {
            for (Size t=0; t<LENGTH(strikes); t++) {
                for (Size h=0; h<LENGTH(type); h++) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(lengths[j], vars.index, strikes[t])
                                .withEffectiveDate(startDate)
                                .withFixedLegTenor(1*Years)
                                .withFixedLegDayCount(vars.fixedDayCount)
                                .withFloatingLegSpread(0.0)
                                .withType(type[h]);
                    for (Size u=0; u<LENGTH(vols); u++) {
                        ext::shared_ptr<Swaption> swaption =
                            vars.makeSwaption(swap, exerciseDate,
                                              vols[u], types[h], methods[h]);
                        // FLOATING_POINT_EXCEPTION
                        ext::shared_ptr<Swaption> swaption1 =
                            vars.makeSwaption(swap, exerciseDate,
                                              vols[u]-shift, types[h], methods[h]);
                        ext::shared_ptr<Swaption> swaption2 =
                            vars.makeSwaption(swap, exerciseDate,
                                              vols[u]+shift, types[h], methods[h]);

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
                                BOOST_FAIL("failed to compute swaption vega:" <<
                                    "\n  option tenor:    " << exercises[i] <<
                                    "\n  volatility:      " << io::rate(vols[u]) <<
                                    "\n  option type:     " << swaption->type() <<
                                    "\n  swap tenor:      " << lengths[j] <<
                                    "\n  strike:          " << io::rate(strikes[t]) <<
                                    "\n  settlement:      " << types[h] <<
                                    "\n  nominal:         " << swaption->underlyingSwap()->nominal() <<
                                    "\n  npv:             " << swaptionNPV <<
                                    "\n  calculated vega: " << analyticalVegaPerPoint <<
                                    "\n  expected vega:   " << numericalVegaPerPoint <<
                                    "\n  discrepancy:     " << io::rate(discrepancy) <<
                                    "\n  tolerance:       " << io::rate(tolerance));
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

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {

            Date exerciseDate = vars.calendar.advance(vars.today,exercises[i]);
            Date startDate = vars.calendar.advance(exerciseDate,
                                                   vars.settlementDays,Days);
            Date maturity =
                vars.calendar.advance(startDate,lengths[j],
                                      vars.floatingConvention);
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
                                fixedSchedule_u,strike,Thirty360(),
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
                                fixedSchedule_a,strike,Thirty360(),
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
                                    Thirty360(),Compounded,
                                    vars.fixedFrequency)));
            Handle<YieldTermStructure> termStructure_a360(
                ext::shared_ptr<YieldTermStructure>(
                    new FlatForward(vars.settlement,swap_a360->fairRate(),
                                    Thirty360(),Compounded,
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
            annuity_u360 = swap_u360->type()==VanillaSwap::Payer ?
                -annuity_u360 : annuity_u360;
            // Fixed leg conventions: ModifiedFollowing, act/365
            Real annuity_a365 = swap_a365->fixedLegBPS() / 0.0001;
            annuity_a365 = swap_a365->type()==VanillaSwap::Payer ?
                -annuity_a365 : annuity_a365;
            // Fixed leg conventions: ModifiedFollowing, 30/360
            Real annuity_a360 = swap_a360->fixedLegBPS() / 0.0001;
            annuity_a360 = swap_a360->type()==VanillaSwap::Payer ?
                -annuity_a360 : annuity_a360;
            // Fixed leg conventions: Unadjusted, act/365
            Real annuity_u365 = swap_u365->fixedLegBPS() / 0.0001;
            annuity_u365 = swap_u365->type()==VanillaSwap::Payer ?
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
                BOOST_ERROR("\n" <<
                            "    The npv's ratio must be equal to " <<
                            " annuities ratio" << "\n"
                            "    Swaption " <<
                            exercises[i].units() << "y x " << lengths[j].units() << "y" <<
                            " (underlying swap fixed leg Unadjusted, 30/360)" << "\n" <<
                            "    Today           : " <<
                            vars.today << "\n" <<
                            "    Settlement date : " <<
                            vars.settlement << "\n" <<
                            "    Exercise date   : " <<
                            exerciseDate << "\n"   <<
                            "    Swap start date : " <<
                            startDate << "\n"   <<
                            "    Swap end date   : " <<
                            maturity <<     "\n"   <<
                            "    physical delivered swaption npv : " <<
                            value_p_u360 << "\t\t\t" <<
                            "    annuity : " <<
                            annuity_u360 << "\n" <<
                            "    cash delivered swaption npv :     " <<
                            value_c_u360 << "\t\t\t" <<
                            "    annuity : " <<
                            cashannuity_u360 << "\n" <<
                            "    npv ratio : " <<
                            npv_ratio_u360 << "\n" <<
                            "    annuity ratio : " <<
                            annuity_ratio_u360 << "\n" <<
                            "    difference : " <<
                            (annuity_ratio_u360-npv_ratio_u360) );
            }
            if (std::fabs(annuity_ratio_a365-npv_ratio_a365)>1e-10) {
                BOOST_ERROR("\n" <<
                            "    The npv's ratio must be equal to " <<
                            " annuities ratio" << "\n"
                            "    Swaption " <<
                            exercises[i].units() << "y x " << lengths[j].units() << "y" <<
                            " (underlying swap fixed leg Modified Following, act/365" << "\n" <<
                            "    Today           : " <<
                            vars.today << "\n" <<
                            "    Settlement date : " <<
                            vars.settlement << "\n" <<
                            "    Exercise date   : " <<
                            exerciseDate <<  "\n"  <<
                            "    Swap start date : " <<
                            startDate << "\n"   <<
                            "    Swap end date   : " <<
                            maturity <<     "\n"   <<
                            "    physical delivered swaption npv : "  <<
                            value_p_a365 << "\t\t\t" <<
                            "    annuity : " <<
                            annuity_a365 << "\n" <<
                            "    cash delivered swaption npv :     "  <<
                            value_c_a365 << "\t\t\t" <<
                            "    annuity : " <<
                            cashannuity_a365 << "\n" <<
                            "    npv ratio : " <<
                            npv_ratio_a365 << "\n" <<
                            "    annuity ratio : " <<
                            annuity_ratio_a365 << "\n" <<
                            "    difference : " <<
                            (annuity_ratio_a365-npv_ratio_a365) );
                }
            if (std::fabs(annuity_ratio_a360-npv_ratio_a360)>1e-10) {
                BOOST_ERROR("\n" <<
                            "    The npv's ratio must be equal to " <<
                            " annuities ratio" << "\n"
                            "    Swaption " <<
                            exercises[i].units() << "y x " << lengths[j].units() << "y" <<
                            " (underlying swap fixed leg Unadjusted, 30/360)" << "\n" <<
                            "    Today           : " <<
                            vars.today << "\n" <<
                            "    Settlement date : " <<
                            vars.settlement << "\n" <<
                            "    Exercise date   : " <<
                            exerciseDate << "\n"   <<
                            "    Swap start date : " <<
                            startDate << "\n"   <<
                            "    Swap end date   : " <<
                            maturity <<     "\n"   <<
                            "    physical delivered swaption npv : " <<
                            value_p_a360 << "\t\t\t" <<
                            "    annuity : " <<
                            annuity_a360 << "\n" <<
                            "    cash delivered swaption npv :     " <<
                            value_c_a360 << "\t\t\t" <<
                            "    annuity : " <<
                            cashannuity_a360 << "\n" <<
                            "    npv ratio : " <<
                            npv_ratio_a360 << "\n" <<
                            "    annuity ratio : " <<
                            annuity_ratio_a360 << "\n" <<
                            "    difference : " <<
                            (annuity_ratio_a360-npv_ratio_a360) );
            }
            if (std::fabs(annuity_ratio_u365-npv_ratio_u365)>1e-10) {
                BOOST_ERROR("\n" <<
                            "    The npv's ratio must be equal to " <<
                            " annuities ratio" << "\n"
                            "    Swaption " <<
                            exercises[i].units() << "y x " << lengths[j].units() << "y" <<
                            " (underlying swap fixed leg Unadjusted, act/365)" << "\n" <<
                            "    Today           : " <<
                            vars.today << "\n" <<
                            "    Settlement date : " <<
                            vars.settlement << "\n" <<
                            "    Exercise date   : " <<
                            exerciseDate << "\n"   <<
                            "    Swap start date : " <<
                            startDate << "\n"   <<
                            "    Swap end date   : " <<
                            maturity <<     "\n"   <<
                            "    physical delivered swaption npv : " <<
                            value_p_u365 << "\t\t\t" <<
                            "    annuity : " <<
                            annuity_u365 << "\n" <<
                            "    cash delivered swaption npv :     " <<
                            value_c_u365 << "\t\t\t" <<
                            "    annuity : " <<
                            cashannuity_u365 << "\n" <<
                            "    npv ratio : " <<
                            npv_ratio_u365 << "\n" <<
                            "    annuity ratio : " <<
                            annuity_ratio_u365 << "\n" <<
                            "    difference : " <<
                            (annuity_ratio_u365-npv_ratio_u365) );
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

    for (Size i=0; i<LENGTH(exercises); i++) {
        for (Size j=0; j<LENGTH(lengths); j++) {
            Date exerciseDate = vars.calendar.advance(vars.today,exercises[i]);
            Date startDate = vars.calendar.advance(exerciseDate,
                                                   vars.settlementDays, Days);

            for (Size t=0; t<LENGTH(strikes); t++) {
                for (Size k=0; k<LENGTH(type); k++) {
                    ext::shared_ptr<VanillaSwap> swap =
                        MakeVanillaSwap(lengths[j], vars.index, strikes[t])
                                .withEffectiveDate(startDate)
                                .withFixedLegTenor(1*Years)
                                .withFixedLegDayCount(vars.fixedDayCount)
                                .withFloatingLegSpread(0.0)
                                .withType(type[k]);
                    for (Size h=0; h<LENGTH(types); h++) {
                        for (Size u=0; u<LENGTH(vols); u++) {
                            ext::shared_ptr<Swaption> swaption =
                                vars.makeSwaption(
                                    swap, exerciseDate, vols[u], types[h],
                                    methods[h],
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
                                BOOST_ERROR("implied vol failure: " <<
                                            exercises[i] << "x" << lengths[j] << " " << type[k] <<
                                            "\nsettlement: " << types[h] <<
                                            "\nstrike      " << strikes[t] <<
                                            "\natm level:  " << io::rate(swap->fairRate()) <<
                                            "\nvol:        " << io::volatility(vols[u]) <<
                                            "\nprice:      " << value <<
                                            "\n" << e.what());
                            }
                            if (std::fabs(implVol-vols[u]) > tolerance) {
                                // the difference might not matter
                                swaption->setPricingEngine(vars.makeEngine(implVol, BlackSwaptionEngine::DiscountCurve));
                                Real value2 = swaption->NPV();
                                if (std::fabs(value-value2) > tolerance) {
                                    BOOST_ERROR("implied vol failure: " <<
                                        exercises[i] << "x" << lengths[j] << " " << type[k] <<
                                        "\nsettlement:    " << types[h] <<
                                        "\nstrike         " << strikes[t] <<
                                        "\natm level:     " << io::rate(swap->fairRate()) <<
                                        "\nvol:           " << io::volatility(vols[u]) <<
                                        "\nprice:         " << value <<
                                        "\nimplied vol:   " << io::volatility(implVol) <<
                                        "\nimplied price: " << value2);
                                }
                             }
                        }
                    }
                }
            }
        }
    }
}


test_suite* SwaptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swaption tests");
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testCashSettledSwaptions));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testStrikeDependency));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testSpreadDependency));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testSpreadTreatment));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testCachedValue));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testImpliedVolatility));

    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&SwaptionTest::testVega));

    return suite;
}
