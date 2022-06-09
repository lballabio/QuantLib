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

#include "array.hpp"
#include "utilities.hpp"
#include <ql/math/array.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace array_test {
    class FSquared {
      public:
        Real operator()(Real x) const { return x*x; }
    };
}

void ArrayTest::testConstruction() {

    BOOST_TEST_MESSAGE("Testing array construction...");

    using namespace array_test;

    // empty array
    Array a1;
    if (!a1.empty())
        BOOST_ERROR("default-initialized array is not empty "
                    "(size = " << a1.size() << ")");

    // sized array
    Size size = 5;
    Array a2(size);
    if (a2.size() != size)
        BOOST_ERROR("array not of the required size"
                    << "\n    required:  " << size
                    << "\n    resulting: " << a2.size());

    // sized array, constant values
    Real value = 42.0;
    Array a3(size, value);
    if (a3.size() != size)
        BOOST_ERROR("array not of the required size"
                    << "\n    required:  " << size
                    << "\n    resulting: " << a3.size());
    Size i;
    for (i=0; i<size; ++i) {
        if (a3[i] != value)
            BOOST_ERROR(io::ordinal(i+1) << " element not with required value"
                        << "\n    required:  " << value
                        << "\n    resulting: " << a3[i]);
    }

    // sized array, incremental values
    Real increment = 3.0;
    Array a4(size, value, increment);
    if (a4.size() != size)
        BOOST_ERROR("array not of the required size"
                    << "\n    required:  " << size
                    << "\n    resulting: " << a4.size());
    for (i=0; i<size; i++) {
        if (a4[i] != value + i*increment)
            BOOST_ERROR(io::ordinal(i+1) << " element not with required value"
                        << "\n    required:  " << value + i*increment
                        << "\n    resulting: " << a4[i]);
    }

    // copy constructor
    Array a5(a1);  // NOLINT(performance-unnecessary-copy-initialization)
    if (a5.size() != a1.size())
        BOOST_ERROR("copy not of the same size as original"
                    << "\n    original:  " << a1.size()
                    << "\n    copy:      " << a5.size());

    Array a6(a3);
    if (a6.size() != a3.size())
        BOOST_ERROR("copy not of the same size as original"
                    << "\n    original:  " << a3.size()
                    << "\n    copy:      " << a6.size());
    for (i=0; i<a3.size(); i++) {
        if (a6[i] != a3[i])
            BOOST_ERROR(io::ordinal(i+1) << " element of copy "
                        "not with same value as original"
                        << "\n    original:  " << a3[i]
                        << "\n    copy:      " << a6[i]);
    }

    // transform
    Array a10(5);
    for (i=0; i < a10.size(); i++) {
        a10[i] = static_cast<Real>(i);
    }
    FSquared f2;
    std::transform(a10.begin(), a10.end(), a10.begin(), FSquared());
    for (i=0; i < a10.size(); i++) {
        Real calculated = f2(static_cast<Real>(i));
        if (std::fabs(a10[i] -  calculated) >= 1e-5) {
            BOOST_ERROR("Array transform test failed " << a10[i] << " "
                        << calculated);
        }
    }
}

void ArrayTest::testArrayFunctions() {

    BOOST_TEST_MESSAGE("Testing array functions...");

    Array a(5);
    for (Size i=0; i < a.size(); ++i) {
        a[i] = std::sin(Real(i))+1.1;
    }

    constexpr double exponential = -2.3;
    const Array p = Pow(a, exponential);
    const Array e = Exp(a);
    const Array l = Log(a);
    const Array s = Sqrt(a);

    constexpr double tol = 10*QL_EPSILON;
    for (Size i=0; i < a.size(); ++i) {
        if (std::fabs(p[i]-std::pow(a[i], exponential)) > tol) {
            BOOST_FAIL("Array function test Pow failed");
        }
        if (std::fabs(e[i]-std::exp(a[i])) > tol) {
            BOOST_FAIL("Array function test Exp failed");
        }
        if (std::fabs(l[i]-std::log(a[i])) > tol) {
            BOOST_FAIL("Array function test Log failed");
        }
        if (std::fabs(s[i]-std::sqrt(a[i])) > tol) {
            BOOST_FAIL("Array function test Sqrt failed");
        }
    }
}

void ArrayTest::testArrayResize() {
    BOOST_TEST_MESSAGE("Testing array resize...");

    Array a(10,1.0,1.0);

    for (Size i=0; i < 10; ++i)
        BOOST_CHECK_CLOSE(a[i], Real(1+i), 10*QL_EPSILON);

    a.resize(5);
    BOOST_CHECK(a.size() == 5);

    for (Size i=0; i < 5; ++i)
        BOOST_CHECK_CLOSE(a[i], Real(1+i), 10*QL_EPSILON);

    a.resize(15);
    BOOST_CHECK(a.size() == 15);

    for (Size i=0; i < 5; ++i)
        BOOST_CHECK_CLOSE(a[i], Real(1+i), 10*QL_EPSILON);

    const Array::const_iterator iter = a.begin();
    a.resize(a.size());
    BOOST_CHECK(iter == a.begin());

    a.resize(10);
    BOOST_CHECK(a.size() == 10);
    BOOST_CHECK(iter == a.begin());
}


test_suite* ArrayTest::suite() {
    auto* suite = BOOST_TEST_SUITE("array tests");
    suite->add(QUANTLIB_TEST_CASE(&ArrayTest::testConstruction));
    suite->add(QUANTLIB_TEST_CASE(&ArrayTest::testArrayFunctions));
    suite->add(QUANTLIB_TEST_CASE(&ArrayTest::testArrayResize));
    return suite;
}

