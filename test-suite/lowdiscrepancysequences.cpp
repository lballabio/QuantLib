/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2007 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/statistics/discrepancystatistics.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/math/randomnumbers/burley2020sobolrsg.hpp>
#include <ql/math/randomnumbers/faurersg.hpp>
#include <ql/math/randomnumbers/haltonrsg.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/randomnumbers/seedgenerator.hpp>
#include <ql/math/randomnumbers/primitivepolynomials.hpp>
#include <ql/math/randomnumbers/randomizedlds.hpp>
#include <ql/math/randomnumbers/randomsequencegenerator.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/math/randomnumbers/latticerules.hpp>
#include <ql/math/randomnumbers/latticersg.hpp>

//#define PRINT_ONLY
#ifdef PRINT_ONLY
#include <fstream>
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::fabs;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(LowDiscrepancyTests)

BOOST_AUTO_TEST_CASE(testSeedGenerator) {
    BOOST_TEST_MESSAGE("Testing random-seed generator...");
    SeedGenerator::instance().get();
}

BOOST_AUTO_TEST_CASE(testPolynomialsModuloTwo) {

    BOOST_TEST_MESSAGE("Testing " << PPMT_MAX_DIM <<
                       " primitive polynomials modulo two...");

    const Size jj[] = {
                 1,       1,       2,       2,       6,       6,      18,
                16,      48,      60,     176,     144,     630,     756,
              1800,    2048,    7710,    7776,   27594,   24000,   84672,
            120032,  356960,  276480, 1296000, 1719900, 4202496
    };

    Size i=0,j=0,n=0;
    BigInteger polynomial=0;
    while (n<PPMT_MAX_DIM || polynomial!=-1) {
        if (polynomial==-1) {
            ++i; // Increase degree index
            j=0; // Reset index of polynomial in degree.
        }
        polynomial = PrimitivePolynomials[i][j];
        if (polynomial==-1) {
            --n;
            if (j!=jj[i]) {
                BOOST_ERROR("Only " << j << " polynomials in degree " << i+1
                            << " instead of " << jj[i]);
            }
        }
        ++j; // Increase index of polynomial in degree i+1
        ++n; // Increase overall polynomial counter
    }

}

BOOST_AUTO_TEST_CASE(testRandomizedLowDiscrepancySequence) {

    BOOST_TEST_MESSAGE("Testing randomized low-discrepancy sequences up to "
                       "dimension " << PPMT_MAX_DIM << "...");

    RandomizedLDS<SobolRsg, RandomSequenceGenerator<MersenneTwisterUniformRng> > rldsg(PPMT_MAX_DIM);
    rldsg.nextSequence();
    rldsg.lastSequence();
    rldsg.nextRandomizer();

    MersenneTwisterUniformRng t0;
    SobolRsg t1(PPMT_MAX_DIM);
    RandomSequenceGenerator<MersenneTwisterUniformRng> t2(PPMT_MAX_DIM);
    RandomizedLDS<SobolRsg, RandomSequenceGenerator<MersenneTwisterUniformRng> > rldsg2(t1, t2);
    rldsg2.nextSequence();
    rldsg2.lastSequence();
    rldsg2.nextRandomizer();

    RandomizedLDS<SobolRsg, RandomSequenceGenerator<MersenneTwisterUniformRng> > rldsg3(t1);
    rldsg3.nextSequence();
    rldsg3.lastSequence();
    rldsg3.nextRandomizer();

}

namespace
{
    void testRandomizedLatticeRule(LatticeRule::type name,
                                   const std::string& nameString)
    {
        Size maxDim = 30;
        Size N = 1024;
        Size numberBatches = 32;

        BOOST_TEST_MESSAGE("Testing randomized lattice sequences (" << nameString
                           << ") up to dimension " << maxDim << "...");

        std::vector<Real> z;

        LatticeRule::getRule(name, z, N);
        LatticeRsg latticeGenerator(maxDim,
            z,
            N);

        unsigned long seed = 12345678UL;
        MersenneTwisterUniformRng rng( seed);

        RandomSequenceGenerator<MersenneTwisterUniformRng> rsg(maxDim,
                                                               rng);

        RandomizedLDS<LatticeRsg, RandomSequenceGenerator<MersenneTwisterUniformRng> > rldsg(latticeGenerator,rsg);

        SequenceStatistics outerStats(maxDim);

        for (Size i=0; i < numberBatches; ++i)
        {
            SequenceStatistics innerStats(maxDim);
            for (Size j=0; j < N; ++j)
            {
                innerStats.add(rldsg.nextSequence().value);
            }
            outerStats.add(innerStats.mean());
            rldsg.nextRandomizer();
        }

        std::vector<Real> means(outerStats.mean());
        std::vector<Real> sds(outerStats.errorEstimate());

        std::vector<Real> errorInSds(maxDim);

        for (Size i=0; i < maxDim; ++i)
            errorInSds[i] = (means[i]-0.5)/ sds[i];

        Real tolerance = 4.0;

        for (Size i=0; i < maxDim; ++i)
            if (fabs(errorInSds[i] ) > tolerance)
                BOOST_ERROR("Lattice generator" << nameString <<" returns  a mean of " <<
                means[i] << " with error equal to  " << errorInSds[i]
            << " standard deviations in dimension " << i);
    }
}

