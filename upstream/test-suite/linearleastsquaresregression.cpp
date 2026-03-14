/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2010 Slava Mazur

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
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/linearleastsquaresregression.hpp>
#include <boost/circular_buffer.hpp>
#include <functional>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(LinearLeastSquaresRegressionTests)

BOOST_AUTO_TEST_CASE(testRegression) {

    BOOST_TEST_MESSAGE("Testing linear least-squares regression...");

    const Real tolerance = 0.05;

    const Size nr=100000;
    PseudoRandom::rng_type rng(PseudoRandom::urng_type(1234U));

    std::vector<std::function<Real(Real)>> v = {
        [](Real x) -> Real { return 1.0; },
        [](Real x) -> Real { return x; },
        [](Real x) -> Real { return x*x; },
        [](Real x) -> Real { return std::sin(x); }
    };

    std::vector<std::function<Real(Real)>> w(v);
    w.emplace_back([](Real x){ return x*x; });

    for (Size k=0; k<3; ++k) {
        Size i;
        const Real a[] = {
            rng.next().value,
            rng.next().value,
            rng.next().value,
            rng.next().value
        };

        std::vector<Real> x(nr), y(nr);
        for (i=0; i<nr; ++i) {
            x[i] = rng.next().value;

            // regression in y = a_1 + a_2*x + a_3*x^2 + a_4*sin(x) + eps
            y[i] =  a[0]*v[0](x[i]) + a[1]*v[1](x[i]) + a[2]*v[2](x[i])
                + a[3]*v[3](x[i]) + rng.next().value;
        }

        LinearRegression m(x, y, v);

        for (i=0; i<v.size(); ++i) {
            if (m.standardErrors()[i] > tolerance) {
                BOOST_ERROR("Failed to reproduce linear regression coef."
                    << "\n    error:     " << m.standardErrors()[i]
                << "\n    tolerance: " << tolerance);
            }
            if (std::fabs(m.coefficients()[i]-a[i]) > 3*m.standardErrors()[i]) {
                BOOST_ERROR("Failed to reproduce linear regression coef."
                    << "\n    calculated: " << m.coefficients()[i]
                << "\n    error:      " << m.standardErrors()[i]
                << "\n    expected:   " << a[i]);
            }
        }

        m = LinearRegression(x, y, w);

        const Real ma[] = {m.coefficients()[0], m.coefficients()[1], 
            m.coefficients()[2]+m.coefficients()[4],
            m.coefficients()[3]};
        const Real err[] = {m.standardErrors()[0], m.standardErrors()[1],
            std::sqrt( m.standardErrors()[2]*m.standardErrors()[2]
        +m.standardErrors()[4]*m.standardErrors()[4]),
            m.standardErrors()[3]};
        for (i=0; i<v.size(); ++i) {
            if (std::fabs(ma[i] - a[i]) > 3*err[i]) {
                BOOST_ERROR("Failed to reproduce linear regression coef."
                    << "\n    calculated: " << ma[i]
                << "\n    error:      " << err[i]
                << "\n    expected:   " << a[i]);
            }
        }
    }
}

struct get_item {
    Size i;
    explicit get_item(Size i) : i(i) {}
    Real operator()(const Array& a) const {
        return a[i];
    }
};


