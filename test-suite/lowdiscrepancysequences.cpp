
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

#include "lowdiscrepancysequences.hpp"
#include <ql/RandomNumbers/mt19937uniformrng.hpp>
#include <ql/RandomNumbers/sobolrsg.hpp>
#include <ql/RandomNumbers/haltonrsg.hpp>
#include <ql/RandomNumbers/randomsequencegenerator.hpp>
#include <ql/RandomNumbers/primitivepolynomials.h>
#include <ql/Math/sequencestatistics.hpp>
#include <ql/Math/discrepancystatistics.hpp>

// #define PRINT_ONLY
#ifdef PRINT_ONLY
#include <fstream>
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

void LowDiscrepancyTest::testPolynomialsModuloTwo() {

    BOOST_MESSAGE("Testing " + IntegerFormatter::toString(PPMT_MAX_DIM) +
                  " primitive polynomials modulo two...");

    static const unsigned long jj[] = {
                 1,       1,       2,       2,       6,       6,      18,
                16,      48,      60,     176,     144,     630,     756,
              1800,    2048,    7710,    7776,   27594,   24000,   84672,
            120032,  356960,  276480, 1296000, 1719900, 4202496
    };

    unsigned long i=0,j=0,n=0;
    long polynomial=0;
    while (n<PPMT_MAX_DIM || polynomial!=-1) {
        if (polynomial==-1) {
            ++i; // Increase degree index
            j=0; // Reset index of polynomial in degree.
        }
        polynomial = PrimitivePolynomials[i][j];
        if (polynomial==-1) {
            --n;
            if (j!=jj[i]) {
                BOOST_FAIL("Only " + IntegerFormatter::toString(j)
                           + " polynomials in degree "
                           + IntegerFormatter::toString(i+1) + " instead of "
                           + IntegerFormatter::toString(jj[i]));
            }
        }
        ++j; // Increase index of polynomial in degree i+1
        ++n; // Increase overall polynomial counter
    }

}


void LowDiscrepancyTest::testSobol() {

    BOOST_MESSAGE("Testing Sobol sequences up to dimension " +
                  IntegerFormatter::toString(PPMT_MAX_DIM) + "...");

    Array point;

    // testing max dimensionality
    Size dimensionality = PPMT_MAX_DIM;
    unsigned long seed = 123456;
    SobolRsg rsg(dimensionality, seed);
    Size points = 100, i;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point.size()!=dimensionality) {
            BOOST_FAIL("Sobol sequence generator returns "
                       " a sequence of wrong dimensionality: " +
                       IntegerFormatter::toString(point.size())
                       + " instead of  " +
                       IntegerFormatter::toString(dimensionality));
        }
    }

    // testing homogeneity properties
    dimensionality = 33;
    seed = 123456;
    rsg = SobolRsg(dimensionality, seed);
    SequenceStatistics<> stat(dimensionality);
    std::vector<double> mean, stdev, variance, skewness, kurtosis;
    Size k = 0;
    for (int j=1; j<5; j++) { // five cycle
        points = Size(QL_POW(2.0, j))-1; // base 2
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        for (i=0; i<dimensionality; i++) {
            if (mean[i] != 0.5) {
                BOOST_FAIL(IntegerFormatter::toOrdinal(i+1) +
                           " dimension mean (" +
                           DoubleFormatter::toString(mean[i]) +
                           ") at the end of the " +
                           IntegerFormatter::toOrdinal(j+1) +
                           " cycle in Sobol sequence is not " +
                           DoubleFormatter::toString(0.5));
            }
        }
    }

    // testing first dimension (van der Corput sequence)
    const double vanderCorputSequenceModuloTwo[] = {
        // first cycle (zero excluded)
        0.50000,
        // second cycle
        0.75000, 0.25000,
        // third cycle
        0.37500, 0.87500, 0.62500, 0.12500,
        // fourth cycle
        0.18750, 0.68750, 0.93750, 0.43750, 0.31250, 0.81250, 0.56250, 0.06250,
        // fifth cycle
        0.09375, 0.59375, 0.84375, 0.34375, 0.46875, 0.96875, 0.71875, 0.21875,
        0.15625, 0.65625, 0.90625, 0.40625, 0.28125, 0.78125, 0.53125, 0.03125
    };

    dimensionality = 1;
    rsg = SobolRsg(dimensionality);
    points = Size(QL_POW(2.0, 5))-1; // five cycles
    for (i=0; i<points; i++) {
        point = rsg .nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            BOOST_FAIL(IntegerFormatter::toOrdinal(i+1) +
                       " draw (" +
                       DoubleFormatter::toString(point[0]) +
                       ") in 1-D Sobol sequence is not in the "
                       "van der Corput sequence modulo two: " +
                       "it should have been " +
                       DoubleFormatter::toString(
                                           vanderCorputSequenceModuloTwo[i]));
        }
    }
}


