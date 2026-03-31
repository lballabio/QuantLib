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
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/integralengine.hpp>
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
        Real strike = fdp.ConsumeFloatingPointInRange<Real>(1.0, 500.0);
        Real spot = fdp.ConsumeFloatingPointInRange<Real>(1.0, 500.0);
        Rate r = fdp.ConsumeFloatingPointInRange<Real>(-0.02, 0.15);
        Rate q = fdp.ConsumeFloatingPointInRange<Real>(0.0, 0.10);
        Volatility vol = fdp.ConsumeFloatingPointInRange<Volatility>(0.01, 3.0);
        auto maturityDays = fdp.ConsumeIntegralInRange<int>(7, 3650);
        auto engineChoice = fdp.ConsumeIntegralInRange<int>(0, 2);

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

        // Select pricing engine
        ext::shared_ptr<PricingEngine> engine;
        switch (engineChoice) {
        case 0:
            engine = ext::make_shared<AnalyticEuropeanEngine>(bsmProcess);
            break;
        case 1:
            engine = ext::make_shared<FdBlackScholesVanillaEngine>(
                bsmProcess, 25, 25);
            break;
        case 2:
            engine = ext::make_shared<IntegralEngine>(bsmProcess);
            break;
        }

        auto payoff = ext::make_shared<PlainVanillaPayoff>(type, strike);
        Date exDate = today + maturityDays;
        auto exercise = ext::make_shared<EuropeanExercise>(exDate);

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        (void)option.NPV();
        (void)option.delta();
        (void)option.gamma();
        (void)option.vega();
        (void)option.theta();
        (void)option.rho();

    } catch (const std::exception&) {
    }
    return 0;
}
