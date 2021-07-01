/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2021 Marcin Rybacki

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

#include "currency.hpp"
#include "utilities.hpp"
#include <ql/currency.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CurrencyTest::testBespokeConstructor() {
    BOOST_TEST_MESSAGE("Testing bespoke currency constructor...");

    std::string name("Some Currency");
    std::string code("CCY");
    std::string symbol("#");

    Currency customCcy(name, code, 100, symbol, "", 100, Rounding(), "");

    if (customCcy.empty())
        BOOST_ERROR("Failed to create bespoke currency.");

    if (customCcy.name() != name)
        BOOST_ERROR("incorrect currency name\n"
                    << "    actual:    " << customCcy.name() << "\n"
                    << "    expected:    " << name << "\n");

    if (customCcy.code() != code)
        BOOST_ERROR("incorrect currency code\n"
                    << "    actual:    " << customCcy.code() << "\n"
                    << "    expected:    " << code << "\n");

    if (customCcy.symbol() != symbol)
        BOOST_ERROR("incorrect currency symbol\n"
                    << "    actual:    " << customCcy.symbol() << "\n"
                    << "    expected:    " << symbol << "\n");
}

test_suite* CurrencyTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Currency tests");

    suite->add(QUANTLIB_TEST_CASE(&CurrencyTest::testBespokeConstructor));

    return suite;
}
