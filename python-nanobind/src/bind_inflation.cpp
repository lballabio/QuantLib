#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <ql/handle.hpp>
#include <ql/indexes/inflation/euhicp.hpp>
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/interpolatedzeroinflationcurve.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/period.hpp>

using namespace QuantLib;

namespace {

using ZeroInflationHelper = BootstrapHelper<ZeroInflationTermStructure>;
using PiecewiseZeroInflationLinear =
    PiecewiseZeroInflationCurve<Linear>;

Handle<ZeroInflationTermStructure> make_zero_inflation_curve(
    const Date& reference_date,
    const std::vector<Date>& dates,
    const std::vector<Rate>& rates,
    Frequency frequency,
    const DayCounter& day_counter) {
    QL_REQUIRE(dates.size() == rates.size(), "dates/rates size mismatch");
    QL_REQUIRE(dates.size() >= 2, "need at least two zero-inflation nodes");
    return Handle<ZeroInflationTermStructure>(
        ext::make_shared<InterpolatedZeroInflationCurve<Linear>>(
            reference_date, dates, rates, frequency, day_counter));
}

Handle<ZeroInflationTermStructure> make_flat_zero_inflation_curve(
    const Date& reference_date,
    const Date& base_date,
    const Date& max_date,
    Rate rate,
    Frequency frequency,
    const DayCounter& day_counter) {
    QL_REQUIRE(max_date > base_date, "max_date must be after base_date");
    std::vector<Date> dates = {base_date, max_date};
    std::vector<Rate> rates = {rate, rate};
    return Handle<ZeroInflationTermStructure>(
        ext::make_shared<InterpolatedZeroInflationCurve<Linear>>(
            reference_date, dates, rates, frequency, day_counter));
}

Handle<ZeroInflationTermStructure> make_piecewise_zero_inflation_curve(
    const Date& reference_date,
    const Date& base_date,
    Frequency frequency,
    const DayCounter& day_counter,
    const std::vector<ext::shared_ptr<ZeroInflationHelper>>& helpers) {
    QL_REQUIRE(!helpers.empty(), "empty zero-inflation helper list");
    return Handle<ZeroInflationTermStructure>(
        ext::make_shared<PiecewiseZeroInflationLinear>(
            reference_date, base_date, frequency, day_counter, helpers));
}

} // namespace

