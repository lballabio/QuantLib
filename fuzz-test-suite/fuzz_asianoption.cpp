/*
 Copyright (C) 2026 David Korczynski

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

#include <ql/exercise.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/pricingengines/asian/analytic_cont_geom_av_price.hpp>
#include <ql/pricingengines/asian/analytic_discr_geom_av_price.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <fuzzer/FuzzedDataProvider.h>

using namespace QuantLib;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);
    SavedSettings saved_settings;

    try {
        auto type = fdp.PickValueInArray({Option::Put, Option::Call});
        auto averageType = fdp.PickValueInArray({Average::Arithmetic, Average::Geometric});
        
        Real strike = fdp.ConsumeFloatingPointInRange<Real>(1.0, 500.0);
        Real spot = fdp.ConsumeFloatingPointInRange<Real>(1.0, 500.0);
        
        Rate r = fdp.ConsumeFloatingPointInRange<Real>(-0.02, 0.15);
        Rate q = fdp.ConsumeFloatingPointInRange<Real>(0.0, 0.10);
        Volatility vol = fdp.ConsumeFloatingPointInRange<Volatility>(0.01, 3.0);
        auto maturityDays = fdp.ConsumeIntegralInRange<int>(7, 3650);
        
        bool continuous = fdp.ConsumeBool();

        Date today(15, January, 2025);
        Settings::instance().evaluationDate() = today;
        DayCounter dc = Actual360();

        auto spotQuote = ext::make_shared<SimpleQuote>(spot);
        auto rTS = ext::make_shared<FlatForward>(today, r, dc);
        auto qTS = ext::make_shared<FlatForward>(today, q, dc);
        auto volTS = ext::make_shared<BlackConstantVol>(
            today, NullCalendar(), vol, dc);

        auto bsmProcess = ext::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(spotQuote),
            Handle<YieldTermStructure>(qTS),
            Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS));

        auto payoff = ext::make_shared<PlainVanillaPayoff>(type, strike);
        Date exDate = today + maturityDays;
        auto exercise = ext::make_shared<EuropeanExercise>(exDate);

        if (continuous) {
            ContinuousAveragingAsianOption option(averageType, payoff, exercise);
            // Analytic engine only supports Geometric average
            if (averageType == Average::Geometric) {
                auto engine = ext::make_shared<AnalyticContinuousGeometricAveragePriceAsianEngine>(bsmProcess);
                option.setPricingEngine(engine);
                (void)option.NPV();
            }
        } else {
            auto fixingCount = fdp.ConsumeIntegralInRange<int>(1, 10);
            std::vector<Date> fixingDates;
            for (int i = 0; i < fixingCount; ++i) {
                fixingDates.push_back(today + fdp.ConsumeIntegralInRange<int>(1, maturityDays));
            }
            std::sort(fixingDates.begin(), fixingDates.end());
            // unique
            fixingDates.erase(std::unique(fixingDates.begin(), fixingDates.end()), fixingDates.end());
            if (fixingDates.empty()) fixingDates.push_back(today + maturityDays);

            DiscreteAveragingAsianOption option(averageType, fixingDates, payoff, exercise);
            if (averageType == Average::Geometric) {
                auto engine = ext::make_shared<AnalyticDiscreteGeometricAveragePriceAsianEngine>(bsmProcess);
                option.setPricingEngine(engine);
                (void)option.NPV();
            }
        }

    } catch (const std::exception&) {
    }
    return 0;
}
