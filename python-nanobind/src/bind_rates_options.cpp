#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>

#include <ql/exercise.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/makeois.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/period.hpp>

using namespace QuantLib;

void bind_rates_options(nb::module_& m) {
    nb::enum_<Settlement::Type>(m, "SettlementType")
        .value("Physical", Settlement::Physical)
        .value("Cash", Settlement::Cash);

    nb::enum_<Settlement::Method>(m, "SettlementMethod")
        .value("PhysicalOTC", Settlement::PhysicalOTC)
        .value("PhysicalCleared", Settlement::PhysicalCleared)
        .value("CollateralizedCashPrice", Settlement::CollateralizedCashPrice)
        .value("ParYieldCurve", Settlement::ParYieldCurve);

    m.def(
        "make_vanilla_swap",
        [](const Period& swap_tenor,
           const ext::shared_ptr<IborIndex>& index,
           Rate fixed_rate,
           const Date& effective_date,
           const Period& fixed_leg_tenor,
           const DayCounter& fixed_day_count,
           Swap::Type type,
           Real nominal,
           Spread floating_spread) {
            MakeVanillaSwap maker(swap_tenor, index, fixed_rate);
            maker.withEffectiveDate(effective_date)
                .withFixedLegTenor(fixed_leg_tenor)
                .withType(type)
                .withNominal(nominal)
                .withFloatingLegSpread(floating_spread);
            if (fixed_day_count != DayCounter())
                maker.withFixedLegDayCount(fixed_day_count);
            return VanillaSwap(maker);
        },
        nb::arg("swap_tenor"),
        nb::arg("index"),
        nb::arg("fixed_rate"),
        nb::arg("effective_date"),
        nb::arg("fixed_leg_tenor") = Period(1, Years),
        nb::arg("fixed_day_count") = DayCounter(),
        nb::arg("type") = Swap::Payer,
        nb::arg("nominal") = 1.0,
        nb::arg("floating_spread") = 0.0,
        "Build a VanillaSwap via QuantLib MakeVanillaSwap (value copy).");

    // Swaption is MI-heavy (Option/Instrument) — standalone wrapper.
    nb::class_<Swaption>(m, "Swaption")
        .def(
            "__init__",
            [](Swaption* self,
               const VanillaSwap& swap,
               const EuropeanExercise& exercise,
               Settlement::Type delivery,
               Settlement::Method settlement_method) {
                new (self) Swaption(
                    ext::make_shared<VanillaSwap>(swap),
                    ext::make_shared<EuropeanExercise>(exercise),
                    delivery,
                    settlement_method);
            },
            nb::arg("swap"),
            nb::arg("exercise"),
            nb::arg("delivery") = Settlement::Physical,
            nb::arg("settlement_method") = Settlement::PhysicalOTC)
        .def("NPV", [](Swaption& s) { return s.NPV(); })
        .def("type", [](const Swaption& s) { return s.type(); })
        .def("settlement_type",
             [](const Swaption& s) { return s.settlementType(); })
        .def("settlement_method",
             [](const Swaption& s) { return s.settlementMethod(); })
        .def("is_expired", [](const Swaption& s) { return s.isExpired(); })
        .def(
            "set_pricing_engine",
            [](Swaption& s,
               const Handle<YieldTermStructure>& discount_curve,
               Volatility volatility,
               const DayCounter& day_counter,
               Real displacement) {
                s.setPricingEngine(ext::make_shared<BlackSwaptionEngine>(
                    discount_curve, volatility, day_counter, displacement));
            },
            nb::arg("discount_curve"),
            nb::arg("volatility"),
            nb::arg("day_counter") = DayCounter(Actual365Fixed()),
            nb::arg("displacement") = 0.0);

    m.def(
        "BlackSwaptionEngine",
        [](const Handle<YieldTermStructure>& discount_curve) {
            return discount_curve;
        },
        nb::arg("discount_curve"),
        "Factory alias: pass the curve (and volatility) to "
        "Swaption.set_pricing_engine(discount_curve, volatility).");

    // Overnight indexed swap (standalone; Swap/Instrument are MI-heavy).
    nb::class_<OvernightIndexedSwap>(m, "OvernightIndexedSwap")
        .def("NPV", [](OvernightIndexedSwap& s) { return s.NPV(); })
        .def("fair_rate", [](OvernightIndexedSwap& s) { return s.fairRate(); })
        .def("fair_spread",
             [](OvernightIndexedSwap& s) { return s.fairSpread(); })
        .def("fixed_leg_NPV",
             [](OvernightIndexedSwap& s) { return s.fixedLegNPV(); })
        .def("overnight_leg_NPV",
             [](OvernightIndexedSwap& s) { return s.overnightLegNPV(); })
        .def(
            "set_pricing_engine",
            [](OvernightIndexedSwap& s,
               const Handle<YieldTermStructure>& discount_curve) {
                s.setPricingEngine(
                    ext::make_shared<DiscountingSwapEngine>(discount_curve));
            },
            nb::arg("discount_curve"));

    m.def(
        "make_ois",
        [](const Period& swap_tenor,
           const ext::shared_ptr<OvernightIndex>& overnight_index,
           Rate fixed_rate,
           const Period& forward_start,
           Swap::Type type,
           Real nominal,
           Spread overnight_spread) {
            MakeOIS maker(swap_tenor, overnight_index, fixed_rate, forward_start);
            maker.withType(type)
                .withNominal(nominal)
                .withOvernightLegSpread(overnight_spread);
            return OvernightIndexedSwap(maker);
        },
        nb::arg("swap_tenor"),
        nb::arg("overnight_index"),
        nb::arg("fixed_rate"),
        nb::arg("forward_start") = Period(0, Days),
        nb::arg("type") = Swap::Payer,
        nb::arg("nominal") = 1.0,
        nb::arg("overnight_spread") = 0.0,
        "Build an OvernightIndexedSwap via QuantLib MakeOIS (value copy).");
}
