#include "bindings.hpp"

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/date.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/period.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/timeunit.hpp>

using namespace QuantLib;

void bind_time(nb::module_& m) {
    nb::enum_<BusinessDayConvention>(m, "BusinessDayConvention")
        .value("Following", Following)
        .value("ModifiedFollowing", ModifiedFollowing)
        .value("Preceding", Preceding)
        .value("ModifiedPreceding", ModifiedPreceding)
        .value("Unadjusted", Unadjusted)
        .value("HalfMonthModifiedFollowing", HalfMonthModifiedFollowing)
        .value("Nearest", Nearest);

    nb::enum_<DateGeneration::Rule>(m, "DateGeneration")
        .value("Backward", DateGeneration::Backward)
        .value("Forward", DateGeneration::Forward)
        .value("Zero", DateGeneration::Zero)
        .value("ThirdWednesday", DateGeneration::ThirdWednesday)
        .value("ThirdWednesdayInclusive", DateGeneration::ThirdWednesdayInclusive)
        .value("Twentieth", DateGeneration::Twentieth)
        .value("TwentiethIMM", DateGeneration::TwentiethIMM)
        .value("OldCDS", DateGeneration::OldCDS)
        .value("CDS", DateGeneration::CDS)
        .value("CDS2015", DateGeneration::CDS2015);

    nb::enum_<ActualActual::Convention>(m, "ActualActualConvention")
        .value("ISMA", ActualActual::ISMA)
        .value("Bond", ActualActual::Bond)
        .value("ISDA", ActualActual::ISDA)
        .value("Historical", ActualActual::Historical)
        .value("Actual365", ActualActual::Actual365)
        .value("AFB", ActualActual::AFB)
        .value("Euro", ActualActual::Euro);

    nb::enum_<Thirty360::Convention>(m, "Thirty360Convention")
        .value("USA", Thirty360::USA)
        .value("BondBasis", Thirty360::BondBasis)
        .value("European", Thirty360::European)
        .value("EurobondBasis", Thirty360::EurobondBasis)
        .value("Italian", Thirty360::Italian)
        .value("German", Thirty360::German)
        .value("ISMA", Thirty360::ISMA)
        .value("ISDA", Thirty360::ISDA)
        .value("NASD", Thirty360::NASD);

    nb::class_<DayCounter>(m, "DayCounter")
        .def("name", &DayCounter::name)
        .def(
            "year_fraction",
            [](const DayCounter& dc, const Date& d1, const Date& d2) {
                return dc.yearFraction(d1, d2);
            },
            nb::arg("d1"),
            nb::arg("d2"))
        .def(
            "day_count",
            [](const DayCounter& dc, const Date& d1, const Date& d2) {
                return dc.dayCount(d1, d2);
            },
            nb::arg("d1"),
            nb::arg("d2"));

    m.def("Actual365Fixed", []() { return DayCounter(Actual365Fixed()); });
    m.def("Actual360", []() { return DayCounter(Actual360()); });
    m.def(
        "ActualActual",
        [](ActualActual::Convention c) { return DayCounter(ActualActual(c)); },
        nb::arg("convention") = ActualActual::ISDA);
    m.def(
        "Thirty360",
        [](Thirty360::Convention c) { return DayCounter(Thirty360(c)); },
        nb::arg("convention") = Thirty360::BondBasis);

    nb::class_<Calendar>(m, "Calendar")
        .def("name", &Calendar::name)
        .def("is_business_day", &Calendar::isBusinessDay, nb::arg("date"))
        .def("is_holiday", &Calendar::isHoliday, nb::arg("date"))
        .def("is_end_of_month", &Calendar::isEndOfMonth, nb::arg("date"))
        .def(
            "advance",
            [](const Calendar& c,
               const Date& d,
               const Period& p,
               BusinessDayConvention convention,
               bool end_of_month) {
                return c.advance(d, p, convention, end_of_month);
            },
            nb::arg("date"),
            nb::arg("period"),
            nb::arg("convention") = Following,
            nb::arg("end_of_month") = false)
        .def(
            "advance",
            [](const Calendar& c,
               const Date& d,
               Integer n,
               TimeUnit unit,
               BusinessDayConvention convention,
               bool end_of_month) {
                return c.advance(d, n, unit, convention, end_of_month);
            },
            nb::arg("date"),
            nb::arg("n"),
            nb::arg("unit"),
            nb::arg("convention") = Following,
            nb::arg("end_of_month") = false)
        .def(
            "adjust",
            [](const Calendar& c, const Date& d, BusinessDayConvention convention) {
                return c.adjust(d, convention);
            },
            nb::arg("date"),
            nb::arg("convention") = Following);

    nb::enum_<UnitedStates::Market>(m, "UnitedStatesMarket")
        .value("Settlement", UnitedStates::Settlement)
        .value("NYSE", UnitedStates::NYSE)
        .value("GovernmentBond", UnitedStates::GovernmentBond)
        .value("NERC", UnitedStates::NERC)
        .value("LiborImpact", UnitedStates::LiborImpact)
        .value("FederalReserve", UnitedStates::FederalReserve)
        .value("SOFR", UnitedStates::SOFR);

    m.def("TARGET", []() { return Calendar(TARGET()); });
    m.def("NullCalendar", []() { return Calendar(NullCalendar()); });
    m.def("WeekendsOnly", []() { return Calendar(WeekendsOnly()); });
    m.def("UnitedKingdom", []() { return Calendar(UnitedKingdom()); });
    m.def(
        "UnitedStates",
        [](UnitedStates::Market market) { return Calendar(UnitedStates(market)); },
        nb::arg("market") = UnitedStates::Settlement);

    nb::class_<Schedule>(m, "Schedule")
        .def(
            "__init__",
            [](Schedule* self,
               const Date& effective_date,
               const Date& termination_date,
               const Period& tenor,
               const Calendar& calendar,
               BusinessDayConvention convention,
               BusinessDayConvention termination_convention,
               DateGeneration::Rule rule,
               bool end_of_month) {
                new (self) Schedule(effective_date,
                                    termination_date,
                                    tenor,
                                    calendar,
                                    convention,
                                    termination_convention,
                                    rule,
                                    end_of_month);
            },
            nb::arg("effective_date"),
            nb::arg("termination_date"),
            nb::arg("tenor"),
            nb::arg("calendar"),
            nb::arg("convention"),
            nb::arg("termination_date_convention"),
            nb::arg("rule"),
            nb::arg("end_of_month"))
        .def("size", &Schedule::size)
        .def("__len__", &Schedule::size)
        .def(
            "__getitem__",
            [](const Schedule& s, Size i) {
                if (i >= s.size())
                    throw nb::index_error("schedule index out of range");
                return s[i];
            },
            nb::arg("i"))
        .def("dates", &Schedule::dates)
        .def("start_date", &Schedule::startDate)
        .def("end_date", &Schedule::endDate)
        .def("empty", &Schedule::empty);
}
