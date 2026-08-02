#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>

#include <ql/cashflows/couponpricer.hpp>
#include <ql/exercise.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/option.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/null.hpp>

using namespace QuantLib;

namespace {

Handle<BlackVolTermStructure> make_black_constant_vol_handle(
    const Date& reference_date,
    const Calendar& calendar,
    Volatility volatility,
    const DayCounter& day_counter) {
    return Handle<BlackVolTermStructure>(ext::make_shared<BlackConstantVol>(
        reference_date, calendar, volatility, day_counter));
}

} // namespace

void bind_instruments(nb::module_& m) {
    nb::class_<Handle<BlackVolTermStructure>>(m, "BlackVolTermStructureHandle")
        .def(nb::init<>())
        .def("empty", &Handle<BlackVolTermStructure>::empty);

    m.def("BlackConstantVol",
          &make_black_constant_vol_handle,
          nb::arg("reference_date"),
          nb::arg("calendar"),
          nb::arg("volatility"),
          nb::arg("day_counter"));

    nb::class_<BlackScholesMertonProcess>(m, "BlackScholesMertonProcess")
        .def(nb::init<const Handle<Quote>&,
                      const Handle<YieldTermStructure>&,
                      const Handle<YieldTermStructure>&,
                      const Handle<BlackVolTermStructure>&>(),
             nb::arg("x0"),
             nb::arg("dividend_ts"),
             nb::arg("risk_free_ts"),
             nb::arg("black_vol_ts"));

    nb::enum_<Option::Type>(m, "OptionType")
        .value("Put", Option::Put)
        .value("Call", Option::Call);

    nb::class_<PlainVanillaPayoff>(m, "PlainVanillaPayoff")
        .def(nb::init<Option::Type, Real>(), nb::arg("type"), nb::arg("strike"))
        .def("strike", [](const PlainVanillaPayoff& p) { return p.strike(); })
        .def("option_type",
             [](const PlainVanillaPayoff& p) { return p.optionType(); });

    nb::class_<EuropeanExercise>(m, "EuropeanExercise")
        .def(nb::init<const Date&>(), nb::arg("date"))
        .def("last_date",
             [](const EuropeanExercise& e) { return e.lastDate(); });

    nb::class_<EuropeanOption>(m, "EuropeanOption")
        .def(
            "__init__",
            [](EuropeanOption* self,
               const PlainVanillaPayoff& payoff,
               const EuropeanExercise& exercise) {
                new (self) EuropeanOption(
                    ext::make_shared<PlainVanillaPayoff>(payoff),
                    ext::make_shared<EuropeanExercise>(exercise));
            },
            nb::arg("payoff"),
            nb::arg("exercise"))
        .def("NPV", [](EuropeanOption& opt) { return opt.NPV(); })
        .def("delta", [](EuropeanOption& opt) { return opt.delta(); })
        .def("gamma", [](EuropeanOption& opt) { return opt.gamma(); })
        .def("vega", [](EuropeanOption& opt) { return opt.vega(); })
        .def(
            "implied_volatility",
            [](EuropeanOption& opt,
               Real target_price,
               const ext::shared_ptr<BlackScholesMertonProcess>& process,
               Real accuracy,
               Size max_evaluations,
               Volatility min_vol,
               Volatility max_vol) {
                return opt.impliedVolatility(target_price,
                                             process,
                                             accuracy,
                                             max_evaluations,
                                             min_vol,
                                             max_vol);
            },
            nb::arg("target_price"),
            nb::arg("process"),
            nb::arg("accuracy") = 1.0e-4,
            nb::arg("max_evaluations") = 100,
            nb::arg("min_vol") = 1.0e-7,
            nb::arg("max_vol") = 4.0)
        .def(
            "set_pricing_engine",
            [](EuropeanOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process) {
                opt.setPricingEngine(
                    ext::make_shared<AnalyticEuropeanEngine>(process));
            },
            nb::arg("process"))
        .def(
            "set_mc_pricing_engine",
            [](EuropeanOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process,
               Size time_steps,
               Size required_samples,
               unsigned long seed,
               bool antithetic,
               bool brownian_bridge) {
                ext::shared_ptr<PricingEngine> engine =
                    MakeMCEuropeanEngine<PseudoRandom>(process)
                        .withSteps(time_steps)
                        .withSamples(required_samples)
                        .withSeed(seed)
                        .withAntitheticVariate(antithetic)
                        .withBrownianBridge(brownian_bridge);
                opt.setPricingEngine(engine);
            },
            nb::arg("process"),
            nb::arg("time_steps"),
            nb::arg("required_samples"),
            nb::arg("seed") = 42UL,
            nb::arg("antithetic") = true,
            nb::arg("brownian_bridge") = false);

    m.def(
        "AnalyticEuropeanEngine",
        [](const ext::shared_ptr<BlackScholesMertonProcess>& process) {
            return process;
        },
        nb::arg("process"));

    // Bonds (standalone; Bond/Instrument use MI via LazyObject).
    nb::class_<FixedRateBond>(m, "FixedRateBond")
        .def(
            "__init__",
            [](FixedRateBond* self,
               Natural settlement_days,
               Real face_amount,
               const Schedule& schedule,
               const std::vector<Rate>& coupons,
               const DayCounter& accrual_day_counter,
               BusinessDayConvention payment_convention,
               Real redemption,
               const Date& issue_date) {
                new (self) FixedRateBond(settlement_days,
                                         face_amount,
                                         schedule,
                                         coupons,
                                         accrual_day_counter,
                                         payment_convention,
                                         redemption,
                                         issue_date);
            },
            nb::arg("settlement_days"),
            nb::arg("face_amount"),
            nb::arg("schedule"),
            nb::arg("coupons"),
            nb::arg("accrual_day_counter"),
            nb::arg("payment_convention") = Following,
            nb::arg("redemption") = 100.0,
            nb::arg("issue_date") = Date())
        .def("NPV", [](FixedRateBond& b) { return b.NPV(); })
        .def("clean_price", [](FixedRateBond& b) { return b.cleanPrice(); })
        .def("dirty_price", [](FixedRateBond& b) { return b.dirtyPrice(); })
        .def("settlement_date",
             [](const FixedRateBond& b) { return b.settlementDate(); })
        .def("maturity_date",
             [](const FixedRateBond& b) { return b.maturityDate(); })
        .def(
            "set_pricing_engine",
            [](FixedRateBond& b, const Handle<YieldTermStructure>& discount_curve) {
                b.setPricingEngine(
                    ext::make_shared<DiscountingBondEngine>(discount_curve));
            },
            nb::arg("discount_curve"));

    // Zero-coupon bond (standalone; Bond/Instrument use MI via LazyObject).
    nb::class_<ZeroCouponBond>(m, "ZeroCouponBond")
        .def(
            "__init__",
            [](ZeroCouponBond* self,
               Natural settlement_days,
               const Calendar& calendar,
               Real face_amount,
               const Date& maturity_date,
               BusinessDayConvention payment_convention,
               Real redemption,
               const Date& issue_date) {
                new (self) ZeroCouponBond(settlement_days,
                                          calendar,
                                          face_amount,
                                          maturity_date,
                                          payment_convention,
                                          redemption,
                                          issue_date);
            },
            nb::arg("settlement_days"),
            nb::arg("calendar"),
            nb::arg("face_amount"),
            nb::arg("maturity_date"),
            nb::arg("payment_convention") = Following,
            nb::arg("redemption") = 100.0,
            nb::arg("issue_date") = Date())
        .def("NPV", [](ZeroCouponBond& b) { return b.NPV(); })
        .def("clean_price", [](ZeroCouponBond& b) { return b.cleanPrice(); })
        .def("dirty_price", [](ZeroCouponBond& b) { return b.dirtyPrice(); })
        .def("settlement_date",
             [](const ZeroCouponBond& b) { return b.settlementDate(); })
        .def("maturity_date",
             [](const ZeroCouponBond& b) { return b.maturityDate(); })
        .def(
            "set_pricing_engine",
            [](ZeroCouponBond& b, const Handle<YieldTermStructure>& discount_curve) {
                b.setPricingEngine(
                    ext::make_shared<DiscountingBondEngine>(discount_curve));
            },
            nb::arg("discount_curve"));

    // Floating-rate bond (standalone; Bond/Instrument use MI via LazyObject).
    // set_pricing_engine attaches DiscountingBondEngine and a BlackIborCouponPricer.
    nb::class_<FloatingRateBond>(m, "FloatingRateBond")
        .def(
            "__init__",
            [](FloatingRateBond* self,
               Natural settlement_days,
               Real face_amount,
               const Schedule& schedule,
               const ext::shared_ptr<IborIndex>& ibor_index,
               const DayCounter& accrual_day_counter,
               BusinessDayConvention payment_convention,
               Natural fixing_days,
               const std::vector<Real>& gearings,
               const std::vector<Spread>& spreads,
               const std::vector<Rate>& caps,
               const std::vector<Rate>& floors,
               bool in_arrears,
               Real redemption,
               const Date& issue_date) {
                const Natural ql_fixing_days =
                    (fixing_days == 0) ? Null<Natural>() : fixing_days;
                const std::vector<Real> ql_gearings =
                    gearings.empty() ? std::vector<Real>{1.0} : gearings;
                const std::vector<Spread> ql_spreads =
                    spreads.empty() ? std::vector<Spread>{0.0} : spreads;
                new (self) FloatingRateBond(settlement_days,
                                            face_amount,
                                            schedule,
                                            ibor_index,
                                            accrual_day_counter,
                                            payment_convention,
                                            ql_fixing_days,
                                            ql_gearings,
                                            ql_spreads,
                                            caps,
                                            floors,
                                            in_arrears,
                                            redemption,
                                            issue_date);
            },
            nb::arg("settlement_days"),
            nb::arg("face_amount"),
            nb::arg("schedule"),
            nb::arg("ibor_index"),
            nb::arg("accrual_day_counter"),
            nb::arg("payment_convention") = Following,
            nb::arg("fixing_days") = 0,
            nb::arg("gearings") = std::vector<Real>(),
            nb::arg("spreads") = std::vector<Spread>(),
            nb::arg("caps") = std::vector<Rate>(),
            nb::arg("floors") = std::vector<Rate>(),
            nb::arg("in_arrears") = false,
            nb::arg("redemption") = 100.0,
            nb::arg("issue_date") = Date())
        .def("NPV", [](FloatingRateBond& b) { return b.NPV(); })
        .def("clean_price", [](FloatingRateBond& b) { return b.cleanPrice(); })
        .def("dirty_price", [](FloatingRateBond& b) { return b.dirtyPrice(); })
        .def("settlement_date",
             [](const FloatingRateBond& b) { return b.settlementDate(); })
        .def("maturity_date",
             [](const FloatingRateBond& b) { return b.maturityDate(); })
        .def(
            "set_pricing_engine",
            [](FloatingRateBond& b,
               const Handle<YieldTermStructure>& discount_curve) {
                b.setPricingEngine(
                    ext::make_shared<DiscountingBondEngine>(discount_curve));
                setCouponPricer(b.cashflows(),
                                ext::make_shared<BlackIborCouponPricer>());
            },
            nb::arg("discount_curve"),
            "Attach DiscountingBondEngine and BlackIborCouponPricer on cashflows.");

    nb::enum_<Swap::Type>(m, "SwapType")
        .value("Receiver", Swap::Receiver)
        .value("Payer", Swap::Payer);

    nb::class_<VanillaSwap>(m, "VanillaSwap")
        .def(
            "__init__",
            [](VanillaSwap* self,
               Swap::Type type,
               Real nominal,
               const Schedule& fixed_schedule,
               Rate fixed_rate,
               const DayCounter& fixed_day_count,
               const Schedule& float_schedule,
               const ext::shared_ptr<IborIndex>& ibor_index,
               Spread spread,
               const DayCounter& floating_day_count) {
                new (self) VanillaSwap(type,
                                       nominal,
                                       fixed_schedule,
                                       fixed_rate,
                                       fixed_day_count,
                                       float_schedule,
                                       ibor_index,
                                       spread,
                                       floating_day_count);
            },
            nb::arg("type"),
            nb::arg("nominal"),
            nb::arg("fixed_schedule"),
            nb::arg("fixed_rate"),
            nb::arg("fixed_day_count"),
            nb::arg("float_schedule"),
            nb::arg("ibor_index"),
            nb::arg("spread"),
            nb::arg("floating_day_count"))
        .def("NPV", [](VanillaSwap& s) { return s.NPV(); })
        .def("fair_rate", [](VanillaSwap& s) { return s.fairRate(); })
        .def("fair_spread", [](VanillaSwap& s) { return s.fairSpread(); })
        .def(
            "set_pricing_engine",
            [](VanillaSwap& s, const Handle<YieldTermStructure>& discount_curve) {
                s.setPricingEngine(
                    ext::make_shared<DiscountingSwapEngine>(discount_curve));
            },
            nb::arg("discount_curve"));
}
