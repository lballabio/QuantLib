
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file haltonrsg.cpp
    \brief Halton low-discrepancy sequence generator

    \fullpath
    ql/RandomNumbers/%haltonrsg.cpp
*/

// $Id$

#include <ql/RandomNumbers/haltonrsg.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        const HaltonRsg::sample_type& HaltonRsg::nextSequence() const {
            ++sequenceCounter_;
            Size b,i,k;
            double f, h;
            for (i=0; i<dimensionality_; ++i) {
                h = 0;
                b = primeNumbers_[i];
                f = 1.;
                k = sequenceCounter_;
                while (k) {
                    f /= b;
                    h += (k%b)*f;
                    k /= b;
                }
                sequence_.value[i] = h;
            }
            return sequence_;
        }

    }

}