void LowDiscrepancyTest::testHalton() {

    BOOST_MESSAGE("Testing Halton sequences...");

    Array point;
    // testing "high" dimensionality
    Size dimensionality = PPMT_MAX_DIM;
    HaltonRsg rsg(dimensionality, 0, false, false);
    Size points = 100, i, k;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point.size()!=dimensionality) {
            BOOST_FAIL("Halton sequence generator returns "
                       " a sequence of wrong dimensionality: " +
                       IntegerFormatter::toString(point.size())
                       + " instead of  " +
                       IntegerFormatter::toString(dimensionality));
        }
    }

    // testing first and second dimension (van der Corput sequence)
    const double vanderCorputSequenceModuloTwo[] = {
        // first cycle (zero excluded)
        0.50000,
        // second cycle
        0.25000, 0.75000,
        // third cycle
        0.12500, 0.62500, 0.37500, 0.87500,
        // fourth cycle
        0.06250, 0.56250, 0.31250, 0.81250, 0.18750, 0.68750, 0.43750, 0.93750,
        // fifth cycle
        0.03125, 0.53125, 0.28125, 0.78125, 0.15625, 0.65625, 0.40625, 0.90625,
        0.09375, 0.59375, 0.34375, 0.84375, 0.21875, 0.71875, 0.46875, 0.96875,
    };

    dimensionality = 1;
    rsg = HaltonRsg(dimensionality, 0, false, false);
    points = Size(QL_POW(2.0, 5))-1;  // five cycles
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            BOOST_FAIL(IntegerFormatter::toOrdinal(i+1) +
                       " draw (" +
                       DoubleFormatter::toString(point[0]) +
                       ") in 1-D Halton sequence is not in the "
                       "van der Corput sequence modulo two: " +
                       "it should have been " +
                       DoubleFormatter::toString(
                                           vanderCorputSequenceModuloTwo[i]));
        }
    }

    static const double vanderCorputSequenceModuloThree[] = {
        // first cycle (zero excluded)
        1.0/3,  2.0/3,
        // second cycle
        1.0/9,  4.0/9,  7.0/9,  2.0/9,  5.0/9,  8.0/9,
        // third cycle
        1.0/27, 10.0/27, 19.0/27,  4.0/27, 13.0/27, 22.0/27,
        7.0/27, 16.0/27, 25.0/27,  2.0/27, 11.0/27, 20.0/27,
        5.0/27, 14.0/27, 23.0/27,  8.0/27, 17.0/27, 26.0/27
    };

    dimensionality = 2;
    rsg = HaltonRsg(dimensionality, 0, false, false);
    points = Size(QL_POW(3.0, 3))-1;  // three cycles of the higher dimension
    for (i=0; i<points; i++) {
        point = rsg .nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            BOOST_FAIL("First component of " +
                       IntegerFormatter::toOrdinal(i+1) +
                       " draw (" +
                       DoubleFormatter::toString(point[0]) +
                       ") in 2-D Halton sequence is not in the "
                       "van der Corput sequence modulo two: " +
                       "it should have been " +
                       DoubleFormatter::toString(
                                           vanderCorputSequenceModuloTwo[i]));
        }
        if (QL_FABS(point[1]-vanderCorputSequenceModuloThree[i])>1.0e-15) {
            BOOST_FAIL("Second component of " +
                       IntegerFormatter::toOrdinal(i+1) +
                       " draw (" +
                       DoubleFormatter::toString(point[1]) +
                       ") in 2-D Halton sequence is not in the "
                       "van der Corput sequence modulo three: "
                       "it should have been " +
                       DoubleFormatter::toString(
                                         vanderCorputSequenceModuloThree[i]));
        }
    }

    // testing homogeneity properties
    dimensionality = 33;
    rsg = HaltonRsg(dimensionality, 0, false, false);
    SequenceStatistics<> stat(dimensionality);
    std::vector<double> mean, stdev, variance, skewness, kurtosis;
    k = 0;
    int j;
    for (j=1; j<5; j++) { // five cycle
        points = Size(QL_POW(2.0, j))-1; // base 2
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        if (mean[0] != 0.5) {
            BOOST_FAIL("First dimension mean (" +
                       DoubleFormatter::toString(mean[0]) +
                       ") at the end of the " +
                       IntegerFormatter::toOrdinal(j+1) +
                       " cycle in Halton sequence is not " +
                       DoubleFormatter::toString(0.5));
        }
    }

    // reset generator and gaussianstatistics
    rsg  = HaltonRsg(dimensionality, 0, false, false);
    stat.reset(dimensionality);
    k = 0;
    for (j=1; j<3; j++) { // three cycle
        points = Size(QL_POW(3.0, j))-1; // base 3
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        if (QL_FABS(mean[1]-0.5)>1e-16) {
            BOOST_FAIL("Second dimension mean (" +
                       DoubleFormatter::toString(mean[1]) +
                       ") at the end of the " +
                       IntegerFormatter::toOrdinal(j+1) +
                       " cycle in Halton sequence is not " +
                       DoubleFormatter::toString(0.5));
        }
    }

}

