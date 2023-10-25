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

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ArrayTest)

BOOST_AUTO_TEST_CASE(testConstruction) {

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

BOOST_AUTO_TEST_CASE(testArrayFunctions) {

    BOOST_TEST_MESSAGE("Testing array functions...");

    auto get_array = []() {
        Array a(5);
        for (Size i=0; i < a.size(); ++i) {
            a[i] = std::sin(Real(i))+1.1;
        }
        return a;
    };

    const Array a = get_array();

    constexpr double exponential = -2.3;
    const Array p_lvalue = Pow(a, exponential);
    const Array e_lvalue = Exp(a);
    const Array l_lvalue = Log(a);
    const Array s_lvalue = Sqrt(a);
    const Array a_lvalue = Abs(a);
    const Array p_rvalue = Pow(get_array(), exponential);
    const Array e_rvalue = Exp(get_array());
    const Array l_rvalue = Log(get_array());
    const Array s_rvalue = Sqrt(get_array());
    const Array a_rvalue = Abs(get_array());

    constexpr double tol = 10*QL_EPSILON;
    for (Size i=0; i < a.size(); ++i) {
        if (std::fabs(p_lvalue[i]-std::pow(a[i], exponential)) > tol) {
            BOOST_FAIL("Array function test Pow failed (lvalue)");
        }
        if (std::fabs(p_rvalue[i]-std::pow(a[i], exponential)) > tol) {
            BOOST_FAIL("Array function test Pow failed (lvalue)");
        }
        if (std::fabs(e_lvalue[i]-std::exp(a[i])) > tol) {
            BOOST_FAIL("Array function test Exp failed (lvalue)");
        }
        if (std::fabs(e_rvalue[i]-std::exp(a[i])) > tol) {
            BOOST_FAIL("Array function test Exp failed (rvalue)");
        }
        if (std::fabs(l_lvalue[i]-std::log(a[i])) > tol) {
            BOOST_FAIL("Array function test Log failed (lvalue)");
        }
        if (std::fabs(l_rvalue[i]-std::log(a[i])) > tol) {
            BOOST_FAIL("Array function test Log failed (rvalue)");
        }
        if (std::fabs(s_lvalue[i]-std::sqrt(a[i])) > tol) {
            BOOST_FAIL("Array function test Sqrt failed (lvalue)");
        }
        if (std::fabs(s_rvalue[i]-std::sqrt(a[i])) > tol) {
            BOOST_FAIL("Array function test Sqrt failed (rvalue)");
        }
        if (std::fabs(a_lvalue[i]-std::abs(a[i])) > tol) {
            BOOST_FAIL("Array function test Abs failed (lvalue)");
        }
        if (std::fabs(a_rvalue[i]-std::abs(a[i])) > tol) {
            BOOST_FAIL("Array function test Abs failed (rvalue)");
        }
    }
}

BOOST_AUTO_TEST_CASE(testArrayResize) {
    BOOST_TEST_MESSAGE("Testing array resize...");

    Array a(10,1.0,1.0);

    for (Size i=0; i < 10; ++i)
        QL_CHECK_CLOSE(a[i], Real(1+i), 10*QL_EPSILON);

    a.resize(5);
    BOOST_CHECK(a.size() == 5);

    for (Size i=0; i < 5; ++i)
        QL_CHECK_CLOSE(a[i], Real(1+i), 10*QL_EPSILON);

    a.resize(15);
    BOOST_CHECK(a.size() == 15);

    for (Size i=0; i < 5; ++i)
        QL_CHECK_CLOSE(a[i], Real(1+i), 10*QL_EPSILON);

    const Array::const_iterator iter = a.begin();
    a.resize(a.size());
    BOOST_CHECK(iter == a.begin());

    a.resize(10);
    BOOST_CHECK(a.size() == 10);
    BOOST_CHECK(iter == a.begin());
}

#define QL_CHECK_CLOSE_ARRAY(actual, expected)                      \
    BOOST_REQUIRE(actual.size() == expected.size());                \
    for (auto i = 0u; i < actual.size(); i++) {                     \
        QL_CHECK_CLOSE(actual[i], expected[i], 100 * QL_EPSILON);   \
    }                                                               \

BOOST_AUTO_TEST_CASE(testArrayOperators) {
    BOOST_TEST_MESSAGE("Testing array operators...");

    auto get_array = []() {
        return Array{1.1, 2.2, 3.3};
    };

    const auto a = get_array();

    const auto positive = Array{1.1, 2.2, 3.3};
    const auto lvalue_positive = +a;
    const auto rvalue_positive = +get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_positive, positive);
    QL_CHECK_CLOSE_ARRAY(rvalue_positive, positive);

    const auto negative = Array{-1.1, -2.2, -3.3};
    const auto lvalue_negative = -a;
    const auto rvalue_negative = -get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_negative, negative);
    QL_CHECK_CLOSE_ARRAY(rvalue_negative, negative);

    const auto array_sum = Array{2.2, 4.4, 6.6};
    const auto lvalue_lvalue_sum = a + a;
    const auto lvalue_rvalue_sum = a + get_array();
    const auto rvalue_lvalue_sum = get_array() + a;
    const auto rvalue_rvalue_sum = get_array() + get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_lvalue_sum, array_sum);
    QL_CHECK_CLOSE_ARRAY(lvalue_rvalue_sum, array_sum);
    QL_CHECK_CLOSE_ARRAY(rvalue_lvalue_sum, array_sum);
    QL_CHECK_CLOSE_ARRAY(rvalue_rvalue_sum, array_sum);

    const auto scalar_sum = Array{2.2, 3.3, 4.4};
    const auto lvalue_real_sum = a + 1.1;
    const auto rvalue_real_sum = get_array() + 1.1;
    const auto real_lvalue_sum = 1.1 + a;
    const auto real_rvalue_sum = 1.1 + get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_real_sum, scalar_sum);
    QL_CHECK_CLOSE_ARRAY(rvalue_real_sum, scalar_sum);
    QL_CHECK_CLOSE_ARRAY(real_lvalue_sum, scalar_sum);
    QL_CHECK_CLOSE_ARRAY(real_rvalue_sum, scalar_sum);

    const auto array_difference = Array{0.0, 0.0, 0.0};
    const auto lvalue_lvalue_difference = a - a;  // NOLINT(misc-redundant-expression)
    const auto lvalue_rvalue_difference = a - get_array();
    const auto rvalue_lvalue_difference = get_array() - a;
    const auto rvalue_rvalue_difference = get_array() - get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_lvalue_difference, array_difference);
    QL_CHECK_CLOSE_ARRAY(lvalue_rvalue_difference, array_difference);
    QL_CHECK_CLOSE_ARRAY(rvalue_lvalue_difference, array_difference);
    QL_CHECK_CLOSE_ARRAY(rvalue_rvalue_difference, array_difference);

    const auto scalar_difference_1 = Array{0.0, +1.1, +2.2};
    const auto scalar_difference_2 = Array{0.0, -1.1, -2.2};
    const auto lvalue_real_difference = a - 1.1;
    const auto rvalue_real_difference = get_array() - 1.1;
    const auto real_lvalue_difference = 1.1 - a;
    const auto real_rvalue_difference = 1.1 - get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_real_difference, scalar_difference_1);
    QL_CHECK_CLOSE_ARRAY(rvalue_real_difference, scalar_difference_1);
    QL_CHECK_CLOSE_ARRAY(real_lvalue_difference, scalar_difference_2);
    QL_CHECK_CLOSE_ARRAY(real_rvalue_difference, scalar_difference_2);

    const auto array_product = Array{1.1 * 1.1, 2.2 * 2.2, 3.3 * 3.3};
    const auto lvalue_lvalue_product = a * a;
    const auto lvalue_rvalue_product = a * get_array();
    const auto rvalue_lvalue_product = get_array() * a;
    const auto rvalue_rvalue_product = get_array() * get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_lvalue_product, array_product);
    QL_CHECK_CLOSE_ARRAY(lvalue_rvalue_product, array_product);
    QL_CHECK_CLOSE_ARRAY(rvalue_lvalue_product, array_product);
    QL_CHECK_CLOSE_ARRAY(rvalue_rvalue_product, array_product);

    const auto scalar_product = Array{1.1 * 1.1, 2.2 * 1.1, 3.3 * 1.1};
    const auto lvalue_real_product = a * 1.1;
    const auto rvalue_real_product = get_array() * 1.1;
    const auto real_lvalue_product = 1.1 * a;
    const auto real_rvalue_product = 1.1 * get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_real_product, scalar_product);
    QL_CHECK_CLOSE_ARRAY(rvalue_real_product, scalar_product);
    QL_CHECK_CLOSE_ARRAY(real_lvalue_product, scalar_product);
    QL_CHECK_CLOSE_ARRAY(real_rvalue_product, scalar_product);

    const auto array_quotient = Array{1.0, 1.0, 1.0};
    const auto lvalue_lvalue_quotient = a / a;  // NOLINT(misc-redundant-expression)
    const auto lvalue_rvalue_quotient = a / get_array();
    const auto rvalue_lvalue_quotient = get_array() / a;
    const auto rvalue_rvalue_quotient = get_array() / get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_lvalue_quotient, array_quotient);
    QL_CHECK_CLOSE_ARRAY(lvalue_rvalue_quotient, array_quotient);
    QL_CHECK_CLOSE_ARRAY(rvalue_lvalue_quotient, array_quotient);
    QL_CHECK_CLOSE_ARRAY(rvalue_rvalue_quotient, array_quotient);

    const auto scalar_quotient_1 = Array{1.1 / 1.1, 2.2 / 1.1, 3.3 / 1.1};
    const auto scalar_quotient_2 = Array{1.1 / 1.1, 1.1 / 2.2, 1.1 / 3.3};
    const auto lvalue_real_quotient = a / 1.1;
    const auto rvalue_real_quotient = get_array() / 1.1;
    const auto real_lvalue_quotient = 1.1 / a;
    const auto real_rvalue_quotient = 1.1 / get_array();

    QL_CHECK_CLOSE_ARRAY(lvalue_real_quotient, scalar_quotient_1);
    QL_CHECK_CLOSE_ARRAY(rvalue_real_quotient, scalar_quotient_1);
    QL_CHECK_CLOSE_ARRAY(real_lvalue_quotient, scalar_quotient_2);
    QL_CHECK_CLOSE_ARRAY(real_rvalue_quotient, scalar_quotient_2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

