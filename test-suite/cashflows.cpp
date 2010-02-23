/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 StatPro Italia srl

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

#include "cashflows.hpp"
#include "utilities.hpp"
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/settings.hpp>

using namespace QuantLib;
using namespace boost;
using namespace boost::unit_test_framework;

void CashFlowsTest::testSettings() {

    BOOST_MESSAGE("Testing cash-flow settings...");

    SavedSettings backup;

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    // cash flows at T+0, T+1, T+2
    std::vector<shared_ptr<CashFlow> > leg;
    for (Integer i=0; i<3; ++i)
        leg.push_back(shared_ptr<CashFlow>(new SimpleCashFlow(1.0, today+i)));


    #define CHECK_INCLUSION(n, days, expected) \
    if ((!leg[n]->hasOccurred(today+days)) != expected) { \
        BOOST_ERROR("cashflow at T+" << n << " " \
                    << (expected ? "not" : "") << "included" \
                    << " at T+" << days); \
    }

    // case 1: don't include reference-date payments, no override at
    //         today's date

    Settings::instance().includeReferenceDateCashFlows() = false;
    Settings::instance().includeTodaysCashFlows() = none;

    CHECK_INCLUSION(0, 0, false);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, false);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, false);
    CHECK_INCLUSION(2, 3, false);

    // case 2: same, but with explicit setting at today's date

    Settings::instance().includeReferenceDateCashFlows() = false;
    Settings::instance().includeTodaysCashFlows() = false;

    CHECK_INCLUSION(0, 0, false);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, false);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, false);
    CHECK_INCLUSION(2, 3, false);

    // case 3: do include reference-date payments, no override at
    //         today's date

    Settings::instance().includeReferenceDateCashFlows() = true;
    Settings::instance().includeTodaysCashFlows() = none;

    CHECK_INCLUSION(0, 0, true);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, true);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, true);
    CHECK_INCLUSION(2, 3, false);

    // case 4: do include reference-date payments, explicit (and same)
    //         setting at today's date

    Settings::instance().includeReferenceDateCashFlows() = true;
    Settings::instance().includeTodaysCashFlows() = true;

    CHECK_INCLUSION(0, 0, true);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, true);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, true);
    CHECK_INCLUSION(2, 3, false);

    // case 5: do include reference-date payments, override at
    //         today's date

    Settings::instance().includeReferenceDateCashFlows() = true;
    Settings::instance().includeTodaysCashFlows() = false;

    CHECK_INCLUSION(0, 0, false);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, true);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, true);
    CHECK_INCLUSION(2, 3, false);


    // no discount to make calculations easier
    InterestRate no_discount(0.0, Actual365Fixed(), Continuous, Annual);

    #define CHECK_NPV(includeRef, expected)                             \
    do {                                                            \
        Real NPV = CashFlows::npv(leg, no_discount, includeRef, today); \
        if (std::fabs(NPV - expected) > 1e-6) {                         \
            BOOST_ERROR("NPV mismatch:\n"                               \
                        << "    calculated: " << NPV << "\n"            \
                        << "    expected: " << expected);               \
        }                                                               \
    } while (false);

    // no override
    Settings::instance().includeTodaysCashFlows() = none;

    CHECK_NPV(false, 2.0);
    CHECK_NPV(true, 3.0);
    
    // override
    Settings::instance().includeTodaysCashFlows() = false;
    
    CHECK_NPV(false, 2.0);
    CHECK_NPV(true, 2.0);

}


test_suite* CashFlowsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cash flows tests");
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testSettings));
    return suite;
}

