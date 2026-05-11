#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/settings.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/schedule.hpp>
#include <fuzzer/FuzzedDataProvider.h>

using namespace QuantLib;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);
    SavedSettings saved_settings;

    try {
        Date today(15, January, 2025);
        Settings::instance().evaluationDate() = today;

        Leg leg;
        size_t numCashFlows = fdp.ConsumeIntegralInRange<size_t>(1, 20);
        for (size_t i = 0; i < numCashFlows; ++i) {
            Date d = today + fdp.ConsumeIntegralInRange<int>(1, 365 * 10);
            Real amount = fdp.ConsumeFloatingPointInRange<Real>(0.0, 10000.0);
            leg.push_back(ext::make_shared<SimpleCashFlow>(amount, d));
        }

        if (fdp.ConsumeBool()) {
            Schedule schedule = MakeSchedule()
                .from(today)
                .to(today + Period(5, Years))
                .withFrequency(Semiannual)
                .withCalendar(NullCalendar());
            
            Rate rate = fdp.ConsumeFloatingPointInRange<Rate>(0.0, 0.2);
            DayCounter dc = Actual360();
            Leg coupons = FixedRateLeg(schedule)
                .withNotionals(100.0)
                .withCouponRates(rate, dc);
            
            leg.insert(leg.end(), coupons.begin(), coupons.end());
        }

        if (leg.empty()) return 0;

        Rate yield = fdp.ConsumeFloatingPointInRange<Rate>(0.0, 0.2);
        DayCounter dc = Actual360();
        Compounding comp = fdp.PickValueInArray({Simple, Compounded, Continuous, SimpleThenCompounded});
        Frequency freq = fdp.PickValueInArray({NoFrequency, Once, Annual, Semiannual, EveryFourthMonth, Quarterly, Bimonthly, Monthly, EveryFourthWeek, Biweekly, Weekly, Daily, OtherFrequency});

        (void)CashFlows::startDate(leg);
        (void)CashFlows::maturityDate(leg);
        (void)CashFlows::isExpired(leg);
        
        (void)CashFlows::npv(leg, yield, dc, comp, freq);
        (void)CashFlows::bps(leg, yield, dc, comp, freq);
        
        Duration::Type durationType = fdp.PickValueInArray({Duration::Simple, Duration::Modified, Duration::Macaulay});
        (void)CashFlows::duration(leg, yield, dc, comp, freq, durationType);
        (void)CashFlows::convexity(leg, yield, dc, comp, freq);
        (void)CashFlows::basisPointValue(leg, yield, dc, comp, freq);
        (void)CashFlows::yieldValueBasisPoint(leg, yield, dc, comp, freq);

        if (numCashFlows <= 10 && fdp.ConsumeBool()) {
            Real npv = fdp.ConsumeFloatingPointInRange<Real>(1.0, 200.0);
            try {
                (void)CashFlows::yield(leg, npv, dc, comp, freq);
            } catch (...) {}
        }

    } catch (...) {}
    return 0;
}
