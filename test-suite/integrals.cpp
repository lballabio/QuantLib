/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/integrals/exponentialintegrals.hpp>
#include <ql/math/integrals/filonintegral.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/integrals/trapezoidintegral.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/integrals/expsinhintegral.hpp>
#include <ql/math/integrals/tanhsinhintegral.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/termstructures/volatility/abcd.hpp>
#include <ql/math/integrals/twodimensionalintegral.hpp>
#include <ql/experimental/math/piecewisefunction.hpp>
#include <ql/experimental/math/piecewiseintegral.hpp>

#include <boost/math/special_functions/sign.hpp>

using namespace QuantLib;
using namespace boost::unit_test;

namespace integrals_test {

    Real tolerance = 1.0e-6;

    template <class T>
    void testSingle(const T& I, const std::string& tag,
                    const ext::function<Real (Real)>& f,
                    Real xMin, Real xMax, Real expected) {
        Real calculated = I(f,xMin,xMax);
        if (std::fabs(calculated-expected) > integrals_test::tolerance) {
            BOOST_FAIL(std::setprecision(10)
                       << "integrating " << tag
                       << "    calculated: " << calculated
                       << "    expected:   " << expected);
        }
    }

    template <class T>
    void testSeveral(const T& I) {
        testSingle(I, "f(x) = 0", [](Real x) -> Real { return 0.0; }, 0.0, 1.0, 0.0);
        testSingle(I, "f(x) = 1", [](Real x) -> Real { return 1.0; }, 0.0, 1.0, 1.0);
        testSingle(I, "f(x) = x", [](Real x) -> Real { return x; }, 0.0, 1.0, 0.5);
        testSingle(I, "f(x) = x^2",
                   [](Real x) -> Real { return x * x; }, 0.0, 1.0, 1.0/3.0);
        testSingle(I, "f(x) = sin(x)",
                   [](Real x) -> Real { return std::sin(x); }, 0.0, M_PI, 2.0);
        testSingle(I, "f(x) = cos(x)",
                   [](Real x) -> Real { return std::cos(x); }, 0.0, M_PI, 0.0);

        testSingle(I, "f(x) = Gaussian(x)",
                   NormalDistribution(), -10.0, 10.0, 1.0);
        testSingle(I, "f(x) = Abcd2(x)",
                   AbcdSquared(0.07, 0.07, 0.5, 0.1, 8.0, 10.0), 5.0, 6.0,
                   AbcdFunction(0.07, 0.07, 0.5, 0.1).covariance(5.0, 6.0, 8.0, 10.0));
    }

    template <class T>
    void testDegeneratedDomain(const T& I) {
        testSingle(I, "f(x) = 0 over [1, 1 + macheps]", [](Real x) -> Real { return 0.0; }, 1.0,
            1.0 + QL_EPSILON, 0.0);
    }

    class sineF {
      public:
        Real operator()(Real x) const {
            return std::exp(-0.5*(x - M_PI_2/100));
        }
    };

    class cosineF {
      public:
        Real operator()(Real x) const {
            return std::exp(-0.5*x);
        }
    };

    Real f1(Real x) {
        return 1.2*x*x+3.2*x+3.1;
    }

    Real f2(Real x) {
        return 4.3*(x-2.34)*(x-2.34)-6.2*(x-2.34) + f1(2.34);
    }

    std::vector<Real> x, y;

    Real pw_fct(const Real t) { return QL_PIECEWISE_FUNCTION(x, y, t); }

