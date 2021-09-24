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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/money.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/math/comparison.hpp>
#include <boost/format.hpp>

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
        const auto & conversion_type = Settings::instance().conversionType();
        if (currency_ == m.currency_) {
            value_ -= m.value_;
        } else if (conversion_type == Money::BaseCurrencyConversion) {
            convertToBase(*this);
            Money tmp = m;
            convertToBase(tmp);
            *this -= tmp;
        } else if (conversion_type == Money::AutomatedConversion) {
            Money tmp = m;
            convertTo(tmp, currency_);
            *this -= tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
        return *this;
    }

    Decimal operator/(const Money& m1, const Money& m2) {
        const auto & conversion_type =
            Money::Settings::instance().conversionType();
        if (m1.currency() == m2.currency()) {
            return m1.value()/m2.value();
        } else if (conversion_type == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return tmp1/tmp2;
        } else if (conversion_type == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return m1/tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool operator==(const Money& m1, const Money& m2) {
        const auto & conversion_type =
            Money::Settings::instance().conversionType();
        if (m1.currency() == m2.currency()) {
            return m1.value() == m2.value();
        } else if (conversion_type == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 == tmp2;
        } else if (conversion_type == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return m1 == tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool operator<(const Money& m1, const Money& m2) {
        const auto & conversion_type =
            Money::Settings::instance().conversionType();
        if (m1.currency() == m2.currency()) {
            return m1.value() < m2.value();
        } else if (conversion_type == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 < tmp2;
        } else if (conversion_type == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return m1 < tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool operator<=(const Money& m1, const Money& m2) {
        const auto & conversion_type =
            Money::Settings::instance().conversionType();
        if (m1.currency() == m2.currency()) {
            return m1.value() <= m2.value();
        } else if (conversion_type == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 <= tmp2;
        } else if (conversion_type == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return m1 <= tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool close(const Money& m1, const Money& m2, Size n) {
        const auto & conversion_type =
            Money::Settings::instance().conversionType();
        if (m1.currency() == m2.currency()) {
            return close(m1.value(),m2.value(),n);
        } else if (conversion_type == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return close(tmp1,tmp2,n);
        } else if (conversion_type == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return close(m1,tmp,n);
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool close_enough(const Money& m1, const Money& m2, Size n) {
        const auto & conversion_type =
            Money::Settings::instance().conversionType();
        if (m1.currency() == m2.currency()) {
            return close_enough(m1.value(),m2.value(),n);
        } else if (conversion_type == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return close_enough(tmp1,tmp2,n);
        } else if (conversion_type == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return close_enough(m1,tmp,n);
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }


    std::ostream& operator<<(std::ostream& out, const Money& m) {
        boost::format fmt(m.currency().format());
        fmt.exceptions(boost::io::all_error_bits ^
                       boost::io::too_many_args_bit);
        return out << fmt % m.rounded().value()
                          % m.currency().code()
                          % m.currency().symbol();
    }

    Money::Settings::Settings() :
        conversionType_(Money::NoConversion),
        baseCurrency_(Currency())
    {
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
