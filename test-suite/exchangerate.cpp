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

#include "exchangerate.hpp"
#include "utilities.hpp"
#include <ql/exchangerate.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/asia.hpp>
#include <ql/currencies/exchangeratemanager.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void ExchangeRateTest::testDirect() {

    BOOST_TEST_MESSAGE("Testing direct exchange rates...");

    Currency EUR = EURCurrency(), USD = USDCurrency();

    ExchangeRate eur_usd = ExchangeRate(EUR, USD, 1.2042);

    Money m1 = 50000.0 * EUR;
    Money m2 = 100000.0 * USD;

    Money::Settings::instance().conversionType() = Money::NoConversion;

    Money calculated = eur_usd.exchange(m1);
    Money expected(m1.value()*eur_usd.rate(), USD);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    calculated = eur_usd.exchange(m2);
    expected = Money(m2.value()/eur_usd.rate(), EUR);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

void ExchangeRateTest::testDerived() {

    BOOST_TEST_MESSAGE("Testing derived exchange rates...");

    Currency EUR = EURCurrency(), USD = USDCurrency(), GBP = GBPCurrency();

    ExchangeRate eur_usd = ExchangeRate(EUR, USD, 1.2042);
    ExchangeRate eur_gbp = ExchangeRate(EUR, GBP, 0.6612);

    ExchangeRate derived = ExchangeRate::chain(eur_usd, eur_gbp);

    Money m1 = 50000.0 * GBP;
    Money m2 = 100000.0 * USD;

    Money::Settings::instance().conversionType() = Money::NoConversion;

    Money calculated = derived.exchange(m1);
    Money expected(m1.value()*eur_usd.rate()/eur_gbp.rate(), USD);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    calculated = derived.exchange(m2);
    expected = Money(m2.value()*eur_gbp.rate()/eur_usd.rate(), GBP);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

void ExchangeRateTest::testDirectLookup() {

    BOOST_TEST_MESSAGE("Testing lookup of direct exchange rates...");

    ExchangeRateManager& rateManager = ExchangeRateManager::instance();
    rateManager.clear();

    Currency EUR = EURCurrency(), USD = USDCurrency();

    ExchangeRate eur_usd1 = ExchangeRate(EUR, USD, 1.1983);
    ExchangeRate eur_usd2 = ExchangeRate(USD, EUR, 1.0/1.2042);
    rateManager.add(eur_usd1, Date(4,August,2004));
    rateManager.add(eur_usd2, Date(5,August,2004));

    Money m1 = 50000.0 * EUR;
    Money m2 = 100000.0 * USD;

    Money::Settings::instance().conversionType() = Money::NoConversion;

    ExchangeRate eur_usd = rateManager.lookup(EUR, USD,
                                              Date(4,August,2004),
                                              ExchangeRate::Direct);
    Money calculated = eur_usd.exchange(m1);
    Money expected(m1.value()*eur_usd1.rate(), USD);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    eur_usd = rateManager.lookup(EUR, USD,
                                 Date(5,August,2004),
                                 ExchangeRate::Direct);
    calculated = eur_usd.exchange(m1);
    expected = Money(m1.value()/eur_usd2.rate(), USD);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    ExchangeRate usd_eur = rateManager.lookup(USD, EUR,
                                              Date(4,August,2004),
                                              ExchangeRate::Direct);

    calculated = usd_eur.exchange(m2);
    expected = Money(m2.value()/eur_usd1.rate(), EUR);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    usd_eur = rateManager.lookup(USD, EUR,
                                 Date(5,August,2004),
                                 ExchangeRate::Direct);

    calculated = usd_eur.exchange(m2);
    expected = Money(m2.value()*eur_usd2.rate(), EUR);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

void ExchangeRateTest::testTriangulatedLookup() {

    BOOST_TEST_MESSAGE("Testing lookup of triangulated exchange rates...");

    ExchangeRateManager& rateManager = ExchangeRateManager::instance();
    rateManager.clear();

    Currency EUR = EURCurrency(), USD = USDCurrency(), ITL = ITLCurrency();

    ExchangeRate eur_usd1 = ExchangeRate(EUR, USD, 1.1983);
    ExchangeRate eur_usd2 = ExchangeRate(EUR, USD, 1.2042);
    rateManager.add(eur_usd1, Date(4,August,2004));
    rateManager.add(eur_usd2, Date(5,August,2004));

    Money m1 = 50000000.0 * ITL;
    Money m2 = 100000.0 * USD;

    Money::Settings::instance().conversionType() = Money::NoConversion;

    ExchangeRate itl_usd = rateManager.lookup(ITL, USD,
                                              Date(4,August,2004));
    Money calculated = itl_usd.exchange(m1);
    Money expected(m1.value()*eur_usd1.rate()/1936.27, USD);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    itl_usd = rateManager.lookup(ITL, USD,
                                 Date(5,August,2004));
    calculated = itl_usd.exchange(m1);
    expected = Money(m1.value()*eur_usd2.rate()/1936.27, USD);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    ExchangeRate usd_itl = rateManager.lookup(USD, ITL,
                                              Date(4,August,2004));

    calculated = usd_itl.exchange(m2);
    expected = Money(m2.value()*1936.27/eur_usd1.rate(), ITL);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    usd_itl = rateManager.lookup(USD, ITL,
                                 Date(5,August,2004));

    calculated = usd_itl.exchange(m2);
    expected = Money(m2.value()*1936.27/eur_usd2.rate(), ITL);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

void ExchangeRateTest::testSmartLookup() {

    BOOST_TEST_MESSAGE("Testing lookup of derived exchange rates...");

    Currency EUR = EURCurrency(), USD = USDCurrency(), GBP = GBPCurrency(),
             CHF = CHFCurrency(), SEK = SEKCurrency(), JPY = JPYCurrency();

    ExchangeRateManager& rateManager = ExchangeRateManager::instance();
    rateManager.clear();

    ExchangeRate eur_usd1 = ExchangeRate(EUR, USD, 1.1983);
    ExchangeRate eur_usd2 = ExchangeRate(USD, EUR, 1.0/1.2042);
    rateManager.add(eur_usd1, Date(4,August,2004));
    rateManager.add(eur_usd2, Date(5,August,2004));

    ExchangeRate eur_gbp1 = ExchangeRate(GBP, EUR, 1.0/0.6596);
    ExchangeRate eur_gbp2 = ExchangeRate(EUR, GBP, 0.6612);
    rateManager.add(eur_gbp1, Date(4,August,2004));
    rateManager.add(eur_gbp2, Date(5,August,2004));

    ExchangeRate usd_chf1 = ExchangeRate(USD, CHF, 1.2847);
    ExchangeRate usd_chf2 = ExchangeRate(CHF, USD, 1.0/1.2774);
    rateManager.add(usd_chf1, Date(4,August,2004));
    rateManager.add(usd_chf2, Date(5,August,2004));

    ExchangeRate chf_sek1 = ExchangeRate(SEK, CHF, 0.1674);
    ExchangeRate chf_sek2 = ExchangeRate(CHF, SEK, 1.0/0.1677);
    rateManager.add(chf_sek1, Date(4,August,2004));
    rateManager.add(chf_sek2, Date(5,August,2004));

    ExchangeRate jpy_sek1 = ExchangeRate(SEK, JPY, 14.5450);
    ExchangeRate jpy_sek2 = ExchangeRate(JPY, SEK, 1.0/14.6110);
    rateManager.add(jpy_sek1, Date(4,August,2004));
    rateManager.add(jpy_sek2, Date(5,August,2004));

    Money m1 = 100000.0 * USD;
    Money m2 = 100000.0 * EUR;
    Money m3 = 100000.0 * GBP;
    Money m4 = 100000.0 * CHF;
    Money m5 = 100000.0 * SEK;
    Money m6 = 100000.0 * JPY;

    Money::Settings::instance().conversionType() = Money::NoConversion;

    // two-rate chain

    ExchangeRate usd_sek = rateManager.lookup(USD, SEK,
                                              Date(4,August,2004));
    Money calculated = usd_sek.exchange(m1);
    Money expected(m1.value()*usd_chf1.rate()/chf_sek1.rate(), SEK);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    usd_sek = rateManager.lookup(SEK, USD, Date(5,August,2004));
    calculated = usd_sek.exchange(m5);
    expected = Money(m5.value()*usd_chf2.rate()/chf_sek2.rate(), USD);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    // three-rate chain

    ExchangeRate eur_sek = rateManager.lookup(EUR, SEK,
                                              Date(4,August,2004));
    calculated = eur_sek.exchange(m2);
    expected = Money(m2.value()*eur_usd1.rate()
                               *usd_chf1.rate()/chf_sek1.rate(), SEK);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    eur_sek = rateManager.lookup(SEK, EUR, Date(5,August,2004));
    calculated = eur_sek.exchange(m5);
    expected = Money(m5.value()*eur_usd2.rate()
                               *usd_chf2.rate()/chf_sek2.rate(), EUR);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    // four-rate chain

    ExchangeRate eur_jpy = rateManager.lookup(EUR, JPY,
                                              Date(4,August,2004));
    calculated = eur_jpy.exchange(m2);
    expected = Money(m2.value()*eur_usd1.rate()*usd_chf1.rate()
                               *jpy_sek1.rate()/chf_sek1.rate(), JPY);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    eur_jpy = rateManager.lookup(JPY, EUR, Date(5,August,2004));
    calculated = eur_jpy.exchange(m6);
    expected = Money(m6.value()*jpy_sek2.rate()*eur_usd2.rate()
                               *usd_chf2.rate()/chf_sek2.rate(), EUR);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    // five-rate chain

    ExchangeRate gbp_jpy = rateManager.lookup(GBP, JPY,
                                              Date(4,August,2004));
    calculated = gbp_jpy.exchange(m3);
    expected = Money(m3.value()*eur_gbp1.rate()*eur_usd1.rate()*usd_chf1.rate()
                               *jpy_sek1.rate()/chf_sek1.rate(), JPY);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }

    gbp_jpy = rateManager.lookup(JPY, GBP, Date(5,August,2004));
    calculated = gbp_jpy.exchange(m6);
    expected = Money(m6.value()*jpy_sek2.rate()*eur_usd2.rate()*usd_chf2.rate()
                               *eur_gbp2.rate()/chf_sek2.rate(), GBP);

    if (!close(calculated,expected)) {
        BOOST_FAIL("Wrong result: \n"
                   << "    expected:   " << expected << "\n"
                   << "    calculated: " << calculated);
    }
}

test_suite* ExchangeRateTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Exchange-rate tests");
    suite->add(QUANTLIB_TEST_CASE(&ExchangeRateTest::testDirect));
    suite->add(QUANTLIB_TEST_CASE(&ExchangeRateTest::testDerived));
    suite->add(QUANTLIB_TEST_CASE(&ExchangeRateTest::testDirectLookup));
    suite->add(QUANTLIB_TEST_CASE(&ExchangeRateTest::testTriangulatedLookup));
    suite->add(QUANTLIB_TEST_CASE(&ExchangeRateTest::testSmartLookup));
    return suite;
}

