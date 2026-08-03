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
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/lattices/binomialtree.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/position.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/date.hpp>

#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

using namespace QuantLib;

namespace {

using PathArray = nb::ndarray<nb::numpy, double, nb::ndim<2>>;
using MeshArray = nb::ndarray<nb::numpy, double, nb::ndim<1>>;
using GridArray = nb::ndarray<nb::numpy, double, nb::ndim<2>>;

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

MeshArray locations_to_numpy(const std::vector<Real>& locations) {
    const size_t n = locations.size();
    auto* data = new double[n];
    for (size_t i = 0; i < n; ++i)
        data[i] = locations[i];
    nb::capsule owner(data, [](void* p) noexcept {
        delete[] static_cast<double*>(p);
    });
    return MeshArray(data, {n}, owner);
}

MeshArray locations_to_numpy(const Array& locations) {
    const size_t n = locations.size();
    auto* data = new double[n];
    for (size_t i = 0; i < n; ++i)
        data[i] = locations[i];
    nb::capsule owner(data, [](void* p) noexcept {
        delete[] static_cast<double*>(p);
    });
    return MeshArray(data, {n}, owner);
}

MeshArray uniform_1d_mesher_locations(Real start, Real end, Size size) {
    Uniform1dMesher mesher(start, end, size);
    return locations_to_numpy(mesher.locations());
}

MeshArray fdm_black_scholes_mesher_locations(
    Size size,
    const ext::shared_ptr<BlackScholesMertonProcess>& process,
    Time maturity,
    Real strike) {
    QL_REQUIRE(process, "null process");
    auto mesher = ext::make_shared<FdmBlackScholesMesher>(
        size, process, maturity, strike);
    // Composite exposes the 1D locations along direction 0.
    FdmMesherComposite composite(mesher);
    return locations_to_numpy(composite.locations(0));
}

GridArray fdm_black_scholes_values(
    const ext::shared_ptr<BlackScholesMertonProcess>& process,
    Real strike,
    Time maturity,
    Option::Type option_type,
    Size t_grid,
    Size x_grid,
    Size damping_steps) {
    QL_REQUIRE(process, "null process");
    QL_REQUIRE(maturity > 0.0, "non-positive maturity");
    QL_REQUIRE(t_grid > 0 && x_grid > 1, "invalid FD grid sizes");

    const Date today = process->riskFreeRate()->referenceDate();
    const DayCounter dc = process->riskFreeRate()->dayCounter();
    // Map the requested maturity onto a calendar date so exercise stopping
    // times and the PDE horizon stay consistent.
    const Date exercise_date = today + Integer(std::lround(maturity * 365.0));
    const Time T = process->time(exercise_date);
    QL_REQUIRE(T > 0.0, "exercise date must be after the process reference");

    auto payoff = ext::make_shared<PlainVanillaPayoff>(option_type, strike);
    auto equity_mesher =
        ext::make_shared<FdmBlackScholesMesher>(x_grid, process, T, strike);
    auto mesher = ext::make_shared<FdmMesherComposite>(equity_mesher);
    auto calculator = ext::make_shared<FdmLogInnerValue>(payoff, mesher, 0);
    auto exercise = ext::make_shared<EuropeanExercise>(exercise_date);

    auto conditions = FdmStepConditionComposite::vanillaComposite(
        DividendSchedule(), exercise, mesher, calculator, today, dc);

    FdmSolverDesc solver_desc = {mesher,
                                 FdmBoundaryConditionSet(),
                                 conditions,
                                 calculator,
                                 T,
                                 t_grid,
                                 damping_steps};

    auto solver = ext::make_shared<FdmBlackScholesSolver>(
        Handle<GeneralizedBlackScholesProcess>(process),
        strike,
        solver_desc);

    const Array ln_s = mesher->locations(0);
    const size_t n = ln_s.size();
    auto* data = new double[n * 2];
    for (size_t i = 0; i < n; ++i) {
        const Real spot = std::exp(ln_s[i]);
        data[i * 2] = spot;
        data[i * 2 + 1] = solver->valueAt(spot);
    }
    nb::capsule owner(data, [](void* p) noexcept {
        delete[] static_cast<double*>(p);
    });
    return GridArray(data, {n, static_cast<size_t>(2)}, owner);
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

    m.def("uniform_1d_mesher_locations",
          &uniform_1d_mesher_locations,
          nb::arg("start"),
          nb::arg("end"),
          nb::arg("size"),
          "Return Uniform1dMesher locations as a NumPy 1-D array.");

    m.def("fdm_black_scholes_mesher_locations",
          &fdm_black_scholes_mesher_locations,
          nb::arg("size"),
          nb::arg("process"),
          nb::arg("maturity"),
          nb::arg("strike"),
          "Return FdmBlackScholesMesher (ln-S) locations as a NumPy 1-D array.");

    m.def("fdm_black_scholes_values",
          &fdm_black_scholes_values,
          nb::arg("process"),
          nb::arg("strike"),
          nb::arg("maturity"),
          nb::arg("option_type") = Option::Call,
          nb::arg("t_grid") = 100,
          nb::arg("x_grid") = 100,
          nb::arg("damping_steps") = 0,
          "Solve a European vanilla on an FD Black–Scholes grid and return a "
          "NumPy array of shape (x_grid, 2) with columns [spot, value].");
}