namespace {

    const double dim002DiscrJackel_Sobol[] = {
        8.33e-004, 4.32e-004, 2.24e-004, 1.12e-004,
        5.69e-005, 2.14e-005 // , null
    };
    const double dim002DiscrMersenneTwis[] = {
        8.84e-003, 5.42e-003, 5.23e-003, 4.47e-003,
        4.75e-003, 3.11e-003, 2.97e-003
    };
    const double dim002DiscrPlain_Halton[] = {
        1.26e-003, 6.73e-004, 3.35e-004, 1.91e-004,
        1.11e-004, 5.05e-005, 2.42e-005
    };
    const double dim002DiscrRShiftHalton[] = {1.32e-003, 7.25e-004};
    const double dim002DiscrRStRShHalton[] = {1.35e-003, 9.43e-004};
    const double dim002DiscrRStartHalton[] = {1.08e-003, 6.40e-004};
    const double dim002Discr_True_Random[] = {
        1.17e-002, 8.24e-003, 5.82e-003, 4.12e-003,
        2.91e-003, 2.06e-003, 1.46e-003
    };
    const double dim002Discr__Unit_Sobol[] = {
        8.33e-004, 4.32e-004, 2.24e-004, 1.12e-004,
        5.69e-005, 2.14e-005 // , null
};

    const double dim003DiscrJackel_Sobol[] = {
        1.21e-003, 6.37e-004, 3.40e-004, 1.75e-004,
        9.21e-005, 4.79e-005, 2.56e-005
    };
    const double dim003DiscrMersenneTwis[] = {
        7.02e-003, 4.94e-003, 4.82e-003, 4.91e-003,
        3.33e-003, 2.80e-003, 2.62e-003
    };
    const double dim003DiscrPlain_Halton[] = {
        1.63e-003, 9.62e-004, 4.83e-004, 2.67e-004,
        1.41e-004, 7.64e-005, 3.93e-005
    };
    const double dim003DiscrRShiftHalton[] = {1.96e-003, 1.03e-003};
    const double dim003DiscrRStRShHalton[] = {2.17e-003, 1.54e-003};
    const double dim003DiscrRStartHalton[] = {1.48e-003, 7.77e-004};
    const double dim003Discr_True_Random[] = {
        9.27e-003, 6.56e-003, 4.63e-003, 3.28e-003,
        2.32e-003, 1.64e-003, 1.16e-003
    };
    const double dim003Discr__Unit_Sobol[] = {
        1.21e-003, 6.37e-004, 3.40e-004, 1.75e-004,
        9.21e-005, 4.79e-005, 2.56e-005
    };

