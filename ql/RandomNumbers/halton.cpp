
// ===========================================================================
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

/*! \file halton.cpp
    \brief Halton low-discrepancy sequence generator

    \fullpath
    ql/RandomNumbers/%halton.cpp
*/

// $Id$

#include <ql/RandomNumbers/halton.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        const std::vector<double>& Halton::nextUniformVector( void ) {
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
                sequenceVector_[i] = h;
            }
            return sequenceVector_;
        }

    }

}

