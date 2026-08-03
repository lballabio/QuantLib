#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/exercise.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/instruments/averagetype.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/barriertype.hpp>
#include <ql/instruments/capfloor.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/option.hpp>
#include <ql/pricingengines/asian/analytic_cont_geom_av_price.hpp>
#include <ql/pricingengines/asian/analytic_discr_geom_av_price.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/period.hpp>
#include <ql/time/schedule.hpp>

using namespace QuantLib;

void bind_experimental(nb::module_& m) {
    // --- Barrier options (standalone; OneAssetOption uses MI) ---------------
    nb::enum_<Barrier::Type>(m, "BarrierType")
        .value("DownIn", Barrier::DownIn)
        .value("UpIn", Barrier::UpIn)
        .value("DownOut", Barrier::DownOut)
        .value("UpOut", Barrier::UpOut);

    // --- Asian options (standalone; OneAssetOption uses MI) -----------------
    nb::enum_<Average::Type>(m, "AverageType")
        .value("Arithmetic", Average::Arithmetic)
        .value("Geometric", Average::Geometric);

    nb::class_<ContinuousAveragingAsianOption>(m, "ContinuousAveragingAsianOption")
        .def(
            "__init__",
            [](ContinuousAveragingAsianOption* self,
               Average::Type average_type,
               const PlainVanillaPayoff& payoff,
               const EuropeanExercise& exercise) {
                new (self) ContinuousAveragingAsianOption(
                    average_type,
                    ext::make_shared<PlainVanillaPayoff>(payoff),
                    ext::make_shared<EuropeanExercise>(exercise));
            },
            nb::arg("average_type"),
            nb::arg("payoff"),
            nb::arg("exercise"))
        .def("NPV",
             [](ContinuousAveragingAsianOption& opt) { return opt.NPV(); })
        .def("delta",
             [](ContinuousAveragingAsianOption& opt) { return opt.delta(); })
        .def("gamma",
             [](ContinuousAveragingAsianOption& opt) { return opt.gamma(); })
        .def(
            "set_pricing_engine",
            [](ContinuousAveragingAsianOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process) {
                opt.setPricingEngine(
                    ext::make_shared<
                        AnalyticContinuousGeometricAveragePriceAsianEngine>(
                        process));
            },
            nb::arg("process"),
            "Attach AnalyticContinuousGeometricAveragePriceAsianEngine.");

    nb::class_<DiscreteAveragingAsianOption>(m, "DiscreteAveragingAsianOption")
        .def(
            "__init__",
            [](DiscreteAveragingAsianOption* self,
               Average::Type average_type,
               Real running_accumulator,
               Size past_fixings,
               const std::vector<Date>& fixing_dates,
               const PlainVanillaPayoff& payoff,
               const EuropeanExercise& exercise) {
                new (self) DiscreteAveragingAsianOption(
                    average_type,
                    running_accumulator,
                    past_fixings,
                    fixing_dates,
                    ext::make_shared<PlainVanillaPayoff>(payoff),
                    ext::make_shared<EuropeanExercise>(exercise));
            },
            nb::arg("average_type"),
            nb::arg("running_accumulator"),
            nb::arg("past_fixings"),
            nb::arg("fixing_dates"),
            nb::arg("payoff"),
            nb::arg("exercise"))
        .def("NPV",
             [](DiscreteAveragingAsianOption& opt) { return opt.NPV(); })
        .def(
            "set_pricing_engine",
            [](DiscreteAveragingAsianOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process) {
                opt.setPricingEngine(
                    ext::make_shared<
                        AnalyticDiscreteGeometricAveragePriceAsianEngine>(
                        process));
            },
            nb::arg("process"),
            "Attach AnalyticDiscreteGeometricAveragePriceAsianEngine.");

    m.def(
        "AnalyticContinuousGeometricAveragePriceAsianEngine",
        [](const ext::shared_ptr<BlackScholesMertonProcess>& process) {
            return process;
        },
        nb::arg("process"),
        "Factory alias for ContinuousAveragingAsianOption.set_pricing_engine.");

    m.def(
        "AnalyticDiscreteGeometricAveragePriceAsianEngine",
        [](const ext::shared_ptr<BlackScholesMertonProcess>& process) {
            return process;
        },
        nb::arg("process"),
        "Factory alias for DiscreteAveragingAsianOption.set_pricing_engine.");

    nb::class_<BarrierOption>(m, "BarrierOption")
        .def(
            "__init__",
            [](BarrierOption* self,
               Barrier::Type barrier_type,
               Real barrier,
               Real rebate,
               const PlainVanillaPayoff& payoff,
               const EuropeanExercise& exercise) {
                new (self) BarrierOption(
                    barrier_type,
                    barrier,
                    rebate,
                    ext::make_shared<PlainVanillaPayoff>(payoff),
                    ext::make_shared<EuropeanExercise>(exercise));
            },
            nb::arg("barrier_type"),
            nb::arg("barrier"),
            nb::arg("rebate"),
            nb::arg("payoff"),
            nb::arg("exercise"))
        .def("NPV", [](BarrierOption& opt) { return opt.NPV(); })
        .def("delta", [](BarrierOption& opt) { return opt.delta(); })
        .def("gamma", [](BarrierOption& opt) { return opt.gamma(); })
        .def("vega", [](BarrierOption& opt) { return opt.vega(); })
        .def(
            "set_pricing_engine",
            [](BarrierOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process) {
                opt.setPricingEngine(
                    ext::make_shared<AnalyticBarrierEngine>(process));
            },
            nb::arg("process"));

    m.def(
        "AnalyticBarrierEngine",
        [](const ext::shared_ptr<BlackScholesMertonProcess>& process) {
            return process;
        },
        nb::arg("process"),
        "Factory alias: pass the returned process to BarrierOption.set_pricing_engine.");

    // --- Cap / Floor + Black engine (standalone; CapFloor uses MI) ----------
    nb::enum_<CapFloor::Type>(m, "CapFloorType")
        .value("Cap", CapFloor::Cap)
        .value("Floor", CapFloor::Floor)
        .value("Collar", CapFloor::Collar);

    nb::class_<CapFloor>(m, "CapFloor")
        .def(
            "__init__",
            [](CapFloor* self,
               CapFloor::Type type,
               const Schedule& schedule,
               const ext::shared_ptr<IborIndex>& index,
               Rate strike,
               Real nominal,
               Natural fixing_days) {
                QL_REQUIRE(type == CapFloor::Cap || type == CapFloor::Floor,
                           "CapFloor constructor supports Cap or Floor");
                const BusinessDayConvention conv = index->businessDayConvention();
                Leg leg = IborLeg(schedule, index)
                              .withNotionals(nominal)
                              .withPaymentDayCounter(index->dayCounter())
                              .withPaymentAdjustment(conv)
                              .withFixingDays(fixing_days);
                new (self) CapFloor(type, leg, std::vector<Rate>(1, strike));
            },
            nb::arg("type"),
            nb::arg("schedule"),
            nb::arg("index"),
            nb::arg("strike"),
            nb::arg("nominal") = 100.0,
            nb::arg("fixing_days") = 2,
            "Build a Cap/Floor from an Ibor schedule (includes all caplets).")
        .def("NPV", [](CapFloor& cf) { return cf.NPV(); })
        .def(
            "atm_rate",
            [](const CapFloor& cf, const Handle<YieldTermStructure>& discount) {
                return cf.atmRate(**discount);
            },
            nb::arg("discount_curve"))
        .def("start_date",
             [](const CapFloor& cf) { return cf.startDate(); })
        .def("maturity_date",
             [](const CapFloor& cf) { return cf.maturityDate(); })
        .def("type", [](const CapFloor& cf) { return cf.type(); })
        .def(
            "set_pricing_engine",
            [](CapFloor& cf,
               const Handle<YieldTermStructure>& discount_curve,
               Volatility volatility,
               const DayCounter& day_counter,
               Real displacement) {
                cf.setPricingEngine(ext::make_shared<BlackCapFloorEngine>(
                    discount_curve, volatility, day_counter, displacement));
            },
            nb::arg("discount_curve"),
            nb::arg("volatility"),
            nb::arg("day_counter") = DayCounter(Actual365Fixed()),
            nb::arg("displacement") = 0.0);

    m.def(
        "make_cap",
        [](const Period& tenor,
           const ext::shared_ptr<IborIndex>& index,
           Rate strike,
           Real nominal,
           const Period& forward_start) {
            CapFloor cap = MakeCapFloor(CapFloor::Cap, tenor, index, strike,
                                        forward_start)
                               .withNominal(nominal);
            return cap;
        },
        nb::arg("tenor"),
        nb::arg("index"),
        nb::arg("strike"),
        nb::arg("nominal") = 100.0,
        nb::arg("forward_start") = Period(0, Days),
        "Build a standard Cap via QuantLib::MakeCapFloor.");

    m.def(
        "make_floor",
        [](const Period& tenor,
           const ext::shared_ptr<IborIndex>& index,
           Rate strike,
           Real nominal,
           const Period& forward_start) {
            CapFloor floor = MakeCapFloor(CapFloor::Floor, tenor, index, strike,
                                          forward_start)
                                 .withNominal(nominal);
            return floor;
        },
        nb::arg("tenor"),
        nb::arg("index"),
        nb::arg("strike"),
        nb::arg("nominal") = 100.0,
        nb::arg("forward_start") = Period(0, Days),
        "Build a standard Floor via QuantLib::MakeCapFloor.");

    m.def(
        "BlackCapFloorEngine",
        [](const Handle<YieldTermStructure>& discount_curve,
           Volatility volatility,
           const DayCounter& day_counter,
           Real displacement) {
            // Token bundle for documentation parity with AnalyticEuropeanEngine.
            // Prefer CapFloor.set_pricing_engine(discount_curve, volatility, ...).
            return discount_curve;
        },
        nb::arg("discount_curve"),
        nb::arg("volatility"),
        nb::arg("day_counter") = DayCounter(Actual365Fixed()),
        nb::arg("displacement") = 0.0,
        "Documentation alias — use CapFloor.set_pricing_engine instead.");
}
