
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

/*! \file sobolrsg.cpp
    \brief Sobol low-discrepancy sequence generator

    \fullpath
    ql/RandomNumbers/%sobolrsg.cpp
*/

// $Id$

#include <ql/RandomNumbers/sobolrsg.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        // it should be the number of bits in the machine unsigned long
        const int SobolRsg::bits_=32;
        // the inverse of the largest unsigned long
//        const double normalizationFactor_=1.0/(1L<<SobolRsg::bits_);
        const double normalizationFactor_=1.0/4294967296L;

        const SobolRsg::sample_type& SobolRsg::nextSequence() const {
            // increment the counter (and avoid the zero-th draw)
            sequenceCounter_++;
            // instead of using the counter n as new unique generating integer
            // for the n-th draw use the Gray code G(n) as proposed
            // by Antonov and Saleev
            unsigned long n=sequenceCounter_;
            // Find rightmost zero bit of n
            int j=0;
            // while(n%2==1) { n >>= 1; j++; }
            while(n & 1) { n >>= 1; j++; }
            /*
            for (j=0; j<bits_; j++) {
                if (!(n & 1)) break;
                n >>= 1;
            }
            QL_REQUIRE(j <= bits_, "bits_ is too small");
            */
            for (Size k=0; k<dimensionality_; k++) {
                // XOR the appropriate direction number into each component of
                // the integer sequence to obtain a new Sobol integer for that
                // component
                integerSequence_[k] ^= directionIntegers_[k][j];
                // normalize to get a double in (0,1)
                sequence_.value[k] = integerSequence_[k]*normalizationFactor_;
            }
            return sequence_;
        }

    }

}
