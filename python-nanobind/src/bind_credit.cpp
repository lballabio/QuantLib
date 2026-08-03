#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>

#include <ql/default.hpp>
#include <ql/handle.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>

using namespace QuantLib;

namespace {

Handle<DefaultProbabilityTermStructure> make_flat_hazard_rate(
    const Date& reference_date,
    Rate hazard_rate,
    const DayCounter& day_counter) {
    return Handle<DefaultProbabilityTermStructure>(
        ext::make_shared<FlatHazardRate>(reference_date, hazard_rate, day_counter));
}

Handle<DefaultProbabilityTermStructure> make_flat_hazard_rate_settlement(
    Natural settlement_days,
    const Calendar& calendar,
    Rate hazard_rate,
    const DayCounter& day_counter) {
    return Handle<DefaultProbabilityTermStructure>(ext::make_shared<FlatHazardRate>(
        settlement_days, calendar, hazard_rate, day_counter));
}

} // namespace

void bind_credit(nb::module_& m) {
    nb::enum_<Protection::Side>(m, "ProtectionSide")
        .value("Buyer", Protection::Buyer)
        .value("Seller", Protection::Seller);

    nb::class_<Handle<DefaultProbabilityTermStructure>>(
        m, "DefaultProbabilityTermStructureHandle")
        .def(nb::init<>())
        .def("empty", &Handle<DefaultProbabilityTermStructure>::empty)
        .def(
            "survival_probability",
            [](const Handle<DefaultProbabilityTermStructure>& h,
               const Date& d,
               bool extrapolate) {
                return h->survivalProbability(d, extrapolate);
            },
            nb::arg("date"),
            nb::arg("extrapolate") = false)
        .def(
            "survival_probability",
            [](const Handle<DefaultProbabilityTermStructure>& h,
               Time t,
               bool extrapolate) {
                return h->survivalProbability(t, extrapolate);
            },
            nb::arg("time"),
            nb::arg("extrapolate") = false)
        .def(
            "hazard_rate",
            [](const Handle<DefaultProbabilityTermStructure>& h,
               const Date& d,
               bool extrapolate) { return h->hazardRate(d, extrapolate); },
            nb::arg("date"),
            nb::arg("extrapolate") = false)
        .def("reference_date",
             [](const Handle<DefaultProbabilityTermStructure>& h) {
                 return h->referenceDate();
             });

    m.def("FlatHazardRate",
          &make_flat_hazard_rate,
          nb::arg("reference_date"),
          nb::arg("hazard_rate"),
          nb::arg("day_counter"),
          "Factory: FlatHazardRate → DefaultProbabilityTermStructureHandle.");

    m.def("FlatHazardRate",
          &make_flat_hazard_rate_settlement,
          nb::arg("settlement_days"),
          nb::arg("calendar"),
          nb::arg("hazard_rate"),
          nb::arg("day_counter"),
          "Factory: FlatHazardRate (settlementDays) → "
          "DefaultProbabilityTermStructureHandle.");

    // CreditDefaultSwap is MI-heavy (Instrument) — standalone wrapper.
    nb::class_<CreditDefaultSwap>(m, "CreditDefaultSwap")
        .def(
            "__init__",
            [](CreditDefaultSwap* self,
               Protection::Side side,
               Real notional,
               Rate spread,
               const Schedule& schedule,
               BusinessDayConvention payment_convention,
               const DayCounter& day_counter,
               bool settles_accrual,
               bool pays_at_default_time) {
                new (self) CreditDefaultSwap(side,
                                             notional,
                                             spread,
                                             schedule,
                                             payment_convention,
                                             day_counter,
                                             settles_accrual,
                                             pays_at_default_time);
            },
            nb::arg("side"),
            nb::arg("notional"),
            nb::arg("spread"),
            nb::arg("schedule"),
            nb::arg("payment_convention"),
            nb::arg("day_counter"),
            nb::arg("settles_accrual") = true,
            nb::arg("pays_at_default_time") = true)
        .def("NPV", [](CreditDefaultSwap& cds) { return cds.NPV(); })
        .def("fair_spread",
             [](CreditDefaultSwap& cds) { return cds.fairSpread(); })
        .def("fair_upfront",
             [](CreditDefaultSwap& cds) { return cds.fairUpfront(); })
        .def("coupon_leg_NPV",
             [](CreditDefaultSwap& cds) { return cds.couponLegNPV(); })
        .def("default_leg_NPV",
             [](CreditDefaultSwap& cds) { return cds.defaultLegNPV(); })
        .def("side", [](const CreditDefaultSwap& cds) { return cds.side(); })
        .def("notional",
             [](const CreditDefaultSwap& cds) { return cds.notional(); })
        .def("running_spread",
             [](const CreditDefaultSwap& cds) { return cds.runningSpread(); })
        .def("is_expired",
             [](const CreditDefaultSwap& cds) { return cds.isExpired(); })
        .def(
            "set_pricing_engine",
            [](CreditDefaultSwap& cds,
               const Handle<DefaultProbabilityTermStructure>& probability,
               Real recovery_rate,
               const Handle<YieldTermStructure>& discount_curve) {
                cds.setPricingEngine(ext::make_shared<MidPointCdsEngine>(
                    probability, recovery_rate, discount_curve));
            },
            nb::arg("probability"),
            nb::arg("recovery_rate"),
            nb::arg("discount_curve"),
            "Attach MidPointCdsEngine.");

    m.def(
        "MidPointCdsEngine",
        [](const Handle<DefaultProbabilityTermStructure>& probability) {
            return probability;
        },
        nb::arg("probability"),
        "Factory alias: pass probability / recovery / discount to "
        "CreditDefaultSwap.set_pricing_engine(...).");
}
