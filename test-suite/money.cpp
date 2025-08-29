/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 StatPro Italia srl

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/money.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(MoneyTests)

bool IsSameCurrencyAndValuesAreClose(const Money& lhs, const Money& rhs) {
    return (lhs.currency() == rhs.currency())
        && std::abs(lhs.value() - rhs.value()) < 0.01;
}

const Currency EUR = EURCurrency();
const Currency GBP = GBPCurrency();
const Currency USD = USDCurrency();
const ExchangeRate eur_usd = ExchangeRate(EUR, USD, 1.2042);
const ExchangeRate eur_gbp = ExchangeRate(EUR, GBP, 0.6612);


BOOST_AUTO_TEST_CASE(testNone) {

    BOOST_TEST_MESSAGE("Testing money arithmetic without conversions...");

    const Money m1 = 50000.0 * EUR;
    const Money m2 = 100000.0 * EUR;
    const Money m3 = 500000.0 * EUR;

    Money::Settings::instance().conversionType() = Money::NoConversion;

    const Money calculated = m1*3.0 + 2.5*m2 - m3/5.0 + m1 * (m2 / m3);
    const Decimal x = m1.value() * 3.0
        + 2.5 * m2.value()
        - m3.value() / 5.0
        + m1.value() * (m2.value() / m3.value());
    const Money expected(x, EUR);

    if (!IsSameCurrencyAndValuesAreClose(calculated, expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testBaseCurrency) {

    BOOST_TEST_MESSAGE("Testing money arithmetic with conversion to base currency...");

    const Money gbp = 50000.0 * GBP;
    const Money eur = 100000.0 * EUR;
    const Money usd = 500000.0 * USD;

    ExchangeRateManager::instance().clear();
    ExchangeRateManager::instance().add(eur_usd);
    ExchangeRateManager::instance().add(eur_gbp);
    const auto GBP_to_EUR = [eur_gbp = eur_gbp.rate()](const Real gbp) -> Real { return gbp / eur_gbp; };
    const auto USD_to_EUR = [eur_usd = eur_usd.rate()](const Real usd) -> Real { return usd / eur_usd; };

    Money::Settings::instance().conversionType() = Money::BaseCurrencyConversion;
    Money::Settings::instance().baseCurrency() = EUR;

    const Money calculated = gbp*3.0 + 2.5*eur - usd/5.0 + gbp * (eur/usd);

    const Decimal x = GBP_to_EUR(gbp.value()) * 3.0
        + 2.5 * eur.value()
        - USD_to_EUR(usd.value()) / 5.0
        + GBP_to_EUR(gbp.value()) * eur.value() / USD_to_EUR(usd.value());
    const Money expected(x, EUR);

    ExchangeRateManager::instance().clear();
    Money::Settings::instance().conversionType() = Money::NoConversion;

    if (!IsSameCurrencyAndValuesAreClose(calculated, expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testAutomated) {

    BOOST_TEST_MESSAGE("Testing money arithmetic with automated conversion...");

    const Money gbp = 50000.0 * GBP;
    const Money eur = 100000.0 * EUR;
    const Money usd = 500000.0 * USD;

    ExchangeRateManager::instance().clear();
    ExchangeRateManager::instance().add(eur_usd);
    ExchangeRateManager::instance().add(eur_gbp);
    const auto EUR_to_GBP = [eur_gbp = eur_gbp.rate()](const Real eur) -> Real { return eur * eur_gbp; };
    const auto USD_to_EUR = [eur_usd = eur_usd.rate()](const Real usd) -> Real { return usd / eur_usd; };
    const auto USD_to_GBP = [eur_gbp = eur_gbp.rate(), eur_usd = eur_usd.rate()](const Real usd) -> Real
        { return usd * eur_gbp / eur_usd; };

    Money::Settings::instance().conversionType() = Money::AutomatedConversion;

    const Money calculated = (gbp*3.0 + 2.5*eur) - usd/5.0 + gbp * (eur/usd);

    const Decimal x = gbp.value() * 3.0
        + 2.5 * EUR_to_GBP(eur.value())
        - USD_to_GBP(usd.value()) / 5.0
        + gbp.value() * eur.value() / USD_to_EUR(usd.value());

    const Money expected(x, GBP);

    ExchangeRateManager::instance().clear();
    Money::Settings::instance().conversionType() = Money::NoConversion;

    if (!IsSameCurrencyAndValuesAreClose(calculated, expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testComparisons) {

    BOOST_TEST_MESSAGE("Testing money comparisons...");

    for (const auto conversionType : {Money::AutomatedConversion, Money::NoConversion, Money::BaseCurrencyConversion}) {
        ExchangeRateManager::instance().add(eur_usd);
        ExchangeRateManager::instance().add(eur_gbp);
        Money::Settings::instance().conversionType() = conversionType;
        if (conversionType == Money::BaseCurrencyConversion)
            Money::Settings::instance().baseCurrency() = EUR;

        // equality
        BOOST_CHECK_EQUAL(Money(123.45, EUR), Money(123.45, EUR));
        if (conversionType != Money::NoConversion)
            BOOST_CHECK_EQUAL(Money(1, EUR), Money(eur_usd.rate(), USD));

        // unequal
        BOOST_CHECK_NE(Money(1, EUR), Money(2, EUR));
        if (conversionType != Money::NoConversion)
            BOOST_CHECK_NE(Money(1, EUR), Money(100, USD));

        // less than
        BOOST_CHECK_LT(Money(1, EUR), Money(2, EUR));
        if (conversionType != Money::NoConversion)
            BOOST_CHECK_LT(Money(1, EUR), Money(100, USD));

        // less or equal than
        BOOST_CHECK_LE(Money(1, EUR), Money(2, EUR));
        BOOST_CHECK_LE(Money(2, EUR), Money(2, EUR));
        if (conversionType != Money::NoConversion)
            BOOST_CHECK_LE(Money(1, EUR), Money(100, USD));

        // greater than
        BOOST_CHECK_GT(Money(2, EUR), Money(1, EUR));
        if (conversionType != Money::NoConversion)
            BOOST_CHECK_GT(Money(100, EUR), Money(1, USD));

        // less or equal than
        BOOST_CHECK_GE(Money(2, EUR), Money(1, EUR));
        BOOST_CHECK_GE(Money(2, EUR), Money(2, EUR));
        if (conversionType != Money::NoConversion)
            BOOST_CHECK_GE(Money(100, EUR), Money(1, USD));

        // close
        BOOST_CHECK(close(Money(1, EUR), Money(1, EUR)));
        BOOST_CHECK(close(Money(1+1e-15, EUR), Money(1, EUR)));
        if (conversionType != Money::NoConversion){
            BOOST_CHECK(close(Money(1, EUR), Money(eur_usd.rate(), USD)));
            BOOST_CHECK(close(Money(1+1e-15, EUR), Money(eur_usd.rate(), USD)));
        }

        // close enough
        BOOST_CHECK(close_enough(Money(1, EUR), Money(1, EUR)));
        BOOST_CHECK(close_enough(Money(1+1e-15, EUR), Money(1, EUR)));
        if (conversionType != Money::NoConversion){
            BOOST_CHECK(close_enough(Money(1, EUR), Money(eur_usd.rate(), USD)));
            BOOST_CHECK(close_enough(Money(1+1e-15, EUR), Money(eur_usd.rate(), USD)));
        }

        ExchangeRateManager::instance().clear();
        Money::Settings::instance().conversionType() = Money::NoConversion;
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
