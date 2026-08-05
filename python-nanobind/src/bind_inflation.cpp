#include "bindings.hpp"

#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <ql/cashflows/yoyinflationcoupon.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/inflation/euhicp.hpp>
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/experimental/inflation/cpicapfloorengines.hpp>
#include <ql/experimental/inflation/cpicapfloortermpricesurface.hpp>
#include <ql/instruments/bonds/cpibond.hpp>
#include <ql/instruments/cpicapfloor.hpp>
#include <ql/instruments/cpiswap.hpp>
#include <ql/instruments/inflationcapfloor.hpp>
#include <ql/instruments/makeyoyinflationcapfloor.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/instruments/yearonyearinflationswap.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/option.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/pricingengines/inflation/inflationcapfloorengines.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/null.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/interpolatedyoyinflationcurve.hpp>
#include <ql/termstructures/inflation/interpolatedzeroinflationcurve.hpp>
#include <ql/termstructures/inflation/piecewiseyoyinflationcurve.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
#include <ql/termstructures/inflation/seasonality.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/volatility/inflation/yoyinflationoptionletvolatilitystructure.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/period.hpp>
#include <ql/time/schedule.hpp>

#include <optional>
#include <string>

using namespace QuantLib;

namespace {

using ZeroInflationHelper = BootstrapHelper<ZeroInflationTermStructure>;
using YoYInflationHelper = BootstrapHelper<YoYInflationTermStructure>;
using PiecewiseZeroInflationLinear =
    PiecewiseZeroInflationCurve<Linear>;
using PiecewiseYoYInflationLinear =
    PiecewiseYoYInflationCurve<Linear>;

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

Handle<YoYInflationTermStructure> make_yoy_inflation_curve(
    const Date& reference_date,
    const std::vector<Date>& dates,
    const std::vector<Rate>& rates,
    Frequency frequency,
    const DayCounter& day_counter) {
    QL_REQUIRE(dates.size() == rates.size(), "dates/rates size mismatch");
    QL_REQUIRE(dates.size() >= 2, "need at least two yoy-inflation nodes");
    return Handle<YoYInflationTermStructure>(
        ext::make_shared<InterpolatedYoYInflationCurve<Linear>>(
            reference_date, dates, rates, frequency, day_counter));
}

Handle<YoYInflationTermStructure> make_flat_yoy_inflation_curve(
    const Date& reference_date,
    const Date& base_date,
    const Date& max_date,
    Rate rate,
    Frequency frequency,
    const DayCounter& day_counter) {
    QL_REQUIRE(max_date > base_date, "max_date must be after base_date");
    std::vector<Date> dates = {base_date, max_date};
    std::vector<Rate> rates = {rate, rate};
    return Handle<YoYInflationTermStructure>(
        ext::make_shared<InterpolatedYoYInflationCurve<Linear>>(
            reference_date, dates, rates, frequency, day_counter));
}

Handle<YoYInflationTermStructure> make_piecewise_yoy_inflation_curve(
    const Date& reference_date,
    const Date& base_date,
    Rate base_yoy_rate,
    Frequency frequency,
    const DayCounter& day_counter,
    const std::vector<ext::shared_ptr<YoYInflationHelper>>& helpers) {
    QL_REQUIRE(!helpers.empty(), "empty yoy-inflation helper list");
    return Handle<YoYInflationTermStructure>(
        ext::make_shared<PiecewiseYoYInflationLinear>(
            reference_date, base_date, base_yoy_rate, frequency, day_counter,
            helpers));
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
             })
        .def(
            "has_seasonality",
            [](const Handle<ZeroInflationTermStructure>& h) {
                return h->hasSeasonality();
            })
        .def(
            "set_seasonality",
            [](Handle<ZeroInflationTermStructure>& h,
               const ext::shared_ptr<Seasonality>& seasonality) {
                h->setSeasonality(seasonality);
            },
            nb::arg("seasonality") = ext::shared_ptr<Seasonality>(),
            "Attach or clear (default/None) multiplicative seasonality.");

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
            nb::arg("extrapolate") = false)
        .def(
            "has_seasonality",
            [](const RelinkableHandle<ZeroInflationTermStructure>& h) {
                return h->hasSeasonality();
            })
        .def(
            "set_seasonality",
            [](RelinkableHandle<ZeroInflationTermStructure>& h,
               const ext::shared_ptr<Seasonality>& seasonality) {
                h->setSeasonality(seasonality);
            },
            nb::arg("seasonality") = ext::shared_ptr<Seasonality>(),
            "Attach or clear (default/None) multiplicative seasonality.");

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

    // --- Phase 13: year-on-year inflation / YYIIS ---

    nb::class_<Handle<YoYInflationTermStructure>>(
        m, "YoYInflationTermStructureHandle")
        .def(nb::init<>())
        .def("empty", &Handle<YoYInflationTermStructure>::empty)
        .def(
            "yoy_rate",
            [](const Handle<YoYInflationTermStructure>& h,
               const Date& d,
               bool extrapolate) { return h->yoyRate(d, extrapolate); },
            nb::arg("date"),
            nb::arg("extrapolate") = false)
        .def("base_date",
             [](const Handle<YoYInflationTermStructure>& h) {
                 return h->baseDate();
             })
        .def("base_rate",
             [](const Handle<YoYInflationTermStructure>& h) {
                 return h->baseRate();
             })
        .def("max_date",
             [](const Handle<YoYInflationTermStructure>& h) {
                 return h->maxDate();
             })
        .def("frequency",
             [](const Handle<YoYInflationTermStructure>& h) {
                 return h->frequency();
             })
        .def("reference_date",
             [](const Handle<YoYInflationTermStructure>& h) {
                 return h->referenceDate();
             })
        .def(
            "has_seasonality",
            [](const Handle<YoYInflationTermStructure>& h) {
                return h->hasSeasonality();
            })
        .def(
            "set_seasonality",
            [](Handle<YoYInflationTermStructure>& h,
               const ext::shared_ptr<Seasonality>& seasonality) {
                h->setSeasonality(seasonality);
            },
            nb::arg("seasonality") = ext::shared_ptr<Seasonality>(),
            "Attach or clear (default/None) multiplicative seasonality.");

    nb::class_<RelinkableHandle<YoYInflationTermStructure>>(
        m, "RelinkableYoYInflationTermStructureHandle")
        .def(nb::init<>())
        .def("empty",
             [](const RelinkableHandle<YoYInflationTermStructure>& h) {
                 return h.empty();
             })
        .def(
            "link_to",
            [](RelinkableHandle<YoYInflationTermStructure>& h,
               const Handle<YoYInflationTermStructure>& target) {
                h.linkTo(target.currentLink());
            },
            nb::arg("handle"))
        .def("as_handle",
             [](const RelinkableHandle<YoYInflationTermStructure>& h) {
                 return Handle<YoYInflationTermStructure>(h);
             })
        .def(
            "yoy_rate",
            [](const RelinkableHandle<YoYInflationTermStructure>& h,
               const Date& d,
               bool extrapolate) { return h->yoyRate(d, extrapolate); },
            nb::arg("date"),
            nb::arg("extrapolate") = false)
        .def(
            "has_seasonality",
            [](const RelinkableHandle<YoYInflationTermStructure>& h) {
                return h->hasSeasonality();
            })
        .def(
            "set_seasonality",
            [](RelinkableHandle<YoYInflationTermStructure>& h,
               const ext::shared_ptr<Seasonality>& seasonality) {
                h->setSeasonality(seasonality);
            },
            nb::arg("seasonality") = ext::shared_ptr<Seasonality>(),
            "Attach or clear (default/None) multiplicative seasonality.");

    // YoYInflationIndex is MI-heavy — opaque shared_ptr holder.
    nb::class_<YoYInflationIndex>(m, "YoYInflationIndex")
        .def("name", [](const YoYInflationIndex& i) { return i.name(); })
        .def("frequency",
             [](const YoYInflationIndex& i) { return i.frequency(); })
        .def("availability_lag",
             [](const YoYInflationIndex& i) { return i.availabilityLag(); })
        .def("last_fixing_date",
             [](const YoYInflationIndex& i) { return i.lastFixingDate(); })
        .def("ratio", [](const YoYInflationIndex& i) { return i.ratio(); })
        .def(
            "add_fixing",
            [](YoYInflationIndex& i,
               const Date& fixing_date,
               Rate fixing,
               bool force) { i.addFixing(fixing_date, fixing, force); },
            nb::arg("fixing_date"),
            nb::arg("fixing"),
            nb::arg("force_overwrite") = false)
        .def(
            "fixing",
            [](const YoYInflationIndex& i,
               const Date& fixing_date,
               bool forecast_today) {
                return i.fixing(fixing_date, forecast_today);
            },
            nb::arg("fixing_date"),
            nb::arg("forecast_today") = false);

    m.def(
        "make_yoy_inflation_index",
        [](const ext::shared_ptr<ZeroInflationIndex>& underlying,
           const Handle<YoYInflationTermStructure>& h) {
            return ext::shared_ptr<YoYInflationIndex>(
                ext::make_shared<YoYInflationIndex>(underlying, h));
        },
        nb::arg("underlying"),
        nb::arg("handle") = Handle<YoYInflationTermStructure>(),
        "Factory: YoYInflationIndex(ZeroInflationIndex, handle) — ratio index.");

    m.def(
        "make_yoy_inflation_index",
        [](const ext::shared_ptr<ZeroInflationIndex>& underlying,
           const RelinkableHandle<YoYInflationTermStructure>& h) {
            return ext::shared_ptr<YoYInflationIndex>(
                ext::make_shared<YoYInflationIndex>(
                    underlying, Handle<YoYInflationTermStructure>(h)));
        },
        nb::arg("underlying"),
        nb::arg("handle"),
        "Factory: YoYInflationIndex(ZeroInflationIndex, relinkable handle).");

    m.def(
        "YYUKRPI",
        [](const Handle<YoYInflationTermStructure>& h) {
            return ext::shared_ptr<YoYInflationIndex>(
                ext::make_shared<YYUKRPI>(h));
        },
        nb::arg("handle") = Handle<YoYInflationTermStructure>(),
        "Factory: YYUKRPI → YoYInflationIndex (quoted YoY).");

    m.def(
        "YYUKRPI",
        [](const RelinkableHandle<YoYInflationTermStructure>& h) {
            return ext::shared_ptr<YoYInflationIndex>(ext::make_shared<YYUKRPI>(
                Handle<YoYInflationTermStructure>(h)));
        },
        nb::arg("handle"),
        "Factory: YYUKRPI(relinkable handle) → YoYInflationIndex.");

    m.def(
        "YYEUHICP",
        [](const Handle<YoYInflationTermStructure>& h) {
            return ext::shared_ptr<YoYInflationIndex>(
                ext::make_shared<YYEUHICP>(h));
        },
        nb::arg("handle") = Handle<YoYInflationTermStructure>(),
        "Factory: YYEUHICP → YoYInflationIndex (quoted YoY).");

    m.def("InterpolatedYoYInflationCurve",
          &make_yoy_inflation_curve,
          nb::arg("reference_date"),
          nb::arg("dates"),
          nb::arg("rates"),
          nb::arg("frequency"),
          nb::arg("day_counter"),
          "Factory: InterpolatedYoYInflationCurve<Linear> → "
          "YoYInflationTermStructureHandle.");

    m.def("FlatYoYInflationCurve",
          &make_flat_yoy_inflation_curve,
          nb::arg("reference_date"),
          nb::arg("base_date"),
          nb::arg("max_date"),
          nb::arg("rate"),
          nb::arg("frequency"),
          nb::arg("day_counter"),
          "Factory: constant YoY-inflation rate via a 2-node linear curve.");

    nb::class_<YoYInflationHelper>(m, "YoYInflationHelper");

    m.def(
        "YearOnYearInflationSwapHelper",
        [](const Handle<Quote>& quote,
           const Period& observation_lag,
           const Date& maturity,
           const Calendar& calendar,
           BusinessDayConvention payment_convention,
           const DayCounter& day_counter,
           const ext::shared_ptr<YoYInflationIndex>& index,
           CPI::InterpolationType observation_interpolation,
           const Handle<YieldTermStructure>& nominal) {
            return ext::shared_ptr<YoYInflationHelper>(
                ext::make_shared<YearOnYearInflationSwapHelper>(
                    quote,
                    observation_lag,
                    maturity,
                    calendar,
                    payment_convention,
                    day_counter,
                    index,
                    observation_interpolation,
                    nominal));
        },
        nb::arg("quote"),
        nb::arg("observation_lag"),
        nb::arg("maturity"),
        nb::arg("calendar"),
        nb::arg("payment_convention"),
        nb::arg("day_counter"),
        nb::arg("index"),
        nb::arg("observation_interpolation"),
        nb::arg("nominal"),
        "Factory: YearOnYearInflationSwapHelper → YoYInflationHelper.");

    m.def("PiecewiseYoYInflationCurve",
          &make_piecewise_yoy_inflation_curve,
          nb::arg("reference_date"),
          nb::arg("base_date"),
          nb::arg("base_yoy_rate"),
          nb::arg("frequency"),
          nb::arg("day_counter"),
          nb::arg("helpers"),
          "Factory: PiecewiseYoYInflationCurve<Linear> → "
          "YoYInflationTermStructureHandle.");

    // YearOnYearInflationSwap is Swap/Instrument (MI) — standalone wrapper.
    nb::class_<YearOnYearInflationSwap>(m, "YearOnYearInflationSwap")
        .def(
            "__init__",
            [](YearOnYearInflationSwap* self,
               Swap::Type type,
               Real nominal,
               const Schedule& fixed_schedule,
               Rate fixed_rate,
               const DayCounter& fixed_day_count,
               const Schedule& yoy_schedule,
               const ext::shared_ptr<YoYInflationIndex>& yoy_index,
               const Period& observation_lag,
               CPI::InterpolationType observation_interpolation,
               Spread spread,
               const DayCounter& yoy_day_count,
               const Calendar& payment_calendar,
               BusinessDayConvention payment_convention) {
                new (self) YearOnYearInflationSwap(type,
                                                   nominal,
                                                   fixed_schedule,
                                                   fixed_rate,
                                                   fixed_day_count,
                                                   yoy_schedule,
                                                   yoy_index,
                                                   observation_lag,
                                                   observation_interpolation,
                                                   spread,
                                                   yoy_day_count,
                                                   payment_calendar,
                                                   payment_convention);
            },
            nb::arg("type"),
            nb::arg("nominal"),
            nb::arg("fixed_schedule"),
            nb::arg("fixed_rate"),
            nb::arg("fixed_day_count"),
            nb::arg("yoy_schedule"),
            nb::arg("yoy_index"),
            nb::arg("observation_lag"),
            nb::arg("observation_interpolation"),
            nb::arg("spread"),
            nb::arg("yoy_day_count"),
            nb::arg("payment_calendar"),
            nb::arg("payment_convention") = ModifiedFollowing)
        .def("NPV", [](YearOnYearInflationSwap& s) { return s.NPV(); })
        .def("fair_rate",
             [](YearOnYearInflationSwap& s) { return s.fairRate(); })
        .def("fair_spread",
             [](YearOnYearInflationSwap& s) { return s.fairSpread(); })
        .def("fixed_rate",
             [](const YearOnYearInflationSwap& s) { return s.fixedRate(); })
        .def("spread",
             [](const YearOnYearInflationSwap& s) { return s.spread(); })
        .def("nominal",
             [](const YearOnYearInflationSwap& s) { return s.nominal(); })
        .def("type", [](const YearOnYearInflationSwap& s) { return s.type(); })
        .def("start_date",
             [](const YearOnYearInflationSwap& s) { return s.startDate(); })
        .def("maturity_date",
             [](const YearOnYearInflationSwap& s) { return s.maturityDate(); })
        .def("fixed_leg_NPV",
             [](YearOnYearInflationSwap& s) { return s.fixedLegNPV(); })
        .def("yoy_leg_NPV",
             [](YearOnYearInflationSwap& s) { return s.yoyLegNPV(); })
        .def("is_expired",
             [](const YearOnYearInflationSwap& s) { return s.isExpired(); })
        .def(
            "set_pricing_engine",
            [](YearOnYearInflationSwap& s,
               const Handle<YieldTermStructure>& discount_curve) {
                s.setPricingEngine(
                    ext::make_shared<DiscountingSwapEngine>(discount_curve));
            },
            nb::arg("discount_curve"),
            "Attach DiscountingSwapEngine (standard for YYIIS).");

    // --- Phase 14: YoY inflation caps / floors ---

    nb::enum_<YoYInflationCapFloor::Type>(m, "YoYInflationCapFloorType")
        .value("Cap", YoYInflationCapFloor::Cap)
        .value("Floor", YoYInflationCapFloor::Floor)
        .value("Collar", YoYInflationCapFloor::Collar);

    nb::class_<Handle<YoYOptionletVolatilitySurface>>(
        m, "YoYOptionletVolatilitySurfaceHandle")
        .def(nb::init<>())
        .def("empty", &Handle<YoYOptionletVolatilitySurface>::empty)
        .def(
            "volatility",
            [](const Handle<YoYOptionletVolatilitySurface>& h,
               const Date& d,
               Rate strike,
               bool extrapolate) {
                return h->volatility(d, strike, Period(-1, Days), extrapolate);
            },
            nb::arg("date"),
            nb::arg("strike"),
            nb::arg("extrapolate") = false);

    m.def(
        "ConstantYoYOptionletVolatility",
        [](Volatility volatility,
           Natural settlement_days,
           const Calendar& calendar,
           BusinessDayConvention bdc,
           const DayCounter& day_counter,
           const Period& observation_lag,
           Frequency frequency,
           bool index_is_interpolated,
           Rate min_strike,
           Rate max_strike,
           VolatilityType vol_type,
           Real displacement) {
            return Handle<YoYOptionletVolatilitySurface>(
                ext::make_shared<ConstantYoYOptionletVolatility>(
                    volatility,
                    settlement_days,
                    calendar,
                    bdc,
                    day_counter,
                    observation_lag,
                    frequency,
                    index_is_interpolated,
                    min_strike,
                    max_strike,
                    vol_type,
                    displacement));
        },
        nb::arg("volatility"),
        nb::arg("settlement_days"),
        nb::arg("calendar"),
        nb::arg("bdc"),
        nb::arg("day_counter"),
        nb::arg("observation_lag"),
        nb::arg("frequency"),
        nb::arg("index_is_interpolated") = false,
        nb::arg("min_strike") = -1.0,
        nb::arg("max_strike") = 100.0,
        nb::arg("vol_type") = ShiftedLognormal,
        nb::arg("displacement") = 0.0,
        "Factory: ConstantYoYOptionletVolatility → "
        "YoYOptionletVolatilitySurfaceHandle.");

    nb::class_<YoYInflationCapFloor>(m, "YoYInflationCapFloor")
        .def(
            "__init__",
            [](YoYInflationCapFloor* self,
               YoYInflationCapFloor::Type type,
               const Schedule& schedule,
               const ext::shared_ptr<YoYInflationIndex>& index,
               const Period& observation_lag,
               CPI::InterpolationType observation_interpolation,
               Rate strike,
               const Calendar& payment_calendar,
               const DayCounter& day_counter,
               Real nominal,
               BusinessDayConvention payment_convention,
               Natural fixing_days,
               std::optional<Rate> floor_strike) {
                Leg leg = yoyInflationLeg(schedule,
                                          payment_calendar,
                                          index,
                                          observation_lag,
                                          observation_interpolation)
                              .withNotionals(nominal)
                              .withPaymentDayCounter(day_counter)
                              .withPaymentAdjustment(payment_convention)
                              .withFixingDays(fixing_days);
                if (type == YoYInflationCapFloor::Cap) {
                    new (self) YoYInflationCap(leg, std::vector<Rate>(1, strike));
                } else if (type == YoYInflationCapFloor::Floor) {
                    new (self)
                        YoYInflationFloor(leg, std::vector<Rate>(1, strike));
                } else if (type == YoYInflationCapFloor::Collar) {
                    QL_REQUIRE(floor_strike.has_value(),
                               "Collar requires floor_strike");
                    new (self) YoYInflationCollar(
                        leg,
                        std::vector<Rate>(1, strike),
                        std::vector<Rate>(1, *floor_strike));
                } else {
                    QL_FAIL("unknown YoYInflationCapFloor type");
                }
            },
            nb::arg("type"),
            nb::arg("schedule"),
            nb::arg("index"),
            nb::arg("observation_lag"),
            nb::arg("observation_interpolation"),
            nb::arg("strike"),
            nb::arg("payment_calendar"),
            nb::arg("day_counter"),
            nb::arg("nominal") = 1000000.0,
            nb::arg("payment_convention") = ModifiedFollowing,
            nb::arg("fixing_days") = 0,
            nb::arg("floor_strike") = nb::none(),
            "Build a YoY inflation Cap/Floor/Collar from a YoY schedule.")
        .def("NPV", [](YoYInflationCapFloor& cf) { return cf.NPV(); })
        .def("type",
             [](const YoYInflationCapFloor& cf) { return cf.type(); })
        .def("start_date",
             [](const YoYInflationCapFloor& cf) { return cf.startDate(); })
        .def("maturity_date",
             [](const YoYInflationCapFloor& cf) { return cf.maturityDate(); })
        .def("is_expired",
             [](const YoYInflationCapFloor& cf) { return cf.isExpired(); })
        .def(
            "atm_rate",
            [](const YoYInflationCapFloor& cf,
               const Handle<YieldTermStructure>& discount) {
                return cf.atmRate(**discount);
            },
            nb::arg("discount_curve"))
        .def(
            "set_pricing_engine",
            [](YoYInflationCapFloor& cf,
               const ext::shared_ptr<YoYInflationIndex>& index,
               const Handle<YoYOptionletVolatilitySurface>& vol,
               const Handle<YieldTermStructure>& nominal,
               const std::string& model) {
                ext::shared_ptr<PricingEngine> engine;
                if (model == "black" || model == "Black") {
                    engine = ext::make_shared<YoYInflationBlackCapFloorEngine>(
                        index, vol, nominal);
                } else if (model == "unit_displaced_black" ||
                           model == "UnitDisplacedBlack") {
                    engine = ext::make_shared<
                        YoYInflationUnitDisplacedBlackCapFloorEngine>(
                        index, vol, nominal);
                } else if (model == "bachelier" || model == "Bachelier") {
                    engine =
                        ext::make_shared<YoYInflationBachelierCapFloorEngine>(
                            index, vol, nominal);
                } else {
                    QL_FAIL("unknown YoY cap/floor model '"
                            << model
                            << "' (use black|unit_displaced_black|bachelier)");
                }
                cf.setPricingEngine(engine);
            },
            nb::arg("index"),
            nb::arg("volatility"),
            nb::arg("nominal"),
            nb::arg("model") = std::string("black"),
            "Attach Black / unit-displaced Black / Bachelier YoY engine.");

    m.def(
        "make_yoy_inflation_capfloor",
        [](YoYInflationCapFloor::Type type,
           const ext::shared_ptr<YoYInflationIndex>& index,
           Size length_years,
           const Calendar& calendar,
           const Period& observation_lag,
           CPI::InterpolationType observation_interpolation,
           Rate strike,
           Real nominal,
           const Date& effective_date,
           const DayCounter& day_counter,
           BusinessDayConvention payment_convention) {
            MakeYoYInflationCapFloor maker(type,
                                           index,
                                           length_years,
                                           calendar,
                                           observation_lag,
                                           observation_interpolation);
            maker.withNominal(nominal).withStrike(strike).withPaymentAdjustment(
                payment_convention);
            if (effective_date != Date())
                maker.withEffectiveDate(effective_date);
            if (day_counter != DayCounter())
                maker.withPaymentDayCounter(day_counter);
            return YoYInflationCapFloor(maker);
        },
        nb::arg("type"),
        nb::arg("index"),
        nb::arg("length_years"),
        nb::arg("calendar"),
        nb::arg("observation_lag"),
        nb::arg("observation_interpolation"),
        nb::arg("strike"),
        nb::arg("nominal") = 1000000.0,
        nb::arg("effective_date") = Date(),
        nb::arg("day_counter") = DayCounter(),
        nb::arg("payment_convention") = ModifiedFollowing,
        "Build a standard YoY inflation Cap/Floor via MakeYoYInflationCapFloor.");

    // --- Phase 15: CPISwap / CPIBond ---

    m.def(
        "cpi_lagged_fixing",
        [](const ext::shared_ptr<ZeroInflationIndex>& index,
           const Date& date,
           const Period& observation_lag,
           CPI::InterpolationType interpolation) {
            return CPI::laggedFixing(index, date, observation_lag, interpolation);
        },
        nb::arg("index"),
        nb::arg("date"),
        nb::arg("observation_lag"),
        nb::arg("interpolation"),
        "CPI::laggedFixing — lagged CPI observation for a payment date.");

    nb::class_<CPISwap>(m, "CPISwap")
        .def(
            "__init__",
            [](CPISwap* self,
               Swap::Type type,
               Real nominal,
               bool subtract_inflation_nominal,
               Spread spread,
               const DayCounter& float_day_count,
               const Schedule& float_schedule,
               BusinessDayConvention float_roll,
               Natural fixing_days,
               std::optional<ext::shared_ptr<IborIndex>> float_index,
               Rate fixed_rate,
               Real base_cpi,
               const DayCounter& fixed_day_count,
               const Schedule& fixed_schedule,
               BusinessDayConvention fixed_roll,
               const Period& observation_lag,
               const ext::shared_ptr<ZeroInflationIndex>& fixed_index,
               CPI::InterpolationType observation_interpolation,
               std::optional<Real> inflation_nominal) {
                const Real inf_nom = inflation_nominal.value_or(Null<Real>());
                ext::shared_ptr<IborIndex> float_idx =
                    float_index.value_or(ext::shared_ptr<IborIndex>());
                new (self) CPISwap(type,
                                   nominal,
                                   subtract_inflation_nominal,
                                   spread,
                                   float_day_count,
                                   float_schedule,
                                   float_roll,
                                   fixing_days,
                                   float_idx,
                                   fixed_rate,
                                   base_cpi,
                                   fixed_day_count,
                                   fixed_schedule,
                                   fixed_roll,
                                   observation_lag,
                                   fixed_index,
                                   observation_interpolation,
                                   inf_nom);
            },
            nb::arg("type"),
            nb::arg("nominal"),
            nb::arg("subtract_inflation_nominal"),
            nb::arg("spread"),
            nb::arg("float_day_count"),
            nb::arg("float_schedule"),
            nb::arg("float_roll"),
            nb::arg("fixing_days"),
            nb::arg("float_index") = nb::none(),
            nb::arg("fixed_rate"),
            nb::arg("base_cpi"),
            nb::arg("fixed_day_count"),
            nb::arg("fixed_schedule"),
            nb::arg("fixed_roll"),
            nb::arg("observation_lag"),
            nb::arg("fixed_index"),
            nb::arg("observation_interpolation") = CPI::Flat,
            nb::arg("inflation_nominal") = nb::none(),
            "Type refers to the floating leg (Payer/Receiver).")
        .def("NPV", [](CPISwap& s) { return s.NPV(); })
        .def("fair_rate", [](CPISwap& s) { return s.fairRate(); })
        .def("fair_spread", [](CPISwap& s) { return s.fairSpread(); })
        .def("fixed_rate", [](const CPISwap& s) { return s.fixedRate(); })
        .def("base_CPI", [](const CPISwap& s) { return s.baseCPI(); })
        .def("spread", [](const CPISwap& s) { return s.spread(); })
        .def("nominal", [](const CPISwap& s) { return s.nominal(); })
        .def("inflation_nominal",
             [](const CPISwap& s) { return s.inflationNominal(); })
        .def("type", [](const CPISwap& s) { return s.type(); })
        .def("fixed_leg_NPV", [](CPISwap& s) { return s.fixedLegNPV(); })
        .def("float_leg_NPV", [](CPISwap& s) { return s.floatLegNPV(); })
        .def("is_expired", [](const CPISwap& s) { return s.isExpired(); })
        .def(
            "set_pricing_engine",
            [](CPISwap& s, const Handle<YieldTermStructure>& discount_curve) {
                s.setPricingEngine(
                    ext::make_shared<DiscountingSwapEngine>(discount_curve));
            },
            nb::arg("discount_curve"),
            "Attach DiscountingSwapEngine.");

    nb::class_<CPIBond>(m, "CPIBond")
        .def(
            "__init__",
            [](CPIBond* self,
               Natural settlement_days,
               Real face_amount,
               Real base_cpi,
               const Period& observation_lag,
               const ext::shared_ptr<ZeroInflationIndex>& cpi_index,
               CPI::InterpolationType observation_interpolation,
               const Schedule& schedule,
               const std::vector<Rate>& coupons,
               const DayCounter& accrual_day_counter,
               BusinessDayConvention payment_convention,
               const Date& issue_date,
               const Calendar& payment_calendar) {
                new (self) CPIBond(settlement_days,
                                   face_amount,
                                   base_cpi,
                                   observation_lag,
                                   cpi_index,
                                   observation_interpolation,
                                   schedule,
                                   coupons,
                                   accrual_day_counter,
                                   payment_convention,
                                   issue_date,
                                   payment_calendar);
            },
            nb::arg("settlement_days"),
            nb::arg("face_amount"),
            nb::arg("base_cpi"),
            nb::arg("observation_lag"),
            nb::arg("cpi_index"),
            nb::arg("observation_interpolation"),
            nb::arg("schedule"),
            nb::arg("coupons"),
            nb::arg("accrual_day_counter"),
            nb::arg("payment_convention") = ModifiedFollowing,
            nb::arg("issue_date") = Date(),
            nb::arg("payment_calendar") = Calendar())
        .def("NPV", [](CPIBond& b) { return b.NPV(); })
        .def("clean_price", [](CPIBond& b) { return b.cleanPrice(); })
        .def("dirty_price", [](CPIBond& b) { return b.dirtyPrice(); })
        .def("base_CPI", [](const CPIBond& b) { return b.baseCPI(); })
        .def("settlement_date",
             [](const CPIBond& b) { return b.settlementDate(); })
        .def("maturity_date",
             [](const CPIBond& b) { return b.maturityDate(); })
        .def("is_expired", [](const CPIBond& b) { return b.isExpired(); })
        .def(
            "set_pricing_engine",
            [](CPIBond& b, const Handle<YieldTermStructure>& discount_curve) {
                b.setPricingEngine(
                    ext::make_shared<DiscountingBondEngine>(discount_curve));
            },
            nb::arg("discount_curve"),
            "Attach DiscountingBondEngine.");

    // --- Phase 16: CPICapFloor + term price surface ---

    nb::class_<Matrix>(m, "Matrix")
        .def(
            "__init__",
            [](Matrix* self, Size rows, Size columns, const std::vector<Real>& data) {
                QL_REQUIRE(data.size() == rows * columns,
                           "Matrix data size must equal rows*columns");
                new (self) Matrix(rows, columns);
                for (Size i = 0; i < rows; ++i)
                    for (Size j = 0; j < columns; ++j)
                        (*self)[i][j] = data[i * columns + j];
            },
            nb::arg("rows"),
            nb::arg("columns"),
            nb::arg("data"),
            "Build a Matrix from row-major flat data.")
        .def("rows", &Matrix::rows)
        .def("columns", &Matrix::columns)
        .def(
            "at",
            [](const Matrix& m, Size row, Size column) {
                QL_REQUIRE(row < m.rows() && column < m.columns(),
                           "Matrix index out of range");
                return m[row][column];
            },
            nb::arg("row"),
            nb::arg("column"));

    nb::class_<Handle<CPICapFloorTermPriceSurface>>(
        m, "CPICapFloorTermPriceSurfaceHandle")
        .def(nb::init<>())
        .def("empty", &Handle<CPICapFloorTermPriceSurface>::empty)
        .def(
            "price",
            [](const Handle<CPICapFloorTermPriceSurface>& h,
               const Period& tenor,
               Rate strike) { return h->price(tenor, strike); },
            nb::arg("tenor"),
            nb::arg("strike"))
        .def(
            "cap_price",
            [](const Handle<CPICapFloorTermPriceSurface>& h,
               const Period& tenor,
               Rate strike) { return h->capPrice(tenor, strike); },
            nb::arg("tenor"),
            nb::arg("strike"))
        .def(
            "floor_price",
            [](const Handle<CPICapFloorTermPriceSurface>& h,
               const Period& tenor,
               Rate strike) { return h->floorPrice(tenor, strike); },
            nb::arg("tenor"),
            nb::arg("strike"))
        .def(
            "atm_rate",
            [](const Handle<CPICapFloorTermPriceSurface>& h, const Date& d) {
                return h->atmRate(d);
            },
            nb::arg("maturity"));

    m.def(
        "InterpolatedCPICapFloorTermPriceSurface",
        [](Real nominal,
           Rate base_rate,
           const Period& observation_lag,
           const Calendar& calendar,
           BusinessDayConvention bdc,
           const DayCounter& day_counter,
           const ext::shared_ptr<ZeroInflationIndex>& index,
           CPI::InterpolationType observation_interpolation,
           const Handle<YieldTermStructure>& nominal_curve,
           const std::vector<Rate>& cap_strikes,
           const std::vector<Rate>& floor_strikes,
           const std::vector<Period>& maturities,
           const Matrix& cap_prices,
           const Matrix& floor_prices) {
            return Handle<CPICapFloorTermPriceSurface>(
                ext::make_shared<
                    InterpolatedCPICapFloorTermPriceSurface<Bilinear>>(
                    nominal,
                    base_rate,
                    observation_lag,
                    calendar,
                    bdc,
                    day_counter,
                    index,
                    observation_interpolation,
                    nominal_curve,
                    cap_strikes,
                    floor_strikes,
                    maturities,
                    cap_prices,
                    floor_prices));
        },
        nb::arg("nominal"),
        nb::arg("base_rate"),
        nb::arg("observation_lag"),
        nb::arg("calendar"),
        nb::arg("bdc"),
        nb::arg("day_counter"),
        nb::arg("index"),
        nb::arg("observation_interpolation"),
        nb::arg("nominal_curve"),
        nb::arg("cap_strikes"),
        nb::arg("floor_strikes"),
        nb::arg("maturities"),
        nb::arg("cap_prices"),
        nb::arg("floor_prices"),
        "Factory: InterpolatedCPICapFloorTermPriceSurface<Bilinear> → handle.");

    nb::class_<CPICapFloor>(m, "CPICapFloor")
        .def(
            "__init__",
            [](CPICapFloor* self,
               Option::Type type,
               Real nominal,
               const Date& start_date,
               Real base_cpi,
               const Date& maturity,
               const Calendar& fix_calendar,
               BusinessDayConvention fix_convention,
               const Calendar& pay_calendar,
               BusinessDayConvention pay_convention,
               Rate strike,
               const ext::shared_ptr<ZeroInflationIndex>& index,
               const Period& observation_lag,
               CPI::InterpolationType observation_interpolation) {
                new (self) CPICapFloor(type,
                                       nominal,
                                       start_date,
                                       base_cpi,
                                       maturity,
                                       fix_calendar,
                                       fix_convention,
                                       pay_calendar,
                                       pay_convention,
                                       strike,
                                       index,
                                       observation_lag,
                                       observation_interpolation);
            },
            nb::arg("type"),
            nb::arg("nominal"),
            nb::arg("start_date"),
            nb::arg("base_cpi"),
            nb::arg("maturity"),
            nb::arg("fix_calendar"),
            nb::arg("fix_convention"),
            nb::arg("pay_calendar"),
            nb::arg("pay_convention"),
            nb::arg("strike"),
            nb::arg("index"),
            nb::arg("observation_lag"),
            nb::arg("observation_interpolation") = CPI::Flat)
        .def("NPV", [](CPICapFloor& o) { return o.NPV(); })
        .def("type", [](const CPICapFloor& o) { return o.type(); })
        .def("nominal", [](const CPICapFloor& o) { return o.nominal(); })
        .def("strike", [](const CPICapFloor& o) { return o.strike(); })
        .def("fixing_date",
             [](const CPICapFloor& o) { return o.fixingDate(); })
        .def("pay_date", [](const CPICapFloor& o) { return o.payDate(); })
        .def("is_expired", [](const CPICapFloor& o) { return o.isExpired(); })
        .def(
            "set_pricing_engine",
            [](CPICapFloor& o,
               const Handle<CPICapFloorTermPriceSurface>& surface) {
                o.setPricingEngine(
                    ext::make_shared<InterpolatingCPICapFloorEngine>(surface));
            },
            nb::arg("price_surface"),
            "Attach InterpolatingCPICapFloorEngine.");

    // --- Phase 17: inflation seasonality ---

    m.def(
        "inflation_period",
        [](const Date& d, Frequency frequency) {
            return inflationPeriod(d, frequency);
        },
        nb::arg("date"),
        nb::arg("frequency"),
        "Return (period_start, period_end) for an inflation fixing date.");

    nb::class_<Seasonality>(m, "Seasonality");

    nb::class_<MultiplicativePriceSeasonality, Seasonality>(
        m, "MultiplicativePriceSeasonality")
        .def(
            "__init__",
            [](MultiplicativePriceSeasonality* self,
               const Date& seasonality_base_date,
               Frequency frequency,
               const std::vector<Rate>& factors) {
                new (self) MultiplicativePriceSeasonality(
                    seasonality_base_date, frequency, factors);
            },
            nb::arg("seasonality_base_date"),
            nb::arg("frequency"),
            nb::arg("seasonality_factors"))
        .def(
            "set",
            [](MultiplicativePriceSeasonality& s,
               const Date& seasonality_base_date,
               Frequency frequency,
               const std::vector<Rate>& factors) {
                s.set(seasonality_base_date, frequency, factors);
            },
            nb::arg("seasonality_base_date"),
            nb::arg("frequency"),
            nb::arg("seasonality_factors"))
        .def("seasonality_base_date",
             &MultiplicativePriceSeasonality::seasonalityBaseDate)
        .def("frequency", &MultiplicativePriceSeasonality::frequency)
        .def("seasonality_factors",
             &MultiplicativePriceSeasonality::seasonalityFactors)
        .def(
            "seasonality_factor",
            [](const MultiplicativePriceSeasonality& s, const Date& d) {
                return s.seasonalityFactor(d);
            },
            nb::arg("date"));

    nb::class_<KerkhofSeasonality, MultiplicativePriceSeasonality>(
        m, "KerkhofSeasonality")
        .def(
            "__init__",
            [](KerkhofSeasonality* self,
               const Date& seasonality_base_date,
               const std::vector<Rate>& factors) {
                new (self) KerkhofSeasonality(seasonality_base_date, factors);
            },
            nb::arg("seasonality_base_date"),
            nb::arg("seasonality_factors"));
}
