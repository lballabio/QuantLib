#include "bindings.hpp"

#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>

#include <optional>
#include <sstream>

#include <ql/currency.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/asia.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/exchangerate.hpp>
#include <ql/handle.hpp>
#include <ql/instruments/fxforward.hpp>
#include <ql/money.hpp>
#include <ql/pricingengines/forward/discountingfxforwardengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/date.hpp>

using namespace QuantLib;

void bind_fx(nb::module_& m) {
    // --- Phase 24: currencies, money, exchange rates, FX forward ---

    // Currency subclasses slice to value Currency (no MI hierarchy in Python).
    nb::class_<Currency>(m, "Currency")
        .def(nb::init<>())
        .def("name", &Currency::name)
        .def("code", &Currency::code)
        .def("numeric_code", &Currency::numericCode)
        .def("symbol", &Currency::symbol)
        .def("fraction_symbol", &Currency::fractionSymbol)
        .def("fractions_per_unit", &Currency::fractionsPerUnit)
        .def("empty", &Currency::empty)
        .def("__eq__",
             [](const Currency& a, const Currency& b) { return a == b; })
        .def("__ne__",
             [](const Currency& a, const Currency& b) { return a != b; })
        .def("__repr__",
             [](const Currency& c) {
                 if (c.empty()) {
                     return std::string("Currency()");
                 }
                 return "Currency('" + c.code() + "')";
             })
        .def("__mul__",
             [](const Currency& c, Decimal value) { return value * c; })
        .def("__rmul__",
             [](const Currency& c, Decimal value) { return value * c; });

    m.def("USDCurrency", []() { return Currency(USDCurrency()); });
    m.def("EURCurrency", []() { return Currency(EURCurrency()); });
    m.def("GBPCurrency", []() { return Currency(GBPCurrency()); });
    m.def("SGDCurrency", []() { return Currency(SGDCurrency()); });

    nb::enum_<Money::ConversionType>(m, "MoneyConversionType")
        .value("NoConversion", Money::NoConversion)
        .value("BaseCurrencyConversion", Money::BaseCurrencyConversion)
        .value("AutomatedConversion", Money::AutomatedConversion);

    m.def(
        "set_money_conversion",
        [](Money::ConversionType type) {
            Money::Settings::instance().conversionType() = type;
        },
        nb::arg("conversion_type"),
        "Set Money::Settings conversion type (default NoConversion).");

    m.def(
        "get_money_conversion",
        []() { return Money::Settings::instance().conversionType(); },
        "Get Money::Settings conversion type.");

    nb::class_<Money>(m, "Money")
        .def(nb::init<>())
        .def(nb::init<Currency, Decimal>(),
             nb::arg("currency"),
             nb::arg("value"))
        .def(nb::init<Decimal, Currency>(),
             nb::arg("value"),
             nb::arg("currency"))
        .def("currency", &Money::currency)
        .def("value", &Money::value)
        .def("rounded", &Money::rounded)
        .def("__eq__", [](const Money& a, const Money& b) { return a == b; })
        .def("__ne__", [](const Money& a, const Money& b) { return a != b; })
        .def("__repr__",
             [](const Money& money) {
                 std::ostringstream oss;
                 oss << money;
                 return oss.str();
             })
        .def("__mul__",
             [](const Money& money, Decimal x) { return money * x; })
        .def("__rmul__",
             [](const Money& money, Decimal x) { return x * money; })
        .def("__truediv__",
             [](const Money& money, Decimal x) { return money / x; });

    nb::enum_<ExchangeRate::Type>(m, "ExchangeRateType")
        .value("Direct", ExchangeRate::Direct)
        .value("Derived", ExchangeRate::Derived);

    nb::class_<ExchangeRate>(m, "ExchangeRate")
        .def(nb::init<>())
        .def(nb::init<Currency, Currency, Decimal>(),
             nb::arg("source"),
             nb::arg("target"),
             nb::arg("rate"))
        .def("source", &ExchangeRate::source)
        .def("target", &ExchangeRate::target)
        .def("type", &ExchangeRate::type)
        .def("rate", &ExchangeRate::rate)
        .def("exchange", &ExchangeRate::exchange, nb::arg("amount"))
        .def_static("chain",
                    &ExchangeRate::chain,
                    nb::arg("r1"),
                    nb::arg("r2"));

    m.def(
        "exchange_rate_manager_clear",
        []() { ExchangeRateManager::instance().clear(); },
        "Clear rates added to ExchangeRateManager.");

    m.def(
        "exchange_rate_manager_add",
        [](const ExchangeRate& rate,
           const Date& start_date,
           const Date& end_date) {
            ExchangeRateManager::instance().add(rate, start_date, end_date);
        },
        nb::arg("rate"),
        nb::arg("start_date") = Date::minDate(),
        nb::arg("end_date") = Date::maxDate(),
        "Add an exchange rate to ExchangeRateManager.");

    m.def(
        "exchange_rate_manager_lookup",
        [](const Currency& source,
           const Currency& target,
           const Date& date,
           ExchangeRate::Type type) {
            return ExchangeRateManager::instance().lookup(
                source, target, date, type);
        },
        nb::arg("source"),
        nb::arg("target"),
        nb::arg("date") = Date(),
        nb::arg("type") = ExchangeRate::Derived,
        "Lookup an exchange rate from ExchangeRateManager.");

    // FxForward is Instrument (MI) — standalone concrete wrapper.
    nb::class_<FxForward>(m, "FxForward")
        .def(
            "__init__",
            [](FxForward* self,
               Real source_nominal,
               const Currency& source_currency,
               Real target_nominal,
               const Currency& target_currency,
               const Date& maturity_date,
               bool pay_source_currency,
               Natural settlement_days,
               const Calendar& payment_calendar) {
                new (self) FxForward(source_nominal,
                                     source_currency,
                                     target_nominal,
                                     target_currency,
                                     maturity_date,
                                     pay_source_currency,
                                     settlement_days,
                                     payment_calendar);
            },
            nb::arg("source_nominal"),
            nb::arg("source_currency"),
            nb::arg("target_nominal"),
            nb::arg("target_currency"),
            nb::arg("maturity_date"),
            nb::arg("pay_source_currency"),
            nb::arg("settlement_days") = 2,
            nb::arg("payment_calendar") = Calendar())
        .def(
            "__init__",
            [](FxForward* self,
               Real source_nominal,
               const Currency& source_currency,
               const Currency& target_currency,
               Real forward_rate,
               const Date& maturity_date,
               bool pay_source_currency,
               Natural settlement_days,
               const Calendar& payment_calendar) {
                new (self) FxForward(source_nominal,
                                     source_currency,
                                     target_currency,
                                     forward_rate,
                                     maturity_date,
                                     pay_source_currency,
                                     settlement_days,
                                     payment_calendar);
            },
            nb::arg("source_nominal"),
            nb::arg("source_currency"),
            nb::arg("target_currency"),
            nb::arg("forward_rate"),
            nb::arg("maturity_date"),
            nb::arg("pay_source_currency"),
            nb::arg("settlement_days") = 2,
            nb::arg("payment_calendar") = Calendar())
        .def("source_nominal", &FxForward::sourceNominal)
        .def("target_nominal", &FxForward::targetNominal)
        .def("source_currency", &FxForward::sourceCurrency)
        .def("target_currency", &FxForward::targetCurrency)
        .def("maturity_date", &FxForward::maturityDate)
        .def("pay_source_currency", &FxForward::paySourceCurrency)
        .def("forward_rate", &FxForward::forwardRate)
        .def("settlement_days", &FxForward::settlementDays)
        .def("settlement_calendar", &FxForward::settlementCalendar)
        .def("settlement_date", &FxForward::settlementDate)
        .def("is_expired", &FxForward::isExpired)
        .def("NPV", [](FxForward& fwd) { return fwd.NPV(); })
        .def("fair_forward_rate", &FxForward::fairForwardRate)
        .def("npv_source_currency", &FxForward::npvSourceCurrency)
        .def("npv_target_currency", &FxForward::npvTargetCurrency)
        .def(
            "set_pricing_engine",
            [](FxForward& fwd,
               const Handle<YieldTermStructure>& source_curve,
               const Handle<YieldTermStructure>& target_curve,
               const Handle<Quote>& spot_fx) {
                fwd.setPricingEngine(
                    ext::make_shared<DiscountingFxForwardEngine>(
                        source_curve, target_curve, spot_fx));
            },
            nb::arg("source_curve"),
            nb::arg("target_curve"),
            nb::arg("spot_fx"),
            "Attach DiscountingFxForwardEngine "
            "(spot_fx = target per unit of source).")
        .def(
            "set_pricing_engine",
            [](FxForward& fwd,
               const Handle<YieldTermStructure>& source_curve,
               const Handle<YieldTermStructure>& target_curve,
               Real spot_fx) {
                fwd.setPricingEngine(
                    ext::make_shared<DiscountingFxForwardEngine>(
                        source_curve,
                        target_curve,
                        Handle<Quote>(ext::make_shared<SimpleQuote>(spot_fx))));
            },
            nb::arg("source_curve"),
            nb::arg("target_curve"),
            nb::arg("spot_fx"),
            "Attach DiscountingFxForwardEngine from a scalar spot FX rate.");
}
