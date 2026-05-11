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
#include <ql/instruments/basketoption.hpp>
#include <ql/pricingengines/basket/stulzengine.hpp>
#include <ql/pricingengines/basket/mceuropeanbasketengine.hpp>
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
        auto basketTypeChoice = fdp.ConsumeIntegralInRange<int>(0, 2);
        
        Real strike = fdp.ConsumeFloatingPointInRange<Real>(1.0, 500.0);
        
        auto nAssets = fdp.ConsumeIntegralInRange<int>(2, 4);
        std::vector<ext::shared_ptr<StochasticProcess1D>> processes;
        
        Date today(15, January, 2025);
        Settings::instance().evaluationDate() = today;
        DayCounter dc = Actual360();

        Rate r = fdp.ConsumeFloatingPointInRange<Real>(-0.02, 0.15);
        auto rTS = ext::make_shared<FlatForward>(today, r, dc);
        Handle<YieldTermStructure> rH(rTS);

        for (int i = 0; i < nAssets; ++i) {
            Real spot = fdp.ConsumeFloatingPointInRange<Real>(1.0, 500.0);
            Volatility vol = fdp.ConsumeFloatingPointInRange<Volatility>(0.01, 3.0);
            Rate q = fdp.ConsumeFloatingPointInRange<Real>(0.0, 0.10);
            
            auto qTS = ext::make_shared<FlatForward>(today, q, dc);
            auto volTS = ext::make_shared<BlackConstantVol>(today, NullCalendar(), vol, dc);
            
            processes.push_back(ext::make_shared<BlackScholesMertonProcess>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(spot)),
                Handle<YieldTermStructure>(qTS),
                rH,
                Handle<BlackVolTermStructure>(volTS)));
        }

        Matrix correlation(nAssets, nAssets, 0.0);
        for (int i = 0; i < nAssets; ++i) {
            correlation[i][i] = 1.0;
            for (int j = 0; j < i; ++j) {
                Real rho = fdp.ConsumeFloatingPointInRange<Real>(-0.99, 0.99);
                correlation[i][j] = correlation[j][i] = rho;
            }
        }

        auto process = ext::make_shared<StochasticProcessArray>(processes, correlation);
        
        ext::shared_ptr<BasketPayoff> payoff;
        auto vanillaPayoff = ext::make_shared<PlainVanillaPayoff>(type, strike);
        switch (basketTypeChoice) {
        case 0:
            payoff = ext::make_shared<MaxBasketPayoff>(vanillaPayoff);
            break;
        case 1:
            payoff = ext::make_shared<MinBasketPayoff>(vanillaPayoff);
            break;
        case 2:
            payoff = ext::make_shared<AverageBasketPayoff>(vanillaPayoff, nAssets);
            break;
        }

        auto maturityDays = fdp.ConsumeIntegralInRange<int>(7, 3650);
        Date exDate = today + maturityDays;
        auto exercise = ext::make_shared<EuropeanExercise>(exDate);

        BasketOption option(payoff, exercise);

        auto engineChoice = fdp.ConsumeIntegralInRange<int>(0, 1);
        ext::shared_ptr<PricingEngine> engine;
        if (engineChoice == 0 && nAssets == 2) {
            auto p1 = ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(processes[0]);
            auto p2 = ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(processes[1]);
            if (p1 && p2) {
                engine = ext::make_shared<StulzEngine>(p1, p2, correlation[0][1]);
            }
        } 
        
        if (!engine) {
            engine = MakeMCEuropeanBasketEngine<PseudoRandom>(process)
                .withSteps(10)
                .withSamples(100)
                .withSeed(42);
        }
        
        option.setPricingEngine(engine);
        (void)option.NPV();

    } catch (const std::exception&) {
    }
    return 0;
}
