/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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
#include <ql/utilities/tracing.hpp>
#include <sstream>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    class TestCaseCleaner { // NOLINT(cppcoreguidelines-special-member-functions)
      public:
        TestCaseCleaner() = default;
        ~TestCaseCleaner() {
            QL_TRACE_ON(std::cerr);
        }
    };

#if defined(__clang__) && __clang_major__ >= 14
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

    void testTraceOutput(bool enable,
#if defined(QL_ENABLE_TRACING)
                         const std::string& result) {
#else
                         const std::string&) {
#endif

        TestCaseCleaner cleaner;

        std::ostringstream output;
        if (enable)
            QL_TRACE_ENABLE;
        else
            QL_TRACE_DISABLE;
        QL_TRACE_ON(output);
        int i = 42;
        QL_TRACE_VARIABLE(i);
        i++;

        #if defined(QL_ENABLE_TRACING)
        std::string expected = result;
        #else
        std::string expected;
        #endif
        if (output.str() != expected) {
            BOOST_FAIL("wrong trace:\n"
                       "    expected:\n"
                       "\""+ expected + "\"\n"
                       "    written:\n"
                       "\""+ output.str() + "\"");
        }
    }

#if defined(__clang__) && __clang_major__ >= 14
#pragma clang diagnostic pop
#endif

}

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(TracingTest)

BOOST_AUTO_TEST_CASE(testOutput) {

    BOOST_TEST_MESSAGE("Testing tracing...");

    testTraceOutput(false, "");
    testTraceOutput(true,  "trace[0]: i = 42\n");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()