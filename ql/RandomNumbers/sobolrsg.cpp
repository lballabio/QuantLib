
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/RandomNumbers/sobolrsg.hpp>
#include <ql/RandomNumbers/primitivepolynomials.h>
#include <ql/RandomNumbers/mt19937uniformrng.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace {

        // coefficients of the free direction integers as given
        // by Jäckel
        const unsigned long dim02initializers[] = {
            1UL, 0UL };
        const unsigned long dim03initializers[] = {
            1UL, 1UL, 0UL };
        const unsigned long dim04initializers[] = {
            1UL, 3UL, 7UL, 0UL };
        const unsigned long dim05initializers[] = {
            1UL, 1UL, 5UL, 0UL };
        const unsigned long dim06initializers[] = {
            1UL, 3UL, 1UL, 1UL, 0UL };
        const unsigned long dim07initializers[] = {
            1UL, 1UL, 3UL, 7UL, 0UL };
        const unsigned long dim08initializers[] = {
            1UL, 3UL, 3UL, 9UL, 9UL, 0UL };
        const unsigned long dim09initializers[] = {
            1UL, 3UL, 7UL, 7UL, 21UL, 0UL };
        const unsigned long dim10initializers[] = {
            1UL, 1UL, 5UL, 11UL, 27UL, 0UL };
        const unsigned long dim11initializers[] = {
            1UL, 1UL, 7UL, 3UL, 29UL, 0UL };
        const unsigned long dim12initializers[] = {
            1UL, 3UL, 7UL, 13UL, 3UL, 0UL };
        const unsigned long dim13initializers[] = {
            1UL, 3UL, 5UL, 1UL, 15UL, 0UL };
        const unsigned long dim14initializers[] = {
            1UL, 1UL, 1UL, 9UL, 23UL, 37UL, 0UL };
        const unsigned long dim15initializers[] = {
            1UL, 1UL, 3UL, 13UL, 11UL, 7UL, 0UL };
        const unsigned long dim16initializers[] = {
            1UL, 3UL, 3UL, 5UL, 19UL, 33UL, 0UL };
        const unsigned long dim17initializers[] = {
            1UL, 1UL, 7UL, 13UL, 25UL, 5UL, 0UL };
        const unsigned long dim18initializers[] = {
            1UL, 1UL, 1UL, 3UL, 13UL, 39UL, 0UL };
        const unsigned long dim19initializers[] = {
            1UL, 3UL, 5UL, 11UL, 7UL, 11UL, 0UL };
        const unsigned long dim20initializers[] = {
            1UL, 3UL, 1UL, 7UL, 3UL, 23UL, 79UL, 0UL };
        const unsigned long dim21initializers[] = {
            1UL, 3UL, 1UL, 15UL, 17UL, 63UL, 13UL, 0UL };
        const unsigned long dim22initializers[] = {
            1UL, 3UL, 3UL, 3UL, 25UL, 17UL, 115UL, 0UL };
        const unsigned long dim23initializers[] = {
            1UL, 3UL, 7UL, 9UL, 31UL, 29UL, 17UL, 0UL };
        const unsigned long dim24initializers[] = {
            1UL, 1UL, 3UL, 15UL, 29UL, 15UL, 41UL, 0UL };
        const unsigned long dim25initializers[] = {
            1UL, 3UL, 1UL, 9UL, 5UL, 21UL, 119UL, 0UL };
        const unsigned long dim26initializers[] = {
            1UL, 1UL, 5UL, 5UL, 1UL, 27UL, 33UL, 0UL };
        const unsigned long dim27initializers[] = {
            1UL, 1UL, 3UL, 1UL, 23UL, 13UL, 75UL, 0UL };
        const unsigned long dim28initializers[] = {
            1UL, 1UL, 7UL, 7UL, 19UL, 25UL, 105UL, 0UL };
        const unsigned long dim29initializers[] = {
            1UL, 3UL, 5UL, 5UL, 21UL, 9UL, 7UL, 0UL };
        const unsigned long dim30initializers[] = {
            1UL, 1UL, 1UL, 15UL, 5UL, 49UL, 59UL, 0UL };
        const unsigned long dim31initializers[] = {
            1UL, 3UL, 5UL, 15UL, 17UL, 19UL, 21UL, 0UL };
        const unsigned long dim32initializers[] = {
            1UL, 1UL, 7UL, 11UL, 13UL, 29UL, 3UL, 0UL };

        const unsigned long * const initializers[31] = {
            dim02initializers,
            dim03initializers,
            dim04initializers,
            dim05initializers,
            dim06initializers,
            dim07initializers,
            dim08initializers,
            dim09initializers,
            dim10initializers,
            dim11initializers,
            dim12initializers,
            dim13initializers,
            dim14initializers,
            dim15initializers,
            dim16initializers,
            dim17initializers,
            dim18initializers,
            dim19initializers,
            dim20initializers,
            dim21initializers,
            dim22initializers,
            dim23initializers,
            dim24initializers,
            dim25initializers,
            dim26initializers,
            dim27initializers,
            dim28initializers,
            dim29initializers,
            dim30initializers,
            dim31initializers,
            dim32initializers
        };

    }

    const int SobolRsg::bits_ = 8*sizeof(unsigned long);
    // 1/(2^bits_) (written as (1/2)/(2^(bits_-1)) to avoid long overflow)
    const double SobolRsg::normalizationFactor_ =
    0.5/(1UL<<(SobolRsg::bits_-1));

    SobolRsg::SobolRsg(Size dimensionality, unsigned long seed,
                       bool unitInitialization)
    : dimensionality_(dimensionality),
      sequenceCounter_(0),
      firstDraw_(true), sequence_(Array(dimensionality), 1.0),
      integerSequence_(dimensionality, 0),
      directionIntegers_(dimensionality,std::vector<unsigned long>(bits_))
    {
        QL_REQUIRE(dimensionality<=PPMT_MAX_DIM,
                   "dimensionality "
                   + SizeFormatter::toString(dimensionality) +
                   " exceeds the number of available "
                   "primitive polynomials modulo two (" +
                   IntegerFormatter::toString(PPMT_MAX_DIM) +")");

        // initializes coefficient array of the k-th primitive polynomial
        // and degree of the k-th primitive polynomial
        std::vector<unsigned int> degree(dimensionality_);
        std::vector<long> ppmt(dimensionality_);
        // degree 0 is not used
        ppmt[0]=0;
        degree[0]=0;
        Size k, index;
        unsigned int currentDegree;
        for (k=1,index=0,currentDegree=1; k<dimensionality_; k++,index++){
            ppmt[k] = PrimitivePolynomials[currentDegree-1][index];
            if (ppmt[k]==-1) {
                ++currentDegree;
                index=0;
                ppmt[k] = PrimitivePolynomials[currentDegree-1][index];
            }
            degree[k] = currentDegree;
        }

        // initializes bits_ direction integers for each dimension
        // and store them into directionIntegers_[dimensionality_][bits_]
        //
        // In each dimension k with its associated primitive polynomial,
        // the first degree_[k] direction integers can be chosen freely
        // provided that only the l leftmost bits can be non-zero, and
        // that the l-th leftmost bit must be set

        // degenerate (no free direction integers) first dimension
        int j;
        for (j=0; j<bits_; j++)
            directionIntegers_[0][j] = (1UL<<(bits_-j-1));


        // maxTabulated=32
        Size maxTabulated =
            sizeof(initializers)/sizeof(unsigned long *)+1;
        // dimensions from 2 (k=1) to maxTabulated (k=maxTabulated-1)
        // included are initialized from tabulated coefficients
        for (k=1; k<QL_MIN(dimensionality_, maxTabulated); k++) {
            j = 0;
            // 0UL marks the end of the coefficients for a given dimension
            while (initializers[k-1][j] != 0UL) {
                if (unitInitialization) {
                    directionIntegers_[k][j] = 1UL;
                } else {
                    directionIntegers_[k][j] = initializers[k-1][j];
                }
                directionIntegers_[k][j] <<= (bits_-j-1);
                j++;
            }
        }

        // random initialization for higher dimensions
        if (dimensionality_>maxTabulated) {
            MersenneTwisterUniformRng uniformRng(seed);
            for (k=maxTabulated; k<dimensionality_; k++) {
                for (Size l=1; l<=degree[k]; l++) {
                    if (unitInitialization)
                        directionIntegers_[k][l-1] = 1UL;
                    else {
                        do {
                            // u is in (0,1)
                            double u = uniformRng.next().value;
                            // the direction integer has at most the
                            // rightmost l bits non-zero
                            directionIntegers_[k][l-1] =
                                (unsigned long)(u*(1UL<<l));
                        } while (!(directionIntegers_[k][l-1] & 1UL));
                        // iterate until the direction integer is odd
                        // that is it has the rightmost bit set
                    }
                    // shifting bits_-l bits to the left
                    // we are guaranteed that the l-th leftmost bit
                    // is set, and only the first l leftmost bit
                    // can be non-zero
                    directionIntegers_[k][l-1] <<= (bits_-l);
                }
            }
        }

        // computation of directionIntegers_[k][l] for l>=degree_[k]
        // by recurrence relation
        for (k=1; k<dimensionality_; k++) {
            unsigned int gk = degree[k];
            for (int l=gk; l<bits_; l++) {
                // eq. 8.19 "Monte Carlo Methods in Finance" by P. Jäckel
                unsigned long n = (directionIntegers_[k][l-gk]>>gk);
                // a[k][j] are the coefficients of the monomials in ppmt[k]
                // The highest order coefficient a[k][0] is not actually
                // used in the recurrence relation, and the lowest order
                // coefficient a[k][gk] is always set: this is the reason
                // why the highest and lowest coefficient of
                // the polynomial ppmt[k] are not included in its encoding,
                // provided that its degree is known.
                // That is: a[k][j] = ppmt[k] >> (gk-j-1)
                for (Size j=1; j<gk; j++) {
                    // XORed with a selection of (unshifted) direction
                    // integers controlled by which of the a[k][j] are set
                    if ((ppmt[k] >> (gk-j-1)) & 1UL)
                        n ^= directionIntegers_[k][l-j];
                }
                // a[k][gk] is always set, so directionIntegers_[k][l-gk]
                // will always enter
                n ^= directionIntegers_[k][l-gk];
                directionIntegers_[k][l]=n;
            }
        }

        // in case one needs to check the directionIntegers used
        /* bool printDirectionIntegers = false;
           if (printDirectionIntegers) {
               std::ofstream outStream("directionIntegers.txt");
               for (k=0; k<QL_MIN(32UL,dimensionality_); k++) {
                   outStream << std::endl << k+1       << "\t"
                                          << degree[k] << "\t"
                                          << ppmt[k]   << "\t";
                   for (j=0; j<10; j++) {
                       outStream << IntegerFormatter::toPowerOfTwo(
                           directionIntegers_[k][j], 3) << "\t";
                   }
               }
               outStream.close();
           }
        */

        // initialize the Sobol integer/double vectors
        for (k=0; k<dimensionality_; k++) {
            integerSequence_[k]=directionIntegers_[k][0];
            // first draw
            sequence_.value[k] = integerSequence_[k]*normalizationFactor_;
        }
    }


    const SobolRsg::sample_type& SobolRsg::nextSequence() const {
        if (firstDraw_) {
            // it was precomputed in the constructor
            firstDraw_ = false;
            return sequence_;
        }
        // increment the counter
        sequenceCounter_++;
        // did we overflow?
        QL_REQUIRE(sequenceCounter_ != 0, "period exceeded");

        // instead of using the counter n as new unique generating integer
        // for the n-th draw use the Gray code G(n) as proposed
        // by Antonov and Saleev
        unsigned long n = sequenceCounter_;
        // Find rightmost zero bit of n
        int j = 0;
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
