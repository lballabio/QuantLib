/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

// ===========================================================================
// NOTE: The following copyright notice applies to the Halton algorithm
// implementation, which was taken from "Monte Carlo Methods in Finance",
// by Peter Jäckel
//
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#include <ql/RandomNumbers/haltonrsg.hpp>
#include <ql/RandomNumbers/rngtraits.hpp>
#include <ql/Math/primenumbers.hpp>

namespace QuantLib {

    HaltonRsg::HaltonRsg(Size dimensionality, unsigned long seed,
                         bool randomStart, bool randomShift)
    : dimensionality_(dimensionality), sequenceCounter_(0),
      sequence_(Array(dimensionality), 1.0),
      randomStart_(dimensionality, 0UL),
      randomShift_(dimensionality, 0.0) {

        if (randomStart || randomShift) {
            RandomSequenceGenerator<MersenneTwisterUniformRng>
                uniformRsg(dimensionality_, seed);
            if (randomStart)
                randomStart_ = uniformRsg.nextInt32Sequence();
            if (randomShift)
                randomShift_ = uniformRsg.nextSequence().value;
        }

    }

    const HaltonRsg::sample_type& HaltonRsg::nextSequence() const {
        ++sequenceCounter_;
        unsigned long b, k;
        double f, h;
        for (Size i=0; i<dimensionality_; ++i) {
            h = 0.0;
            b = PrimeNumbers::get(i);
            f = 1.0;
            k = sequenceCounter_+randomStart_[i];
            while (k) {
                f /= b;
                h += (k%b)*f;
                k /= b;
            }
            sequence_.value[i] = h+randomShift_[i];
            sequence_.value[i] -= long(sequence_.value[i]);
        }
        return sequence_;
    }

}