    const double dim005DiscrJackel_Sobol[] = {
        1.59e-003, 9.55e-004, 5.33e-004, 3.22e-004,
        1.63e-004, 9.41e-005, 5.19e-005
    };
    const double dim005DiscrMersenneTwis[] = {
        4.28e-003, 3.48e-003, 2.48e-003, 1.98e-003,
        1.57e-003, 1.39e-003, 6.33e-004
    };
    const double dim005DiscrPlain_Halton[] = {
        1.93e-003, 1.23e-003, 6.89e-004, 4.22e-004,
        2.13e-004, 1.25e-004, 7.17e-005
    };
    const double dim005DiscrRShiftHalton[] = {2.02e-003, 1.36e-003};
    const double dim005DiscrRStRShHalton[] = {2.11e-003, 1.25e-003};
    const double dim005DiscrRStartHalton[] = {1.74e-003, 1.08e-003};
    const double dim005Discr_True_Random[] = {
        5.15e-003, 3.64e-003, 2.57e-003, 1.82e-003,
        1.29e-003, 9.10e-004, 6.43e-004
    };
    const double dim005Discr__Unit_Sobol[] = {
        1.85e-003, 9.39e-004, 5.19e-004, 2.99e-004,
        1.75e-004, 9.51e-005, 5.55e-005
    };

    const double dim010DiscrJackel_Sobol[] = {
        7.08e-004, 5.31e-004, 3.60e-004, 2.18e-004,
        1.57e-004, 1.12e-004, 6.39e-005
    };
    const double dim010DiscrMersenneTwis[] = {
        8.83e-004, 6.56e-004, 4.87e-004, 3.37e-004,
        3.06e-004, 1.73e-004, 1.43e-004
    };
    const double dim010DiscrPlain_Halton[] = {
        1.23e-003, 6.89e-004, 4.03e-004, 2.83e-004,
        1.61e-004, 1.08e-004, 6.69e-005
    };
    const double dim010DiscrRShiftHalton[] = {9.25e-004, 6.40e-004};
    const double dim010DiscrRStRShHalton[] = {8.41e-004, 5.42e-004};
    const double dim010DiscrRStartHalton[] = {7.89e-004, 5.33e-004};
    const double dim010Discr_True_Random[] = {
        9.69e-004, 6.85e-004, 4.84e-004, 3.42e-004,
        2.42e-004, 1.71e-004, 1.21e-004
    };
    const double dim010Discr__Unit_Sobol[] = {
        7.67e-004, 4.92e-004, 3.47e-004, 2.34e-004,
        1.39e-004, 9.47e-005, 5.72e-005
    };

    const double dim015DiscrJackel_Sobol[] = {
        1.59e-004, 1.23e-004, 7.73e-005, 5.51e-005,
        3.91e-005, 2.73e-005, 1.96e-005
    };
    const double dim015DiscrMersenneTwis[] = {
        1.63e-004, 1.12e-004, 8.36e-005, 6.09e-005,
        4.34e-005, 2.95e-005, 2.10e-005
    };
    const double dim015DiscrPlain_Halton[] = {
        5.75e-004, 3.12e-004, 1.70e-004, 9.89e-005,
        5.33e-005, 3.45e-005, 2.11e-005
    };
    const double dim015DiscrRShiftHalton[] = {1.75e-004, 1.19e-004};
    const double dim015DiscrRStRShHalton[] = {1.66e-004, 1.34e-004};
    const double dim015DiscrRStartHalton[] = {2.09e-004, 1.30e-004};
    const double dim015Discr_True_Random[] = {
        1.73e-004, 1.22e-004, 8.62e-005, 6.10e-005,
        4.31e-005, 3.05e-005, 2.16e-005
    };
    const double dim015Discr__Unit_Sobol[] = {
        2.24e-004, 1.39e-004, 9.86e-005, 6.02e-005,
        4.39e-005, 3.06e-005, 2.32e-005
    };

    const double dim030DiscrJackel_Sobol[] = {
        6.43e-007, 5.28e-007, 3.88e-007, 2.49e-007,
        2.09e-007, 1.55e-007, 1.07e-007
    };
    const double dim030DiscrMersenneTwis[] = {
        4.38e-007, 3.25e-007, 4.47e-007, 2.85e-007,
        2.03e-007, 1.50e-007, 1.17e-007
    };
    const double dim030DiscrPlain_Halton[] = {
        4.45e-004, 2.23e-004, 1.11e-004, 5.56e-005,
        2.78e-005, 1.39e-005, 6.95e-006
    };
    const double dim030DiscrRShiftHalton[] = {8.11e-007, 6.05e-007};
    const double dim030DiscrRStRShHalton[] = {1.85e-006, 1.03e-006};
    const double dim030DiscrRStartHalton[] = {4.42e-007, 4.64e-007};
    const double dim030Discr_True_Random[] = {
        9.54e-007, 6.75e-007, 4.77e-007, 3.37e-007,
        2.38e-007, 1.69e-007, 1.19e-007
    };
    const double dim030Discr__Unit_Sobol[] = {
        4.35e-005, 2.17e-005, 1.09e-005, 5.43e-006,
        2.73e-006, 1.37e-006, 6.90e-007
    };

