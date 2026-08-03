#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <ql/exercise.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/makeois.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/pricingengines/swaption/fdhullwhiteswaptionengine.hpp>
#include <ql/pricingengines/swaption/gaussian1dswaptionengine.hpp>
#include <ql/pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <ql/pricingengines/swaption/treeswaptionengine.hpp>
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

    // HullWhite is MI-heavy (Vasicek + TermStructureConsistentModel) —
    // bind as a concrete type without exposing C++ bases.
    nb::class_<HullWhite>(m, "HullWhite")
        .def(nb::init<const Handle<YieldTermStructure>&, Real, Real>(),
             nb::arg("term_structure"),
             nb::arg("a") = 0.1,
             nb::arg("sigma") = 0.01);

    // Gsr / Gaussian1dModel are MI-heavy — standalone concrete wrapper.
    nb::class_<Gsr>(m, "Gsr")
        .def(
            "__init__",
            [](Gsr* self,
               const Handle<YieldTermStructure>& term_structure,
               const std::vector<Date>& vol_step_dates,
               const std::vector<Real>& volatilities,
               Real reversion,
               Real T) {
                new (self) Gsr(
                    term_structure, vol_step_dates, volatilities, reversion, T);
            },
            nb::arg("term_structure"),
            nb::arg("vol_step_dates"),
            nb::arg("volatilities"),
            nb::arg("reversion"),
            nb::arg("T") = 60.0)
        .def(
            "__init__",
            [](Gsr* self,
               const Handle<YieldTermStructure>& term_structure,
               const std::vector<Date>& vol_step_dates,
               const std::vector<Real>& volatilities,
               const std::vector<Real>& reversions,
               Real T) {
                new (self) Gsr(
                    term_structure, vol_step_dates, volatilities, reversions, T);
            },
            nb::arg("term_structure"),
            nb::arg("vol_step_dates"),
            nb::arg("volatilities"),
            nb::arg("reversions"),
            nb::arg("T") = 60.0)
        .def(
            "zerobond",
            [](const Gsr& model, Time maturity, Time t, Real y) {
                return model.zerobond(maturity, t, y);
            },
            nb::arg("maturity"),
            nb::arg("t") = 0.0,
            nb::arg("y") = 0.0,
            "Gaussian1dModel::zerobond(T, t, y).")
        .def("numeraire_time",
             [](const Gsr& model) { return model.numeraireTime(); });

    nb::class_<BermudanExercise>(m, "BermudanExercise")
        .def(nb::init<const std::vector<Date>&, bool>(),
             nb::arg("dates"),
             nb::arg("payoff_at_expiry") = false)
        .def("dates",
             [](const BermudanExercise& e) { return e.dates(); })
        .def("last_date",
             [](const BermudanExercise& e) { return e.lastDate(); });

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
        .def(
            "__init__",
            [](Swaption* self,
               const VanillaSwap& swap,
               const BermudanExercise& exercise,
               Settlement::Type delivery,
               Settlement::Method settlement_method) {
                new (self) Swaption(
                    ext::make_shared<VanillaSwap>(swap),
                    ext::make_shared<BermudanExercise>(exercise),
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
            nb::arg("displacement") = 0.0,
            "Attach BlackSwaptionEngine (European).")
        .def(
            "set_tree_pricing_engine",
            [](Swaption& s,
               const ext::shared_ptr<HullWhite>& model,
               Size time_steps) {
                s.setPricingEngine(
                    ext::make_shared<TreeSwaptionEngine>(model, time_steps));
            },
            nb::arg("model"),
            nb::arg("time_steps") = 50,
            "Attach TreeSwaptionEngine on a HullWhite model (Bermudan/European).")
        .def(
            "set_jamshidian_pricing_engine",
            [](Swaption& s, const ext::shared_ptr<HullWhite>& model) {
                s.setPricingEngine(
                    ext::make_shared<JamshidianSwaptionEngine>(model));
            },
            nb::arg("model"),
            "Attach JamshidianSwaptionEngine on a HullWhite model (European).")
        .def(
            "set_gaussian1d_pricing_engine",
            [](Swaption& s,
               const ext::shared_ptr<Gsr>& model,
               int integration_points,
               Real stddevs,
               bool extrapolate_payoff,
               bool flat_payoff_extrapolation) {
                s.setPricingEngine(ext::make_shared<Gaussian1dSwaptionEngine>(
                    model,
                    integration_points,
                    stddevs,
                    extrapolate_payoff,
                    flat_payoff_extrapolation));
            },
            nb::arg("model"),
            nb::arg("integration_points") = 64,
            nb::arg("stddevs") = 7.0,
            nb::arg("extrapolate_payoff") = true,
            nb::arg("flat_payoff_extrapolation") = false,
            "Attach Gaussian1dSwaptionEngine on a Gsr model.")
        .def(
            "set_fd_hullwhite_pricing_engine",
            [](Swaption& s,
               const ext::shared_ptr<HullWhite>& model,
               Size t_grid,
               Size x_grid,
               Size damping_steps) {
                s.setPricingEngine(ext::make_shared<FdHullWhiteSwaptionEngine>(
                    model, t_grid, x_grid, damping_steps));
            },
            nb::arg("model"),
            nb::arg("t_grid") = 100,
            nb::arg("x_grid") = 100,
            nb::arg("damping_steps") = 0,
            "Attach FdHullWhiteSwaptionEngine (Bermudan/European).");

    m.def(
        "BlackSwaptionEngine",
        [](const Handle<YieldTermStructure>& discount_curve) {
            return discount_curve;
        },
        nb::arg("discount_curve"),
        "Factory alias: pass the curve (and volatility) to "
        "Swaption.set_pricing_engine(discount_curve, volatility).");

    m.def(
        "TreeSwaptionEngine",
        [](const ext::shared_ptr<HullWhite>& model) { return model; },
        nb::arg("model"),
        "Factory alias: pass model to Swaption.set_tree_pricing_engine.");

    m.def(
        "Gaussian1dSwaptionEngine",
        [](const ext::shared_ptr<Gsr>& model) { return model; },
        nb::arg("model"),
        "Factory alias: pass model to Swaption.set_gaussian1d_pricing_engine.");

    m.def(
        "FdHullWhiteSwaptionEngine",
        [](const ext::shared_ptr<HullWhite>& model) { return model; },
        nb::arg("model"),
        "Factory alias: pass model to Swaption.set_fd_hullwhite_pricing_engine.");

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
