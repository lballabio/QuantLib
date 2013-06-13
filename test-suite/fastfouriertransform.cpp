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
#include <ql/math/fastfouriertransform.hpp>
#include <ql/math/array.hpp>
#include <complex>
#include <vector>
#include <functional>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using namespace std;

void FastFourierTransformTest::testSimple() {
    BOOST_TEST_MESSAGE("Testing complex direct FFT...");
    typedef std::complex<Real> cx;
    cx a[] = { cx(0,0), cx(1,1), cx(3,3), cx(4,4),
               cx(4,4), cx(3,3), cx(1,1), cx(0,0) };
    cx b[8];
    FastFourierTransform fft(3);
    fft.transform(a, a+8, b);
    cx expected[] = { cx(16,16), cx(-4.8284,-11.6569),
                      cx(0,0),   cx(-0.3431,0.8284),
                      cx(0,0),   cx(0.8284, -0.3431),
                      cx(0,0),   cx(-11.6569,-4.8284) };
    for (size_t i = 0; i<8; i++) {
        if ((std::fabs(b[i].real() - expected[i].real()) > 1.0e-2) ||
            (std::fabs(b[i].imag() - expected[i].imag()) > 1.0e-2))
            BOOST_ERROR("Convolution(" << i << ")\n"
                        << std::setprecision(4) << QL_SCIENTIFIC
                        << "    calculated: " << b[i] << "\n"
                        << "    expected:   " << expected[i]);
    }
}

void FastFourierTransformTest::testInverse() {
    BOOST_TEST_MESSAGE("Testing convolution via inverse FFT...");
    Array x(3);
    x[0] = 1;
    x[1] = 2;
    x[2] = 3;

    size_t order = FastFourierTransform::min_order(x.size())+1;
    FastFourierTransform fft(order);
    size_t nFrq = fft.output_size();
    std::vector< std::complex<Real> > ft (nFrq);
    std::vector< Real > tmp (nFrq);
    std::complex<Real> z = std::complex<Real>();

    fft.inverse_transform(x.begin(), x.end(), ft.begin());
    for (Size i=0; i<nFrq; ++i) {
        tmp[i] = std::norm<Real>(ft[i]);
        ft[i] = z;
    }
    fft.inverse_transform(tmp.begin(), tmp.end(), ft.begin());

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
    suite->add(QUANTLIB_TEST_CASE(&FastFourierTransformTest::testSimple));
    suite->add(QUANTLIB_TEST_CASE(&FastFourierTransformTest::testInverse));
    return suite;
}

