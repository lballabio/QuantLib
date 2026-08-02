#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>

#include <ql/errors.hpp>
#include <ql/exercise.hpp>
#include <ql/handle.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/option.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace nb = nanobind;
using namespace QuantLib;

namespace {

Date settings_get_evaluation_date() {
    return Date(Settings::instance().evaluationDate());
}

void settings_set_evaluation_date(const Date& d) {
    Settings::instance().evaluationDate() = d;
}

// Factory helpers avoid binding MI-heavy QuantLib types as Python subclasses
// where nanobind cannot adjust base pointers.
Handle<YieldTermStructure> make_flat_forward_handle(const Date& reference_date,
                                                    Rate forward,
                                                    const DayCounter& day_counter) {
    return Handle<YieldTermStructure>(
        ext::make_shared<FlatForward>(reference_date, forward, day_counter));
}

Handle<YieldTermStructure> make_flat_forward_handle_quote(
    const Date& reference_date,
    const Handle<Quote>& forward,
    const DayCounter& day_counter) {
    return Handle<YieldTermStructure>(
        ext::make_shared<FlatForward>(reference_date, forward, day_counter));
}

Handle<BlackVolTermStructure> make_black_constant_vol_handle(
    const Date& reference_date,
    const Calendar& calendar,
    Volatility volatility,
    const DayCounter& day_counter) {
    return Handle<BlackVolTermStructure>(ext::make_shared<BlackConstantVol>(
        reference_date, calendar, volatility, day_counter));
}

DiscountFactor yield_discount_date(const Handle<YieldTermStructure>& h,
                                   const Date& d,
                                   bool extrapolate) {
    return h->discount(d, extrapolate);
}

DiscountFactor yield_discount_time(const Handle<YieldTermStructure>& h,
                                   Time t,
                                   bool extrapolate) {
    return h->discount(t, extrapolate);
}

} // namespace