    void pw_check(const Integrator &in, const Real a, const Real b,
                  const Real expected) {
        Real calculated = in(pw_fct, a, b);
        if (!close(calculated, expected))
            BOOST_FAIL(std::setprecision(16)
                       << "piecewise integration over [" << a << "," << b
                       << "] failed: "
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   difference: " << (calculated - expected));
    }

    template <class T>
    void reportSiCiFail(
        const std::string& name, T z, T c, T e, Real diff, Real tol) {
        BOOST_FAIL(std::setprecision(16)
                   << name << " calculation failed for " << z
                   << "\n calculated: " << c
                   << "\n expected:   " << e
                   << "\n difference: " << diff
                   << "\n tolerance:  " << tol);
    }
}

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(IntegralTest)

BOOST_AUTO_TEST_CASE(testSegment) {
    BOOST_TEST_MESSAGE("Testing segment integration...");

    using namespace integrals_test;

    testSeveral(SegmentIntegral(10000));
    testDegeneratedDomain(SegmentIntegral(10000));
}

BOOST_AUTO_TEST_CASE(testTrapezoid) {
    BOOST_TEST_MESSAGE("Testing trapezoid integration...");

    using namespace integrals_test;

    testSeveral(TrapezoidIntegral<Default>(integrals_test::tolerance, 10000));
    testDegeneratedDomain(TrapezoidIntegral<Default>(integrals_test::tolerance, 10000));
}

BOOST_AUTO_TEST_CASE(testMidPointTrapezoid) {
    BOOST_TEST_MESSAGE("Testing mid-point trapezoid integration...");

    using namespace integrals_test;

    testSeveral(TrapezoidIntegral<MidPoint>(integrals_test::tolerance, 10000));
    testDegeneratedDomain(TrapezoidIntegral<MidPoint>(integrals_test::tolerance, 10000));
}

BOOST_AUTO_TEST_CASE(testSimpson) {
    BOOST_TEST_MESSAGE("Testing Simpson integration...");

    using namespace integrals_test;

    testSeveral(SimpsonIntegral(integrals_test::tolerance, 10000));
    testDegeneratedDomain(SimpsonIntegral(integrals_test::tolerance, 10000));
}

BOOST_AUTO_TEST_CASE(testGaussKronrodAdaptive) {
    BOOST_TEST_MESSAGE("Testing adaptive Gauss-Kronrod integration...");

    using namespace integrals_test;

    Size maxEvaluations = 1000;
    testSeveral(GaussKronrodAdaptive(integrals_test::tolerance, maxEvaluations));
    testDegeneratedDomain(GaussKronrodAdaptive(integrals_test::tolerance, maxEvaluations));
}

BOOST_AUTO_TEST_CASE(testGaussLobatto) {
    BOOST_TEST_MESSAGE("Testing adaptive Gauss-Lobatto integration...");

    using namespace integrals_test;

    Size maxEvaluations = 1000;
    testSeveral(GaussLobattoIntegral(maxEvaluations, integrals_test::tolerance));
    // on degenerated domain [1,1+macheps] an exception is thrown
    // which is also ok, but not tested here
}

#ifdef QL_BOOST_HAS_TANH_SINH
BOOST_AUTO_TEST_CASE(testTanhSinh) {
    BOOST_TEST_MESSAGE("Testing tanh-sinh integration...");

    using namespace integrals_test;
    testSeveral(TanhSinhIntegral());
}
#endif

#ifdef QL_BOOST_HAS_EXP_SINH
BOOST_AUTO_TEST_CASE(testExpSinh) {
    BOOST_TEST_MESSAGE("Testing exp-sinh integration...");

    using namespace integrals_test;

    const ExpSinhIntegral integrator;
    testSingle(integrator,
        "f(x) = Gaussian(x)", NormalDistribution(),
        0.0, std::numeric_limits<Real>::max(), 0.5);

    testSingle(integrator,
        "f(x) = x*e^(-x)", [](Real x) { return x*std::exp(-x); },
        0.0, std::numeric_limits<Real>::max(), 1.0);
}
#endif

BOOST_AUTO_TEST_CASE(testGaussLegendreIntegrator) {
    BOOST_TEST_MESSAGE("Testing Gauss-Legendre integrator...");

    using namespace integrals_test;

    const GaussLegendreIntegrator integrator(64);
    testSeveral(integrator);
    testDegeneratedDomain(integrator);
}

BOOST_AUTO_TEST_CASE(testGaussChebyshevIntegrator) {
    BOOST_TEST_MESSAGE("Testing Gauss-Chebyshev integrator...");

    using namespace integrals_test;

    const GaussChebyshevIntegrator integrator(64);
    testSingle(integrator, "f(x) = Gaussian(x)",
               NormalDistribution(), -10.0, 10.0, 1.0);
    testDegeneratedDomain(integrator);
}

BOOST_AUTO_TEST_CASE(testGaussChebyshev2ndIntegrator) {
    BOOST_TEST_MESSAGE("Testing Gauss-Chebyshev 2nd integrator...");

    using namespace integrals_test;

    const GaussChebyshev2ndIntegrator integrator(64);
    testSingle(integrator, "f(x) = Gaussian(x)",
               NormalDistribution(), -10.0, 10.0, 1.0);
    testDegeneratedDomain(integrator);
}

BOOST_AUTO_TEST_CASE(testGaussKronrodNonAdaptive) {
    BOOST_TEST_MESSAGE("Testing non-adaptive Gauss-Kronrod integration...");

    using namespace integrals_test;

    Real precision = integrals_test::tolerance;
    Size maxEvaluations = 100;
    Real relativeAccuracy = integrals_test::tolerance;
    GaussKronrodNonAdaptive gaussKronrodNonAdaptive(precision, maxEvaluations,
                                                    relativeAccuracy);
    testSeveral(gaussKronrodNonAdaptive);
    testDegeneratedDomain(gaussKronrodNonAdaptive);
}

BOOST_AUTO_TEST_CASE(testTwoDimensionalIntegration) {
    BOOST_TEST_MESSAGE("Testing two dimensional adaptive "
                       "Gauss-Lobatto integration...");

    using namespace integrals_test;

    const Size maxEvaluations = 1000;
    const Real calculated = TwoDimensionalIntegral(
        ext::shared_ptr<Integrator>(
            new TrapezoidIntegral<Default>(integrals_test::tolerance, maxEvaluations)),
        ext::shared_ptr<Integrator>(
            new TrapezoidIntegral<Default>(integrals_test::tolerance, maxEvaluations)))(
        std::multiplies<>(),
        std::make_pair(0.0, 0.0), std::make_pair(1.0, 2.0));

    const Real expected = 1.0;
    if (std::fabs(calculated-expected) > integrals_test::tolerance) {
        BOOST_FAIL(std::setprecision(10)
                   << "two dimensional integration: "
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }
}

BOOST_AUTO_TEST_CASE(testFolinIntegration) {
    BOOST_TEST_MESSAGE("Testing Folin's integral formulae...");

    using namespace integrals_test;

    // Examples taken from
    // http://www.tat.physik.uni-tuebingen.de/~kokkotas/Teaching/Num_Methods_files/Comp_Phys5.pdf
    const Size nr[] = { 4, 8, 16, 128, 256, 1024, 2048 };
    const Real expected[] = { 4.55229440e-5,4.72338540e-5, 4.72338540e-5,
                              4.78308678e-5,4.78404787e-5, 4.78381120e-5,
                              4.78381084e-5};

    const Real t = 100;
    const Real o = M_PI_2/t;

    const Real tol = 1e-12;

    for (Size i=0; i < LENGTH(nr); ++i) {
        const Size n = nr[i];
        const Real calculatedCosine
            = FilonIntegral(FilonIntegral::Cosine, t, n)(cosineF(),0,2*M_PI);
        const Real calculatedSine
            = FilonIntegral(FilonIntegral::Sine, t, n)
                (sineF(), o,2*M_PI + o);

        if (std::fabs(calculatedCosine-expected[i]) > tol) {
            BOOST_FAIL(std::setprecision(10)
                << "Filon Cosine integration failed: "
                << "\n    calculated: " << calculatedCosine
                << "\n    expected:   " << expected[i]);
        }
        if (std::fabs(calculatedSine-expected[i]) > tol) {
            BOOST_FAIL(std::setprecision(10)
                << "Filon Sine integration failed: "
                << "\n    calculated: " << calculatedCosine
                << "\n    expected:   " << expected[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(testDiscreteIntegrals) {
    BOOST_TEST_MESSAGE("Testing discrete integral formulae...");

    using namespace integrals_test;

    Array x(6), f(6);
    x[0] = 1.0; x[1] = 2.02; x[2] = 2.34; x[3] = 3.3; x[4] = 4.2; x[5] = 4.6;

    std::transform(x.begin(), x.begin()+3, f.begin(),   f1);
    std::transform(x.begin()+3, x.end(),   f.begin()+3, f2);

    const Real expectedSimpson =
        16.0401216 + 30.4137528 + 0.2*f2(4.2) + 0.2*f2(4.6);
    const Real expectedTrapezoid =
          0.5*(f1(1.0)  + f1(2.02))*1.02
        + 0.5*(f1(2.02) + f1(2.34))*0.32
        + 0.5*(f2(2.34) + f2(3.3) )*0.96
        + 0.5*(f2(3.3)  + f2(4.2) )*0.9
        + 0.5*(f2(4.2)  + f2(4.6) )*0.4;

    const Real calculatedSimpson =  DiscreteSimpsonIntegral()(x, f);
    const Real calculatedTrapezoid = DiscreteTrapezoidIntegral()(x, f);

    const Real tol = 1e-12;
    if (std::fabs(calculatedSimpson-expectedSimpson) > tol) {
        BOOST_FAIL(std::setprecision(16)
            << "discrete Simpson integration failed: "
            << "\n    calculated: " << calculatedSimpson
            << "\n    expected:   " << expectedSimpson);
    }

    if (std::fabs(calculatedTrapezoid-expectedTrapezoid) > tol) {
        BOOST_FAIL(std::setprecision(16)
            << "discrete Trapezoid integration failed: "
            << "\n    calculated: " << calculatedTrapezoid
            << "\n    expected:   " << expectedTrapezoid);
    }
}

BOOST_AUTO_TEST_CASE(testDiscreteIntegrator) {
    BOOST_TEST_MESSAGE("Testing discrete integrator formulae...");

    using namespace integrals_test;

    testSeveral(DiscreteSimpsonIntegrator(300));
    testSeveral(DiscreteTrapezoidIntegrator(3000));
}

BOOST_AUTO_TEST_CASE(testPiecewiseIntegral) {
    BOOST_TEST_MESSAGE("Testing piecewise integral...");

    using namespace integrals_test;

    x = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    y = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    ext::shared_ptr<Integrator> segment =
        ext::make_shared<SegmentIntegral>(1);
    ext::shared_ptr<Integrator> piecewise =
        ext::make_shared<PiecewiseIntegral>(segment, x);
    pw_check(*piecewise, -1.0, 0.0, 1.0);
    pw_check(*piecewise, 0.0, 1.0, 1.0);
    pw_check(*piecewise, 0.0, 1.5, 2.0);
    pw_check(*piecewise, 0.0, 2.0, 3.0);
    pw_check(*piecewise, 0.0, 2.5, 4.5);
    pw_check(*piecewise, 0.0, 3.0, 6.0);
    pw_check(*piecewise, 0.0, 4.0, 10.0);
    pw_check(*piecewise, 0.0, 5.0, 15.0);
    pw_check(*piecewise, 0.0, 6.0, 21.0);
    pw_check(*piecewise, 0.0, 7.0, 27.0);
    pw_check(*piecewise, 3.5, 4.5, 4.5);
    pw_check(*piecewise, 5.0, 10.0, 30.0);
    pw_check(*piecewise, 9.0, 10.0, 6.0);
}

BOOST_AUTO_TEST_CASE(testExponentialIntegral) {
    BOOST_TEST_MESSAGE("Testing exponential integrals...");

    using namespace ExponentialIntegral;

    // reference values are calculated with Mathematica or Python/mpmath
    const Real data[][10] = {
        {1e-10, 0.0, 1.0e-10, 0.0, -22.4486352650389, 0.0, -22.4486352649389, 0.0, 22.4486352651389, 0.0},
        {7.0710678118655e-11, 7.0710678118655e-11, 7.0710678118655e-11, 7.0710678118655e-11, -22.4486352650389, 0.785398163397448, -22.4486352649682, 0.785398163468159, 22.4486352651096, -0.785398163326738},
        {3.0901699437495e-11, 9.5105651629515e-11, 3.0901699437495e-11, 9.5105651629515e-11, -22.4486352650389, 1.25663706143591, -22.448635265008, 1.25663706153102, 22.4486352650698, -1.25663706134081},
        {0.0, 1e-10, 0.0, 1.0e-10, -22.4486352650389, 1.5707963267949, -22.4486352650389, 1.5707963268949, 22.4486352650389, -1.5707963266949},
        {0.0, 1e-10, 0.0, 1.0e-10, -22.4486352650389, 1.5707963267949, -22.4486352650389, 1.5707963268949, 22.4486352650389, -1.5707963266949},
        {-8.0901699437495e-11, 5.8778525229247e-11, -8.0901699437495e-11, 5.8778525229247e-11, -22.4486352650389, 2.51327412287184, -22.4486352651198, 2.51327412293062, 22.448635264958, -2.51327412281306},
        {-1e-10, 0.0, -1.0e-10, 0.0, -22.4486352650389, 3.14159265358979, -22.4486352651389, 0.0, 22.4486352649389, -3.14159265358979},
        {-8.0901699437495e-11, -5.8778525229247e-11, -8.0901699437495e-11, -5.8778525229247e-11, -22.4486352650389, -2.51327412287184, -22.4486352651198, -2.51327412293062, 22.448635264958, 2.51327412281306},
        {0.0, -1e-10, 0.0, -1.0e-10, -22.4486352650389, -1.5707963267949, -22.4486352650389, -1.5707963268949, 22.4486352650389, 1.5707963266949},
        {3.0901699437495e-11, -9.5105651629515e-11, 3.0901699437495e-11, -9.5105651629515e-11, -22.4486352650389, -1.25663706143591, -22.448635265008, -1.25663706153102, 22.4486352650698, 1.25663706134081},
        {9.8768834059514e-11, -1.5643446504023002e-11, 9.8768834059514e-11, -1.5643446504023e-11, -22.4486352650389, -0.157079632679488, -22.4486352649402, -0.157079632695132, 22.4486352651377, 0.157079632663845},
        {0.15, 0.0, 0.149812626514082, 0.0, -1.32552404918277, 0.0, -1.16408641729839, 0.0, 1.46446167052028, 0.0},
        {0.1060660171779825, 0.1060660171779825, 0.106198510172016, 0.105933345197561, -1.31990959342105, 0.779773166034167, -1.21397624822349, 0.897221670932746, 1.42584293861861, -0.684824650588713},
        {0.0463525491562425, 0.14265847744427249, 0.0465043664443717, 0.1427686871506, -1.31535197062462, 1.25332575154654, -1.27825242518864, 1.40248660838809, 1.37065439517488, -1.11739007291224},
        {0.0, 0.15, 0.0, 0.150187626610941, -1.31427404390933, 1.5707963267949, -1.32552404918277, 1.72060895330898, 1.32552404918277, -1.42098370028081},
        {0.0, 0.15, 0.0, 0.150187626610941, -1.31427404390933, 1.5707963267949, -1.32552404918277, 1.72060895330898, 1.32552404918277, -1.42098370028081},
        {-0.1213525491562425, 0.0881677878438705, -0.121410363295163, 0.0879894647931175, -1.32164680474487, 2.51862071457814, -1.43946484971679, 2.59626744276408, 1.19687588593211, -2.41957522097486},
        {-0.15, 0.0, -0.149812626514082, 0.0, -1.32552404918277, 3.14159265358979, -1.46446167052028, 0.0, 1.16408641729839, -3.14159265358979},
        {-0.1213525491562425, -0.0881677878438705, -0.121410363295163, -0.0879894647931175, -1.32164680474487, -2.51862071457814, -1.43946484971679, -2.59626744276408, 1.19687588593211, 2.41957522097486},
        {0.0, -0.15, 0.0, -0.150187626610941, -1.31427404390933, -1.5707963267949, -1.32552404918277, -1.72060895330898, 1.32552404918277, 1.42098370028081},
        {0.0463525491562425, -0.14265847744427249, 0.0465043664443717, -0.1427686871506, -1.31535197062462, -1.25332575154654, -1.27825242518864, -1.40248660838809, 1.37065439517488, 1.11739007291224},
        {0.148153251089271, -0.0234651697560345, 0.147986276837203, -0.0233801359873959, -1.32524974813753, -0.155344509602526, -1.16622995490181, -0.182371337566645, 1.46287076355731, 0.135270572544445},
        {0.25, 0.0, 0.249133570319757, 0.0, -0.824663062580946, 0.0, -0.542543264661914, 0.0, 1.04428263444374, 0.0},
        {0.1767766952966375, 0.1767766952966375, 0.177389351153991, 0.17616173766105, -0.809119386275216, 0.769773219911456, -0.632957648614166, 0.978412458037432, 0.985281123936265, -0.623633755729451},
        {0.0772542485937375, 0.2377641290737875, 0.0779581492943877, 0.238274358309521, -0.796425249249655, 1.24741416450428, -0.745153392294084, 1.50303646097033, 0.898260598498369, -1.02852866129867},
        {0.0, 0.25, 0.0, 0.250869684890912, -0.793412949552826, 1.5707963267949, -0.824663062580946, 1.81992989711465, 0.824663062580946, -1.32166275647514},
        {0.0, 0.25, 0.0, 0.250869684890912, -0.793412949552826, 1.5707963267949, -0.824663062580946, 1.81992989711465, 0.824663062580946, -1.32166275647514},
        {-0.2022542485937375, 0.1469463130731175, -0.20252086544385, 0.146120744825161, -0.813939960005834, 2.52811043072268, -1.00626764691037, 2.64616186234439, 0.60229889383601, -2.35061809970499},
        {-0.25, 0.0, -0.249133570319757, 0.0, -0.824663062580946, 3.14159265358979, -1.04428263444374, 0.0, 0.542543264661914, -3.14159265358979},
        {-0.2022542485937375, -0.1469463130731175, -0.20252086544385, -0.146120744825161, -0.813939960005834, -2.52811043072268, -1.00626764691037, -2.64616186234439, 0.60229889383601, 2.35061809970499},
        {0.0, -0.25, 0.0, -0.250869684890912, -0.793412949552826, -1.5707963267949, -0.824663062580946, -1.81992989711465, 0.824663062580946, 1.32166275647514},
        {0.0772542485937375, -0.2377641290737875, 0.0779581492943877, -0.238274358309521, -0.796425249249655, -1.24741416450428, -0.745153392294084, -1.50303646097033, 0.898260598498369, 1.02852866129867},
        {0.246922085148785, -0.0391086162600575, 0.24614979209014, -0.0387156766342252, -0.823906068503191, -0.152275113509673, -0.546488805945054, -0.201435843693654, 1.04188216592042, 0.122428128357486},
        {1.0, 0.0, 0.946083070367183, 0.0, 0.337403922900968, 0.0, 1.89511781635594, 0.0, 0.21938393439552, 0.0},
        {0.70710678118655, 0.70710678118655, 0.745192155353662, 0.666664817419508, 0.566802098259312, 0.535629617322428, 1.23346691567882, 1.78035886482613, 0.0998627191601961, -0.289974554118806},
        {0.30901699437495, 0.95105651629515, 0.355652074843551, 0.983694298574337, 0.782614772996823, 1.09956193553216, 0.643964830804846, 2.31231301720838, -0.112533957890793, -0.475476714030747},
        {0.0, 1.0, 0.0, 1.05725087537573, 0.837866940980208, 1.5707963267949, 0.337403922900968, 2.51687939716208, -0.337403922900968, -0.624713256427714},
        {0.0, 1.0, 0.0, 1.05725087537573, 0.837866940980208, 1.5707963267949, 0.337403922900968, 2.51687939716208, -0.337403922900968, -0.624713256427714},
        {-0.80901699437495, 0.58778525229247, -0.824526943360603, 0.5349755552469, 0.491722358913221, 2.74478237579885, -0.14431784116889, 2.91012082986304, -1.43603057378731, -1.62893165104155},
        {-1.0, 0.0, -0.946083070367183, 0.0, 0.337403922900968, 3.14159265358979, -0.21938393439552, 0.0, -1.89511781635594, -3.14159265358979},
        {-0.80901699437495, -0.58778525229247, -0.824526943360603, -0.5349755552469, 0.491722358913221, -2.74478237579885, -0.14431784116889, -2.91012082986304, -1.43603057378731, 1.62893165104155},
        {0.0, -1.0, 0.0, -1.05725087537573, 0.837866940980208, -1.5707963267949, 0.337403922900968, -2.51687939716208, -0.337403922900968, 0.624713256427714},
        {0.30901699437495, -0.95105651629515, 0.355652074843551, -0.983694298574337, 0.782614772996823, -1.09956193553216, 0.643964830804846, -2.31231301720838, -0.112533957890793, 0.475476714030747},
        {0.98768834059514, -0.15643446504023, 0.939353669480516, -0.132366326809511, 0.347743692745538, -0.0857637957494435, 1.86192420379474, -0.4235071237, 0.214836056406461, 0.0577866622153682},
        {5.0, 0.0, 1.54993124494467, 0.0, -0.190029749656644, 0.0, 40.1852753558032, 0.0, 0.00114829559127533, 0.0},
        {3.53553390593275, 3.53553390593275, 3.68715086115432, -3.15718137390906, -3.15476810467167, -2.11185029092794, -6.31194947858072, 7.36979747887716, -0.00241326923739065, 0.00450424343148012},
        {1.5450849718747501, 4.75528258147575, 14.299679516973, 6.85221185491562, 6.85257226323722, -12.7303117750282, -0.931350039879264, 2.99045284011251, 0.0356665739529384, 0.0160488285537158},
        {0.0, 5.0, 0.0, 20.0932118256972, 20.0920635301059, 1.5707963267949, -0.190029749656644, 3.12072757173957, 0.190029749656644, -0.0208650818502225},
        {0.0, 5.0, 0.0, 20.0932118256972, 20.0920635301059, 1.5707963267949, -0.190029749656644, 3.12072757173957, 0.190029749656644, -0.0208650818502225},
        {-4.04508497187475, 2.93892626146235, -2.0577013528011, -1.96223940975232, -1.9637046590567, 3.61921566552724, 0.00286020292932927, 3.14261835694337, 6.84905720502975, 11.1883945116728},
        {-5.0, 0.0, -1.54993124494467, 0.0, -0.190029749656644, 3.14159265358979, -0.00114829559127533, 0.0, -40.1852753558032, -3.14159265358979},
        {-4.04508497187475, -2.93892626146235, -2.0577013528011, 1.96223940975232, -1.9637046590567, -3.61921566552724, 0.00286020292932927, -3.14261835694337, 6.84905720502975, -11.1883945116728},
        {0.0, -5.0, 0.0, -20.0932118256972, 20.0920635301059, -1.5707963267949, -0.190029749656644, -3.12072757173957, 0.190029749656644, 0.0208650818502225},
        {1.5450849718747501, -4.75528258147575, 14.299679516973, -6.85221185491562, 6.85257226323722, 12.7303117750282, -0.931350039879264, -2.99045284011251, 0.0356665739529384, -0.0160488285537158},
        {4.9384417029757, -0.7821723252011501, 1.53351371140353, 0.167535111630988, -0.252671967618136, -0.0455545136665558, 31.7637646606649, -20.6127722347705, 0.000742118122850436, 0.000971589948194675},
        {10.0, 0.0, 1.65834759421887, 0.0, -0.0454564330044554, 0.0, 2492.22897624188, 0.0, 4.15696892968532e-6, 0.0},
        {7.0710678118655, 7.0710678118655, -3.77451753034182, 62.6425755592338, 62.6425711229056, 5.34523470197841, 125.285146682139, -7.54895590552534, 4.43632828562146e-6, -7.91551583068017e-5},
        {3.0901699437495003, 9.5105651629515, 303.07292777526, -690.037761260879, -690.037754650298, -301.502129842997, -0.659900725018632, 5.27667742385125, -0.00134856502993308, 0.00415958644984393},
        {0.0, 10.0, 0.0, 1246.11449019942, 1246.11448604245, 1.5707963267949, -0.0454564330044554, 3.22914392101377, 0.0454564330044554, 0.0875512674239774},
        {0.0, 10.0, 0.0, 1246.11449019942, 1246.11448604245, 1.5707963267949, -0.0454564330044554, 3.22914392101377, 0.0454564330044554, 0.0875512674239774},
        {-8.0901699437495, 5.8778525229247, -14.6236949578037, 13.4643508624518, 13.4645870261785, 16.1946084513107, -2.79815608075126e-5, 3.14158769865141, -157.085481478947, -317.2439811058},
        {-10.0, 0.0, -1.65834759421887, 0.0, -0.0454564330044554, 3.14159265358979, -4.15696892968532e-6, 0.0, -2492.22897624188, -3.14159265358979},
        {-8.0901699437495, -5.8778525229247, -14.6236949578037, -13.4643508624518, 13.4645870261785, -16.1946084513107, -2.79815608075126e-5, -3.14158769865141, -157.085481478947, 317.2439811058},
        {0.0, -10.0, 0.0, -1246.11449019942, 1246.11448604245, -1.5707963267949, -0.0454564330044554, -3.22914392101377, 0.0454564330044554, -0.0875512674239774},
        {3.0901699437495003, -9.5105651629515, 303.07292777526, 690.037761260879, -690.037754650298, 301.502129842997, -0.659900725018632, -5.27667742385125, -0.00134856502993308, -0.00415958644984393},
        {9.8768834059514, -1.5643446504023002, 1.78956084261706, 0.114701769782499, -0.118816490702582, 0.198823504802007, 411.904076239608, -2157.22483235914, -6.48699583272709e-7, 4.66032253043785e-6},
        {25.0, 0.0, 1.53148255099996, 0.0, -0.00684859717970259, 0.0, 3005950906.52555, 0.0, 5.34889975534022e-13, 0.0},
        {17.67766952966375, 17.67766952966375, -894423.548678786, -396595.979622699, -396595.9796227, 894425.119475113, -793191.959245399, -1788847.09735757, 7.48981460647877e-10, 3.27816276287981e-10},
        {7.72542485937375, 23.77641290737875, 395787595.545024, 194501516.12134, 194501516.12134, -395787593.974227, -80.7948153607822, -39.8888851700048, 1.72503667797818e-5, 2.36415887840135e-6},
        {0.0, 25.0, 0.0, 1502975453.26277, 1502975453.26277, 1.5707963267949, -0.00684859717970259, 3.10227887779486, 0.00684859717970259, -0.0393137757949353},
        {0.0, 25.0, 0.0, 1502975453.26277, 1502975453.26277, 1.5707963267949, -0.00684859717970259, 3.10227887779486, 0.00684859717970259, -0.0393137757949353},
        {-20.22542485937375, 14.69463130731175, -19129.3494470458, 45406.0213041107, 45406.0213041213, 19130.9202433848, 5.85665949258649e-11, 3.14159265356458, -2432061.38760638, 25010638.0968068},
        {-25.0, 0.0, -1.53148255099996, 0.0, -0.00684859717970259, 3.14159265358979, -5.34889975534022e-13, 0.0, -3005950906.52555, -3.14159265358979},
        {-20.22542485937375, -14.69463130731175, -19129.3494470458, -45406.0213041107, 45406.0213041213, -19130.9202433848, 5.85665949258649e-11, -3.14159265356458, -2432061.38760638, -25010638.0968068},
        {0.0, -25.0, 0.0, -1502975453.26277, 1502975453.26277, -1.5707963267949, -0.00684859717970259, -3.10227887779486, 0.00684859717970259, 0.0393137757949353},
        {7.72542485937375, -23.77641290737875, 395787595.545024, -194501516.12134, 194501516.12134, 395787593.974227, -80.7948153607822, 39.8888851700048, 1.72503667797818e-5, -2.36415887840135e-6},
        {24.6922085148785, -3.91086162600575, 0.61973692887531, 0.318459426938049, -0.318931296543192, -0.950420524151913, -1816162045.63054, 1255955799.5082, -4.40593065675657e-13, -5.79490191675286e-13},
        {50.0, 0.0, 1.55161707248594, 0.0, -0.00562838632411631, 0.0, 1.05856368971317e+20, 0.0, 3.78326402955046e-24, 0.0},
        {35.3553390593275, 35.3553390593275, 53807668130.5995, -22948660925283.2, -22948660925283.2, -53807668129.0287, -45897321850566.4, 107615336261.199, -9.9766761181828e-21, 8.71502630154959e-18},
        {15.4508497187475, 47.5528258147575, 2.49903843573354e+18, -3.83240358282137e+18, -3.83240358282137e+18, -2.49903843573354e+18, -68343.3715391731, 77339.6040605891, 3.71621275609622e-10, 3.85406628982992e-9},
        {0.0, 50.0, 0.0, 5.29281844856585e+19, 5.29281844856585e+19, 1.5707963267949, -0.00562838632411631, 3.12241339928083, 0.00562838632411631, -0.0191792543089607},
        {0.0, 50.0, 0.0, 5.29281844856585e+19, 5.29281844856585e+19, 1.5707963267949, -0.00562838632411631, 3.12241339928083, 0.00562838632411631, -0.0191792543089607},
        {-40.4508497187475, 29.3892626146235, -57258797567.9644, -12906669326.6389, -12906669326.6389, 57258797569.5352, -8.55226617604501e-21, 3.14159265358979, 6.68228261723918e+15, -3.43017053184612e+15},
        {-50.0, 0.0, -1.55161707248594, 0.0, -0.00562838632411631, 3.14159265358979, -3.78326402955046e-24, 0.0, -1.05856368971317e+20, -3.14159265358979},
        {-40.4508497187475, -29.3892626146235, -57258797567.9644, 12906669326.6389, -12906669326.6389, -57258797569.5352, -8.55226617604501e-21, -3.14159265358979, 6.68228261723918e+15, 3.43017053184612e+15},
        {0.0, -50.0, 0.0, -5.29281844856585e+19, 5.29281844856585e+19, -1.5707963267949, -0.00562838632411631, -3.12241339928083, 0.00562838632411631, 0.0191792543089607},
        {15.4508497187475, -47.5528258147575, 2.49903843573354e+18, 3.83240358282137e+18, -3.83240358282137e+18, 2.49903843573354e+18, -68343.3715391731, -77339.6040605891, 3.71621275609622e-10, -3.85406628982992e-9},
        {49.384417029757, -7.8217232520115, -16.8292457944994, 16.9326906903424, -16.9326976506474, -18.4000381995002, 1.09489979806082e+19, -5.61228684199658e+19, -8.51344869310291e-25, 6.95142343223447e-24},
        {700.0, 0.0, 1.57199393223749, 0.0, 0.000778810012739756, 0.0, 1.45097873605256e+301, 0.0, 1.40651876623403e-307, 0.0},
        {494.974746830585, 494.974746830585, -5.39480977313549e+211, -3.7907051625115e+211, -3.7907051625115e+211, 5.39480977313549e+211, -7.58141032502299e+211, -1.0789619546271e+212, 1.26627531288803e-218, 8.89746644202181e-219},
        {216.311896062465, 665.7395614066049, 6.68861022474796e+285, -6.86204916856497e+285, -6.86204916856497e+285, -6.68861022474796e+285, 4.35129688126332e+89, -1.25283433405018e+91, 9.10599247691995e-98, -1.3494793845188e-97},
        {0.0, 700.0, 0.0, 7.2548936802628e+300, 7.2548936802628e+300, 1.5707963267949, 0.000778810012739756, 3.14279025903239, -0.000778810012739756, 0.00119760544259495},
        {0.0, 700.0, 0.0, 7.2548936802628e+300, 7.2548936802628e+300, 1.5707963267949, 0.000778810012739756, 3.14279025903239, -0.000778810012739756, 0.00119760544259495},
        {-566.311896062465, 411.449676604729, 4.13964135191794e+174, 3.47943069430311e+175, 3.47943069430311e+175, -4.13964135191794e+174, 1.39494929258574e-249, 3.14159265358979, 9.43022777090499e+242, 8.40743888884655e+242},
        {-700.0, 0.0, -1.57199393223749, 0.0, 0.000778810012739756, 3.14159265358979, -1.40651876623403e-307, 0.0, -1.45097873605256e+301, -3.14159265358979},
        {-566.311896062465, -411.449676604729, 4.13964135191794e+174, -3.47943069430311e+175, 3.47943069430311e+175, 4.13964135191794e+174, 1.39494929258574e-249, -3.14159265358979, 9.43022777090499e+242, -8.40743888884655e+242},
        {0.0, -700.0, 0.0, -7.2548936802628e+300, 7.2548936802628e+300, -1.5707963267949, 0.000778810012739756, -3.14279025903239, -0.000778810012739756, -0.00119760544259495},
        {216.311896062465, -665.7395614066049, 6.68861022474796e+285, 6.86204916856497e+285, -6.86204916856497e+285, 6.68861022474796e+285, 4.35129688126332e+89, 1.25283433405018e+91, 9.10599247691995e-98, 1.3494793845188e-97},
        {691.381838416598, -109.50412552816101, -2.38570018769502e+44, -9.72638025849046e+43, 9.72638025849046e+43, -2.38570018769502e+44, -2.15172979114587e+297, -1.50043260461905e+297, -7.44435180959991e-304, 2.26013762375079e-304}
    };
    const Real tol = 100*QL_EPSILON;

    for (const auto& i : data) {
        const Real x = i[0];
        const Real y = (std::abs(i[1]) < 1e-12) ? 0.0 : i[1];
        const std::complex<Real> z(x, y);

        const std::complex<Real> si = Si(z);
        std::complex<Real> ref(i[2], i[3]);
        Real diff = std::abs(si-ref)/std::abs(ref);
        if (diff > tol || std::isnan(diff)
            || (std::abs(ref.real()) < tol && std::abs(si.real()) > tol)
            || (std::abs(ref.imag()) < tol && std::abs(si.imag()) > tol)) {
            integrals_test::reportSiCiFail("Si", z, si, ref, diff, tol);
        }

        const std::complex<Real> ci = Ci(z);
        ref = std::complex<Real>(i[4], i[5]);
        diff = std::min(std::abs(ci-ref), std::abs(ci-ref)/std::abs(ref));
        if (diff > tol || std::isnan(diff)
            || (std::abs(ref.real()) < tol && std::abs(ci.real()) > tol)
            || (std::abs(ref.imag()) < tol && std::abs(ci.imag()) > tol)) {
            integrals_test::reportSiCiFail("Ci", z, ci, ref, diff, tol);
        }

        const std::complex<Real> ei = Ei(z);
        ref = std::complex<Real>(i[6], i[7]);
        diff = std::abs(ei-ref)/std::abs(ref);
        if (diff > tol || std::isnan(diff)
            || (std::abs(ref.real()) < tol && std::abs(ei.real()) > tol)
            || (std::abs(ref.imag()) < tol && std::abs(ei.imag()) > tol)) {
            integrals_test::reportSiCiFail("Ei", z, ei, ref, diff, tol);
        }

        const std::complex<Real> e1 = E1(z);
        ref = std::complex<Real>(i[8], i[9]);
        diff = std::abs(e1-ref)/std::abs(ref);
        if (diff > 10*tol || std::isnan(diff)
            || (std::abs(ref.real()) < tol && std::abs(e1.real()) > tol)
            || (std::abs(ref.imag()) < tol && std::abs(e1.imag()) > tol)) {
            integrals_test::reportSiCiFail("E1", z, e1, ref, diff, tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testRealSiCiIntegrals) {
    BOOST_TEST_MESSAGE("Testing real Ci and Si...");

    using namespace ExponentialIntegral;

    // reference values are calculated with Mathematica or Python/mpmath
    const Real data[][3] = {
            {1e-12, 1e-12, -27.0538054510270153677},
            {0.1, 0.09994446110827695570, -1.7278683866572965838},
            {1.0, 0.9460830703671830149, 0.3374039229009681347},
            {1.9999, 1.6053675097543679041, 0.4230016343635392},
            {3.9999, 1.758222058430840841, -0.140965355646150101},
            {4.0001, 1.758184218306157867, -0.140998037827177150},
            {5.0, 1.5499312449446741373, -0.19002974965664387862},
            {7.0, 1.4545966142480935906, 0.076695278482184518383,},
            {10.0, 1.6583475942188740493, -0.045456433004455372635},
            {15.0, 1.6181944437083687391, 0.046278677674360439604},
            {20.0, 1.5482417010434398402, 0.04441982084535331654},
            {24.9, 1.532210740207620024, -0.010788215638781789846},
            {25.1, 1.5311526281483412938, -0.0028719014454227088097},
            {30.0, 1.566756540030351111, -0.033032417282071143779},
            {40.0, 1.5869851193547845068, 0.019020007896208766962},
            {400.0, 1.5721148692738117518, -0.00212398883084634893},
            {4000.0, 1.5709788562309441985, -0.00017083030544201591130}
    };


    const Real tol = 1e-12;

    for (const auto& i : data) {
        Real x = i[0];
        Real si = Si(x);

        Real diff = std::fabs(si - i[1]);
        if (diff > tol) {
            integrals_test::reportSiCiFail("SineIntegral", x, si, i[1], diff, tol);
        }

        const Real ci = Ci(x);
        diff = std::fabs(ci - i[2]);
        if (diff > tol) {
            integrals_test::reportSiCiFail("CosineIntegral", x, ci, i[2], diff, tol);
        }

        x = -i[0];
        si = Si(x);
        diff = std::fabs(si + i[1]);
        if (diff > tol) {
            integrals_test::reportSiCiFail("SineIntegral", x, si, Real(-i[1]), diff, tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testExponentialIntegralLimits) {
    BOOST_TEST_MESSAGE("Testing limits for Ei...");

    using namespace ExponentialIntegral;

    const Real largeValue = 0.75*std::log(0.1*QL_MAX_REAL);

    const std::complex<Real> largeValuePosImag =
        Ei(std::complex<Real>(largeValue, std::numeric_limits<Real>::min()));

    const Real tol = 1000*QL_EPSILON;

    BOOST_CHECK_CLOSE(largeValuePosImag.imag(), M_PI, tol);

    BOOST_CHECK_CLOSE(
        largeValuePosImag.real(), std::exp(largeValue)/largeValue, 1e3/largeValue);

    const std::complex<Real> largeValueNegImag =
        Ei(std::complex<Real>(largeValue, -std::numeric_limits<Real>::min()));

    BOOST_CHECK_CLOSE(largeValueNegImag.imag(), -M_PI, tol);
    BOOST_CHECK_CLOSE(
        largeValueNegImag.real(), std::exp(largeValue)/largeValue, 1e3/largeValue);

    const std::complex<Real> largeValueZeroImag =
        Ei(std::complex<Real>(largeValue));
    BOOST_CHECK(largeValueZeroImag.imag() == Real(0.0));

    if (std::numeric_limits<Real>::has_infinity) {
        const std::complex<Real> ei_0 = Ei(std::complex<Real>(0.0));
        BOOST_CHECK(
            ei_0 == std::complex<Real>(-std::numeric_limits<Real>::infinity()));
    }

    const Real smallR = QL_EPSILON*QL_EPSILON;
    for (Integer x = -100; x < 100; ++x) {
        const Real phi = x/100.0 * M_PI;
        const std::complex<Real> z = std::polar(smallR, phi);
        const std::complex<Real> ei = Ei(z);

        // principal branch
        const std::complex<Real> limit_ei = M_EULER_MASCHERONI + std::log(z);

        BOOST_CHECK_CLOSE(ei.real(), limit_ei.real(), tol);
        BOOST_CHECK_CLOSE(ei.imag(), limit_ei.imag(), tol);
    }

    const Real largeR = largeValue;
    for (Integer x = -10; x < 10; ++x) {
        const Real phi = x/10.0 * M_PI;
        if (std::abs(phi) > 0.5*M_PI) {
            const std::complex<Real> z = std::polar(largeR, phi);
            const std::complex<Real> ei = Ei(z);

            const Real limit_ei_imag = boost::math::sign(z.imag())*M_PI;
            BOOST_CHECK(close_enough(ei.real(), 0.0));
            BOOST_CHECK_CLOSE(ei.imag(), limit_ei_imag, tol);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
