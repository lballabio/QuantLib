#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <ql/experimental/callablebonds/callablebond.hpp>
#include <ql/experimental/callablebonds/treecallablebondengine.hpp>
#include <ql/handle.hpp>
#include <ql/instruments/bond.hpp>
#include <ql/instruments/callabilityschedule.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>

using namespace QuantLib;

void bind_callable(nb::module_& m) {
    // --- Phase 23: callable / puttable bonds ---

    nb::enum_<Bond::Price::Type>(m, "BondPriceType")
        .value("Clean", Bond::Price::Clean)
        .value("Dirty", Bond::Price::Dirty);

    nb::class_<Bond::Price>(m, "BondPrice")
        .def(nb::init<Real, Bond::Price::Type>(),
             nb::arg("amount"),
             nb::arg("type") = Bond::Price::Clean)
        .def("amount", &Bond::Price::amount)
        .def("type", &Bond::Price::type)
        .def("is_valid", &Bond::Price::isValid);

    nb::enum_<Callability::Type>(m, "CallabilityType")
        .value("Call", Callability::Call)
        .value("Put", Callability::Put);

    // Callability is Event (MI) — opaque type + factory returning shared_ptr.
    nb::class_<Callability>(m, "Callability")
        .def("price",
             [](const Callability& c) { return c.price(); })
        .def("type", [](const Callability& c) { return c.type(); })
        .def("date", [](const Callability& c) { return c.date(); });

    m.def(
        "make_callability",
        [](const Bond::Price& price,
           Callability::Type type,
           const Date& date) {
            return ext::shared_ptr<Callability>(
                ext::make_shared<Callability>(price, type, date));
        },
        nb::arg("price"),
        nb::arg("type"),
        nb::arg("date"),
        "Factory: BondPrice + CallabilityType + date → Callability.");

    m.def(
        "make_callability",
        [](Real amount,
           Bond::Price::Type price_type,
           Callability::Type type,
           const Date& date) {
            return ext::shared_ptr<Callability>(ext::make_shared<Callability>(
                Bond::Price(amount, price_type), type, date));
        },
        nb::arg("amount"),
        nb::arg("price_type"),
        nb::arg("type"),
        nb::arg("date"),
        "Factory: amount + BondPriceType + CallabilityType + date → "
        "Callability.");

    // CallableFixedRateBond is Bond/Instrument (MI) — standalone wrapper.
    nb::class_<CallableFixedRateBond>(m, "CallableFixedRateBond")
        .def(
            "__init__",
            [](CallableFixedRateBond* self,
               Natural settlement_days,
               Real face_amount,
               const Schedule& schedule,
               const std::vector<Rate>& coupons,
               const DayCounter& accrual_day_counter,
               BusinessDayConvention payment_convention,
               Real redemption,
               const Date& issue_date,
               const CallabilitySchedule& put_call_schedule) {
                new (self) CallableFixedRateBond(settlement_days,
                                                 face_amount,
                                                 schedule,
                                                 coupons,
                                                 accrual_day_counter,
                                                 payment_convention,
                                                 redemption,
                                                 issue_date,
                                                 put_call_schedule);
            },
            nb::arg("settlement_days"),
            nb::arg("face_amount"),
            nb::arg("schedule"),
            nb::arg("coupons"),
            nb::arg("accrual_day_counter"),
            nb::arg("payment_convention") = Following,
            nb::arg("redemption") = 100.0,
            nb::arg("issue_date") = Date(),
            nb::arg("put_call_schedule") = CallabilitySchedule())
        .def("NPV", [](CallableFixedRateBond& b) { return b.NPV(); })
        .def("clean_price",
             [](CallableFixedRateBond& b) { return b.cleanPrice(); })
        .def("dirty_price",
             [](CallableFixedRateBond& b) { return b.dirtyPrice(); })
        .def("settlement_date",
             [](const CallableFixedRateBond& b) { return b.settlementDate(); })
        .def("maturity_date",
             [](const CallableFixedRateBond& b) { return b.maturityDate(); })
        .def(
            "set_tree_pricing_engine",
            [](CallableFixedRateBond& b,
               const ext::shared_ptr<HullWhite>& model,
               Size time_steps,
               const Handle<YieldTermStructure>& discount_curve) {
                b.setPricingEngine(
                    ext::make_shared<TreeCallableFixedRateBondEngine>(
                        model, time_steps, discount_curve));
            },
            nb::arg("model"),
            nb::arg("time_steps") = 240,
            nb::arg("discount_curve") = Handle<YieldTermStructure>(),
            "Attach TreeCallableFixedRateBondEngine on a HullWhite model.");

    nb::class_<CallableZeroCouponBond>(m, "CallableZeroCouponBond")
        .def(
            "__init__",
            [](CallableZeroCouponBond* self,
               Natural settlement_days,
               Real face_amount,
               const Calendar& calendar,
               const Date& maturity_date,
               const DayCounter& day_counter,
               BusinessDayConvention payment_convention,
               Real redemption,
               const Date& issue_date,
               const CallabilitySchedule& put_call_schedule) {
                new (self) CallableZeroCouponBond(settlement_days,
                                                  face_amount,
                                                  calendar,
                                                  maturity_date,
                                                  day_counter,
                                                  payment_convention,
                                                  redemption,
                                                  issue_date,
                                                  put_call_schedule);
            },
            nb::arg("settlement_days"),
            nb::arg("face_amount"),
            nb::arg("calendar"),
            nb::arg("maturity_date"),
            nb::arg("day_counter"),
            nb::arg("payment_convention") = Following,
            nb::arg("redemption") = 100.0,
            nb::arg("issue_date") = Date(),
            nb::arg("put_call_schedule") = CallabilitySchedule())
        .def("NPV", [](CallableZeroCouponBond& b) { return b.NPV(); })
        .def("clean_price",
             [](CallableZeroCouponBond& b) { return b.cleanPrice(); })
        .def("dirty_price",
             [](CallableZeroCouponBond& b) { return b.dirtyPrice(); })
        .def("settlement_date",
             [](const CallableZeroCouponBond& b) {
                 return b.settlementDate();
             })
        .def("maturity_date",
             [](const CallableZeroCouponBond& b) { return b.maturityDate(); })
        .def(
            "set_tree_pricing_engine",
            [](CallableZeroCouponBond& b,
               const ext::shared_ptr<HullWhite>& model,
               Size time_steps,
               const Handle<YieldTermStructure>& discount_curve) {
                b.setPricingEngine(
                    ext::make_shared<TreeCallableZeroCouponBondEngine>(
                        model, time_steps, discount_curve));
            },
            nb::arg("model"),
            nb::arg("time_steps") = 240,
            nb::arg("discount_curve") = Handle<YieldTermStructure>(),
            "Attach TreeCallableZeroCouponBondEngine on a HullWhite model.");
}
