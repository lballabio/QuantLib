#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <ql/default.hpp>
#include <ql/handle.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/pricingengines/credit/isdacdsengine.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/termstructures/credit/defaultprobabilityhelpers.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/credit/interpolatedhazardratecurve.hpp>
#include <ql/termstructures/credit/piecewisedefaultcurve.hpp>
#include <ql/termstructures/credit/probabilitytraits.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/period.hpp>
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

Handle<DefaultProbabilityTermStructure> make_interpolated_hazard_rate_curve(
    const std::vector<Date>& dates,
    const std::vector<Rate>& hazard_rates,
    const DayCounter& day_counter,
    const Calendar& calendar) {
    QL_REQUIRE(dates.size() == hazard_rates.size(),
               "dates/hazard_rates size mismatch");
    QL_REQUIRE(dates.size() >= 2, "need at least two hazard-rate nodes");
    return Handle<DefaultProbabilityTermStructure>(
        ext::make_shared<InterpolatedHazardRateCurve<BackwardFlat>>(
            dates, hazard_rates, day_counter, calendar));
}

using PiecewiseHazardRateCurve =
    PiecewiseDefaultCurve<HazardRate, BackwardFlat>;

Handle<DefaultProbabilityTermStructure> make_piecewise_hazard_rate_curve(
    const Date& reference_date,
    const std::vector<ext::shared_ptr<DefaultProbabilityHelper>>& helpers,
    const DayCounter& day_counter) {
    QL_REQUIRE(!helpers.empty(), "empty default-probability helper list");
    return Handle<DefaultProbabilityTermStructure>(
        ext::make_shared<PiecewiseHazardRateCurve>(
            reference_date, helpers, day_counter));
}

} // namespace