BOOST_AUTO_TEST_CASE(testMultiDimRegression) {

    BOOST_TEST_MESSAGE(
        "Testing multi-dimensional linear least-squares regression...");

    const Size nr=100000;
    const Size dims = 4;
    const Real tolerance = 0.01;
    PseudoRandom::rng_type rng(PseudoRandom::urng_type(1234U));

    std::vector<std::function<Real(Array)> > v;
    v.emplace_back([](const Array& x) { return 1.0; });
    for (Size i=0; i < dims; ++i) {
        v.emplace_back(get_item(i));
    }

    Array coeff(v.size());
    for (Size i=0; i < v.size(); ++i) {
        coeff[i] = rng.next().value;
    }

    std::vector<Real> y(nr, 0.0);
    std::vector<Array> x(nr, Array(dims));
    for (Size i=0; i < nr; ++i) {
        for (Size j=0; j < dims; ++j) {
            x[i][j] = rng.next().value;
        }

        for (Size j=0; j < v.size(); ++j) {
            y[i] += coeff[j]*v[j](x[i]);
        }
        y[i] += rng.next().value;
    }

    LinearRegression m(x, y, v);

    for (Size i=0; i < v.size(); ++i) {
        if (m.standardErrors()[i] > tolerance) {
            BOOST_ERROR("Failed to reproduce linear regression coef."
                << "\n    error:     " << m.standardErrors()[i]
            << "\n    tolerance: " << tolerance);
        }

        if (std::fabs(m.coefficients()[i]-coeff[i]) > 3*tolerance) {
            BOOST_ERROR("Failed to reproduce linear regression coef."
                << "\n    calculated: " << m.coefficients()[i]
            << "\n    error:      " << m.standardErrors()[i]
            << "\n    expected:   " << coeff[i]);
        }
    }

    // much simpler
    LinearRegression m1(x, y, Real(1.0));

    for (Size i=0; i < m1.dim(); ++i) {
        if (m1.standardErrors()[i] > tolerance) {
            BOOST_ERROR("Failed to reproduce linear regression coef."
                << "\n    error:     " << m1.standardErrors()[i]
            << "\n    tolerance: " << tolerance);
        }

        if (std::fabs(m1.coefficients()[i]-coeff[i]) > 3*tolerance) {
            BOOST_ERROR("Failed to reproduce linear regression coef."
                << "\n    calculated: " << m1.coefficients()[i]
            << "\n    error:      " << m1.standardErrors()[i]
            << "\n    expected:   " << coeff[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(test1dLinearRegression) {

    BOOST_TEST_MESSAGE("Testing 1D simple linear least-squares regression...");

    /* Example taken from the QuantLib-User list, see posting
    * Multiple linear regression/weighted regression, Boris Skorodumov */

    std::vector<Real> x = {2.4, 1.8, 2.5, 3.0, 2.1, 1.2, 2.0, 2.7, 3.6};
    std::vector<Real> y = {7.8, 5.5, 8.0, 9.0, 6.5, 4.0, 6.3, 8.4, 10.2};

    std::vector<std::function<Real(Real)>> v = {
        [](Real x) { return 1.0; },
        [](Real x) { return x; }
    };

    LinearRegression m(x, y);

    const Real tol = 0.0002;
    const Real coeffExpected[]  = { 0.9448, 2.6853 };
    const Real errorsExpected[] = { 0.3654, 0.1487 };

    for (Size i=0; i < 2; ++i) {
        if (std::fabs(m.standardErrors()[i]-errorsExpected[i]) > tol) {
            BOOST_ERROR("Failed to reproduce linear regression standard errors"
                << "\n    calculated: " << m.standardErrors()[i]
            << "\n    expected:   " << errorsExpected[i]                                          
            << "\n    tolerance:  " << tol);
        }

        if (std::fabs(m.coefficients()[i]-coeffExpected[i]) > tol) {
            BOOST_ERROR("Failed to reproduce linear regression coef."
                << "\n    calculated: " << m.coefficients()[i]
            << "\n    expected:   " << coeffExpected[i]
            << "\n    tolerance:  " << tol);
        }
    }    

    // an alternative container type 
    boost::circular_buffer<Real> cx(x.begin(), x.end()), cy(y.begin(), y.end());
    LinearRegression m1(cx, cy);

    for (Size i=0; i < 2; ++i) {
        if (std::fabs(m1.standardErrors()[i]-errorsExpected[i]) > tol) {
            BOOST_ERROR("Failed to reproduce linear regression standard errors"
                << "\n    calculated: " << m1.standardErrors()[i]
            << "\n    expected:   " << errorsExpected[i]                                          
            << "\n    tolerance:  " << tol);
        }

        if (std::fabs(m1.coefficients()[i]-coeffExpected[i]) > tol) {
            BOOST_ERROR("Failed to reproduce linear regression coef."
                << "\n    calculated: " << m1.coefficients()[i]
            << "\n    expected:   " << coeffExpected[i]
            << "\n    tolerance:  " << tol);
        }
    }    
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
