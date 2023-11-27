/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 StatPro Italia srl

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/money.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/exchangeratemanager.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(MoneyTest)

BOOST_AUTO_TEST_CASE(testNone) {

    BOOST_TEST_MESSAGE("Testing money arithmetic without conversions...");

    Currency EUR = EURCurrency();

    Money m1 = 50000.0 * EUR;
    Money m2 = 100000.0 * EUR;
    Money m3 = 500000.0 * EUR;

    Money::Settings::instance().conversionType() = Money::NoConversion;

    Money calculated = m1*3.0 + 2.5*m2 - m3/5.0 + m1 * (m2 / m3);
    Decimal x = m1.value() * 3.0
        + 2.5 * m2.value()
        - m3.value() / 5.0
        + m1.value() * (m2.value() / m3.value());
    Money expected(x, EUR);

    if (calculated != expected) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testBaseCurrency) {

    BOOST_TEST_MESSAGE("Testing money arithmetic with conversion "
                       "to base currency...");

    Currency EUR = EURCurrency(), GBP = GBPCurrency(), USD = USDCurrency();

    Money m1 = 50000.0 * GBP;
    Money m2 = 100000.0 * EUR;
    Money m3 = 500000.0 * USD;

    ExchangeRateManager::instance().clear();
    ExchangeRate eur_usd = ExchangeRate(EUR, USD, 1.2042);
    ExchangeRate eur_gbp = ExchangeRate(EUR, GBP, 0.6612);
    ExchangeRateManager::instance().add(eur_usd);
    ExchangeRateManager::instance().add(eur_gbp);

    auto & money_settings = Money::Settings::instance();
    money_settings.conversionType() = Money::BaseCurrencyConversion;
    money_settings.baseCurrency() = EUR;

    Money calculated = m1*3.0 + 2.5*m2 - m3/5.0 + m1 * (m2/m3);

    const Rounding& round = money_settings.baseCurrency().rounding();
    Decimal x = round(m1.value()*3.0/eur_gbp.rate())
        + 2.5*m2.value()
        - round(m3.value()/(5.0*eur_usd.rate()))
        + round((m1.value()* (m2.value() / round(m3.value()/eur_usd.rate()))) / eur_gbp.rate()) ;
    Money expected(x, EUR);

    money_settings.conversionType() = Money::NoConversion;

    if (calculated != expected) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testAutomated) {

    BOOST_TEST_MESSAGE("Testing money arithmetic with automated conversion...");

    Currency EUR = EURCurrency(), GBP = GBPCurrency(), USD = USDCurrency();

    Money m1 = 50000.0 * GBP;
    Money m2 = 100000.0 * EUR;
    Money m3 = 500000.0 * USD;

    ExchangeRateManager::instance().clear();
    ExchangeRate eur_usd = ExchangeRate(EUR, USD, 1.2042);
    ExchangeRate eur_gbp = ExchangeRate(EUR, GBP, 0.6612);
    ExchangeRateManager::instance().add(eur_usd);
    ExchangeRateManager::instance().add(eur_gbp);

    auto & money_settings = Money::Settings::instance();
    money_settings.conversionType() = Money::AutomatedConversion;

    Money calculated = (m1*3.0 + 2.5*m2) - m3/5.0 + m1 * (m2/m3);

    const Rounding& round1 = m1.currency().rounding();
    const Rounding& round2 = m2.currency().rounding();
    Decimal x = m1.value()*3.0
        + round1(2.5*m2.value()*eur_gbp.rate())
        - round1((m3.value()/5.0)*eur_gbp.rate()/eur_usd.rate())
        + m1.value() * ( m2.value() / round2(m3.value()/eur_usd.rate()));

    Money expected(x, GBP);

    money_settings.conversionType() = Money::NoConversion;

    if (calculated != expected) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()