
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

/*! \file sobolrsg.hpp
    \brief Sobol low-discrepancy sequence generator

    \fullpath
    ql/RandomNumbers/%sobolrsg.hpp
*/

// $Id$

#ifndef quantlib_sobol_ld_rsg_h
#define quantlib_sobol_ld_rsg_h

#include <PrimitivePolynomialsModuloTwoUpToDegree27.h>
#include <ql/array.hpp>
#include <ql/dataformatters.hpp>
#include <ql/MonteCarlo/sample.hpp>
#include <ql/RandomNumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    namespace RandomNumbers {
        //! Sobol low-discrepancy sequence generator
        /*! Sobol low-discrepancy sequence generator using the Gray code
            counter and bitwise operations for very fast sequence generation.

            The implementation relies on primitive polynomials modulo two and
            initialization numbers from the book "Monte Carlo Methods in
            Finance" by Peter Jäckel

            Primitive polynomials modulo two are in the separate library
            PrimitivePolynomialsModuloTwo of the QuantLib CVS. There are
            8 129 334 polynomials as provided by Jäckel, but they are compiled
            only up to PPMT_MAX_DIM. If you need higher dimension you must
            recompile the PrimitivePolynomialsModuloTwo library.

            The choice of initialization numbers is crucial for the homogeneity
            properties of the sequence. Jäckel's initialization numbers are
            superior to the "unit initialization" suggested in "Numerical
            Recipes in C" by Press, Teukolsky, Vetterling, and Flannery.

            For more info on Sobol sequences see "Monte Carlo Methods in
            Finance", by Peter Jäckel, section 8.3 and "Numerical Recipes in
            C", 2nd edition, by Press, Teukolsky, Vetterling, and Flannery,
            section 7.7
        */
        class SobolRsg {
          public:
            typedef MonteCarlo::Sample<Array> sample_type;
            // dimensionality must be <= PPMT_MAX_DIM
            SobolRsg(Size dimensionality,
                     unsigned long seed = 0);
            const sample_type& nextSequence() const;
            const sample_type& lastSequence() const {
                return sequence_;
            }
            Size dimension() const {return dimensionality_;}
          private:
            static const int bits_;
            static const double normalizationFactor_;
            Size dimensionality_;
            mutable Size sequenceCounter_;
            mutable sample_type sequence_;
            mutable std::vector<unsigned long> integerSequence_;
            std::vector<std::vector<unsigned long> > directionIntegers_;
            std::vector<std::vector<unsigned long> > ppmt_;
            std::vector<unsigned long> degree_;
        };

        inline SobolRsg::SobolRsg(Size dimensionality, unsigned long seed)
        : dimensionality_(dimensionality), sequenceCounter_(0),
          sequence_(Array(dimensionality), 1.0),
          integerSequence_(dimensionality, 0),
          directionIntegers_(dimensionality,std::vector<unsigned long>(bits_)),
          ppmt_(dimensionality, std::vector<unsigned long>()),
          degree_(dimensionality) {
            QL_REQUIRE(dimensionality<=PPMT_MAX_DIM,
                "SobolRsg::SobolRsg : dimensionality "
                + IntegerFormatter::toString(dimensionality) +
                " exceeds the number of available primitive polynomials modulo"
                " two " + IntegerFormatter::toString(PPMT_MAX_DIM));

            Size degree=0, fixedDegreePolynomialCounter=0, k=0;
            long polynomial=0;
            // initializes coefficient array of the k-th primitive polynomial
            // and degree of the k-th primitive polynomial
            while (k<dimensionality_ || polynomial!=-1) {
                if (polynomial==-1) {
                    ++degree;
                    fixedDegreePolynomialCounter=0;
                }
                ppmt_[k].push_back(
                  PrimitivePolynomials[degree][fixedDegreePolynomialCounter]);
                degree_[k] = degree;
                if (polynomial==-1) {
                    --k;
                }
                ++fixedDegreePolynomialCounter;
                ++k;
            }

            // initializes bits_ direction integers for each dimension
            // and store them into directionIntegers_[dimensionality_][bits_]
            //
            // In each dimension with its associated primitive polynomials
            // ppmt_[k], the first degree_[k] direction integers
            // directionIntegers_[k][l] for l=0,...,degree_[k]-1 can be
            // chosen freely provided that only the leftmost bits can be
            // non-zero, and that the l-th leftmost bit of
            // directionIntegers_[k][l] must be set

            // first dimension
            int j;
            for(j=0; j<bits_; j++)
                directionIntegers_[0][j] = (1L<<(bits_-j-1));

            if (dimensionality_>1)
                directionIntegers_[1][0]=(1L<<(bits_-1));

            if (dimensionality_>2) {
                directionIntegers_[2][0]=(1L<<(bits_-1));
                directionIntegers_[2][1]=(1L<<(bits_-2));
            }

            if (dimensionality_>3) {
                directionIntegers_[3][0]=(1L<<(bits_-1));
                directionIntegers_[3][1]=(3L<<(bits_-2));
                directionIntegers_[3][2]=(7L<<(bits_-3));
            }

            if (dimensionality_>4) {
                directionIntegers_[4][0]=(1L<<(bits_-1));
                directionIntegers_[4][1]=(1L<<(bits_-2));
                directionIntegers_[4][2]=(5L<<(bits_-3));
            }

            if (dimensionality_>5) {
                directionIntegers_[5][0]=(1L<<(bits_-1));
                directionIntegers_[5][1]=(3L<<(bits_-2));
                directionIntegers_[5][2]=(1L<<(bits_-3));
                directionIntegers_[5][3]=(1L<<(bits_-4));
            }

            if (dimensionality_>6) {
                directionIntegers_[6][0]=(1L<<(bits_-1));
                directionIntegers_[6][1]=(1L<<(bits_-2));
                directionIntegers_[6][2]=(3L<<(bits_-3));
                directionIntegers_[6][3]=(7L<<(bits_-4));
            }

            if (dimensionality_>7) {
                directionIntegers_[7][0]=(1L<<(bits_-1));
                directionIntegers_[7][1]=(3L<<(bits_-2));
                directionIntegers_[7][2]=(3L<<(bits_-3));
                directionIntegers_[7][3]=(9L<<(bits_-4));
                directionIntegers_[7][4]=(9L<<(bits_-5));
            }

            if (dimensionality_>8) {
                directionIntegers_[8][0]=(1L<<(bits_-1));
                directionIntegers_[8][1]=(3L<<(bits_-2));
                directionIntegers_[8][2]=(7L<<(bits_-3));
                directionIntegers_[8][3]=(7L<<(bits_-4));
                directionIntegers_[8][4]=(21L<<(bits_-5));
            }

            if (dimensionality_>9) {
                directionIntegers_[9][0]=(1L<<(bits_-1));
                directionIntegers_[9][1]=(1L<<(bits_-2));
                directionIntegers_[9][2]=(5L<<(bits_-3));
                directionIntegers_[9][3]=(11L<<(bits_-4));
                directionIntegers_[9][4]=(27L<<(bits_-5));
            }

            if (dimensionality_>10) {
                directionIntegers_[10][0]=(1L<<(bits_-1));
                directionIntegers_[10][1]=(1L<<(bits_-2));
                directionIntegers_[10][2]=(7L<<(bits_-3));
                directionIntegers_[10][3]=(3L<<(bits_-4));
                directionIntegers_[10][4]=(29L<<(bits_-5));
            }

            if (dimensionality_>11) {
                directionIntegers_[11][0]=(1L<<(bits_-1));
                directionIntegers_[11][1]=(3L<<(bits_-2));
                directionIntegers_[11][2]=(7L<<(bits_-3));
                directionIntegers_[11][3]=(13L<<(bits_-4));
                directionIntegers_[11][4]=(3L<<(bits_-5));
            }

            if (dimensionality_>12) {
                directionIntegers_[12][0]=(1L<<(bits_-1));
                directionIntegers_[12][1]=(3L<<(bits_-2));
                directionIntegers_[12][2]=(5L<<(bits_-3));
                directionIntegers_[12][3]=(1L<<(bits_-4));
                directionIntegers_[12][4]=(15L<<(bits_-5));
            }

            if (dimensionality_>13) {
                directionIntegers_[13][0]=(1L<<(bits_-1));
                directionIntegers_[13][1]=(1L<<(bits_-2));
                directionIntegers_[13][2]=(1L<<(bits_-3));
                directionIntegers_[13][3]=(9L<<(bits_-4));
                directionIntegers_[13][4]=(23L<<(bits_-5));
                directionIntegers_[13][5]=(37L<<(bits_-6));
            }

            if (dimensionality_>14) {
                directionIntegers_[14][0]=(1L<<(bits_-1));
                directionIntegers_[14][1]=(1L<<(bits_-2));
                directionIntegers_[14][2]=(3L<<(bits_-3));
                directionIntegers_[14][3]=(13L<<(bits_-4));
                directionIntegers_[14][4]=(11L<<(bits_-5));
                directionIntegers_[14][0]=(7L<<(bits_-6));
            }

            if (dimensionality_>15) {
                directionIntegers_[15][0]=(1L<<(bits_-1));
                directionIntegers_[15][1]=(3L<<(bits_-2));
                directionIntegers_[15][2]=(3L<<(bits_-3));
                directionIntegers_[15][3]=(5L<<(bits_-4));
                directionIntegers_[15][4]=(19L<<(bits_-5));
                directionIntegers_[15][5]=(33L<<(bits_-6));
            }

            if (dimensionality_>16) {
                directionIntegers_[16][0]=(1L<<(bits_-1));
                directionIntegers_[16][1]=(1L<<(bits_-2));
                directionIntegers_[16][2]=(7L<<(bits_-3));
                directionIntegers_[16][3]=(13L<<(bits_-4));
                directionIntegers_[16][4]=(25L<<(bits_-5));
                directionIntegers_[16][5]=(5L<<(bits_-6));
            }

            if (dimensionality_>17) {
                directionIntegers_[17][0]=(1L<<(bits_-1));
                directionIntegers_[17][1]=(1L<<(bits_-2));
                directionIntegers_[17][2]=(1L<<(bits_-3));
                directionIntegers_[17][3]=(13L<<(bits_-4));
                directionIntegers_[17][4]=(15L<<(bits_-5));
                directionIntegers_[17][5]=(39L<<(bits_-6));
            }

            if (dimensionality_>18) {
                directionIntegers_[18][0]=(1L<<(bits_-1));
                directionIntegers_[18][1]=(3L<<(bits_-2));
                directionIntegers_[18][2]=(5L<<(bits_-3));
                directionIntegers_[18][3]=(11L<<(bits_-4));
                directionIntegers_[18][4]=(7L<<(bits_-5));
                directionIntegers_[18][5]=(11L<<(bits_-6));
            }

            if (dimensionality_>19) {
                directionIntegers_[19][0]=(1L<<(bits_-1));
                directionIntegers_[19][1]=(3L<<(bits_-2));
                directionIntegers_[19][2]=(1L<<(bits_-3));
                directionIntegers_[19][3]=(7L<<(bits_-4));
                directionIntegers_[19][4]=(3L<<(bits_-5));
                directionIntegers_[19][5]=(23L<<(bits_-6));
                directionIntegers_[19][6]=(79L<<(bits_-7));
            }

            if (dimensionality_>20) {
                directionIntegers_[20][0]=(1L<<(bits_-1));
                directionIntegers_[20][1]=(3L<<(bits_-2));
                directionIntegers_[20][2]=(1L<<(bits_-3));
                directionIntegers_[20][3]=(15L<<(bits_-4));
                directionIntegers_[20][4]=(17L<<(bits_-5));
                directionIntegers_[20][5]=(63L<<(bits_-6));
                directionIntegers_[20][6]=(13L<<(bits_-7));
            }

            if (dimensionality_>21) {
                directionIntegers_[21][0]=(1L<<(bits_-1));
                directionIntegers_[21][1]=(3L<<(bits_-2));
                directionIntegers_[21][2]=(3L<<(bits_-3));
                directionIntegers_[21][3]=(3L<<(bits_-4));
                directionIntegers_[21][4]=(25L<<(bits_-5));
                directionIntegers_[21][5]=(17L<<(bits_-6));
                directionIntegers_[21][6]=(115L<<(bits_-7));
            }

            if (dimensionality_>22) {
                directionIntegers_[22][0]=(1L<<(bits_-1));
                directionIntegers_[22][1]=(3L<<(bits_-2));
                directionIntegers_[22][2]=(7L<<(bits_-3));
                directionIntegers_[12][3]=(9L<<(bits_-4));
                directionIntegers_[22][4]=(31L<<(bits_-5));
                directionIntegers_[22][5]=(29L<<(bits_-6));
                directionIntegers_[22][6]=(17L<<(bits_-7));
            }

            if (dimensionality_>23) {
                directionIntegers_[23][0]=(1L<<(bits_-1));
                directionIntegers_[23][1]=(1L<<(bits_-2));
                directionIntegers_[23][2]=(3L<<(bits_-3));
                directionIntegers_[23][3]=(15L<<(bits_-4));
                directionIntegers_[23][4]=(29L<<(bits_-5));
                directionIntegers_[23][5]=(15L<<(bits_-6));
                directionIntegers_[23][6]=(41L<<(bits_-7));
            }

            if (dimensionality_>24) {
                directionIntegers_[24][0]=(1L<<(bits_-1));
                directionIntegers_[24][1]=(3L<<(bits_-2));
                directionIntegers_[24][2]=(1L<<(bits_-3));
                directionIntegers_[24][3]=(9L<<(bits_-4));
                directionIntegers_[24][4]=(5L<<(bits_-5));
                directionIntegers_[24][5]=(21L<<(bits_-6));
                directionIntegers_[24][6]=(119L<<(bits_-7));
            }

            if (dimensionality_>25) {
                directionIntegers_[25][0]=(1L<<(bits_-1));
                directionIntegers_[25][1]=(1L<<(bits_-2));
                directionIntegers_[25][2]=(5L<<(bits_-3));
                directionIntegers_[25][3]=(5L<<(bits_-4));
                directionIntegers_[25][4]=(1L<<(bits_-5));
                directionIntegers_[25][5]=(27L<<(bits_-6));
                directionIntegers_[25][6]=(33L<<(bits_-7));
            }

            if (dimensionality_>26) {
                directionIntegers_[26][0]=(1L<<(bits_-1));
                directionIntegers_[26][1]=(1L<<(bits_-2));
                directionIntegers_[26][2]=(3L<<(bits_-3));
                directionIntegers_[26][3]=(1L<<(bits_-4));
                directionIntegers_[26][4]=(23L<<(bits_-5));
                directionIntegers_[26][5]=(13L<<(bits_-6));
                directionIntegers_[26][6]=(75L<<(bits_-7));
            }

            if (dimensionality_>27) {
                directionIntegers_[27][0]=(1L<<(bits_-1));
                directionIntegers_[27][1]=(1L<<(bits_-2));
                directionIntegers_[27][2]=(7L<<(bits_-3));
                directionIntegers_[27][3]=(7L<<(bits_-4));
                directionIntegers_[27][4]=(19L<<(bits_-5));
                directionIntegers_[27][5]=(25L<<(bits_-6));
                directionIntegers_[27][6]=(105L<<(bits_-7));
            }

            if (dimensionality_>28) {
                directionIntegers_[28][0]=(1L<<(bits_-1));
                directionIntegers_[28][1]=(3L<<(bits_-2));
                directionIntegers_[28][2]=(5L<<(bits_-3));
                directionIntegers_[28][3]=(5L<<(bits_-4));
                directionIntegers_[28][4]=(21L<<(bits_-5));
                directionIntegers_[28][5]=(9L<<(bits_-6));
                directionIntegers_[28][6]=(7L<<(bits_-7));
            }

            if (dimensionality_>29) {
                directionIntegers_[29][0]=(1L<<(bits_-1));
                directionIntegers_[29][1]=(1L<<(bits_-2));
                directionIntegers_[29][2]=(1L<<(bits_-3));
                directionIntegers_[29][3]=(15L<<(bits_-4));
                directionIntegers_[29][4]=(5L<<(bits_-5));
                directionIntegers_[29][5]=(49L<<(bits_-6));
                directionIntegers_[29][6]=(59L<<(bits_-7));
            }

            if (dimensionality_>30) {
                directionIntegers_[30][0]=(1L<<(bits_-1));
                directionIntegers_[30][1]=(3L<<(bits_-2));
                directionIntegers_[30][2]=(5L<<(bits_-3));
                directionIntegers_[30][3]=(15L<<(bits_-4));
                directionIntegers_[30][4]=(17L<<(bits_-5));
                directionIntegers_[30][5]=(19L<<(bits_-6));
                directionIntegers_[30][6]=(21L<<(bits_-7));
            }

            if (dimensionality_>31) {
                directionIntegers_[31][0]=(1L<<(bits_-1));
                directionIntegers_[31][1]=(1L<<(bits_-2));
                directionIntegers_[31][2]=(7L<<(bits_-3));
                directionIntegers_[31][3]=(11L<<(bits_-4));
                directionIntegers_[31][4]=(13L<<(bits_-5));
                directionIntegers_[31][5]=(29L<<(bits_-6));
                directionIntegers_[31][6]=(3L<<(bits_-7));
            }

            // random initialization in dimension higher than 32
            if (dimensionality_>32) {
                MersenneTwisterUniformRng uniformRng(seed);
                for (k=32; k<dimensionality_; k++) {
                    for(int l=0; l<degree_[k]; l++) {
                        double u = uniformRng.next().value;
                     // unsigned long n = (u*(1L<<(l-1)));
                        unsigned long n = (u*(1L<<(l+1)));
                        while(n%2==0) {
                            u = uniformRng.next().value;
//                          n = (u*(1L<<(l+1)));
                            n = (u*(1L<<(l+1)));
                        }
                     // directionIntegers_[k][l]=(n<<(bits_-l  ));
                        directionIntegers_[k][l]=(n<<(bits_-l-1));
                    }
                }
            }

            // computation of directionIntegers_[k][l] for l>=degree_[k]
            // by recurrence relation
            for (k=1; k<dimensionality_; k++) {
                for (int l=degree_[k]; l<bits_; l++) {
                    unsigned long n = (directionIntegers_[k][l-degree_[k]]
                                                                >>degree_[k]);
                    for (int j=1; j<=degree_[k]; j++) {
                        if (ppmt_[k][j]!=0)
                            n ^= directionIntegers_[k][l-j];
                    }
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
    }

}

#endif
