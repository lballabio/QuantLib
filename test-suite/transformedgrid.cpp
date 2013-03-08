/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

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

#include "transformedgrid.hpp"
#include "utilities.hpp"
#include <ql/math/transformedgrid.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/grid.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

class PlusOne : std::unary_function<Real,Real> {
public:
    Real operator()(Real x) const { return x+1;};
};

void TransformedGridTest::testConstruction() {

    BOOST_TEST_MESSAGE("Testing transformed grid construction...");

    PlusOne p1;
    Array grid = BoundedGrid(0, 100, 100);
    TransformedGrid tg(grid, p1);
    if (std::fabs(tg.grid(0) - 0.0) > 1e-5) {
        BOOST_ERROR("grid creation failed");
    }

    if (std::fabs(tg.transformedGrid(0) - 1.0) > 1e-5)
        BOOST_ERROR("grid transformation failed");
}

test_suite* TransformedGridTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("transformed grid");
    suite->add(QUANTLIB_TEST_CASE(&TransformedGridTest::testConstruction));
    return suite;
}

