
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
// $Id$

#include "integrals.hpp"

using namespace QuantLib;
using QuantLib::Math::SegmentIntegral;
using QuantLib::Math::NormalDistribution;

namespace {

    double tolerance = 1.0e-4;

    double one(double) { return 1.0; }
    double id(double x) { return x; }
    double square(double x) { return x*x; }

    template <class F>
    void test(const SegmentIntegral& I, const std::string& tag,
              const F& f, double xMin, double xMax, double expected) {
        double calculated = I(f,xMin,xMax);
        if (QL_FABS(calculated-expected) > tolerance) {
            CPPUNIT_FAIL(
                "integrating " + tag +
                "    calculated: " + DoubleFormatter::toString(calculated) +
                "    expected:   " + DoubleFormatter::toString(expected));
        }
    }
                   
}

void IntegralTest::runTest() {

    SegmentIntegral I(10000);

    test(I, "f(x) = 1",   std::ptr_fun(one),    0.0, 1.0, 1.0);
    test(I, "f(x) = x",   std::ptr_fun(id),     0.0, 1.0, 0.5);
    test(I, "f(x) = x^2", std::ptr_fun(square), 0.0, 1.0, 1.0/3.0);

    test(I, "f(x) = sin(x)", 
         std::ptr_fun<double,double>(QL_SIN), 0.0, M_PI, 2.0);
    test(I, "f(x) = cos(x)", 
         std::ptr_fun<double,double>(QL_COS), 0.0, M_PI, 0.0);

    test(I, "f(x) = Gaussian(x)", NormalDistribution(), -10.0, 10.0, 1.0);

}