void bind_credit(nb::module_& m) {
    nb::enum_<Protection::Side>(m, "ProtectionSide")
        .value("Buyer", Protection::Buyer)
        .value("Seller", Protection::Seller);

    nb::enum_<CreditDefaultSwap::PricingModel>(m, "CdsPricingModel")
        .value("Midpoint", CreditDefaultSwap::Midpoint)
        .value("ISDA", CreditDefaultSwap::ISDA);

    nb::enum_<IsdaCdsEngine::NumericalFix>(m, "IsdaCdsNumericalFix")
        .value("None", IsdaCdsEngine::None)
        .value("Taylor", IsdaCdsEngine::Taylor);

    nb::enum_<IsdaCdsEngine::AccrualBias>(m, "IsdaCdsAccrualBias")
        .value("HalfDayBias", IsdaCdsEngine::HalfDayBias)
        .value("NoBias", IsdaCdsEngine::NoBias);

    nb::enum_<IsdaCdsEngine::ForwardsInCouponPeriod>(
        m, "IsdaCdsForwardsInCouponPeriod")
        .value("Flat", IsdaCdsEngine::Flat)
        .value("Piecewise", IsdaCdsEngine::Piecewise);

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
        .def(
            "default_probability",
            [](const Handle<DefaultProbabilityTermStructure>& h,
               const Date& d,
               bool extrapolate) {
                return h->defaultProbability(d, extrapolate);
            },
            nb::arg("date"),
            nb::arg("extrapolate") = false)
        .def("reference_date",
             [](const Handle<DefaultProbabilityTermStructure>& h) {
                 return h->referenceDate();
             })
        .def("max_date",
             [](const Handle<DefaultProbabilityTermStructure>& h) {
                 return h->maxDate();
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

    m.def("InterpolatedHazardRateCurve",
          &make_interpolated_hazard_rate_curve,
          nb::arg("dates"),
          nb::arg("hazard_rates"),
          nb::arg("day_counter"),
          nb::arg("calendar") = Calendar(),
          "Factory: InterpolatedHazardRateCurve<BackwardFlat> → "
          "DefaultProbabilityTermStructureHandle.");

    // Default-probability helpers (opaque, like RateHelper).
    nb::class_<DefaultProbabilityHelper>(m, "DefaultProbabilityHelper");

    m.def(
        "SpreadCdsHelper",
        [](Rate running_spread,
           const Period& tenor,
           Integer settlement_days,
           const Calendar& calendar,
           Frequency frequency,
           BusinessDayConvention payment_convention,
           DateGeneration::Rule rule,
           const DayCounter& day_counter,
           Real recovery_rate,
           const Handle<YieldTermStructure>& discount_curve,
           bool settles_accrual,
           bool pays_at_default_time,
           CreditDefaultSwap::PricingModel model) {
            return ext::shared_ptr<DefaultProbabilityHelper>(
                ext::make_shared<SpreadCdsHelper>(running_spread,
                                                  tenor,
                                                  settlement_days,
                                                  calendar,
                                                  frequency,
                                                  payment_convention,
                                                  rule,
                                                  day_counter,
                                                  recovery_rate,
                                                  discount_curve,
                                                  settles_accrual,
                                                  pays_at_default_time,
                                                  Date(),
                                                  DayCounter(),
                                                  true,
                                                  model));
        },
        nb::arg("running_spread"),
        nb::arg("tenor"),
        nb::arg("settlement_days"),
        nb::arg("calendar"),
        nb::arg("frequency"),
        nb::arg("payment_convention"),
        nb::arg("rule"),
        nb::arg("day_counter"),
        nb::arg("recovery_rate"),
        nb::arg("discount_curve"),
        nb::arg("settles_accrual") = true,
        nb::arg("pays_at_default_time") = true,
        nb::arg("model") = CreditDefaultSwap::Midpoint,
        "Factory: SpreadCdsHelper → DefaultProbabilityHelper.");

    m.def("PiecewiseHazardRateCurve",
          &make_piecewise_hazard_rate_curve,
          nb::arg("reference_date"),
          nb::arg("helpers"),
          nb::arg("day_counter"),
          "Factory: PiecewiseDefaultCurve<HazardRate, BackwardFlat> → "
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
            "Attach MidPointCdsEngine.")
        .def(
            "set_isda_pricing_engine",
            [](CreditDefaultSwap& cds,
               const Handle<DefaultProbabilityTermStructure>& probability,
               Real recovery_rate,
               const Handle<YieldTermStructure>& discount_curve,
               IsdaCdsEngine::NumericalFix numerical_fix,
               IsdaCdsEngine::AccrualBias accrual_bias,
               IsdaCdsEngine::ForwardsInCouponPeriod forwards) {
                cds.setPricingEngine(ext::make_shared<IsdaCdsEngine>(
                    probability,
                    recovery_rate,
                    discount_curve,
                    std::nullopt,
                    numerical_fix,
                    accrual_bias,
                    forwards));
            },
            nb::arg("probability"),
            nb::arg("recovery_rate"),
            nb::arg("discount_curve"),
            nb::arg("numerical_fix") = IsdaCdsEngine::Taylor,
            nb::arg("accrual_bias") = IsdaCdsEngine::HalfDayBias,
            nb::arg("forwards_in_coupon_period") = IsdaCdsEngine::Piecewise,
            "Attach IsdaCdsEngine (ISDA Standard Model numerical settings).");

    m.def(
        "MidPointCdsEngine",
        [](const Handle<DefaultProbabilityTermStructure>& probability) {
            return probability;
        },
        nb::arg("probability"),
        "Factory alias: pass probability / recovery / discount to "
        "CreditDefaultSwap.set_pricing_engine(...).");

    m.def(
        "IsdaCdsEngine",
        [](const Handle<DefaultProbabilityTermStructure>& probability) {
            return probability;
        },
        nb::arg("probability"),
        "Factory alias: pass probability / recovery / discount to "
        "CreditDefaultSwap.set_isda_pricing_engine(...).");
}
