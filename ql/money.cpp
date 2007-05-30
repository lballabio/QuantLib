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
#include <assert.h>

namespace QuantLib {

    Money::ConversionType Money::conversionType = Money::NoConversion;
    Currency Money::baseCurrency = Currency();

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
            QL_REQUIRE(!Money::baseCurrency.empty(), "no base currency set");
            convertTo(m, Money::baseCurrency);
        }

    }

    Money& Money::operator+=(const Money& m) {
        if (currency_ == m.currency_) {
            value_ += m.value_;
        } else if (Money::conversionType == Money::BaseCurrencyConversion) {
            convertToBase(*this);
            Money tmp = m;
            convertToBase(tmp);
            *this += tmp;
        } else if (Money::conversionType == Money::AutomatedConversion) {
            Money tmp = m;
            convertTo(tmp, currency_);
            *this += tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
        return *this;
    }

    Money& Money::operator-=(const Money& m) {
        if (currency_ == m.currency_) {
            value_ -= m.value_;
        } else if (Money::conversionType == Money::BaseCurrencyConversion) {
            convertToBase(*this);
            Money tmp = m;
            convertToBase(tmp);
            *this -= tmp;
        } else if (Money::conversionType == Money::AutomatedConversion) {
            Money tmp = m;
            convertTo(tmp, currency_);
            *this -= tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
        return *this;
    }

    Decimal operator/(const Money& m1, const Money& m2) {
        if (m1.currency() == m2.currency()) {
            return m1.value()/m2.value();
        } else if (Money::conversionType == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return tmp1/tmp2;
        } else if (Money::conversionType == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return m1/tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool operator==(const Money& m1, const Money& m2) {
        if (m1.currency() == m2.currency()) {
            return m1.value() == m2.value();
        } else if (Money::conversionType == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 == tmp2;
        } else if (Money::conversionType == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return m1 == tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool operator<(const Money& m1, const Money& m2) {
        if (m1.currency() == m2.currency()) {
            return m1.value() < m2.value();
        } else if (Money::conversionType == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 < tmp2;
        } else if (Money::conversionType == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return m1 < tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool operator<=(const Money& m1, const Money& m2) {
        if (m1.currency() == m2.currency()) {
            return m1.value() <= m2.value();
        } else if (Money::conversionType == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 <= tmp2;
        } else if (Money::conversionType == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return m1 <= tmp;
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool close(const Money& m1, const Money& m2, Size n) {
        if (m1.currency() == m2.currency()) {
            return close(m1.value(),m2.value(),n);
        } else if (Money::conversionType == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return close(tmp1,tmp2,n);
        } else if (Money::conversionType == Money::AutomatedConversion) {
            Money tmp = m2;
            convertTo(tmp, m1.currency());
            return close(m1,tmp,n);
        } else {
            QL_FAIL("currency mismatch and no conversion specified");
        }
    }

    bool close_enough(const Money& m1, const Money& m2, Size n) {
        if (m1.currency() == m2.currency()) {
            return close_enough(m1.value(),m2.value(),n);
        } else if (Money::conversionType == Money::BaseCurrencyConversion) {
            Money tmp1 = m1;
            convertToBase(tmp1);
            Money tmp2 = m2;
            convertToBase(tmp2);
            return close_enough(tmp1,tmp2,n);
        } else if (Money::conversionType == Money::AutomatedConversion) {
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

}
