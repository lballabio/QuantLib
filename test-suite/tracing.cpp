
/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "tracing.hpp"
#include "utilities.hpp"
#include <ql/settings.hpp>
#include <sstream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    std::ostream* original;

    void setup() {
        original = &(Settings::instance().tracing().stream());
    }

    void teardown() {
        Settings::instance().tracing().setStream(*original);
    }

    void testTraceOutput(bool enable, Tracing::Level level,
                         const std::string& result) {
        std::ostringstream output;
        if (enable)
            Settings::instance().tracing().enable();
        else
            Settings::instance().tracing().disable();
        Settings::instance().tracing().setLevel(level);
        Settings::instance().tracing().setStream(output);
        QL_TRACE(Tracing::Coarse, "C");
        QL_TRACE(Tracing::Normal, "N");
        QL_TRACE(Tracing::Detailed, "D");

        #if defined(QL_ENABLE_TRACING)
        std::string expected = result;
        #else
        std::string expected = "";
        #endif
        if (output.str() != expected) {
            BOOST_FAIL("wrong trace:\n"
                       "    expected:\n"
                       "\""+ expected + "\"\n"
                       "    written:\n"
                       "\""+ output.str() + "\"");
        }
    }

}

// tests

void TracingTest::testOutput() {

    BOOST_MESSAGE("Testing tracing...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testTraceOutput(false, Tracing::Coarse, "");
    testTraceOutput(false, Tracing::Normal, "");
    testTraceOutput(false, Tracing::Detailed, "");
    testTraceOutput(true, Tracing::Coarse, "C\n");
    testTraceOutput(true, Tracing::Normal, "C\nN\n");
    testTraceOutput(true, Tracing::Detailed, "C\nN\nD\n");

    QL_TEST_TEARDOWN
}


test_suite* TracingTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Tracing tests");

    suite->add(BOOST_TEST_CASE(&TracingTest::testOutput));
    return suite;
}