NB_MODULE(_qlnb, m) {
    m.doc() = "Phase-0 nanobind bindings for QuantLib";

    nb::enum_<Month>(m, "Month")
        .value("January", January)
        .value("February", February)
        .value("March", March)
        .value("April", April)
        .value("May", May)
        .value("June", June)
        .value("July", July)
        .value("August", August)
        .value("September", September)
        .value("October", October)
        .value("November", November)
        .value("December", December);

    nb::class_<Date>(m, "Date")
        .def(nb::init<>())
        .def(nb::init<Date::serial_type>(), nb::arg("serial_number"))
        .def(nb::init<Day, Month, Year>(),
             nb::arg("day"),
             nb::arg("month"),
             nb::arg("year"))
        .def("day_of_month", &Date::dayOfMonth)
        .def("month", &Date::month)
        .def("year", &Date::year)
        .def("serial_number", &Date::serialNumber)
        .def_static("todays_date", &Date::todaysDate)
        .def_static("min_date", &Date::minDate)
        .def_static("max_date", &Date::maxDate)
        .def("__repr__",
             [](const Date& d) {
                 return "Date(" + std::to_string(d.dayOfMonth()) + ", " +
                        std::to_string(static_cast<int>(d.month())) + ", " +
                        std::to_string(d.year()) + ")";
             })
        .def("__eq__", [](const Date& a, const Date& b) { return a == b; })
        .def("__ne__", [](const Date& a, const Date& b) { return a != b; })
        .def("__lt__", [](const Date& a, const Date& b) { return a < b; })
        .def("__le__", [](const Date& a, const Date& b) { return a <= b; })
        .def("__gt__", [](const Date& a, const Date& b) { return a > b; })
        .def("__ge__", [](const Date& a, const Date& b) { return a >= b; })
        .def("__add__",
             [](const Date& d, Date::serial_type n) { return d + n; })
        .def("__sub__",
             [](const Date& d, Date::serial_type n) { return d - n; });

    m.def("get_evaluation_date", &settings_get_evaluation_date);
    m.def("set_evaluation_date", &settings_set_evaluation_date, nb::arg("date"));

    nb::class_<Settings>(m, "Settings")
        .def_static(
            "instance",
            []() -> Settings& { return Settings::instance(); },
            nb::rv_policy::reference)
        .def_prop_rw(
            "evaluation_date",
            [](Settings&) { return settings_get_evaluation_date(); },
            [](Settings&, const Date& d) { settings_set_evaluation_date(d); })
        .def("anchor_evaluation_date",
             [](Settings&) { Settings::instance().anchorEvaluationDate(); })
        .def("reset_evaluation_date",
             [](Settings&) { Settings::instance().resetEvaluationDate(); });

    nb::class_<Quote>(m, "Quote")
        .def("value", &Quote::value)
        .def("is_valid", &Quote::isValid);

    nb::class_<SimpleQuote, Quote>(m, "SimpleQuote")
        .def(nb::init<Real>(), nb::arg("value"))
        .def("set_value",
             [](SimpleQuote& q, Real value) { return q.setValue(value); },
             nb::arg("value"))
        .def("reset", &SimpleQuote::reset);

    nb::class_<Handle<Quote>>(m, "QuoteHandle")
        .def(nb::init<>())
        .def(nb::init<const ext::shared_ptr<Quote>&>(), nb::arg("value"))
        .def("empty", &Handle<Quote>::empty)
        .def("current_link",
             [](const Handle<Quote>& h) { return h.currentLink(); });

    nb::class_<RelinkableHandle<Quote>>(m, "RelinkableQuoteHandle")
        .def(nb::init<>())
        .def(nb::init<const ext::shared_ptr<Quote>&>(), nb::arg("value"))
        .def("empty",
             [](const RelinkableHandle<Quote>& h) { return h.empty(); })
        .def("current_link",
             [](const RelinkableHandle<Quote>& h) { return h.currentLink(); })
        .def(
            "link_to",
            [](RelinkableHandle<Quote>& h, const ext::shared_ptr<Quote>& q) {
                h.linkTo(q);
            },
            nb::arg("value"))
        .def("reset", &RelinkableHandle<Quote>::reset)
        .def("as_handle",
             [](const RelinkableHandle<Quote>& h) {
                 return Handle<Quote>(h);
             });

    m.def(
        "make_quote_handle",
        [](Real value) {
            return Handle<Quote>(ext::make_shared<SimpleQuote>(value));
        },
        nb::arg("value"));

    nb::class_<DayCounter>(m, "DayCounter")
        .def("name", &DayCounter::name)
        .def(
            "year_fraction",
            [](const DayCounter& dc, const Date& d1, const Date& d2) {
                return dc.yearFraction(d1, d2);
            },
            nb::arg("d1"),
            nb::arg("d2"));

    // Value-semantic types: bind derived calendars/day counters as factories
    // returning the base value type (pimpl), avoiding inheritance issues.
    m.def("Actual365Fixed", []() { return DayCounter(Actual365Fixed()); });
    m.def("TARGET", []() { return Calendar(TARGET()); });

    nb::class_<Calendar>(m, "Calendar")
        .def("name", &Calendar::name)
        .def("is_business_day", &Calendar::isBusinessDay, nb::arg("date"));

    nb::class_<Handle<YieldTermStructure>>(m, "YieldTermStructureHandle")
        .def(nb::init<>())
        .def("empty", &Handle<YieldTermStructure>::empty)
        .def(
            "discount",
            [](const Handle<YieldTermStructure>& h, const Date& d, bool extrapolate) {
                return yield_discount_date(h, d, extrapolate);
            },
            nb::arg("date"),
            nb::arg("extrapolate") = false)
        .def(
            "discount",
            [](const Handle<YieldTermStructure>& h, Time t, bool extrapolate) {
                return yield_discount_time(h, t, extrapolate);
            },
            nb::arg("time"),
            nb::arg("extrapolate") = false)
        .def("reference_date",
             [](const Handle<YieldTermStructure>& h) {
                 return h->referenceDate();
             });

    m.def("FlatForward",
          &make_flat_forward_handle,
          nb::arg("reference_date"),
          nb::arg("forward"),
          nb::arg("day_counter"));
    m.def("FlatForward",
          &make_flat_forward_handle_quote,
          nb::arg("reference_date"),
          nb::arg("forward"),
          nb::arg("day_counter"));

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
        .def("strike",
             [](const PlainVanillaPayoff& p) { return p.strike(); })
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
        .def(
            "set_pricing_engine",
            [](EuropeanOption& opt,
               const ext::shared_ptr<BlackScholesMertonProcess>& process) {
                opt.setPricingEngine(
                    ext::make_shared<AnalyticEuropeanEngine>(process));
            },
            nb::arg("process"));

    // Phase-0 sugar: AnalyticEuropeanEngine(process) returns the process for
    // option.set_pricing_engine(...). A dedicated engine type can come later.
    m.def(
        "AnalyticEuropeanEngine",
        [](const ext::shared_ptr<BlackScholesMertonProcess>& process) {
            return process;
        },
        nb::arg("process"));

    m.attr("__version__") = "0.1.0";
}