    const double dim050DiscrJackel_Sobol[] = {
        2.98e-010, 2.91e-010, 2.62e-010, 1.53e-010,
        1.48e-010, 1.15e-010, 8.41e-011
    };
    const double dim050DiscrMersenneTwis[] = {
        3.27e-010, 2.42e-010, 1.47e-010, 1.98e-010,
        2.31e-010, 1.30e-010, 8.09e-011
    };
    const double dim050DiscrPlain_Halton[] = {
        4.04e-004, 2.02e-004, 1.01e-004, 5.05e-005,
        2.52e-005, 1.26e-005, 6.31e-006
    };
    const double dim050DiscrRShiftHalton[] = {1.14e-010, 1.25e-010};
    const double dim050DiscrRStRShHalton[] = {2.92e-010, 5.02e-010};
    const double dim050DiscrRStartHalton[] = {1.93e-010, 6.82e-010};
    const double dim050Discr_True_Random[] = {
        9.32e-010, 6.59e-010, 4.66e-010, 3.29e-010,
        2.33e-010, 1.65e-010, 1.16e-010
    };
    const double dim050Discr__Unit_Sobol[] = {
        1.63e-005, 8.14e-006, 4.07e-006, 2.04e-006,
        1.02e-006, 5.09e-007, 2.54e-007
    };

    const double dim100DiscrJackel_Sobol[] = {
        1.26e-018, 1.55e-018, 8.46e-019, 4.43e-019,
        4.04e-019, 2.44e-019, 4.86e-019
    };
    const double dim100DiscrMersenneTwis[] = {
        5.30e-019, 7.29e-019, 3.71e-019, 3.33e-019,
        1.33e-017, 6.70e-018, 3.36e-018
    };
    const double dim100DiscrPlain_Halton[] = {
        3.63e-004, 1.81e-004, 9.07e-005, 4.53e-005,
        2.27e-005, 1.13e-005, 5.66e-006
    };
    const double dim100DiscrRShiftHalton[] = {3.36e-019, 2.19e-019};
    const double dim100DiscrRStRShHalton[] = {4.44e-019, 2.24e-019};
    const double dim100DiscrRStartHalton[] = {9.85e-020, 8.34e-019};
    const double dim100Discr_True_Random[] = {
        2.78e-017, 1.96e-017, 1.39e-017, 9.81e-018,
        6.94e-018, 4.91e-018, 3.47e-018
    };
    const double dim100Discr__Unit_Sobol[] = {
        4.97e-006, 2.48e-006, 1.24e-006, 6.20e-007,
        3.10e-007, 1.55e-007, 7.76e-008
    };

    const unsigned long dimensionality[] = {2, 3, 5, 10, 15, 30, 50, 100 };

    // 7 (all tabulated dimensions) loops on all sequence generators would
    // take a few days ... too long for usual/frequent test running
    const Size minimumLoops = 1;

    // let's add some generality here...

    class MersenneFactory {
      public:
        typedef RandomSequenceGenerator<MersenneTwisterUniformRng>
            MersenneTwisterUniformRsg;
        typedef MersenneTwisterUniformRsg generator_type;
        MersenneTwisterUniformRsg make(unsigned long dim,
                                       unsigned long seed) const {
            return MersenneTwisterUniformRsg(dim,seed);
        }
        std::string name() const { return "Mersenne Twister"; }
    };

    class SobolFactory {
      public:
        typedef SobolRsg generator_type;
        SobolFactory(bool unit = false) : unit_(unit) {}
        SobolRsg make(unsigned long dim,
                      unsigned long seed) const {
            return SobolRsg(dim,seed,unit_);
        }
        std::string name() const {
            std::string prefix = unit_ ?
                                 "unit-initialized " :
                                 "";
            return prefix + "Sobol";
        }
      private:
        bool unit_;
    };

    class HaltonFactory {
      public:
        typedef HaltonRsg generator_type;
        HaltonFactory(bool randomStart, bool randomShift)
        : start_(randomStart), shift_(randomShift) {}
        HaltonRsg make(unsigned long dim,
                       unsigned long seed) const {
            return HaltonRsg(dim,seed,start_,shift_);
        }
        std::string name() const {
            std::string prefix = start_ ?
                                 "random-start " :
                                 "";
            if (shift_)
                prefix += "random-shift ";
            return prefix + "Halton";
        }
      private:
        bool start_, shift_;
    };

