#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>

#include <ql/handle.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/time/date.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/period.hpp>
#include <ql/time/timeunit.hpp>

namespace nb = nanobind;
using namespace QuantLib;

namespace {

Date settings_get_evaluation_date() {
    return Date(Settings::instance().evaluationDate());
}

void settings_set_evaluation_date(const Date& d) {
    Settings::instance().evaluationDate() = d;
}

} // namespace

NB_MODULE(_qlnb, m) {
    m.doc() = "Nanobind bindings for QuantLib (phase 6)";
    m.attr("__version__") = "0.7.0";

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

    // Period/TimeUnit before Date so Date can overload Period arithmetic.
    nb::enum_<TimeUnit>(m, "TimeUnit")
        .value("Days", Days)
        .value("Weeks", Weeks)
        .value("Months", Months)
        .value("Years", Years)
        .value("Hours", Hours)
        .value("Minutes", Minutes)
        .value("Seconds", Seconds)
        .value("Milliseconds", Milliseconds)
        .value("Microseconds", Microseconds);

    nb::enum_<Frequency>(m, "Frequency")
        .value("NoFrequency", NoFrequency)
        .value("Once", Once)
        .value("Annual", Annual)
        .value("Semiannual", Semiannual)
        .value("EveryFourthMonth", EveryFourthMonth)
        .value("Quarterly", Quarterly)
        .value("Bimonthly", Bimonthly)
        .value("Monthly", Monthly)
        .value("EveryFourthWeek", EveryFourthWeek)
        .value("Biweekly", Biweekly)
        .value("Weekly", Weekly)
        .value("Daily", Daily)
        .value("OtherFrequency", OtherFrequency);

    nb::class_<Period>(m, "Period")
        .def(nb::init<>())
        .def(nb::init<Integer, TimeUnit>(), nb::arg("n"), nb::arg("units"))
        .def(nb::init<Frequency>(), nb::arg("frequency"))
        .def("length", &Period::length)
        .def("units", &Period::units)
        .def("frequency", &Period::frequency)
        .def("normalize", &Period::normalize)
        .def("normalized", &Period::normalized)
        .def("__repr__",
             [](const Period& p) {
                 return "Period(" + std::to_string(p.length()) + ", " +
                        std::to_string(static_cast<int>(p.units())) + ")";
             })
        .def("__eq__", [](const Period& a, const Period& b) { return a == b; })
        .def("__ne__", [](const Period& a, const Period& b) { return a != b; })
        .def("__mul__", [](const Period& p, Integer n) { return p * n; });

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
        .def("__add__", [](const Date& d, const Period& p) { return d + p; })
        .def("__sub__",
             [](const Date& d, Date::serial_type n) { return d - n; })
        .def("__sub__", [](const Date& d, const Period& p) { return d - p; });

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
        .def("as_handle", [](const RelinkableHandle<Quote>& h) {
            return Handle<Quote>(h);
        });

    m.def(
        "make_quote_handle",
        [](Real value) {
            return Handle<Quote>(ext::make_shared<SimpleQuote>(value));
        },
        nb::arg("value"));

    bind_time(m);
    bind_curves(m);
    bind_instruments(m);
    bind_pricing(m);
    bind_rates_options(m);
    bind_experimental(m);
}
