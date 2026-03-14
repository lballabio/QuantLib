
/*
 Copyright (C) 2023 Nathaniel Brough
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2005 Joseph Wang

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

#include <ql/any.hpp>
#include <ql/exercise.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/integral.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesshoutengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/juquadraticengine.hpp>
#include <ql/pricingengines/vanilla/qdfpamericanengine.hpp>
#include <ql/pricingengines/vanilla/qdplusamericanengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <fuzzer/FuzzedDataProvider.h>
#include <limits>
#include <map>


using namespace QuantLib;

namespace {

    struct AmericanOptionData {
        Option::Type type;
        Real strike;
        Real s;       // spot
        Rate q;       // dividend
        Rate r;       // risk-free rate
        Time t;       // time to maturity
        Volatility v; // volatility
    };

}

AmericanOptionData fuzzedAmericanOptionData(FuzzedDataProvider& fdp) {
    return AmericanOptionData{
        .type = fdp.PickValueInArray({Option::Type::Put, Option::Type::Call}),
        .strike = fdp.ConsumeFloatingPoint<Real>(),
        .s = fdp.ConsumeFloatingPointInRange<Real>(0.0, std::numeric_limits<Real>::max()),
        .q = fdp.ConsumeProbability<Rate>(),
        .r = fdp.ConsumeProbability<Rate>(),
        .t = fdp.ConsumeFloatingPointInRange<Time>(0.0, std::numeric_limits<Real>::max()),
        .v = fdp.ConsumeFloatingPointInRange<Volatility>(0.0, 10.0),
    };
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);
    // Ensure that settings are reset between each fuzzing iteration.
    SavedSettings saved_settings;

    constexpr size_t kMaxValues = 1024;
    auto length = fdp.ConsumeIntegralInRange<size_t>(0, kMaxValues);
    std::vector<AmericanOptionData> values;
    values.reserve(length);
    for (size_t i = 0; i < length; i++) {
        values.push_back(fuzzedAmericanOptionData(fdp));
    }

    Date today(2, March, 2020);
    DayCounter dc = Actual360();
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));

    ext::shared_ptr<YieldTermStructure> qTS =
        ext::shared_ptr<YieldTermStructure>(new FlatForward(today, Handle<Quote>(qRate), dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS =
        ext::shared_ptr<YieldTermStructure>(new FlatForward(today, Handle<Quote>(rRate), dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = ext::shared_ptr<BlackVolTermStructure>(
        new BlackConstantVol(today, NullCalendar(), Handle<Quote>(vol), dc));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new BlackScholesMertonProcess(
            Handle<Quote>(spot), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
        new BaroneAdesiWhaleyApproximationEngine(stochProcess));

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));
        Date exDate = today + Integer(std::lround(365*value.t));
        ext::shared_ptr<Exercise> exercise(new AmericanExercise(today, exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        (void)option.NPV();

    }

    return 0;
}