    template <class T>
    void testGeneratorDiscrepancy(const T& generatorFactory,
                                  const double * const discrepancy[8],
                                  const std::string& fileName,
                                  const std::string& arrayName) {

        Array point;
        unsigned long dim;
        unsigned long seed = 123456;
        double discr, tolerance = 1e-2;
        // 7 loops would take too long for usual/frequent test running
        Size sampleLoops = Size(QL_MAX(1.0, double(minimumLoops)));

        #ifdef PRINT_ONLY
        std::ofstream outStream(fileName.c_str());
        #endif
        for (int i = 0; i<8; i++) {
            #ifdef PRINT_ONLY
            outStream << std::endl;
            #endif

            dim = dimensionality[i];
            DiscrepancyStatistics stat(dim);

            typename T::generator_type rsg = generatorFactory.make(dim, seed);

            Size j, k=0, jMin=10;
            stat.reset();
            #ifdef PRINT_ONLY
            outStream << "static const double dim" << dim
                      << arrayName << "[] = {" ;
            #endif
            for (j=jMin; j<jMin+sampleLoops; j++) {
                Size points = Size(QL_POW(2.0, int(j)))-1;
                for (; k<points; k++) {
                    point = rsg.nextSequence().value;
                    stat.add(point);
                }

                discr = stat.discrepancy();

                #ifdef PRINT_ONLY
                if (j!=jMin)
                    outStream << ", ";
                outStream << DoubleFormatter::toExponential(discr, 2);
                #else
                if (QL_FABS(discr-discrepancy[i][j-jMin]) > tolerance*discr) {
                    BOOST_FAIL(generatorFactory.name() +
                               "discrepancy dimension " +
                               IntegerFormatter::toString(
                                                 dimensionality[i]) + " at " +
                               IntegerFormatter::toString(points) +
                               " samples is " +
                               DoubleFormatter::toExponential(discr,2) +
                               " instead of "+
                               DoubleFormatter::toExponential(
                                                  discrepancy[i][j-jMin], 2));
                }
                #endif
            }
            #ifdef PRINT_ONLY
            outStream << "};" << std::endl;
            #endif
        }
        #ifdef PRINT_ONLY
        outStream.close();
        #endif
    }

}


void LowDiscrepancyTest::testMersenneTwisterDiscrepancy() {

    BOOST_MESSAGE("Testing Mersenne twister discrepancy...");

    const double * const discrepancy[8] = {
        dim002DiscrMersenneTwis, dim003DiscrMersenneTwis,
        dim005DiscrMersenneTwis, dim010DiscrMersenneTwis,
        dim015DiscrMersenneTwis, dim030DiscrMersenneTwis,
        dim050DiscrMersenneTwis, dim100DiscrMersenneTwis
    };

    testGeneratorDiscrepancy(MersenneFactory(),
                             discrepancy,
                             "MersenneDiscrepancy.txt",
                             "DiscrMersenneTwis");
}

void LowDiscrepancyTest::testPlainHaltonDiscrepancy() {

    BOOST_MESSAGE("Testing plain Halton discrepancy...");

    const double * const discrepancy[8] = {
        dim002DiscrPlain_Halton, dim003DiscrPlain_Halton,
        dim005DiscrPlain_Halton, dim010DiscrPlain_Halton,
        dim015DiscrPlain_Halton, dim030DiscrPlain_Halton,
        dim050DiscrPlain_Halton, dim100DiscrPlain_Halton};

    testGeneratorDiscrepancy(HaltonFactory(false,false),
                             discrepancy,
                             "PlainHaltonDiscrepancy.txt",
                             "DiscrPlain_Halton");
}

void LowDiscrepancyTest::testRandomStartHaltonDiscrepancy() {

    BOOST_MESSAGE("Testing random-start Halton discrepancy...");

    const double * const discrepancy[8] = {
        dim002DiscrRStartHalton, dim003DiscrRStartHalton,
        dim005DiscrRStartHalton, dim010DiscrRStartHalton,
        dim015DiscrRStartHalton, dim030DiscrRStartHalton,
        dim050DiscrRStartHalton, dim100DiscrRStartHalton};

    testGeneratorDiscrepancy(HaltonFactory(true,false),
                             discrepancy,
                             "RandomStartHaltonDiscrepancy.txt",
                             "DiscrRStartHalton");
}

