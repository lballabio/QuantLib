/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/math/mersennetwister_multithreaded.hpp>
#include <boost/make_shared.hpp>

#define MT_MT_DISPATCH(method)                                                 \
    switch (threadId) {                                                        \
    case 0:                                                                    \
        return m0_->method();                                                  \
    case 1:                                                                    \
        return m1_->method();                                                  \
    case 2:                                                                    \
        return m2_->method();                                                  \
    case 3:                                                                    \
        return m3_->method();                                                  \
    case 4:                                                                    \
        return m4_->method();                                                  \
    case 5:                                                                    \
        return m5_->method();                                                  \
    case 6:                                                                    \
        return m6_->method();                                                  \
    case 7:                                                                    \
        return m7_->method();                                                  \
    default:                                                                   \
        QL_FAIL("thread " << threadId << " out of range [0...7]");             \
    }

namespace QuantLib {

MersenneTwisterMultiThreaded::MersenneTwisterMultiThreaded(
    const unsigned long seed) {
    m0_ = boost::make_shared<MersenneTwisterCustomRng<Mtdesc19937_0> >(seed);
    m1_ = boost::make_shared<MersenneTwisterCustomRng<Mtdesc19937_1> >(seed);
    m2_ = boost::make_shared<MersenneTwisterCustomRng<Mtdesc19937_2> >(seed);
    m3_ = boost::make_shared<MersenneTwisterCustomRng<Mtdesc19937_3> >(seed);
    m4_ = boost::make_shared<MersenneTwisterCustomRng<Mtdesc19937_4> >(seed);
    m5_ = boost::make_shared<MersenneTwisterCustomRng<Mtdesc19937_5> >(seed);
    m6_ = boost::make_shared<MersenneTwisterCustomRng<Mtdesc19937_6> >(seed);
    m7_ = boost::make_shared<MersenneTwisterCustomRng<Mtdesc19937_7> >(seed);
}

MersenneTwisterMultiThreaded::sample_type
MersenneTwisterMultiThreaded::next(unsigned int threadId) const {
    MT_MT_DISPATCH(next);
}

Real MersenneTwisterMultiThreaded::nextReal(unsigned int threadId) const {
    MT_MT_DISPATCH(nextReal);
}

unsigned long MersenneTwisterMultiThreaded::
operator()(unsigned int threadId) const {
    MT_MT_DISPATCH(operator());
}

inline unsigned long
MersenneTwisterMultiThreaded::nextInt32(unsigned int threadId) const {
    MT_MT_DISPATCH(nextInt32);
}
}
