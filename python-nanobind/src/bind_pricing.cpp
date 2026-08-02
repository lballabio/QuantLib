#include "bindings.hpp"

#include <nanobind/ndarray.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <ql/exercise.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/forwardrateagreement.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/methods/lattices/binomialtree.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/position.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/date.hpp>

#include <cstdint>
#include <vector>

using namespace QuantLib;

namespace {

using PathArray = nb::ndarray<nb::numpy, double, nb::ndim<2>>;

PathArray simulate_gbm_paths(
    const ext::shared_ptr<BlackScholesMertonProcess>& process,
    Time length,
    Size time_steps,
    Size samples,
    unsigned long seed) {
    QL_REQUIRE(process, "null process");
    QL_REQUIRE(length > 0.0, "non-positive path length");
    QL_REQUIRE(time_steps > 0, "time_steps must be positive");
    QL_REQUIRE(samples > 0, "samples must be positive");

    using rsg_type = PseudoRandom::rsg_type;
    rsg_type rsg = PseudoRandom::make_sequence_generator(time_steps, seed);
    PathGenerator<rsg_type> generator(
        process, length, time_steps, rsg, /*brownianBridge=*/false);

    const size_t cols = static_cast<size_t>(time_steps) + 1;
    auto* data = new double[static_cast<size_t>(samples) * cols];
    for (Size i = 0; i < samples; ++i) {
        const Path& path = generator.next().value;
        QL_REQUIRE(path.length() == cols, "unexpected path length");
        for (Size j = 0; j < cols; ++j) {
            data[static_cast<size_t>(i) * cols + static_cast<size_t>(j)] =
                path[j];
        }
    }

    nb::capsule owner(data, [](void* p) noexcept {
        delete[] static_cast<double*>(p);
    });
    return PathArray(data, {static_cast<size_t>(samples), cols}, owner);
}

} // namespace

