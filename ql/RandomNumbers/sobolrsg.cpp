
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
#include <ql/RandomNumbers/primitivepolynomials.h>
#include <ql/RandomNumbers/mt19937uniformrng.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        namespace {

            // coefficients of the free direction integers as given
            // by Jäckel

            const unsigned long degree01initializers[] = {
                1UL, 0UL };
            const unsigned long degree02initializers[] = {
                1UL, 1UL, 0UL };
            const unsigned long degree03initializers[] = {
                1UL, 3UL, 7UL, 0UL };
            const unsigned long degree04initializers[] = {
                1UL, 1UL, 5UL, 0UL };
            const unsigned long degree05initializers[] = {
                1UL, 3UL, 1UL, 1UL, 0UL };
            const unsigned long degree06initializers[] = {
                1UL, 1UL, 3UL, 7UL, 0UL };
            const unsigned long degree07initializers[] = {
                1UL, 3UL, 3UL, 9UL, 9UL, 0UL };
            const unsigned long degree08initializers[] = {
                1UL, 3UL, 7UL, 7UL, 21UL, 0UL };
            const unsigned long degree09initializers[] = {
                1UL, 1UL, 5UL, 11UL, 27UL, 0UL };
            const unsigned long degree10initializers[] = {
                1UL, 1UL, 7UL, 3UL, 29UL, 0UL };
            const unsigned long degree11initializers[] = {
                1UL, 3UL, 7UL, 13UL, 3UL, 0UL };
            const unsigned long degree12initializers[] = {
                1UL, 3UL, 5UL, 1UL, 15UL, 0UL };
            const unsigned long degree13initializers[] = {
                1UL, 1UL, 1UL, 9UL, 23UL, 37UL, 0UL };
            const unsigned long degree14initializers[] = {
                1UL, 1UL, 3UL, 13UL, 11UL, 7UL, 0UL };
            const unsigned long degree15initializers[] = {
                1UL, 3UL, 3UL, 5UL, 19UL, 33UL, 0UL };
            const unsigned long degree16initializers[] = {
                1UL, 1UL, 7UL, 13UL, 25UL, 5UL, 0UL };
            const unsigned long degree17initializers[] = {
                1UL, 1UL, 1UL, 3UL, 13UL, 39UL, 0UL };
            const unsigned long degree18initializers[] = {
                1UL, 3UL, 5UL, 11UL, 7UL, 11UL, 0UL };
            const unsigned long degree19initializers[] = {
                1UL, 3UL, 1UL, 7UL, 3UL, 23UL, 79UL, 0UL };
            const unsigned long degree20initializers[] = {
                1UL, 3UL, 1UL, 15UL, 17UL, 63UL, 13UL, 0UL };
            const unsigned long degree21initializers[] = {
                1UL, 3UL, 3UL, 3UL, 25UL, 17UL, 115UL, 0UL };
            const unsigned long degree22initializers[] = {
                1UL, 3UL, 7UL, 9UL, 31UL, 29UL, 17UL, 0UL };
            const unsigned long degree23initializers[] = {
                1UL, 1UL, 3UL, 15UL, 29UL, 15UL, 41UL, 0UL };
            const unsigned long degree24initializers[] = {
                1UL, 3UL, 1UL, 9UL, 5UL, 21UL, 119UL, 0UL };
            const unsigned long degree25initializers[] = {
                1UL, 1UL, 5UL, 5UL, 1UL, 27UL, 33UL, 0UL };
            const unsigned long degree26initializers[] = {
                1UL, 1UL, 3UL, 1UL, 23UL, 13UL, 75UL, 0UL };
            const unsigned long degree27initializers[] = {
                1UL, 1UL, 7UL, 7UL, 19UL, 25UL, 105UL, 0UL };
            const unsigned long degree28initializers[] = {
                1UL, 3UL, 5UL, 5UL, 21UL, 9UL, 7UL, 0UL };
            const unsigned long degree29initializers[] = {
                1UL, 1UL, 1UL, 15UL, 5UL, 49UL, 59UL, 0UL };
            const unsigned long degree30initializers[] = {
                1UL, 3UL, 5UL, 15UL, 17UL, 19UL, 21UL, 0UL };
            const unsigned long degree31initializers[] = {
                1UL, 1UL, 7UL, 11UL, 13UL, 29UL, 3UL, 0UL };

            const unsigned long * const initializers[31] = {
                degree01initializers,
                degree02initializers,
                degree03initializers,
                degree04initializers,
                degree05initializers,
                degree06initializers,
                degree07initializers,
                degree08initializers,
                degree09initializers,
                degree10initializers,
                degree11initializers,
                degree12initializers,
                degree13initializers,
                degree14initializers,
                degree15initializers,
                degree16initializers,
                degree17initializers,
                degree18initializers,
                degree19initializers,
                degree20initializers,
                degree21initializers,
                degree22initializers,
                degree23initializers,
                degree24initializers,
                degree25initializers,
                degree26initializers,
                degree27initializers,
                degree28initializers,
                degree29initializers,
                degree30initializers,
                degree31initializers
            };

        }

        const int SobolRsg::bits_ = 8*sizeof(unsigned long);
        // 1/(2^bits_) (written as (1/2)/(2^(bits_-1)) to avoid long overflow)
        const double SobolRsg::normalizationFactor_ = 
            0.5/(1UL<<(SobolRsg::bits_-1));

        SobolRsg::SobolRsg(Size dimensionality, unsigned long seed)
        : dimensionality_(dimensionality), sequenceCounter_(0),
          sequence_(Array(dimensionality), 1.0),
          integerSequence_(dimensionality, 0),
          directionIntegers_(dimensionality,std::vector<unsigned long>(bits_))
        {
            QL_REQUIRE(dimensionality<=PPMT_MAX_DIM,
                "SobolRsg::SobolRsg : dimensionality "
                + IntegerFormatter::toString(dimensionality) +
                " exceeds the number of available primitive polynomials modulo"
                " two (" + IntegerFormatter::toString(PPMT_MAX_DIM) +")");

            std::vector<unsigned int> degree(dimensionality_);
            std::vector<long> ppmt(dimensionality_);
            // initializes coefficient array of the k-th primitive polynomial
            // and degree of the k-th primitive polynomial
            Size k, index, currentDegree;
            for (k=0, index=0, currentDegree = 0;
                      k<dimensionality_; k++, index++) {
                long polynomial = PrimitivePolynomials[currentDegree][index];
                if (polynomial==-1) {
                    ++currentDegree;
                    index=0;
                    polynomial = PrimitivePolynomials[currentDegree][index];
                }
                degree[k] = currentDegree;
                ppmt[k] = polynomial;
            }

            // initializes bits_ direction integers for each dimension
            // and store them into directionIntegers_[dimensionality_][bits_]
            //
            // In each dimension k with its associated primitive polynomial,
            // the first degree_[k] direction integers
            // directionIntegers_[k][l] for l=0,...,degree_[k]-1 can be
            // chosen freely provided that only the l leftmost bits can be
            // non-zero, and that the l-th leftmost bit of
            // directionIntegers_[k][l] must be set

            // degenerate (no free integers) first dimension
            int j;
            for(j=0; j<bits_; j++)
                directionIntegers_[0][j] = (1UL<<(bits_-j-1));

            unsigned long maxTabulated = 
                sizeof(initializers)/sizeof(unsigned long *)+1;
            // dimensions from 2 to maxTabulated included are initialized
            // from tabulated coefficients
            for (k=1; k<QL_MIN(dimensionality_,maxTabulated); k++) {
                j = 0;
                while (initializers[k-1][j] != 0UL) {
                    directionIntegers_[k][j] =
                        (initializers[k-1][j] << (bits_-j-1));
                    j++;
                }
            }

            // random initialization for higher dimensions
            if (dimensionality_>maxTabulated) {
                MersenneTwisterUniformRng uniformRng(seed);
                for (k=maxTabulated; k<dimensionality_; k++) {
                    for (Size l=1; l<=degree[k]; l++) {
                        unsigned long n;
                        do {
                            double u = uniformRng.next().value;
                            // u is in (0,1)
                            // n has at most the rightmost l bits non-zero
                            n = (unsigned long)(u*(1UL<<l));
                        } while (n & 1UL); // requiring odd number

                        // shifting bits_-l bits to the left
                        directionIntegers_[k][l] = (n<<(bits_-l));
                    }
                }
            }

            // computation of directionIntegers_[k][l] for l>=degree_[k]
            // by recurrence relation
            for (k=1; k<dimensionality_; k++) {
                unsigned int gk = degree[k];
                for (int l=gk; l<bits_; l++) {
                    unsigned long n = (directionIntegers_[k][l-gk]>>gk);
                    for (Size j=1; j<gk; j++) {
                        if ((ppmt[k] >> (gk-j-1)) & 1UL)
                            n ^= directionIntegers_[k][l-j];
                    }
                    n ^= directionIntegers_[k][l-gk];
                    directionIntegers_[k][l]=n;
                }
            }

            // initialize the Sobol integer/double vectors
            for(k=0; k<dimensionality_; k++) {
                integerSequence_[k]=directionIntegers_[k][0];
                // the following is probably useless
                // since we reject the zero-th draw
                sequence_.value[k] = integerSequence_[k]*normalizationFactor_;
            }

        }


        const SobolRsg::sample_type& SobolRsg::nextSequence() const {
            // increment the counter (and avoid the zero-th draw)
            sequenceCounter_++;
            // did we overflow?
//            QL_REQUIRE(sequenceCounter_ != 0,
//                       "SobolRsg::nextSequence() : "
//                       "period exceeded");

            // instead of using the counter n as new unique generating integer
            // for the n-th draw use the Gray code G(n) as proposed
            // by Antonov and Saleev
            unsigned long n=sequenceCounter_;
            // Find rightmost zero bit of n
            int j=0;
            while (n & 1) { n >>= 1; j++; }
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
