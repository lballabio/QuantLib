/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang
 Copyright (C) 2009 Liquidnet Holdings, Inc.

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

#include "fastfouriertransform.hpp"
#include "utilities.hpp"
#include <ql/experimental/math/fastfouriertransform.hpp>
#include <ql/math/array.hpp>
#include <complex>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using namespace std;

void FastFourierTransformTest::testConstruction() {
    BOOST_MESSAGE("Testing fast fourier transform construction...");
    FastFourierTransform(2);
}

void FastFourierTransformTest::testConvolution() {
    BOOST_MESSAGE("Testing convolution via FFT...");
    Array x(3);
    x[0] = 1;
    x[1] = 2;
    x[2] = 3;

    size_t nConv = 2;
    size_t Log2_N = static_cast<size_t>(std::log((double)(x.size()+nConv)) / std::log (2.0) + 1);
    FastFourierTransform fft(Log2_N);
    size_t nFrq = 1 << Log2_N;
    std::vector< std::complex<Real> > ft (nFrq);
    std::vector< Real > tmp (nFrq);

    fft.transform(x.begin(), x.end(), ft.begin());
    std::transform (ft.begin(), ft.end(), tmp.begin(), std::norm<Real>);
    std::fill (ft.begin(), ft.end(), std::complex<Real>());
    fft.transform(tmp.begin(), tmp.end(), ft.begin());

    // 0
    Real calculated = ft[0].real() / nFrq;
    Real expected = x[0]*x[0] + x[1]*x[1] + x[2]*x[2];
    if (fabs (calculated - expected) > 1.0e-10)
        BOOST_ERROR("Convolution(0)\n"
                    << std::setprecision(16) << QL_SCIENTIFIC
                    << "    calculated: " << calculated << "\n"
                    << "    expected:   " << expected);

    // 1
    calculated = ft[1].real() / nFrq;
    expected = x[0]*x[1] + x[1]*x[2];
    if (fabs (calculated - expected) > 1.0e-10)
        BOOST_ERROR("Convolution(1)\n"
                    << std::setprecision(16) << QL_SCIENTIFIC
                    << "    calculated: " << calculated << "\n"
                    << "    expected:   " << expected);

    // 2
    calculated = ft[2].real() / nFrq;
    expected = x[0]*x[2];
    if (fabs (calculated - expected) > 1.0e-10)
        BOOST_ERROR("Convolution(1)\n"
                    << std::setprecision(16) << QL_SCIENTIFIC
                    << "    calculated: " << calculated << "\n"
                    << "    expected:   " << expected);

}

test_suite* FastFourierTransformTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("fast fourier transform tests");
    suite->add(QUANTLIB_TEST_CASE(&FastFourierTransformTest::testConstruction));
    suite->add(QUANTLIB_TEST_CASE(&FastFourierTransformTest::testConvolution));
    return suite;
}

