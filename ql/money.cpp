/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Decillion Pty(Ltd)
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/money.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/math/comparison.hpp>

namespace QuantLib {

    namespace {

        void convertTo(Money& m, const Currency& target) {
            if (m.currency() != target) {
                ExchangeRate rate =
                    ExchangeRateManager::instance().lookup(m.currency(),
                                                           target);
                m = rate.exchange(m).rounded();
            }
        }

        void convertToBase(Money& m) {
            const auto & base_currency =
                Money::Settings::instance().baseCurrency();
            QL_REQUIRE(!base_currency.empty(), "no base currency set");
            convertTo(m, base_currency);
        }

        template< typename ReturnValue, typename Function >
        ReturnValue apply(const Money& m1, const Money& m2, const Function f) {
            const auto & conversion_type =
                Money::Settings::instance().conversionType();
            if (m1.currency() == m2.currency()) {
                return f(m1.value(), m2.value());
            } else if (conversion_type == Money::BaseCurrencyConversion) {
                Money tmp1 = m1;
                convertToBase(tmp1);
                Money tmp2 = m2;
                convertToBase(tmp2);
                return f(tmp1.value(), tmp2.value());
            } else if (conversion_type == Money::AutomatedConversion) {
                Money tmp = m2;
                convertTo(tmp, m1.currency());
                return f(m1.value(), tmp.value());
            } else {
                QL_FAIL("currency mismatch and no conversion specified");
            }
        }
    }

    Money& Money::operator+=(const Money& m) {
        const auto & conversion_type = Settings::instance().conversionType();
        if (currency_ == m.currency_) {
            value_ += m.value_;
        } else if (conversion_type == Money::BaseCurrencyConversion) {
            convertToBase(*this);
            Money tmp = m;
            convertToBase(tmp);
            *this += tmp;
        } else if (conversion_type == Money::AutomatedConversion) {
            Money tmp = m;
            convertTo(tmp, currency_);
            *this += tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
        return *this;
    }

    Money& Money::operator-=(const Money& m) {
        return *this += (-m);
    }

    Decimal operator/(const Money& m1, const Money& m2) {
        return apply<Decimal>(
                    m1, m2,
                    [](const Real x, const Real y) { return x / y; });
    }

    bool operator==(const Money& m1, const Money& m2) {
        return apply<bool>(
                    m1, m2,
                    [](const Real x, const Real y) { return x == y; });
    }

    bool operator<(const Money& m1, const Money& m2) {
        return apply<bool>(
                    m1, m2,
                    [](const Real x, const Real y) { return x < y; });
    }

    bool operator<=(const Money& m1, const Money& m2) {
        return apply<bool>(
                    m1, m2,
                    [](const Real x, const Real y) { return x <= y; });
    }

    bool close(const Money& m1, const Money& m2, Size n) {
        return apply<bool>(
                    m1, m2,
                    [n](const Real x, const Real y) { return close(x, y, n); });
    }

    bool close_enough(const Money& m1, const Money& m2, Size n) {
        return apply<bool>(
                    m1, m2,
                    [n](const Real x, const Real y) { return close_enough(x, y, n); });
    }

    std::ostream& operator<<(std::ostream& out, const Money& m) {
        return out << m.rounded().value() << " " << m.currency().code();
    }

    const Money::ConversionType & Money::Settings::conversionType() const
    {
        return conversionType_;
    }

    Money::ConversionType & Money::Settings::conversionType()
    {
        return conversionType_;
    }

    const Currency & Money::Settings::baseCurrency() const
    {
        return baseCurrency_;
    }

    Currency & Money::Settings::baseCurrency()
    {
        return baseCurrency_;
    }

    Money::BaseCurrencyProxy& Money::BaseCurrencyProxy::operator=(const Currency& c) {
        Money::Settings::instance().baseCurrency() = c;
        return *this;
    }

    Money::BaseCurrencyProxy::operator Currency() const {
        return Money::Settings::instance().baseCurrency();
    }

    Money::ConversionTypeProxy& Money::ConversionTypeProxy::operator=(ConversionType t) {
        Money::Settings::instance().conversionType() = t;
        return *this;
    }

    Money::ConversionTypeProxy::operator Money::ConversionType() const {
        return Money::Settings::instance().conversionType();
    }

}