void bind_pricing(nb::module_& m) {
    nb::class_<AmericanExercise>(m, "AmericanExercise")
        .def(nb::init<const Date&, const Date&, bool>(),
             nb::arg("earliest_date"),
             nb::arg("latest_date"),
             nb::arg("payoff_at_expiry") = false)
        .def(nb::init<const Date&, bool>(),
             nb::arg("latest_date"),
             nb::arg("payoff_at_expiry") = false)
        .def("last_date",
             [](const AmericanExercise& e) { return e.lastDate(); });

    // Standalone VanillaOption — American (BAW default) plus tree/FD engines.
    // Not declared as a Python subclass of Instrument/OneAssetOption (MI).
    nb::class_<VanillaOption>(m, "VanillaOption")
        .def(
            "__init__",
            [](VanillaOption* self,
               const PlainVanillaPayoff& payoff,
               const AmericanExercise& exercise) {
                new (self) VanillaOption(
                    ext::make_shared<PlainVanillaPayoff>(payoff),
                    ext::make_shared<AmericanExercise>(exercise));
            },
            nb::arg("payoff"),
            nb::arg("exercise"))
        .def(
            "__init__",
            [](VanillaOption* self,
               const PlainVanillaPayoff& payoff,
               const EuropeanExercise& exercise) {
                new (self) VanillaOption(
                    ext::make_shared<PlainVanillaPayoff>(payoff),
                    ext::make_shared<EuropeanExercise>(exercise));
            },
            nb::arg("payoff"),
            nb::arg("exercise"))
        .def("NPV", [](VanillaOption& opt) { return opt.NPV(); })
        .def("delta", [](VanillaOption& opt) { return opt.delta(); })
        .def("gamma", [](VanillaOption& opt) { return opt.gamma(); })
        .def("vega", [](VanillaOption& opt) { return opt.vega(); })
        .def(
            "set_pricing_engine",
            [](VanillaOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process) {
                opt.setPricingEngine(
                    ext::make_shared<BaroneAdesiWhaleyApproximationEngine>(
                        process));
            },
            nb::arg("process"),
            "Attach Barone-Adesi-Whaley approximation engine (American).")
        .def(
            "set_binomial_pricing_engine",
            [](VanillaOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process,
               Size steps) {
                opt.setPricingEngine(
                    ext::make_shared<BinomialVanillaEngine<CoxRossRubinstein>>(
                        process, steps));
            },
            nb::arg("process"),
            nb::arg("steps") = 801,
            "Attach Cox-Ross-Rubinstein binomial tree engine.")
        .def(
            "set_fd_pricing_engine",
            [](VanillaOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process,
               Size t_grid,
               Size x_grid,
               Size damping_steps) {
                opt.setPricingEngine(ext::make_shared<FdBlackScholesVanillaEngine>(
                    process, t_grid, x_grid, damping_steps));
            },
            nb::arg("process"),
            nb::arg("t_grid") = 100,
            nb::arg("x_grid") = 100,
            nb::arg("damping_steps") = 0,
            "Attach FdBlackScholesVanillaEngine (NPV-only for v1; no grid export).");

    m.def(
        "BaroneAdesiWhaleyEngine",
        [](const ext::shared_ptr<BlackScholesMertonProcess>& process) {
            return process;
        },
        nb::arg("process"),
        "Factory alias: pass the returned process to VanillaOption.set_pricing_engine.");

    nb::enum_<Position::Type>(m, "Position")
        .value("Long", Position::Long)
        .value("Short", Position::Short);

    // ForwardRateAgreement is MI-heavy via Instrument/LazyObject — standalone.
    nb::class_<ForwardRateAgreement>(m, "ForwardRateAgreement")
        .def(
            "__init__",
            [](ForwardRateAgreement* self,
               const ext::shared_ptr<IborIndex>& index,
               const Date& value_date,
               Position::Type type,
               Rate strike_forward_rate,
               Real notional_amount,
               const Handle<YieldTermStructure>& discount_curve) {
                new (self) ForwardRateAgreement(index,
                                                value_date,
                                                type,
                                                strike_forward_rate,
                                                notional_amount,
                                                discount_curve);
            },
            nb::arg("index"),
            nb::arg("value_date"),
            nb::arg("type"),
            nb::arg("strike_forward_rate"),
            nb::arg("notional_amount"),
            nb::arg("discount_curve") = Handle<YieldTermStructure>())
        .def(
            "__init__",
            [](ForwardRateAgreement* self,
               const ext::shared_ptr<IborIndex>& index,
               const Date& value_date,
               const Date& maturity_date,
               Position::Type type,
               Rate strike_forward_rate,
               Real notional_amount,
               const Handle<YieldTermStructure>& discount_curve) {
                new (self) ForwardRateAgreement(index,
                                                value_date,
                                                maturity_date,
                                                type,
                                                strike_forward_rate,
                                                notional_amount,
                                                discount_curve);
            },
            nb::arg("index"),
            nb::arg("value_date"),
            nb::arg("maturity_date"),
            nb::arg("type"),
            nb::arg("strike_forward_rate"),
            nb::arg("notional_amount"),
            nb::arg("discount_curve") = Handle<YieldTermStructure>())
        .def("NPV", [](ForwardRateAgreement& fra) { return fra.NPV(); })
        .def("amount", [](const ForwardRateAgreement& fra) { return fra.amount(); })
        .def("forward_rate",
             [](const ForwardRateAgreement& fra) { return fra.forwardRate(); })
        .def("fixing_date",
             [](const ForwardRateAgreement& fra) { return fra.fixingDate(); });

    m.def("simulate_gbm_paths",
          &simulate_gbm_paths,
          nb::arg("process"),
          nb::arg("length"),
          nb::arg("time_steps"),
          nb::arg("samples"),
          nb::arg("seed") = 42UL,
          "Simulate GBM paths under a BlackScholesMertonProcess.\n"
          "Returns a NumPy array of shape (samples, time_steps+1).");
}
