/*
 Copyright (C) 2023 Nathaniel Brough

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
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/instruments/bonds/amortizingfixedratebond.hpp>
#include <ql/settings.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/business252.hpp>
#include <cassert>
#include <fuzzer/FuzzedDataProvider.h>
#include <limits>
#define _unused(x) ((void)(x))

using namespace QuantLib;

std::vector<Real> fuzzedRates(FuzzedDataProvider& fdp, const size_t length) {
    std::vector<Real> result;
    for (size_t i = 0; i < length; i++) {
        result.push_back(fdp.ConsumeProbability<Real>());
    }
    return result;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    FuzzedDataProvider fdp(Data, Size);
    // Ensure settings are reset each iteration of the fuzzing loop.
    // NOTE: this class manages the settings singleton using default
    // (con/des)tructors.
    SavedSettings saved_settings;

    constexpr size_t kMaxSize = 512;
    auto length = fdp.ConsumeIntegralInRange<size_t>(0, kMaxSize);

    Date refDate = Settings::instance().evaluationDate();

    auto rates = fuzzedRates(fdp, length);
    Frequency freq = Monthly;

    for (size_t i = 0; i < length; ++i) {

        auto schedule = sinkingSchedule(refDate, Period(30, Years), freq, NullCalendar());
        auto notionals = sinkingNotionals(Period(30, Years), freq, rates[i], 100.0);

        AmortizingFixedRateBond myBond(0, notionals, schedule, {rates[i]},
                                       ActualActual(ActualActual::ISMA));

        Leg cashflows = myBond.cashflows();

        Real lastTotalAmount = 0.0;
        for (size_t k = 0; k < cashflows.size() / 2; ++k) {
            Real coupon = cashflows[2 * k]->amount();
            Real principal = cashflows[2 * k + 1]->amount();
            Real totalAmount = coupon + principal;
            // Assert invariants, these should always be true.
            assert(coupon > 0.0);
            assert(principal > 0.0);
            assert(totalAmout > lastTotalAmount);
            lastTotalAmount = totalAmount;
            _unused(lastTotalAmount);
        }
    }
    return 0;
}
