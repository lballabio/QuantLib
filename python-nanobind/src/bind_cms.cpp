#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/makecms.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/period.hpp>

using namespace QuantLib;

void bind_cms(nb::module_& m) {
    nb::enum_<GFunctionFactory::YieldCurveModel>(m, "YieldCurveModel")
        .value("Standard", GFunctionFactory::Standard)
        .value("ExactYield", GFunctionFactory::ExactYield)
        .value("ParallelShifts", GFunctionFactory::ParallelShifts)
        .value("NonParallelShifts", GFunctionFactory::NonParallelShifts);

    nb::enum_<VolatilityType>(m, "VolatilityType")
        .value("ShiftedLognormal", ShiftedLognormal)
        .value("Normal", Normal);

    // SwapIndex is MI-heavy (InterestRateIndex) — opaque shared_ptr holder.
    nb::class_<SwapIndex>(m, "SwapIndex")
        .def("name", [](const SwapIndex& i) { return i.name(); })
        .def("tenor", [](const SwapIndex& i) { return i.tenor(); })
        .def("fixing_days", [](const SwapIndex& i) { return i.fixingDays(); })
        .def("fixing_calendar",
             [](const SwapIndex& i) { return i.fixingCalendar(); })
        .def("day_counter", [](const SwapIndex& i) { return i.dayCounter(); });

    m.def(
        "EuriborSwapIsdaFixA",
        [](const Period& tenor, const Handle<YieldTermStructure>& h) {
            return ext::shared_ptr<SwapIndex>(
                ext::make_shared<EuriborSwapIsdaFixA>(tenor, h));
        },
        nb::arg("tenor"),
        nb::arg("handle") = Handle<YieldTermStructure>(),
        "Factory: EuriborSwapIsdaFixA → SwapIndex.");

    m.def(
        "EuriborSwapIsdaFixA",
        [](const Period& tenor,
           const Handle<YieldTermStructure>& forwarding,
           const Handle<YieldTermStructure>& discounting) {
            return ext::shared_ptr<SwapIndex>(ext::make_shared<EuriborSwapIsdaFixA>(
                tenor, forwarding, discounting));
        },
        nb::arg("tenor"),
        nb::arg("forwarding"),
        nb::arg("discounting"),
        "Factory: EuriborSwapIsdaFixA(forwarding, discounting) → SwapIndex.");

    nb::class_<Handle<SwaptionVolatilityStructure>>(
        m, "SwaptionVolatilityStructureHandle")
        .def(nb::init<>())
        .def("empty", &Handle<SwaptionVolatilityStructure>::empty)
        .def(
            "volatility",
            [](const Handle<SwaptionVolatilityStructure>& h,
               const Date& option_date,
               const Period& swap_tenor,
               Rate strike,
               bool extrapolate) {
                return h->volatility(option_date, swap_tenor, strike, extrapolate);
            },
            nb::arg("option_date"),
            nb::arg("swap_tenor"),
            nb::arg("strike"),
            nb::arg("extrapolate") = false);

    m.def(
        "ConstantSwaptionVolatility",
        [](const Date& reference_date,
           const Calendar& calendar,
           BusinessDayConvention bdc,
           Volatility volatility,
           const DayCounter& day_counter,
           VolatilityType type,
           Real shift) {
            return Handle<SwaptionVolatilityStructure>(
                ext::make_shared<ConstantSwaptionVolatility>(
                    reference_date, calendar, bdc, volatility, day_counter, type,
                    shift));
        },
        nb::arg("reference_date"),
        nb::arg("calendar"),
        nb::arg("bdc"),
        nb::arg("volatility"),
        nb::arg("day_counter"),
        nb::arg("type") = ShiftedLognormal,
        nb::arg("shift") = 0.0,
        "Factory: ConstantSwaptionVolatility → "
        "SwaptionVolatilityStructureHandle.");

    // CmsCouponPricer hierarchy is MI-heavy — opaque + factories.
    nb::class_<CmsCouponPricer>(m, "CmsCouponPricer");

    m.def(
        "AnalyticHaganPricer",
        [](const Handle<SwaptionVolatilityStructure>& swaption_vol,
           GFunctionFactory::YieldCurveModel model,
           const Handle<Quote>& mean_reversion) {
            return ext::shared_ptr<CmsCouponPricer>(
                ext::make_shared<AnalyticHaganPricer>(
                    swaption_vol, model, mean_reversion));
        },
        nb::arg("swaption_vol"),
        nb::arg("model"),
        nb::arg("mean_reversion"),
        "Factory: AnalyticHaganPricer → CmsCouponPricer.");

    m.def(
        "NumericHaganPricer",
        [](const Handle<SwaptionVolatilityStructure>& swaption_vol,
           GFunctionFactory::YieldCurveModel model,
           const Handle<Quote>& mean_reversion,
           Rate lower_limit,
           Rate upper_limit,
           Real precision) {
            return ext::shared_ptr<CmsCouponPricer>(
                ext::make_shared<NumericHaganPricer>(swaption_vol,
                                                     model,
                                                     mean_reversion,
                                                     lower_limit,
                                                     upper_limit,
                                                     precision));
        },
        nb::arg("swaption_vol"),
        nb::arg("model"),
        nb::arg("mean_reversion"),
        nb::arg("lower_limit") = 0.0,
        nb::arg("upper_limit") = 1.0,
        nb::arg("precision") = 1.0e-6,
        "Factory: NumericHaganPricer → CmsCouponPricer.");

    // CmsCoupon is FloatingRateCoupon (MI) — standalone wrapper.
    nb::class_<CmsCoupon>(m, "CmsCoupon")
        .def(
            "__init__",
            [](CmsCoupon* self,
               const Date& payment_date,
               Real nominal,
               const Date& start_date,
               const Date& end_date,
               Natural fixing_days,
               const ext::shared_ptr<SwapIndex>& index,
               Real gearing,
               Spread spread,
               const Date& ref_period_start,
               const Date& ref_period_end,
               const DayCounter& day_counter,
               bool is_in_arrears) {
                new (self) CmsCoupon(payment_date,
                                     nominal,
                                     start_date,
                                     end_date,
                                     fixing_days,
                                     index,
                                     gearing,
                                     spread,
                                     ref_period_start,
                                     ref_period_end,
                                     day_counter,
                                     is_in_arrears);
            },
            nb::arg("payment_date"),
            nb::arg("nominal"),
            nb::arg("start_date"),
            nb::arg("end_date"),
            nb::arg("fixing_days"),
            nb::arg("index"),
            nb::arg("gearing") = 1.0,
            nb::arg("spread") = 0.0,
            nb::arg("ref_period_start") = Date(),
            nb::arg("ref_period_end") = Date(),
            nb::arg("day_counter") = DayCounter(),
            nb::arg("is_in_arrears") = false)
        .def("rate", [](CmsCoupon& c) { return c.rate(); })
        .def("amount", [](CmsCoupon& c) { return c.amount(); })
        .def("nominal", [](const CmsCoupon& c) { return c.nominal(); })
        .def("accrual_start_date",
             [](const CmsCoupon& c) { return c.accrualStartDate(); })
        .def("accrual_end_date",
             [](const CmsCoupon& c) { return c.accrualEndDate(); })
        .def(
            "set_pricer",
            [](CmsCoupon& c, const ext::shared_ptr<CmsCouponPricer>& pricer) {
                c.setPricer(pricer);
            },
            nb::arg("pricer"));

    // Generic Swap from MakeCms (Instrument/Swap MI) — standalone wrapper.
    nb::class_<Swap>(m, "Swap")
        .def("NPV", [](Swap& s) { return s.NPV(); })
        .def("is_expired", [](const Swap& s) { return s.isExpired(); })
        .def("number_of_legs", [](const Swap& s) { return s.numberOfLegs(); })
        .def(
            "set_pricing_engine",
            [](Swap& s, const Handle<YieldTermStructure>& discount_curve) {
                s.setPricingEngine(
                    ext::make_shared<DiscountingSwapEngine>(discount_curve));
            },
            nb::arg("discount_curve"))
        .def(
            "set_cms_coupon_pricer",
            [](Swap& s, const ext::shared_ptr<CmsCouponPricer>& pricer) {
                // CMS leg is leg 0 in MakeCms.
                setCouponPricer(s.leg(0), pricer);
            },
            nb::arg("pricer"),
            "Attach a CmsCouponPricer to leg 0 (CMS leg from make_cms).");

    m.def(
        "make_cms",
        [](const Period& swap_tenor,
           const ext::shared_ptr<SwapIndex>& swap_index,
           const ext::shared_ptr<IborIndex>& ibor_index,
           Spread ibor_spread,
           const Period& forward_start,
           const Handle<YieldTermStructure>& discount_curve,
           const ext::shared_ptr<CmsCouponPricer>& pricer,
           Real nominal) {
            MakeCms maker(swap_tenor, swap_index, ibor_index, ibor_spread,
                         forward_start);
            maker.withNominal(nominal);
            if (!discount_curve.empty())
                maker.withDiscountingTermStructure(discount_curve);
            if (pricer)
                maker.withCmsCouponPricer(pricer);
            return Swap(maker);
        },
        nb::arg("swap_tenor"),
        nb::arg("swap_index"),
        nb::arg("ibor_index"),
        nb::arg("ibor_spread") = 0.0,
        nb::arg("forward_start") = Period(0, Days),
        nb::arg("discount_curve") = Handle<YieldTermStructure>(),
        nb::arg("pricer") = ext::shared_ptr<CmsCouponPricer>(),
        nb::arg("nominal") = 1.0,
        "Build a CMS vs Ibor Swap via QuantLib MakeCms (value copy).");
}