void bind_inflation(nb::module_& m) {
    nb::enum_<CPI::InterpolationType>(m, "CPIInterpolationType")
        .value("Flat", CPI::Flat)
        .value("Linear", CPI::Linear);

    nb::class_<Handle<ZeroInflationTermStructure>>(
        m, "ZeroInflationTermStructureHandle")
        .def(nb::init<>())
        .def("empty", &Handle<ZeroInflationTermStructure>::empty)
        .def(
            "zero_rate",
            [](const Handle<ZeroInflationTermStructure>& h,
               const Date& d,
               bool extrapolate) { return h->zeroRate(d, extrapolate); },
            nb::arg("date"),
            nb::arg("extrapolate") = false)
        .def("base_date",
             [](const Handle<ZeroInflationTermStructure>& h) {
                 return h->baseDate();
             })
        .def("max_date",
             [](const Handle<ZeroInflationTermStructure>& h) {
                 return h->maxDate();
             })
        .def("frequency",
             [](const Handle<ZeroInflationTermStructure>& h) {
                 return h->frequency();
             })
        .def("reference_date",
             [](const Handle<ZeroInflationTermStructure>& h) {
                 return h->referenceDate();
             });

    nb::class_<RelinkableHandle<ZeroInflationTermStructure>>(
        m, "RelinkableZeroInflationTermStructureHandle")
        .def(nb::init<>())
        .def("empty",
             [](const RelinkableHandle<ZeroInflationTermStructure>& h) {
                 return h.empty();
             })
        .def(
            "link_to",
            [](RelinkableHandle<ZeroInflationTermStructure>& h,
               const Handle<ZeroInflationTermStructure>& target) {
                h.linkTo(target.currentLink());
            },
            nb::arg("handle"))
        .def("as_handle",
             [](const RelinkableHandle<ZeroInflationTermStructure>& h) {
                 return Handle<ZeroInflationTermStructure>(h);
             })
        .def(
            "zero_rate",
            [](const RelinkableHandle<ZeroInflationTermStructure>& h,
               const Date& d,
               bool extrapolate) { return h->zeroRate(d, extrapolate); },
            nb::arg("date"),
            nb::arg("extrapolate") = false);

    // ZeroInflationIndex is MI-heavy — opaque shared_ptr holder.
    nb::class_<ZeroInflationIndex>(m, "ZeroInflationIndex")
        .def("name", [](const ZeroInflationIndex& i) { return i.name(); })
        .def("frequency",
             [](const ZeroInflationIndex& i) { return i.frequency(); })
        .def("availability_lag",
             [](const ZeroInflationIndex& i) { return i.availabilityLag(); })
        .def("last_fixing_date",
             [](const ZeroInflationIndex& i) { return i.lastFixingDate(); })
        .def(
            "add_fixing",
            [](ZeroInflationIndex& i,
               const Date& fixing_date,
               Rate fixing,
               bool force) { i.addFixing(fixing_date, fixing, force); },
            nb::arg("fixing_date"),
            nb::arg("fixing"),
            nb::arg("force_overwrite") = false)
        .def(
            "fixing",
            [](const ZeroInflationIndex& i,
               const Date& fixing_date,
               bool forecast_today) {
                return i.fixing(fixing_date, forecast_today);
            },
            nb::arg("fixing_date"),
            nb::arg("forecast_today") = false);

    m.def(
        "UKRPI",
        [](const Handle<ZeroInflationTermStructure>& h) {
            return ext::shared_ptr<ZeroInflationIndex>(ext::make_shared<UKRPI>(h));
        },
        nb::arg("handle") = Handle<ZeroInflationTermStructure>(),
        "Factory: UKRPI → ZeroInflationIndex.");

    m.def(
        "UKRPI",
        [](const RelinkableHandle<ZeroInflationTermStructure>& h) {
            return ext::shared_ptr<ZeroInflationIndex>(
                ext::make_shared<UKRPI>(Handle<ZeroInflationTermStructure>(h)));
        },
        nb::arg("handle"),
        "Factory: UKRPI(relinkable handle) → ZeroInflationIndex.");

    m.def(
        "EUHICP",
        [](const Handle<ZeroInflationTermStructure>& h) {
            return ext::shared_ptr<ZeroInflationIndex>(
                ext::make_shared<EUHICP>(h));
        },
        nb::arg("handle") = Handle<ZeroInflationTermStructure>(),
        "Factory: EUHICP → ZeroInflationIndex.");

    m.def("InterpolatedZeroInflationCurve",
          &make_zero_inflation_curve,
          nb::arg("reference_date"),
          nb::arg("dates"),
          nb::arg("rates"),
          nb::arg("frequency"),
          nb::arg("day_counter"),
          "Factory: InterpolatedZeroInflationCurve<Linear> → "
          "ZeroInflationTermStructureHandle.");

    m.def("FlatZeroInflationCurve",
          &make_flat_zero_inflation_curve,
          nb::arg("reference_date"),
          nb::arg("base_date"),
          nb::arg("max_date"),
          nb::arg("rate"),
          nb::arg("frequency"),
          nb::arg("day_counter"),
          "Factory: constant zero-inflation rate via a 2-node linear curve.");

    nb::class_<ZeroInflationHelper>(m, "ZeroInflationHelper");

    m.def(
        "ZeroCouponInflationSwapHelper",
        [](const Handle<Quote>& quote,
           const Period& observation_lag,
           const Date& maturity,
           const Calendar& calendar,
           BusinessDayConvention payment_convention,
           const DayCounter& day_counter,
           const ext::shared_ptr<ZeroInflationIndex>& index,
           CPI::InterpolationType observation_interpolation) {
            return ext::shared_ptr<ZeroInflationHelper>(
                ext::make_shared<ZeroCouponInflationSwapHelper>(
                    quote,
                    observation_lag,
                    maturity,
                    calendar,
                    payment_convention,
                    day_counter,
                    index,
                    observation_interpolation));
        },
        nb::arg("quote"),
        nb::arg("observation_lag"),
        nb::arg("maturity"),
        nb::arg("calendar"),
        nb::arg("payment_convention"),
        nb::arg("day_counter"),
        nb::arg("index"),
        nb::arg("observation_interpolation"),
        "Factory: ZeroCouponInflationSwapHelper → ZeroInflationHelper.");

    m.def("PiecewiseZeroInflationCurve",
          &make_piecewise_zero_inflation_curve,
          nb::arg("reference_date"),
          nb::arg("base_date"),
          nb::arg("frequency"),
          nb::arg("day_counter"),
          nb::arg("helpers"),
          "Factory: PiecewiseZeroInflationCurve<Linear> → "
          "ZeroInflationTermStructureHandle.");

    // ZeroCouponInflationSwap is Swap/Instrument (MI) — standalone wrapper.
    nb::class_<ZeroCouponInflationSwap>(m, "ZeroCouponInflationSwap")
        .def(
            "__init__",
            [](ZeroCouponInflationSwap* self,
               Swap::Type type,
               Real nominal,
               const Date& start_date,
               const Date& maturity,
               const Calendar& fix_calendar,
               BusinessDayConvention fix_convention,
               const DayCounter& day_counter,
               Rate fixed_rate,
               const ext::shared_ptr<ZeroInflationIndex>& index,
               const Period& observation_lag,
               CPI::InterpolationType observation_interpolation,
               bool adjust_inf_obs_dates) {
                new (self) ZeroCouponInflationSwap(type,
                                                   nominal,
                                                   start_date,
                                                   maturity,
                                                   fix_calendar,
                                                   fix_convention,
                                                   day_counter,
                                                   fixed_rate,
                                                   index,
                                                   observation_lag,
                                                   observation_interpolation,
                                                   adjust_inf_obs_dates);
            },
            nb::arg("type"),
            nb::arg("nominal"),
            nb::arg("start_date"),
            nb::arg("maturity"),
            nb::arg("fix_calendar"),
            nb::arg("fix_convention"),
            nb::arg("day_counter"),
            nb::arg("fixed_rate"),
            nb::arg("index"),
            nb::arg("observation_lag"),
            nb::arg("observation_interpolation"),
            nb::arg("adjust_inf_obs_dates") = false)
        .def("NPV", [](ZeroCouponInflationSwap& s) { return s.NPV(); })
        .def("fair_rate",
             [](ZeroCouponInflationSwap& s) { return s.fairRate(); })
        .def("fixed_rate",
             [](const ZeroCouponInflationSwap& s) { return s.fixedRate(); })
        .def("nominal",
             [](const ZeroCouponInflationSwap& s) { return s.nominal(); })
        .def("type", [](const ZeroCouponInflationSwap& s) { return s.type(); })
        .def("start_date",
             [](const ZeroCouponInflationSwap& s) { return s.startDate(); })
        .def("maturity_date",
             [](const ZeroCouponInflationSwap& s) { return s.maturityDate(); })
        .def("fixed_leg_NPV",
             [](ZeroCouponInflationSwap& s) { return s.fixedLegNPV(); })
        .def("inflation_leg_NPV",
             [](ZeroCouponInflationSwap& s) { return s.inflationLegNPV(); })
        .def("is_expired",
             [](const ZeroCouponInflationSwap& s) { return s.isExpired(); })
        .def(
            "set_pricing_engine",
            [](ZeroCouponInflationSwap& s,
               const Handle<YieldTermStructure>& discount_curve) {
                s.setPricingEngine(
                    ext::make_shared<DiscountingSwapEngine>(discount_curve));
            },
            nb::arg("discount_curve"),
            "Attach DiscountingSwapEngine (standard for ZCIS).");
}
