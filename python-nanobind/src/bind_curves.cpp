#include "bindings.hpp"

#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <ql/compounding.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/interestrate.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/period.hpp>

using namespace QuantLib;

namespace {

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

using DiscountLogLinearCurve = PiecewiseYieldCurve<Discount, LogLinear>;

} // namespace

void bind_curves(nb::module_& m) {
    nb::enum_<Compounding>(m, "Compounding")
        .value("Simple", Simple)
        .value("Compounded", Compounded)
        .value("Continuous", Continuous)
        .value("SimpleThenCompounded", SimpleThenCompounded)
        .value("CompoundedThenSimple", CompoundedThenSimple);

    nb::class_<InterestRate>(m, "InterestRate")
        .def("rate", &InterestRate::rate)
        .def("day_counter", &InterestRate::dayCounter)
        .def("compounding", &InterestRate::compounding)
        .def("frequency", &InterestRate::frequency)
        .def("__float__", [](const InterestRate& r) { return r.rate(); })
        .def("__repr__",
             [](const InterestRate& r) {
                 return "InterestRate(" + std::to_string(r.rate()) + ")";
             });

    nb::class_<Handle<YieldTermStructure>>(m, "YieldTermStructureHandle")
        .def(nb::init<>())
        .def("empty", &Handle<YieldTermStructure>::empty)
        .def(
            "discount",
            [](const Handle<YieldTermStructure>& h, const Date& d, bool extrapolate) {
                return h->discount(d, extrapolate);
            },
            nb::arg("date"),
            nb::arg("extrapolate") = false)
        .def(
            "discount",
            [](const Handle<YieldTermStructure>& h, Time t, bool extrapolate) {
                return h->discount(t, extrapolate);
            },
            nb::arg("time"),
            nb::arg("extrapolate") = false)
        .def("reference_date",
             [](const Handle<YieldTermStructure>& h) { return h->referenceDate(); })
        .def(
            "zero_rate",
            [](const Handle<YieldTermStructure>& h,
               const Date& d,
               const DayCounter& dc,
               Compounding comp,
               Frequency freq,
               bool extrapolate) {
                return h->zeroRate(d, dc, comp, freq, extrapolate);
            },
            nb::arg("date"),
            nb::arg("day_counter"),
            nb::arg("compounding"),
            nb::arg("frequency") = Annual,
            nb::arg("extrapolate") = false)
        .def(
            "forward_rate",
            [](const Handle<YieldTermStructure>& h,
               const Date& d1,
               const Date& d2,
               const DayCounter& dc,
               Compounding comp,
               Frequency freq,
               bool extrapolate) {
                return h->forwardRate(d1, d2, dc, comp, freq, extrapolate);
            },
            nb::arg("d1"),
            nb::arg("d2"),
            nb::arg("day_counter"),
            nb::arg("compounding"),
            nb::arg("frequency") = Annual,
            nb::arg("extrapolate") = false);

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

    // Rate helpers are opaque shared_ptrs (MI-heavy).
    nb::class_<RateHelper>(m, "RateHelper");

    m.def(
        "DepositRateHelper",
        [](Rate rate,
           const Period& tenor,
           Natural fixing_days,
           const Calendar& calendar,
           BusinessDayConvention convention,
           bool end_of_month,
           const DayCounter& day_counter) {
            return ext::shared_ptr<RateHelper>(ext::make_shared<DepositRateHelper>(
                rate, tenor, fixing_days, calendar, convention, end_of_month,
                day_counter));
        },
        nb::arg("rate"),
        nb::arg("tenor"),
        nb::arg("fixing_days"),
        nb::arg("calendar"),
        nb::arg("convention"),
        nb::arg("end_of_month"),
        nb::arg("day_counter"));

    m.def(
        "DepositRateHelper",
        [](const Handle<Quote>& rate,
           const Period& tenor,
           Natural fixing_days,
           const Calendar& calendar,
           BusinessDayConvention convention,
           bool end_of_month,
           const DayCounter& day_counter) {
            return ext::shared_ptr<RateHelper>(ext::make_shared<DepositRateHelper>(
                rate, tenor, fixing_days, calendar, convention, end_of_month,
                day_counter));
        },
        nb::arg("rate"),
        nb::arg("tenor"),
        nb::arg("fixing_days"),
        nb::arg("calendar"),
        nb::arg("convention"),
        nb::arg("end_of_month"),
        nb::arg("day_counter"));

    m.def(
        "PiecewiseLogLinearDiscountCurve",
        [](const Date& reference_date,
           const std::vector<ext::shared_ptr<RateHelper>>& helpers,
           const DayCounter& day_counter) {
            return Handle<YieldTermStructure>(
                ext::make_shared<DiscountLogLinearCurve>(
                    reference_date, helpers, day_counter));
        },
        nb::arg("reference_date"),
        nb::arg("helpers"),
        nb::arg("day_counter"));

    // Ibor indexes as opaque shared_ptrs.
    nb::class_<IborIndex>(m, "IborIndex")
        .def("name", [](const IborIndex& i) { return i.name(); })
        .def("tenor", [](const IborIndex& i) { return i.tenor(); })
        .def("fixing_calendar",
             [](const IborIndex& i) { return i.fixingCalendar(); })
        .def("day_counter", [](const IborIndex& i) { return i.dayCounter(); });

    m.def("Euribor3M", []() {
        return ext::shared_ptr<IborIndex>(ext::make_shared<Euribor3M>());
    });
    m.def(
        "Euribor3M",
        [](const Handle<YieldTermStructure>& h) {
            return ext::shared_ptr<IborIndex>(ext::make_shared<Euribor3M>(h));
        },
        nb::arg("handle"));
    m.def("Euribor6M", []() {
        return ext::shared_ptr<IborIndex>(ext::make_shared<Euribor6M>());
    });
    m.def(
        "Euribor6M",
        [](const Handle<YieldTermStructure>& h) {
            return ext::shared_ptr<IborIndex>(ext::make_shared<Euribor6M>(h));
        },
        nb::arg("handle"));
}