void LowDiscrepancyTest::testRandomShiftHaltonDiscrepancy() {

    BOOST_MESSAGE("Testing random-shift Halton discrepancy...");

    const double * const discrepancy[8] = {
        dim002DiscrRShiftHalton, dim003DiscrRShiftHalton,
        dim005DiscrRShiftHalton, dim010DiscrRShiftHalton,
        dim015DiscrRShiftHalton, dim030DiscrRShiftHalton,
        dim050DiscrRShiftHalton, dim100DiscrRShiftHalton};

    testGeneratorDiscrepancy(HaltonFactory(false,true),
                             discrepancy,
                             "RandomShiftHaltonDiscrepancy.txt",
                             "DiscrRShiftHalton");
}

void LowDiscrepancyTest::testRandomStartRandomShiftHaltonDiscrepancy() {

    BOOST_MESSAGE("Testing random-start, random-shift Halton discrepancy...");

    const double * const discrepancy[8] = {
        dim002DiscrRStRShHalton, dim003DiscrRStRShHalton,
        dim005DiscrRStRShHalton, dim010DiscrRStRShHalton,
        dim015DiscrRStRShHalton, dim030DiscrRStRShHalton,
        dim050DiscrRStRShHalton, dim100DiscrRStRShHalton};

    testGeneratorDiscrepancy(HaltonFactory(true,true),
                             discrepancy,
                             "RandomStartRandomShiftHaltonDiscrepancy.txt",
                             "DiscrRStRShHalton");
}

void LowDiscrepancyTest::testJackelSobolDiscrepancy() {

    BOOST_MESSAGE("Testing Jäckel-Sobol discrepancy...");

    const double * const discrepancy[8] = {
        dim002DiscrJackel_Sobol, dim003DiscrJackel_Sobol,
        dim005DiscrJackel_Sobol, dim010DiscrJackel_Sobol,
        dim015DiscrJackel_Sobol, dim030DiscrJackel_Sobol,
        dim050DiscrJackel_Sobol, dim100DiscrJackel_Sobol};

    testGeneratorDiscrepancy(SobolFactory(),
                             discrepancy,
                             "JackelSobolDiscrepancy.txt",
                             "DiscrJackel_Sobol");
}

void LowDiscrepancyTest::testUnitSobolDiscrepancy() {

    BOOST_MESSAGE("Testing unit Sobol discrepancy...");

    const double * const discrepancy[8] = {
        dim002Discr__Unit_Sobol, dim003Discr__Unit_Sobol,
        dim005Discr__Unit_Sobol, dim010Discr__Unit_Sobol,
        dim015Discr__Unit_Sobol, dim030Discr__Unit_Sobol,
        dim050Discr__Unit_Sobol, dim100Discr__Unit_Sobol};

    testGeneratorDiscrepancy(SobolFactory(true),
                             discrepancy,
                             "UnitSobolDiscrepancy.txt",
                             "Discr__Unit_Sobol");
}


test_suite* LowDiscrepancyTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Low-discrepancy sequence tests");
    suite->add(BOOST_TEST_CASE(
           &LowDiscrepancyTest::testMersenneTwisterDiscrepancy));
    suite->add(BOOST_TEST_CASE(&LowDiscrepancyTest::testHalton));
    suite->add(BOOST_TEST_CASE(
           &LowDiscrepancyTest::testPlainHaltonDiscrepancy));
    suite->add(BOOST_TEST_CASE(
           &LowDiscrepancyTest::testRandomStartHaltonDiscrepancy));
    suite->add(BOOST_TEST_CASE(
           &LowDiscrepancyTest::testRandomShiftHaltonDiscrepancy));
    suite->add(BOOST_TEST_CASE(
           &LowDiscrepancyTest::testRandomStartRandomShiftHaltonDiscrepancy));
    suite->add(BOOST_TEST_CASE(&LowDiscrepancyTest::testPolynomialsModuloTwo));
    suite->add(BOOST_TEST_CASE(&LowDiscrepancyTest::testSobol));
    suite->add(BOOST_TEST_CASE(
           &LowDiscrepancyTest::testJackelSobolDiscrepancy));
    suite->add(BOOST_TEST_CASE(&LowDiscrepancyTest::testUnitSobolDiscrepancy));
    return suite;
}