BOOST_AUTO_TEST_CASE(testRandomizedLattices){

    testRandomizedLatticeRule(LatticeRule::A, "A");
    testRandomizedLatticeRule(LatticeRule::B, "B");
    testRandomizedLatticeRule(LatticeRule::C, "C");
    testRandomizedLatticeRule(LatticeRule::D, "D");
}

BOOST_AUTO_TEST_CASE(testSobol) {

    BOOST_TEST_MESSAGE("Testing Sobol sequences up to dimension "
                       << PPMT_MAX_DIM << "...");

    std::vector<Real> point;
    Real tolerance = 1.0e-15;

    // testing max dimensionality
    Size dimensionality = PPMT_MAX_DIM;
    BigNatural seed = 123456;
    SobolRsg rsg(dimensionality, seed);
    Size points = 100, i;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point.size()!=dimensionality) {
            BOOST_ERROR("Sobol sequence generator returns "
                        " a sequence of wrong dimensionality: " << point.size()
                        << " instead of  " << dimensionality);
        }
    }

    // testing homogeneity properties
    dimensionality = 33;
    seed = 123456;
    rsg = SobolRsg(dimensionality, seed);
    SequenceStatistics stat(dimensionality);
    std::vector<Real> mean;
    Size k = 0;
    for (Integer j=1; j<5; j++) { // five cycle
        points = Size(std::pow(2.0, j))-1; // base 2
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        for (i=0; i<dimensionality; i++) {
            Real error = std::fabs(mean[i]-0.5);
            if (error > tolerance) {
                BOOST_ERROR(io::ordinal(i+1) << " dimension: "
                            << std::fixed
                            << "mean (" << mean[i]
                            << ") at the end of the " << io::ordinal(j+1)
                            << " cycle in Sobol sequence is not " << 0.5
                            << std::scientific
                            << " (error = " << error << ")");
            }
        }
    }

    // testing first dimension (van der Corput sequence)
    const Real vanderCorputSequenceModuloTwo[] = {
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
    points = Size(std::pow(2.0, 5))-1; // five cycles
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        Real error = std::fabs(point[0]-vanderCorputSequenceModuloTwo[i]);
        if (error > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " draw ("
                        << std::fixed << point[0]
                        << ") in 1-D Sobol sequence is not in the "
                        << "van der Corput sequence modulo two: "
                        << "it should have been "
                        << vanderCorputSequenceModuloTwo[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
    }
}

BOOST_AUTO_TEST_CASE(testFaure) {

    BOOST_TEST_MESSAGE("Testing Faure sequences...");

    std::vector<Real> point;
    Real tolerance = 1.0e-15;

    // testing "high" dimensionality
    Size dimensionality = PPMT_MAX_DIM;
    FaureRsg rsg(dimensionality);
    Size points = 100, i;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point.size()!=dimensionality) {
            BOOST_ERROR("Faure sequence generator returns "
                        " a sequence of wrong dimensionality: " << point.size()
                        << " instead of  " << dimensionality);
        }
    }

    // 1-dimension Faure (van der Corput sequence base 2)
    const Real vanderCorputSequenceModuloTwo[] = {
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
    rsg = FaureRsg(dimensionality);
    points = Size(std::pow(2.0, 5))-1; // five cycles
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        Real error = std::fabs(point[0]-vanderCorputSequenceModuloTwo[i]);
        if (error > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " draw, dimension 1 ("
                        << std::fixed << point[0]
                        << ") in 3-D Faure sequence should have been "
                        << vanderCorputSequenceModuloTwo[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
    }

    // 2nd dimension of the 2-dimensional Faure sequence
    // (shuffled van der Corput sequence base 2)
    // checked with the code provided with "Economic generation of
    // low-discrepancy sequences with a b-ary gray code", by E. Thiemard
    const Real FaureDimensionTwoOfTwo[] = {
        // first cycle (zero excluded)
        0.50000,
        // second cycle
        0.25000, 0.75000,
        // third cycle
        0.37500, 0.87500, 0.12500, 0.62500,
        // fourth cycle
        0.31250, 0.81250, 0.06250, 0.56250, 0.18750, 0.68750, 0.43750, 0.93750,
        // fifth cycle
        0.46875, 0.96875, 0.21875, 0.71875, 0.09375, 0.59375, 0.34375, 0.84375,
        0.15625, 0.65625, 0.40625, 0.90625, 0.28125, 0.78125, 0.03125, 0.53125
    };
    dimensionality = 2;
    rsg = FaureRsg(dimensionality);
    points = Size(std::pow(2.0, 5))-1; // five cycles
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        Real error = std::fabs(point[0]-vanderCorputSequenceModuloTwo[i]);
        if (error > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " draw, dimension 1 ("
                        << std::fixed << point[0]
                        << ") in 3-D Faure sequence should have been "
                        << vanderCorputSequenceModuloTwo[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
        error = std::fabs(point[1]-FaureDimensionTwoOfTwo[i]);
        if (error > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " draw, dimension 2 ("
                        << std::fixed << point[1]
                        << ") in 3-D Faure sequence should have been "
                        << FaureDimensionTwoOfTwo[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
    }

    // 3-dimension Faure sequence (shuffled van der Corput sequence base 3)
    // see "Monte Carlo Methods in Financial Engineering,"
    // by Paul Glasserman, 2004 Springer Verlag, pag. 299
    const Real FaureDimensionOneOfThree[] = {
        // first cycle (zero excluded)
        1.0/3,  2.0/3,
        // second cycle
        7.0/9,  1.0/9,  4.0/9,  5.0/9,  8.0/9,  2.0/9
    };
    const Real FaureDimensionTwoOfThree[] = {
        // first cycle (zero excluded)
        1.0/3,  2.0/3,
        // second cycle
        1.0/9,  4.0/9,  7.0/9,  2.0/9,  5.0/9,  8.0/9
    };
    const Real FaureDimensionThreeOfThree[] = {
        // first cycle (zero excluded)
        1.0/3,  2.0/3,
        // second cycle
        4.0/9,  7.0/9,  1.0/9,  8.0/9,  2.0/9,  5.0/9
    };

    dimensionality = 3;
    rsg = FaureRsg(dimensionality);
    points = Size(std::pow(3.0, 2))-1; // three cycles
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        Real error = std::fabs(point[0]-FaureDimensionOneOfThree[i]);
        if (error > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " draw, dimension 1 ("
                        << std::fixed << point[0]
                        << ") in 3-D Faure sequence should have been "
                        << FaureDimensionOneOfThree[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
        error = std::fabs(point[1]-FaureDimensionTwoOfThree[i]);
        if (error > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " draw, dimension 2 ("
                        << std::fixed << point[1]
                        << ") in 3-D Faure sequence should have been "
                        << FaureDimensionTwoOfThree[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
        error = std::fabs(point[2]-FaureDimensionThreeOfThree[i]);
        if (error > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " draw, dimension 3 ("
                        << std::fixed << point[2]
                        << ") in 3-D Faure sequence should have been "
                        << FaureDimensionThreeOfThree[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
    }
}

BOOST_AUTO_TEST_CASE(testHalton) {

    BOOST_TEST_MESSAGE("Testing Halton sequences...");

    std::vector<Real> point;
    Real tolerance = 1.0e-15;

    // testing "high" dimensionality
    Size dimensionality = PPMT_MAX_DIM;
    HaltonRsg rsg(dimensionality, 0, false, false);
    Size points = 100, i, k;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point.size()!=dimensionality) {
            BOOST_ERROR("Halton sequence generator returns "
                        " a sequence of wrong dimensionality: " << point.size()
                        << " instead of  " << dimensionality);
        }
    }

    // testing first and second dimension (van der Corput sequence)
    const Real vanderCorputSequenceModuloTwo[] = {
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
    points = Size(std::pow(2.0, 5))-1;  // five cycles
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        Real error = std::fabs(point[0]-vanderCorputSequenceModuloTwo[i]);
        if (error > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " draw ("
                        << std::fixed << point[0]
                        << ") in 1-D Halton sequence is not in the "
                        << "van der Corput sequence modulo two: "
                        << "it should have been "
                        << vanderCorputSequenceModuloTwo[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
    }

    const Real vanderCorputSequenceModuloThree[] = {
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
    points = Size(std::pow(3.0, 3))-1;  // three cycles of the higher dimension
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        Real error = std::fabs(point[0]-vanderCorputSequenceModuloTwo[i]);
        if (error > tolerance) {
            BOOST_ERROR("First component of " << io::ordinal(i+1)
                        << " draw (" << std::fixed << point[0]
                        << ") in 2-D Halton sequence is not in the "
                        << "van der Corput sequence modulo two: "
                        << "it should have been "
                        << vanderCorputSequenceModuloTwo[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
        error = std::fabs(point[1]-vanderCorputSequenceModuloThree[i]);
        if (error > tolerance) {
            BOOST_ERROR("Second component of " << io::ordinal(i+1)
                        << " draw (" << std::fixed << point[1]
                        << ") in 2-D Halton sequence is not in the "
                        << "van der Corput sequence modulo three: "
                        << "it should have been "
                        << vanderCorputSequenceModuloThree[i]
                        << std::scientific
                        << " (error = " << error << ")");
        }
    }

    // testing homogeneity properties
    dimensionality = 33;
    rsg = HaltonRsg(dimensionality, 0, false, false);
    SequenceStatistics stat(dimensionality);
    std::vector<Real> mean, stdev, variance, skewness, kurtosis;
    k = 0;
    Integer j;
    for (j=1; j<5; j++) { // five cycle
        points = Size(std::pow(2.0, j))-1; // base 2
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        Real error = std::fabs(mean[0] - 0.5);
        if (error > tolerance) {
            BOOST_ERROR("First dimension mean (" << std::fixed << mean[0]
                        << ") at the end of the " << io::ordinal(j+1)
                        << " cycle in Halton sequence is not " << 0.5
                        << std::scientific
                        << " (error = " << error << ")");
        }
    }

    // reset generator and gaussianstatistics
    rsg  = HaltonRsg(dimensionality, 0, false, false);
    stat.reset(dimensionality);
    k = 0;
    for (j=1; j<3; j++) { // three cycle
        points = Size(std::pow(3.0, j))-1; // base 3
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        Real error = std::fabs(mean[1] - 0.5);
        if (error > tolerance) {
            BOOST_ERROR("Second dimension mean (" << std::fixed << mean[1]
                        << ") at the end of the " << io::ordinal(j+1)
                        << " cycle in Halton sequence is not " << 0.5
                        << std::scientific
                        << " (error = " << error << ")");
        }
    }

}

const Real dim002Discr_Sobol[] = {
    8.33e-004, 4.32e-004, 2.24e-004, 1.12e-004,
    5.69e-005, 2.14e-005 // , null
};
const Real dim002DiscrMersenneTwis[] = {
    8.84e-003, 5.42e-003, 5.23e-003, 4.47e-003,
    4.75e-003, 3.11e-003, 2.97e-003
};
const Real dim002DiscrPlain_Halton[] = {
    1.26e-003, 6.73e-004, 3.35e-004, 1.91e-004,
    1.11e-004, 5.05e-005, 2.42e-005
};
const Real dim002DiscrRShiftHalton[] = {1.32e-003, 7.25e-004};
const Real dim002DiscrRStRShHalton[] = {1.35e-003, 9.43e-004};
const Real dim002DiscrRStartHalton[] = {1.08e-003, 6.40e-004};
const Real dim002Discr_Unit_Sobol[] = {
    8.33e-004, 4.32e-004, 2.24e-004, 1.12e-004, 5.69e-005, 2.14e-005 // , null
};

const Real dim003Discr_Sobol[] = {
    1.21e-003, 6.37e-004, 3.40e-004, 1.75e-004,
    9.21e-005, 4.79e-005, 2.56e-005
};
const Real dim003DiscrMersenneTwis[] = {
    7.02e-003, 4.94e-003, 4.82e-003, 4.91e-003,
    3.33e-003, 2.80e-003, 2.62e-003
};
const Real dim003DiscrPlain_Halton[] = {
    1.63e-003, 9.62e-004, 4.83e-004, 2.67e-004,
    1.41e-004, 7.64e-005, 3.93e-005
};
const Real dim003DiscrRShiftHalton[] = {1.96e-003, 1.03e-003};
const Real dim003DiscrRStRShHalton[] = {2.17e-003, 1.54e-003};
const Real dim003DiscrRStartHalton[] = {1.48e-003, 7.77e-004};
const Real dim003Discr_Unit_Sobol[] = {1.21e-003, 6.37e-004, 3.40e-004, 1.75e-004,
                                       9.21e-005, 4.79e-005, 2.56e-005};

const Real dim005Discr_Sobol[] = {
    1.59e-003, 9.55e-004, 5.33e-004, 3.22e-004,
    1.63e-004, 9.41e-005, 5.19e-005
};
const Real dim005DiscrMersenneTwis[] = {
    4.28e-003, 3.48e-003, 2.48e-003, 1.98e-003,
    1.57e-003, 1.39e-003, 6.33e-004
};
const Real dim005DiscrPlain_Halton[] = {
    1.93e-003, 1.23e-003, 6.89e-004, 4.22e-004,
    2.13e-004, 1.25e-004, 7.17e-005
};
const Real dim005DiscrRShiftHalton[] = {2.02e-003, 1.36e-003};
const Real dim005DiscrRStRShHalton[] = {2.11e-003, 1.25e-003};
const Real dim005DiscrRStartHalton[] = {1.74e-003, 1.08e-003};
const Real dim005Discr_Unit_Sobol[] = {1.85e-003, 9.39e-004, 5.19e-004, 2.99e-004,
                                       1.75e-004, 9.51e-005, 5.55e-005};

const Real dim010DiscrJackel_Sobol[] = {
    7.08e-004, 5.31e-004, 3.60e-004, 2.18e-004,
    1.57e-004, 1.12e-004, 6.39e-005
};
const Real dim010DiscrSobLev_Sobol[] = {
    7.01e-004, 5.10e-004, 3.28e-004, 2.21e-004,
    1.57e-004, 1.08e-004, 6.38e-005
};
const Real dim010DiscrMersenneTwis[] = {
    8.83e-004, 6.56e-004, 4.87e-004, 3.37e-004,
    3.06e-004, 1.73e-004, 1.43e-004
};
const Real dim010DiscrPlain_Halton[] = {
    1.23e-003, 6.89e-004, 4.03e-004, 2.83e-004,
    1.61e-004, 1.08e-004, 6.69e-005
};
const Real dim010DiscrRShiftHalton[] = {9.25e-004, 6.40e-004};
const Real dim010DiscrRStRShHalton[] = {8.41e-004, 5.42e-004};
const Real dim010DiscrRStartHalton[] = {7.89e-004, 5.33e-004};
const Real dim010Discr_Unit_Sobol[] = {7.67e-004, 4.92e-004, 3.47e-004, 2.34e-004,
                                       1.39e-004, 9.47e-005, 5.72e-005};

const Real dim015DiscrJackel_Sobol[] = {
    1.59e-004, 1.23e-004, 7.73e-005, 5.51e-005,
    3.91e-005, 2.73e-005, 1.96e-005
};
const Real dim015DiscrSobLev_Sobol[] = {
    1.48e-004, 1.06e-004, 8.19e-005, 6.29e-005,
    4.16e-005, 2.54e-005, 1.73e-005
};
const Real dim015DiscrMersenneTwis[] = {
    1.63e-004, 1.12e-004, 8.36e-005, 6.09e-005,
    4.34e-005, 2.95e-005, 2.10e-005
};
const Real dim015DiscrPlain_Halton[] = {
    5.75e-004, 3.12e-004, 1.70e-004, 9.89e-005,
    5.33e-005, 3.45e-005, 2.11e-005
};
const Real dim015DiscrRShiftHalton[] = {1.75e-004, 1.19e-004};
const Real dim015DiscrRStRShHalton[] = {1.66e-004, 1.34e-004};
const Real dim015DiscrRStartHalton[] = {2.09e-004, 1.30e-004};
const Real dim015Discr_Unit_Sobol[] = {2.24e-004, 1.39e-004, 9.86e-005, 6.02e-005,
                                       4.39e-005, 3.06e-005, 2.32e-005};

const Real dim030DiscrJackel_Sobol[] = {
    6.43e-007, 5.28e-007, 3.88e-007, 2.49e-007,
    2.09e-007, 1.55e-007, 1.07e-007
};
const Real dim030DiscrSobLev_Sobol[] = {
    1.03e-006, 6.06e-007, 3.81e-007, 2.71e-007,
    2.68e-007, 1.73e-007, 1.21e-007
};
const Real dim030DiscrMersenneTwis[] = {
    4.38e-007, 3.25e-007, 4.47e-007, 2.85e-007,
    2.03e-007, 1.50e-007, 1.17e-007
};
const Real dim030DiscrPlain_Halton[] = {
    4.45e-004, 2.23e-004, 1.11e-004, 5.56e-005,
    2.78e-005, 1.39e-005, 6.95e-006
};
const Real dim030DiscrRShiftHalton[] = {8.11e-007, 6.05e-007};
const Real dim030DiscrRStRShHalton[] = {1.85e-006, 1.03e-006};
const Real dim030DiscrRStartHalton[] = {4.42e-007, 4.64e-007};
const Real dim030Discr_Unit_Sobol[] = {4.35e-005, 2.17e-005, 1.09e-005, 5.43e-006,
                                       2.73e-006, 1.37e-006, 6.90e-007};

const Real dim050DiscrJackel_Sobol[] = {
    2.98e-010, 2.91e-010, 2.62e-010, 1.53e-010,
    1.48e-010, 1.15e-010, 8.41e-011
};
const Real dim050DiscrSobLev_Sobol[] = {
    3.11e-010, 2.52e-010, 1.61e-010, 1.54e-010,
    1.11e-010, 8.60e-011, 1.17e-010
};
const Real dim050DiscrSobLem_Sobol[] = {
    4.57e-010, 6.84e-010, 3.68e-010, 2.20e-010,
    1.81e-010, 1.14e-010, 8.31e-011
};
const Real dim050DiscrMersenneTwis[] = {
    3.27e-010, 2.42e-010, 1.47e-010, 1.98e-010,
    2.31e-010, 1.30e-010, 8.09e-011
};
const Real dim050DiscrPlain_Halton[] = {
    4.04e-004, 2.02e-004, 1.01e-004, 5.05e-005,
    2.52e-005, 1.26e-005, 6.31e-006
};
const Real dim050DiscrRShiftHalton[] = {1.14e-010, 1.25e-010};
const Real dim050DiscrRStRShHalton[] = {2.92e-010, 5.02e-010};
const Real dim050DiscrRStartHalton[] = {1.93e-010, 6.82e-010};
const Real dim050Discr_Unit_Sobol[] = {1.63e-005, 8.14e-006, 4.07e-006, 2.04e-006,
                                       1.02e-006, 5.09e-007, 2.54e-007};

const Real dim100DiscrJackel_Sobol[] = {
    1.26e-018, 1.55e-018, 8.46e-019, 4.43e-019,
    4.04e-019, 2.44e-019, 4.86e-019
};
const Real dim100DiscrSobLev_Sobol[] = {
    1.17e-018, 2.65e-018, 1.45e-018, 7.28e-019,
    6.33e-019, 3.36e-019, 3.43e-019
};
const Real dim100DiscrSobLem_Sobol[] = {
    8.79e-019, 4.60e-019, 6.69e-019, 7.17e-019,
    5.81e-019, 2.97e-019, 2.64e-019
};
const Real dim100DiscrMersenneTwis[] = {
    5.30e-019, 7.29e-019, 3.71e-019, 3.33e-019,
    1.33e-017, 6.70e-018, 3.36e-018
};
const Real dim100DiscrPlain_Halton[] = {
    3.63e-004, 1.81e-004, 9.07e-005, 4.53e-005,
    2.27e-005, 1.13e-005, 5.66e-006
};
const Real dim100DiscrRShiftHalton[] = {3.36e-019, 2.19e-019};
const Real dim100DiscrRStRShHalton[] = {4.44e-019, 2.24e-019};
const Real dim100DiscrRStartHalton[] = {9.85e-020, 8.34e-019};
const Real dim100Discr_Unit_Sobol[] = {4.97e-006, 2.48e-006, 1.24e-006, 6.20e-007,
                                       3.10e-007, 1.55e-007, 7.76e-008};

const Size dimensionality[] = {2, 3, 5, 10, 15, 30, 50, 100 };

// 7 discrepancy measures for each dimension of all sequence generators
// would take a few days ... too long for usual/frequent test running
const Size discrepancyMeasuresNumber = 1;

// let's add some generality here...

class MersenneFactory {
  public:
    typedef RandomSequenceGenerator<MersenneTwisterUniformRng>
    MersenneTwisterUniformRsg;
    typedef MersenneTwisterUniformRsg generator_type;
    MersenneTwisterUniformRsg make(Size dim,
                                   BigNatural seed) const {
        return MersenneTwisterUniformRsg(dim,seed);
    }
    std::string name() const { return "Mersenne Twister"; }
};

class SobolFactory {
  public:
    typedef SobolRsg generator_type;
    explicit SobolFactory(SobolRsg::DirectionIntegers unit) : unit_(unit) {}
    SobolRsg make(Size dim,
                  BigNatural seed) const {
        return SobolRsg(dim,seed,unit_);
    }
    std::string name() const {
        std::string prefix;
        switch (unit_) {
          case SobolRsg::Unit:
            prefix = "unit-initialized ";
            break;
          case SobolRsg::Jaeckel:
            prefix = "Jaeckel-initialized ";
            break;
          case SobolRsg::SobolLevitan:
            prefix = "SobolLevitan-initialized ";
            break;
          case SobolRsg::SobolLevitanLemieux:
            prefix = "SobolLevitanLemieux-initialized ";
            break;
          case SobolRsg::Kuo:
            prefix = "Kuo";
            break;
          case SobolRsg::Kuo2:
            prefix = "Kuo2";
            break;
          case SobolRsg::Kuo3:
            prefix = "Kuo3";
            break;
          default:
            QL_FAIL("unknown direction integers");
        }
        return prefix + "Sobol sequences: ";
    }
  private:
    SobolRsg::DirectionIntegers unit_;
};

class HaltonFactory {
  public:
    typedef HaltonRsg generator_type;
    HaltonFactory(bool randomStart, bool randomShift)
    : start_(randomStart), shift_(randomShift) {}
    HaltonRsg make(Size dim,
                   BigNatural seed) const {
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
                              #ifndef PRINT_ONLY
                              const Real * const discrepancy[8],
                              const std::string&,
                              const std::string&
                              #else
                              const Real * const [8],
                              const std::string& fileName,
                              const std::string& arrayName
                              #endif
                              ) {

    #ifndef PRINT_ONLY
    Real tolerance = 1.0e-2;
    #endif

    std::vector<Real> point;
    Size dim;
    BigNatural seed = 123456;
    Real discr;
    // more than 1 discrepancy measures take long time
    Size sampleLoops = std::max<Size>(1, discrepancyMeasuresNumber);

    #ifdef PRINT_ONLY
    std::ofstream outStream(fileName.c_str());
    #endif
    for (Integer i = 0; i<8; i++) {
        #ifdef PRINT_ONLY
        outStream << std::endl;
        #endif

        dim = dimensionality[i];
        DiscrepancyStatistics stat(dim);

        typename T::generator_type rsg = generatorFactory.make(dim, seed);

        Size j, k=0, jMin=10;
        stat.reset();
        #ifdef PRINT_ONLY
        outStream << "const Real dim" << dim
                  << arrayName << "[] = {" ;
        #endif
        for (j=jMin; j<jMin+sampleLoops; j++) {
            Size points = Size(std::pow(2.0, Integer(j)))-1;
            for (; k<points; k++) {
                point = rsg.nextSequence().value;
                stat.add(point);
            }

            discr = stat.discrepancy();

            #ifdef PRINT_ONLY
            if (j!=jMin)
                outStream << ", ";
            outStream << std::fixed << std::setprecision(2) << discr;
            #else
            if (std::fabs(discr-discrepancy[i][j-jMin])>tolerance*discr) {
                BOOST_ERROR(generatorFactory.name()
                            << "discrepancy dimension " << dimensionality[i]
                            << " at " << points << " samples is "
                            << discr << " instead of "
                            << discrepancy[i][j-jMin]);
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


BOOST_AUTO_TEST_CASE(testMersenneTwisterDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing Mersenne-twister discrepancy...");

    const Real * const discrepancy[8] = {
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

BOOST_AUTO_TEST_CASE(testPlainHaltonDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing plain Halton discrepancy...");

    const Real * const discrepancy[8] = {
        dim002DiscrPlain_Halton, dim003DiscrPlain_Halton,
        dim005DiscrPlain_Halton, dim010DiscrPlain_Halton,
        dim015DiscrPlain_Halton, dim030DiscrPlain_Halton,
        dim050DiscrPlain_Halton, dim100DiscrPlain_Halton};

    testGeneratorDiscrepancy(HaltonFactory(false,false),
                             discrepancy,
                             "PlainHaltonDiscrepancy.txt",
                             "DiscrPlain_Halton");
}

BOOST_AUTO_TEST_CASE(testRandomStartHaltonDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing random-start Halton discrepancy...");

    const Real * const discrepancy[8] = {
        dim002DiscrRStartHalton, dim003DiscrRStartHalton,
        dim005DiscrRStartHalton, dim010DiscrRStartHalton,
        dim015DiscrRStartHalton, dim030DiscrRStartHalton,
        dim050DiscrRStartHalton, dim100DiscrRStartHalton};

    testGeneratorDiscrepancy(HaltonFactory(true,false),
                             discrepancy,
                             "RandomStartHaltonDiscrepancy.txt",
                             "DiscrRStartHalton");
}

BOOST_AUTO_TEST_CASE(testRandomShiftHaltonDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing random-shift Halton discrepancy...");

    const Real * const discrepancy[8] = {
        dim002DiscrRShiftHalton, dim003DiscrRShiftHalton,
        dim005DiscrRShiftHalton, dim010DiscrRShiftHalton,
        dim015DiscrRShiftHalton, dim030DiscrRShiftHalton,
        dim050DiscrRShiftHalton, dim100DiscrRShiftHalton};

    testGeneratorDiscrepancy(HaltonFactory(false,true),
                             discrepancy,
                             "RandomShiftHaltonDiscrepancy.txt",
                             "DiscrRShiftHalton");
}

BOOST_AUTO_TEST_CASE(testRandomStartRandomShiftHaltonDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing random-start, random-shift Halton discrepancy...");

    const Real * const discrepancy[8] = {
        dim002DiscrRStRShHalton, dim003DiscrRStRShHalton,
        dim005DiscrRStRShHalton, dim010DiscrRStRShHalton,
        dim015DiscrRStRShHalton, dim030DiscrRStRShHalton,
        dim050DiscrRStRShHalton, dim100DiscrRStRShHalton};

    testGeneratorDiscrepancy(HaltonFactory(true,true),
                             discrepancy,
                             "RandomStartRandomShiftHaltonDiscrepancy.txt",
                             "DiscrRStRShHalton");
}

BOOST_AUTO_TEST_CASE(testJackelSobolDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing Jaeckel-Sobol discrepancy...");

    const Real * const discrepancy[8] = {
        dim002Discr_Sobol, dim003Discr_Sobol,
        dim005Discr_Sobol, dim010DiscrJackel_Sobol,
        dim015DiscrJackel_Sobol, dim030DiscrJackel_Sobol,
        dim050DiscrJackel_Sobol, dim100DiscrJackel_Sobol};

    testGeneratorDiscrepancy(SobolFactory(SobolRsg::Jaeckel),
                             discrepancy,
                             "JackelSobolDiscrepancy.txt",
                             "DiscrJackel_Sobol");
}

BOOST_AUTO_TEST_CASE(testSobolLevitanSobolDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing Levitan-Sobol discrepancy...");

    const Real * const discrepancy[8] = {
        dim002Discr_Sobol, dim003Discr_Sobol,
        dim005Discr_Sobol, dim010DiscrSobLev_Sobol,
        dim015DiscrSobLev_Sobol, dim030DiscrSobLev_Sobol,
        dim050DiscrSobLev_Sobol, dim100DiscrSobLev_Sobol};

    testGeneratorDiscrepancy(SobolFactory(SobolRsg::SobolLevitan),
                             discrepancy,
                             "SobolLevitanSobolDiscrepancy.txt",
                             "DiscrSobLev_Sobol");
}

BOOST_AUTO_TEST_CASE(testSobolLevitanLemieuxSobolDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing Levitan-Lemieux-Sobol discrepancy...");

    const Real * const discrepancy[8] = {
        dim002Discr_Sobol, dim003Discr_Sobol,
        dim005Discr_Sobol, dim010DiscrSobLev_Sobol,
        dim015DiscrSobLev_Sobol, dim030DiscrSobLev_Sobol,
        dim050DiscrSobLem_Sobol, dim100DiscrSobLem_Sobol};

    testGeneratorDiscrepancy(SobolFactory(SobolRsg::SobolLevitanLemieux),
                             discrepancy,
                             "SobolLevitanLemieuxSobolDiscrepancy.txt",
                             "DiscrSobLevLem_Sobol");
}

BOOST_AUTO_TEST_CASE(testUnitSobolDiscrepancy) {

    BOOST_TEST_MESSAGE("Testing unit Sobol discrepancy...");

    const Real* const discrepancy[8] = {dim002Discr_Unit_Sobol, dim003Discr_Unit_Sobol,
                                        dim005Discr_Unit_Sobol, dim010Discr_Unit_Sobol,
                                        dim015Discr_Unit_Sobol, dim030Discr_Unit_Sobol,
                                        dim050Discr_Unit_Sobol, dim100Discr_Unit_Sobol};

    testGeneratorDiscrepancy(SobolFactory(SobolRsg::Unit), discrepancy, "UnitSobolDiscrepancy.txt",
                             "Discr__Unit_Sobol");
}

BOOST_AUTO_TEST_CASE(testSobolSkipping) {

    BOOST_TEST_MESSAGE("Testing Sobol sequence skipping...");

    unsigned long seed = 42;
    Size dimensionality[] = { 1, 10, 100, 1000 };
    unsigned long skip[] = { 0, 1, 42, 512, 100000 };
    SobolRsg::DirectionIntegers integers[] = { SobolRsg::Unit,
                                               SobolRsg::Jaeckel,
                                               SobolRsg::SobolLevitan,
                                               SobolRsg::SobolLevitanLemieux };

    for (auto& integer : integers) {
        for (Size& j : dimensionality) {
            for (unsigned long& k : skip) {

                // extract n samples
                SobolRsg rsg1(j, seed, integer);
                for (Size l = 0; l < k; l++)
                    rsg1.nextInt32Sequence();

                // skip n samples at once
                SobolRsg rsg2(j, seed, integer);
                rsg2.skipTo(k);

                // compare next 100 samples
                for (Size m = 0; m < 100; m++) {
                    const std::vector<std::uint_least32_t>& s1 = rsg1.nextInt32Sequence();
                    const std::vector<std::uint_least32_t>& s2 = rsg2.nextInt32Sequence();
                    for (Size n = 0; n < s1.size(); n++) {
                        if (s1[n] != s2[n]) {
                            BOOST_ERROR("Mismatch after skipping:"
                                        << "\n  size:     " << j << "\n  integers: " << integer
                                        << "\n  skipped:  " << k << "\n  at index: " << n
                                        << "\n  expected: " << s1[n] << "\n  found:    " << s2[n]);
                        }
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testSobolBurleySkipping) {

    BOOST_TEST_MESSAGE("Testing Sobol Burley sequence skipping...");

    unsigned long seed = 42;
    unsigned long scramblingSeed = 43;
    Size dimensionality[] = { 1, 10, 100, 1000 };
    unsigned long skip[] = { 0, 1, 42, 512, 100000 };
    SobolRsg::DirectionIntegers integers[] = {
        SobolRsg::Jaeckel,
        SobolRsg::SobolLevitan,
        SobolRsg::SobolLevitanLemieux };

        for (auto& integer : integers) {
            for (Size& j : dimensionality) {
                for (unsigned long& k : skip) {

                    // extract n samples
                    Burley2020SobolRsg rsg1(j, seed, integer, scramblingSeed);
                    for (Size l = 0; l < k; l++)
                        rsg1.nextInt32Sequence();

                    // skip n samples at once
                    Burley2020SobolRsg rsg2(j, seed, integer, scramblingSeed);
                    rsg2.skipTo(k);

                    // compare next 100 samples
                    for (Size m = 0; m < 100; m++) {
                        const std::vector<std::uint_least32_t>& s1 = rsg1.nextInt32Sequence();
                        const std::vector<std::uint_least32_t>& s2 = rsg2.nextInt32Sequence();
                        for (Size n = 0; n < s1.size(); n++) {
                            if (s1[n] != s2[n]) {
                                BOOST_ERROR("Mismatch after skipping:"
                                << "\n  size:     " << j << "\n  integers: " << integer
                                << "\n  skipped:  " << k << "\n  at index: " << n
                                << "\n  expected: " << s1[n] << "\n  found:    " << s2[n]);
                            }
                        }
                    }
                }
            }
        }
}

BOOST_AUTO_TEST_CASE(testHighDimensionalIntegrals, *precondition(if_speed(Slow))) {
    BOOST_TEST_MESSAGE("Testing high-dimensional integrals...");

    /* We are running "Integration test 1, results for high dimensions" (Figure 9) from:

       Sobol, Asotsky, Kreinin, Kucherenko: Construction and Comparison of High-Dimensional Sobol’
       Generators, available at https://www.broda.co.uk/doc/HD_SobolGenerator.pdf

       We check the error of Kuo1 (using Gray code and sequential numbers) roughly against what
       their graph suggests. In addition we check the error of the Burley2020-scrambled version of
       Kuo1 against what we experimentally find - the error turns out to be more than one order
       better than the unscrambled version. */

    auto integrand = [](const std::vector<Real>& c, const std::vector<Real>& x) {
        Real p = 1.0;
        for (Size i = 0; i < c.size(); ++i) {
            p *= 1.0 + c[i] * (x[i] - 0.5);
        }
        return p;
    };

    Size N = 30031;

    std::vector<Size> dimension = {1000, 2000, 5000};
    std::vector<std::vector<Real>> expectedOrderOfError = {
        {-3.0, -3.0, -4.5}, {-2.5, -2.5, -4.0}, {-2.0, -2.0, -4.0}};

    for (Size d = 0; d < dimension.size(); ++d) {

        std::vector<Real> c1(dimension[d], 0.01);

        SobolRsg s1(dimension[d], 42, SobolRsg::DirectionIntegers::Kuo, true);
        SobolRsg s2(dimension[d], 42, SobolRsg::DirectionIntegers::Kuo, false);
        Burley2020SobolRsg s3(dimension[d], 42, SobolRsg::DirectionIntegers::Kuo, 43);

        Real I1 = 0.0, I2 = 0.0, I3 = 0.0;
        for (Size i = 0; i < N; ++i) {
            I1 += integrand(c1, s1.nextSequence().value) / static_cast<double>(N);
            I2 += integrand(c1, s2.nextSequence().value) / static_cast<double>(N);
            I3 += integrand(c1, s3.nextSequence().value) / static_cast<double>(N);
        }

        Real errOrder1 = std::log10(std::abs(I1 - 1.0));
        Real errOrder2 = std::log10(std::abs(I2 - 1.0));
        Real errOrder3 = std::log10(std::abs(I3 - 1.0));

        BOOST_CHECK_MESSAGE(errOrder1 < expectedOrderOfError[d][0],
                            "order of error for dimension " + std::to_string(dimension[d]) + " is" +
                                std::to_string(errOrder1) + " expected " +
                                std::to_string(expectedOrderOfError[d][0]));
        BOOST_CHECK_MESSAGE(errOrder2 < expectedOrderOfError[d][1],
                            "order of error for dimension " + std::to_string(dimension[d]) + " is" +
                                std::to_string(errOrder2) + " expected " +
                                std::to_string(expectedOrderOfError[d][1]));
        BOOST_CHECK_MESSAGE(errOrder3 < expectedOrderOfError[d][2],
                            "order of error for dimension " + std::to_string(dimension[d]) + " is" +
                                std::to_string(errOrder3) + " expected " +
                                std::to_string(expectedOrderOfError[d][2]));
    }
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
