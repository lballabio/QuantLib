/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2014, 2015 Johannes Göttker-Schnetmann
  Copyright (C) 2014, 2015 Klaus Spanderen

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

#include "hestonslvmodel.hpp"
#include "utilities.hpp"

#include <iomanip>

#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/localconstantvol.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/schemes/douglasscheme.hpp>
#include <ql/methods/finitedifferences/schemes/hundsdorferscheme.hpp>
#include <ql/methods/finitedifferences/schemes/craigsneydscheme.hpp>
#include <ql/methods/finitedifferences/schemes/modifiedcraigsneydscheme.hpp>
#include <ql/methods/finitedifferences/schemes/impliciteulerscheme.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmmesherintegral.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/experimental/models/hestonslvmodel.hpp>
#include <ql/experimental/finitedifferences/fdmhestonfwdop.hpp>
#include <ql/experimental/finitedifferences/fdmsquarerootfwdop.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesfwdop.hpp>
#include <ql/experimental/finitedifferences/fdmlocalvolfwdop.hpp>
#include <ql/experimental/finitedifferences/fdmhestongreensfct.hpp>
#include <ql/experimental/finitedifferences/squarerootprocessrndcalculator.hpp>
#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>

#include <boost/assign/std/vector.hpp>
#include <boost/math/special_functions/gamma.hpp>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <iostream>

using namespace QuantLib;
using namespace boost::assign;
using boost::unit_test_framework::test_suite;

namespace {
    Real fokkerPlanckPrice1D(const boost::shared_ptr<FdmMesher>& mesher,
                             const boost::shared_ptr<FdmLinearOpComposite>& op,
                             const boost::shared_ptr<StrikedTypePayoff>& payoff,
                             Real x0, Time maturity, Size tGrid) {

        const Array x = mesher->locations(0);
        Array p(x.size(), 0.0);

        QL_REQUIRE(x.size() > 3 && x[1] <= x0 && x[x.size()-2] >= x0,
                   "insufficient mesher");

        const Array::const_iterator upperb
            = std::upper_bound(x.begin(), x.end(), x0);
        const Array::const_iterator lowerb = upperb-1;

        if (close_enough(*upperb, x0)) {
            const Size idx = std::distance(x.begin(), upperb);
            const Real dx = (x[idx+1]-x[idx-1])/2.0;
            p[idx] = 1.0/dx;
        }
        else if (close_enough(*lowerb, x0)) {
            const Size idx = std::distance(x.begin(), lowerb);
            const Real dx = (x[idx+1]-x[idx-1])/2.0;
            p[idx] = 1.0/dx;
        } else {
            const Real dx = *upperb - *lowerb;
            const Real lowerP = (*upperb - x0)/dx;
            const Real upperP = (x0 - *lowerb)/dx;

            const Size lowerIdx = std::distance(x.begin(), lowerb);
            const Size upperIdx = std::distance(x.begin(), upperb);

            const Real lowerDx = (x[lowerIdx+1]-x[lowerIdx-1])/2.0;
            const Real upperDx = (x[upperIdx+1]-x[upperIdx-1])/2.0;

            p[lowerIdx] = lowerP/lowerDx;
            p[upperIdx] = upperP/upperDx;
        }

        DouglasScheme evolver(FdmSchemeDesc::Douglas().theta, op);
        const Time dt = maturity/tGrid;
        evolver.setStep(dt);

        for (Time t=dt; t <= maturity+20*QL_EPSILON; t+=dt) {
            evolver.step(p, t);
        }

        Array payoffTimesDensity(x.size());
        for (Size i=0; i < x.size(); ++i) {
            payoffTimesDensity[i] = payoff->operator()(std::exp(x[i]))*p[i];
        }

        CubicNaturalSpline f(x.begin(), x.end(), payoffTimesDensity.begin());
        f.enableExtrapolation();
        return GaussLobattoIntegral(1000, 1e-6)(f, x.front(), x.back());
    }
}

void HestonSLVModelTest::testBlackScholesFokkerPlanckFwdEquation() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation for BS process...");

    SavedSettings backup;

    const DayCounter dc = ActualActual();
    const Date todaysDate = Date(28, Dec, 2012);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturityDate = todaysDate + Period(2, Years);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = 100;
    const Real x0 = std::log(s0);
    const Rate r = 0.035;
    const Rate q = 0.01;
    const Volatility v = 0.35;

    const Size xGrid = 2*100+1;
    const Size tGrid = 400;

    const Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(s0)));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));
    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<BlackVolTermStructure> vTS(flatVol(v, dc));

    const boost::shared_ptr<GeneralizedBlackScholesProcess> process(
        new GeneralizedBlackScholesProcess(spot, qTS, rTS, vTS));

    const boost::shared_ptr<PricingEngine> engine(
        new AnalyticEuropeanEngine(process));

    const boost::shared_ptr<FdmMesher> uniformMesher(
        new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
            new FdmBlackScholesMesher(xGrid, process, maturity, s0))));

    const boost::shared_ptr<FdmLinearOpComposite> uniformBSFwdOp(
        new FdmBlackScholesFwdOp(uniformMesher, process, s0, 0));

    const boost::shared_ptr<FdmMesher> concentratedMesher(
        new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
            new FdmBlackScholesMesher(xGrid, process, maturity, s0,
                                      Null<Real>(), Null<Real>(), 0.0001, 1.5,
                                      std::pair<Real, Real>(s0, 0.1)))));

    const boost::shared_ptr<FdmLinearOpComposite> concentratedBSFwdOp(
        new FdmBlackScholesFwdOp(concentratedMesher, process, s0, 0));

    const boost::shared_ptr<FdmMesher> shiftedMesher(
        new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
            new FdmBlackScholesMesher(xGrid, process, maturity, s0,
                                      Null<Real>(), Null<Real>(), 0.0001, 1.5,
                                      std::pair<Real, Real>(s0*1.1, 0.2)))));

    const boost::shared_ptr<FdmLinearOpComposite> shiftedBSFwdOp(
        new FdmBlackScholesFwdOp(shiftedMesher, process, s0, 0));

    const boost::shared_ptr<Exercise> exercise(
        new EuropeanExercise(maturityDate));
    const Real strikes[] = { 50, 80, 100, 130, 150 };

    for (Size i=0; i < LENGTH(strikes); ++i) {
        const boost::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(Option::Call, strikes[i]));

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        const Real expected = option.NPV()/rTS->discount(maturityDate);
        const Real calcUniform
            = fokkerPlanckPrice1D(uniformMesher, uniformBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real calcConcentrated
            = fokkerPlanckPrice1D(concentratedMesher, concentratedBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real calcShifted
            = fokkerPlanckPrice1D(shiftedMesher, shiftedBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real tol = 0.02;

        if (std::fabs(expected - calcUniform) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with an uniform mesher"
                       << "\n   strike:     " << strikes[i]
                       << QL_FIXED << std::setprecision(8)
                       << "\n   calculated: " << calcUniform
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
        if (std::fabs(expected - calcConcentrated) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with a concentrated mesher"
                       << "\n   strike:     " << strikes[i]
                       << QL_FIXED << std::setprecision(8)
                       << "\n   calculated: " << calcConcentrated
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
        if (std::fabs(expected - calcShifted) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with a shifted mesher"
                       << "\n   strike:     " << strikes[i]
                       << QL_FIXED << std::setprecision(8)
                       << "\n   calculated: " << calcShifted
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
    }
}


namespace {
    Real stationaryLogProbabilityFct(Real kappa, Real theta,
                                   Real sigma, Real z) {
        const Real alpha = 2*kappa*theta/(sigma*sigma);
        const Real beta = alpha/theta;

        return std::pow(beta, alpha)*std::exp(z*alpha)
                *std::exp(-beta*std::exp(z)-GammaFunction().logValue(alpha));
    }
}

void HestonSLVModelTest::testSquareRootZeroFlowBC() {
    BOOST_TEST_MESSAGE("Testing zero-flow BC for the square root process...");

    SavedSettings backup;

    const Real kappa = 1.0;
    const Real theta = 0.4;
    const Real sigma = 0.8;
    const Real v_0   = 0.1;
    const Time t     = 1.0;

    const Real vmin = 0.0005;
    const Real h    = 0.0001;

    const Real expected[5][5]
        = {{ 0.000548, -0.000245, -0.005657, -0.001167, -0.000024},
           {-0.000595, -0.000701, -0.003296, -0.000883, -0.000691},
           {-0.001277, -0.001320, -0.003128, -0.001399, -0.001318},
           {-0.001979, -0.002002, -0.003425, -0.002047, -0.002001},
           {-0.002715, -0.002730, -0.003920, -0.002760, -0.002730} };

    for (Size i=0; i < 5; ++i) {
        const Real v = vmin + i*0.001;
        const Real vm2 = v - 2*h;
        const Real vm1 = v - h;
        const Real v0  = v;
        const Real v1  = v + h;
        const Real v2  = v + 2*h;

        const SquareRootProcessRNDCalculator rndCalculator(
        	v_0, kappa, theta, sigma);

        const Real pm2 = rndCalculator.pdf(vm2, t);
        const Real pm1 = rndCalculator.pdf(vm1, t);
        const Real p0  = rndCalculator.pdf(v0 , t);
        const Real p1  = rndCalculator.pdf(v1 , t);
        const Real p2  = rndCalculator.pdf(v2 , t);

        // test derivatives
        const Real flowSym2Order = sigma*sigma*v0/(4*h)*(p1-pm1)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real flowSym4Order
            = sigma*sigma*v0/(24*h)*(-p2 + 8*p1 - 8*pm1 + pm2)
              + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd1Order = sigma*sigma*v0/(2*h)*(p1-p0)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd2Order = sigma*sigma*v0/(4*h)*(4*p1-3*p0-p2)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd3Order
            = sigma*sigma*v0/(12*h)*(-p2 + 6*p1 - 3*p0 - 2*pm1)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real tol = 0.000002;
        if (   std::fabs(expected[i][0] - flowSym2Order) > tol
            || std::fabs(expected[i][1] - flowSym4Order) > tol
            || std::fabs(expected[i][2] - fwd1Order) > tol
            || std::fabs(expected[i][3] - fwd2Order) > tol
            || std::fabs(expected[i][4] - fwd3Order) > tol ) {
            BOOST_ERROR("failed to reproduce Zero Flow BC at"
                       << "\n   v:          " << v
                       << "\n   tolerance:  " << tol);
        }
    }
}


namespace {
    boost::shared_ptr<FdmMesher> createStationaryDistributionMesher(
        Real kappa, Real theta, Real sigma, Size vGrid) {

        const Real qMin = 0.01;
        const Real qMax = 0.99;
        const Real dq = (qMax-qMin)/(vGrid-1);

        const SquareRootProcessRNDCalculator rnd(theta, kappa, theta, sigma);
        std::vector<Real> v(vGrid);
        for (Size i=0; i < vGrid; ++i) {
            v[i] = rnd.stationary_invcdf(qMin + i*dq);
        }

        return boost::shared_ptr<FdmMesher>(
            new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
                new Predefined1dMesher(v))));
    }
}


void HestonSLVModelTest::testTransformedZeroFlowBC() {
    BOOST_TEST_MESSAGE("Testing zero-flow BC for transformed "
                       "Fokker-Planck forward equation...");

    SavedSettings backup;

    const Real kappa = 1.0;
    const Real theta = 0.4;
    const Real sigma = 2.0;
    const Size vGrid = 100;

    const boost::shared_ptr<FdmMesher> mesher
        = createStationaryDistributionMesher(kappa, theta, sigma, vGrid);
    const Array v = mesher->locations(0);

    Array p(vGrid);
    const SquareRootProcessRNDCalculator rnd(theta, kappa, theta, sigma);
    for (Size i=0; i < v.size(); ++i)
        p[i] =  rnd.stationary_pdf(v[i]);


    const Real alpha = 1.0 - 2*kappa*theta/(sigma*sigma);
    const Array q = Pow(v, alpha)*p;

    for (Size i=0; i < vGrid/2; ++i) {
        const Real hm = v[i+1] - v[i];
        const Real hp = v[i+2] - v[i+1];

        const Real eta=1.0/(hm*(hm+hp)*hp);
        const Real a = -eta*(square<Real>()(hm+hp) - hm*hm);
        const Real b  = eta*square<Real>()(hm+hp);
        const Real c = -eta*hm*hm;

        const Real df = a*q[i] + b*q[i+1] + c*q[i+2];
        const Real flow = 0.5*sigma*sigma*v[i]*df + kappa*v[i]*q[i];

        const Real tol = 1e-6;
        if (std::fabs(flow) > tol) {
            BOOST_ERROR("failed to reproduce Zero Flow BC at"
                       << "\n v:          " << v
                       << "\n flow:       " << flow
                       << "\n tolerance:  " << tol);
        }
    }
}

namespace {
    class q_fct : public std::unary_function<Real, Real> {
      public:
        q_fct(const Array& v, const Array& p, const Real alpha)
        : v_(v), q_(Pow(v, alpha)*p), alpha_(alpha) {
            spline_ = boost::shared_ptr<CubicInterpolation>(
                new CubicNaturalSpline(v_.begin(), v_.end(), q_.begin()));
        }

        Real operator()(Real v) {
            return (*spline_)(v, true)*std::pow(v, -alpha_);
        }
      private:

        const Array v_, q_;
        const Real alpha_;
        boost::shared_ptr<CubicInterpolation> spline_;
    };
}

void HestonSLVModelTest::testSquareRootEvolveWithStationaryDensity() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the square root process with stationary density...");

    // Documentation for this test case:
    // http://www.spanderen.de/2013/05/04/fokker-planck-equation-feller-constraint-and-boundary-conditions/
    SavedSettings backup;

    const Real kappa = 2.5;
    const Real theta = 0.2;
    const Size vGrid = 100;
    const Real eps = 1e-2;

    for (Real sigma = 0.2; sigma < 2.01; sigma+=0.1) {
        const Real alpha = (1.0 - 2*kappa*theta/(sigma*sigma));

        const SquareRootProcessRNDCalculator rnd(theta, kappa, theta, sigma);
        const Real vMin = rnd.stationary_invcdf(eps);
        const Real vMax = rnd.stationary_invcdf(1-eps);

        const boost::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
                    new Uniform1dMesher(vMin, vMax, vGrid))));

        const Array v = mesher->locations(0);
        const FdmSquareRootFwdOp::TransformationType transform =
            (sigma < 0.75) ?
               FdmSquareRootFwdOp::Plain :
               FdmSquareRootFwdOp::Power;

        Array vq (v.size());
        Array vmq(v.size());
        for (Size i=0; i < v.size(); ++i) {
            vmq[i] = 1.0/(vq[i] = std::pow(v[i], alpha));
        }

        Array p(vGrid);
        for (Size i=0; i < v.size(); ++i) {
            p[i] =  rnd.stationary_pdf(v[i]);
            if (transform == FdmSquareRootFwdOp::Power)
                p[i] *= vq[i];
        }

        const boost::shared_ptr<FdmSquareRootFwdOp> op(
            new FdmSquareRootFwdOp(mesher, kappa, theta,
                                   sigma, 0, transform));

        const Array eP = p;

        const Size n = 100;
        const Time dt = 0.01;
        DouglasScheme evolver(0.5, op);
        evolver.setStep(dt);

        for (Size i=1; i <= n; ++i) {
            evolver.step(p, i*dt);
        }

        const Real expected = 1-2*eps;

        if (transform == FdmSquareRootFwdOp::Power)
            for (Size i=0; i < v.size(); ++i) {
                p[i] *= vmq[i];
            }

        const Real calculated = GaussLobattoIntegral(1000000, 1e-6)(
                                        q_fct(v,p,alpha), v.front(), v.back());

        const Real tol = 0.005;
        if (std::fabs(calculated-expected) > tol) {
            BOOST_ERROR("failed to reproduce stationary probability function"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    tolerance:  " << tol);
        }
    }
}

void HestonSLVModelTest::testSquareRootLogEvolveWithStationaryDensity() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the square root log process with stationary density...");

    // Documentation for this test case:
    // nowhere yet :)
    SavedSettings backup;

    const Real kappa = 2.5;
    const Real theta = 0.2;
    const Size vGrid = 1000;
    const Real eps = 1e-2;

    for (Real sigma = 0.2; sigma < 2.01; sigma+=0.1) {
        //BOOST_TEST_MESSAGE("testing log process sigma =  " << sigma << "\n");
        const Real lowerLimit = 0.001;
        // should not go to very large negative values, distributions flattens with sigma
        // causing numerical instabilities log/exp evaluations

        const SquareRootProcessRNDCalculator rnd(theta, kappa, theta, sigma);

        const Real vMin = std::max(lowerLimit, rnd.stationary_invcdf(eps));
        const Real lowEps = std::max(eps, rnd.stationary_cdf(lowerLimit));

        const Real expected = 1-eps-lowEps;
        const Real vMax = rnd.stationary_invcdf(1-eps);

        std::vector<Real> critialPoints;
        std::vector<boost::tuple<Real, Real, bool> > critPoints;
        critPoints.push_back(boost::tuple<Real, Real, bool>(log(vMin), 0.001, false));

        const boost::shared_ptr<FdmMesherComposite> mesher(
            new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
                new Uniform1dMesher(log(vMin), log(vMax), vGrid))));

        const Array v = mesher->locations(0);

        Array p(vGrid);
        for (Size i=0; i < v.size(); ++i)
            p[i] =  stationaryLogProbabilityFct(kappa, theta, sigma, v[i]);

        const boost::shared_ptr<FdmSquareRootFwdOp> op(
            new FdmSquareRootFwdOp(mesher, kappa, theta,
                                   sigma, 0,
                                   FdmSquareRootFwdOp::Log));

        const Size n = 100;
        const Time dt = 0.01;
        FdmBoundaryConditionSet bcSet;
        DouglasScheme evolver(0.5, op);
        evolver.setStep(dt);

        for (Size i=1; i <= n; ++i) {
            evolver.step(p, i*dt);
        }

        const Real calculated
            = FdmMesherIntegral(mesher, DiscreteSimpsonIntegral()).integrate(p);

        const Real tol = 0.005;
        if (std::fabs(calculated-expected) > tol) {
            BOOST_ERROR("failed to reproduce stationary probability function for "
                    << "\n    sigma:      " << sigma
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    tolerance:  " << tol);
        }
    }
}

void HestonSLVModelTest::testSquareRootFokkerPlanckFwdEquation() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the square root process with Dirac start...");

    SavedSettings backup;

    const Real kappa = 1.2;
    const Real theta = 0.4;
    const Real sigma = 0.7;
    const Real v0 = theta;
    const Real alpha = 1.0 - 2*kappa*theta/(sigma*sigma);

    const Time maturity = 1.0;

    const Size xGrid = 1001;
    const Size tGrid = 500;

    const Real vol = sigma*std::sqrt(theta/(2*kappa));
    const Real upperBound = theta+6*vol;
    const Real lowerBound = std::max(0.0002, theta-6*vol);

    const boost::shared_ptr<FdmMesher> mesher(
        new FdmMesherComposite(boost::shared_ptr<Fdm1dMesher>(
                new Uniform1dMesher(lowerBound, upperBound, xGrid))));

    const Array x(mesher->locations(0));

    const boost::shared_ptr<FdmSquareRootFwdOp> op(
        new FdmSquareRootFwdOp(mesher, kappa, theta, sigma, 0)); //!

    const Time dt = maturity/tGrid;
    const Size n = 5;

    Array p(xGrid);
    SquareRootProcessRNDCalculator rndCalculator(v0, kappa, theta, sigma);
    for (Size i=0; i < p.size(); ++i) {
        p[i] = rndCalculator.pdf(x[i], n*dt);
    }
    Array q = Pow(x, alpha)*p;

    DouglasScheme evolver(0.5, op);
    evolver.setStep(dt);

    for (Time t=(n+1)*dt; t <= maturity+20*QL_EPSILON; t+=dt) {
        evolver.step(p, t);
        evolver.step(q, t);
    }

    const Real tol = 0.002;

    Array y(x.size());
    for (Size i=0; i < x.size(); ++i) {
        const Real expected = rndCalculator.pdf(x[i], maturity);

        const Real calculated = p[i];
        if (std::fabs(expected - calculated) > tol) {
            BOOST_FAIL("failed to reproduce pdf at"
                       << QL_FIXED << std::setprecision(5)
                       << "\n   x:          " << x[i]
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
    }
}



namespace {
    Real fokkerPlanckPrice2D(const Array& p,
                       const boost::shared_ptr<FdmMesherComposite>& mesher) {

        std::vector<Real> x, y;
        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();

        x.reserve(layout->dim()[0]);
        y.reserve(layout->dim()[1]);

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
              ++iter) {
            if (!iter.coordinates()[1]) {
                x.push_back(mesher->location(iter, 0));
            }
            if (!iter.coordinates()[0]) {
                y.push_back(mesher->location(iter, 1));
            }
        }

        return FdmMesherIntegral(mesher,
                                 DiscreteSimpsonIntegral()).integrate(p);
    }

    Real hestonPxBoundary(
        Time maturity, Real eps,
        const boost::shared_ptr<HestonModel>& model) {

        const AnalyticPDFHestonEngine pdfEngine(model);
        const Real sInit = model->process()->s0()->value();
        const Real xMin = Brent().solve(
            boost::bind(std::minus<Real>(),
                boost::bind(&AnalyticPDFHestonEngine::cdf,
                            &pdfEngine, _1, maturity), eps),
                        sInit*1e-3, sInit, sInit*0.001, 1000*sInit);

        return xMin;
    }

    struct FokkerPlanckFwdTestCase {
        const Real s0, r, q, v0, kappa, theta, rho, sigma;
        const Size xGrid, vGrid, tGridPerYear, tMinGridPerYear;
        const Real avgEps, eps;
        const FdmSquareRootFwdOp::TransformationType trafoType;
        const FdmHestonGreensFct::Algorithm greensAlgorithm;
        const FdmSchemeDesc::FdmSchemeType schemeType;
    };

    void hestonFokkerPlanckFwdEquationTest(
        const FokkerPlanckFwdTestCase& testCase) {

        SavedSettings backup;

        const DayCounter dc = ActualActual();
        const Date todaysDate = Date(28, Dec, 2014);
        Settings::instance().evaluationDate() = todaysDate;

        std::vector<Period> maturities;
        maturities+=Period(1, Months),
                    Period(3, Months), Period(6, Months), Period(9, Months),
                    Period(1, Years), Period(2, Years), Period(3, Years);

        const Date maturityDate = todaysDate + maturities.back();
        const Time maturity = dc.yearFraction(todaysDate, maturityDate);

        const Real s0 = testCase.s0;
        const Real x0 = std::log(s0);
        const Rate r = testCase.r;
        const Rate q = testCase.q;

        const Real kappa = testCase.kappa;
        const Real theta = testCase.theta;
        const Real rho   = testCase.rho;
        const Real sigma = testCase.sigma;
        const Real v0    = testCase.v0;
        const Real alpha = 1.0 - 2*kappa*theta/(sigma*sigma);

        const Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(s0)));
        const Handle<YieldTermStructure> rTS(flatRate(r, dc));
        const Handle<YieldTermStructure> qTS(flatRate(q, dc));

        const boost::shared_ptr<HestonProcess> process(
            new HestonProcess(rTS, qTS, spot, v0, kappa, theta, sigma, rho));

        const boost::shared_ptr<HestonModel> model(new HestonModel(process));

        const boost::shared_ptr<PricingEngine> engine(
            new AnalyticHestonEngine(model));

        const Size xGrid = testCase.xGrid;
        const Size vGrid = testCase.vGrid;
        const Size tGridPerYear = testCase.tGridPerYear;

        const FdmSquareRootFwdOp::TransformationType transformationType
            = testCase.trafoType;
        Real lowerBound, upperBound;
        std::vector<boost::tuple<Real, Real, bool> > cPoints;

        const SquareRootProcessRNDCalculator rnd(v0, kappa, theta, sigma);
        switch (transformationType) {
        case FdmSquareRootFwdOp::Log:
          {
            upperBound = std::log(rnd.stationary_invcdf(0.9995));
            lowerBound = std::log(0.00001);

            const Real v0Center = std::log(v0);
            const Real v0Density = 10.0;
            const Real upperBoundDensity = 100;
            const Real lowerBoundDensity = 1.0;
            cPoints += boost::make_tuple(lowerBound, lowerBoundDensity, false),
                       boost::make_tuple(v0Center, v0Density, true),
                       boost::make_tuple(upperBound, upperBoundDensity, false);
          }
        break;
        case FdmSquareRootFwdOp::Plain:
          {
            upperBound = rnd.stationary_invcdf(0.9995);
            lowerBound = rnd.stationary_invcdf(1e-5);

            const Real v0Center = v0;
            const Real v0Density = 0.1;
            const Real lowerBoundDensity = 0.0001;
            cPoints += boost::make_tuple(lowerBound, lowerBoundDensity, false),
                       boost::make_tuple(v0Center, v0Density, true);
          }
        break;
        case FdmSquareRootFwdOp::Power:
          {
            upperBound = rnd.stationary_invcdf(0.9995);
            lowerBound = 0.000075;

            const Real v0Center = v0;
            const Real v0Density = 1.0;
            const Real lowerBoundDensity = 0.005;
            cPoints += boost::make_tuple(lowerBound, lowerBoundDensity, false),
                       boost::make_tuple(v0Center, v0Density, true);
          }
        break;
        default:
            QL_FAIL("unknown transformation type");
        }

        const boost::shared_ptr<Fdm1dMesher> varianceMesher(
            new Concentrating1dMesher(lowerBound, upperBound,
                                      vGrid, cPoints, 1e-12));

        const Real sEps = 1e-4;
        const Real sLowerBound
            = std::log(hestonPxBoundary(maturity, sEps, model));
        const Real sUpperBound
            = std::log(hestonPxBoundary(maturity, 1-sEps,model));

        const boost::shared_ptr<Fdm1dMesher> spotMesher(
            new Concentrating1dMesher(sLowerBound, sUpperBound, xGrid,
                std::make_pair(x0, 0.1), true));

        const boost::shared_ptr<FdmMesherComposite>
            mesher(new FdmMesherComposite(spotMesher, varianceMesher));

        const boost::shared_ptr<FdmLinearOpComposite> hestonFwdOp(
            new FdmHestonFwdOp(mesher, process, transformationType));

        ModifiedCraigSneydScheme evolver(
        	FdmSchemeDesc::ModifiedCraigSneyd().theta,
        	FdmSchemeDesc::ModifiedCraigSneyd().mu, hestonFwdOp);

        // step one days using non-correlated process
        const Time eT = 1.0/365;
        Array p = FdmHestonGreensFct(mesher, process, testCase.trafoType)
                .get(eT, testCase.greensAlgorithm);

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const Real strikes[] = { 50, 80, 90, 100, 110, 120, 150, 200 };

        Time t=eT;
        for (std::vector<Period>::const_iterator iter = maturities.begin();
                iter != maturities.end(); ++iter) {

            // calculate step size
            const Date nextMaturityDate = todaysDate + *iter;
            const Time nextMaturityTime
                = dc.yearFraction(todaysDate, nextMaturityDate);

            Time dt = (nextMaturityTime - t)/tGridPerYear;
            evolver.setStep(dt);

            for (Size i=0; i < tGridPerYear; ++i, t+=dt) {
                evolver.step(p, t+dt);
            }

            Real avg=0, min=QL_MAX_REAL, max=0;
            for (Size i=0; i < LENGTH(strikes); ++i) {
                const Real strike = strikes[i];
                const boost::shared_ptr<StrikedTypePayoff> payoff(
                    new PlainVanillaPayoff((strike > s0) ? Option::Call :
                                                           Option::Put, strike));

                Array pd(p.size());
                for (FdmLinearOpIterator iter = layout->begin();
                    iter != layout->end(); ++iter) {
                    const Size idx = iter.index();
                    const Real s = std::exp(mesher->location(iter, 0));

                    pd[idx] = payoff->operator()(s)*p[idx];
                    if (transformationType == FdmSquareRootFwdOp::Power) {
                        const Real v = mesher->location(iter, 1);
                        pd[idx] *= std::pow(v, -alpha);
                    }
                }

                const Real calculated = fokkerPlanckPrice2D(pd, mesher)
                    * rTS->discount(nextMaturityDate);

                const boost::shared_ptr<Exercise> exercise(
                    new EuropeanExercise(nextMaturityDate));

                VanillaOption option(payoff, exercise);
                option.setPricingEngine(engine);

                const Real expected = option.NPV();
                const Real absDiff = std::fabs(expected - calculated);
                const Real relDiff = absDiff / std::max(QL_EPSILON, expected);
                const Real diff = std::min(absDiff, relDiff);

                avg += diff;
                min = std::min(diff, min);
                max = std::max(diff, max);

                if (diff > testCase.eps) {
                    BOOST_FAIL("failed to reproduce Heston SLV prices at"
                              << "\n   strike      " << strike
                              << "\n   kappa       " << kappa
                              << "\n   theta       " << theta
                              << "\n   rho         " << rho
                              << "\n   sigma       " << sigma
                              << "\n   v0          " << v0
                              << "\n   transform   " << transformationType
                              << QL_FIXED << std::setprecision(5)
                              << "\n   calculated: " << calculated
                              << "\n   expected:   " << expected
                              << "\n   tolerance:  " << testCase.eps);
                }
            }

            avg/=LENGTH(strikes);

            if (avg > testCase.avgEps) {
            	BOOST_FAIL("failed to reproduce Heston SLV prices"
            			   " on average at"
                        << "\n   kappa       " << kappa
                        << "\n   theta       " << theta
                        << "\n   rho         " << rho
                        << "\n   sigma       " << sigma
                        << "\n   v0          " << v0
                        << "\n   transform   " << transformationType
                        << QL_FIXED << std::setprecision(5)
                        << "\n   average diff: " << avg
                        << "\n   tolerance:  " << testCase.avgEps);
            }
        }
    }
}

void HestonSLVModelTest::testHestonFokkerPlanckFwdEquation() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the Heston process...");

    FokkerPlanckFwdTestCase testCases[] = {
        {
            100.0, 0.01, 0.02,
            // Feller constraint violated, high vol-of-vol case
            // \frac{2\kappa\theta}{\sigma^2} = 2.0 * 1.0 * 0.05 / 0.2 = 0.5 < 1
            0.05, 1.0, 0.05, -0.75, std::sqrt(0.2),
            101, 401, 25, 25,
			0.02, 0.05,
            FdmSquareRootFwdOp::Power,
            FdmHestonGreensFct::Gaussian,
            FdmSchemeDesc::DouglasType
        },
        {
            100.0, 0.01, 0.02,
            // Feller constraint violated, high vol-of-vol case
            // \frac{2\kappa\theta}{\sigma^2} = 2.0 * 1.0 * 0.05 / 0.2 = 0.5 < 1
            0.05, 1.0, 0.05, -0.75, std::sqrt(0.2),
            201, 501, 10, 10,
			0.005, 0.02,
            FdmSquareRootFwdOp::Log,
            FdmHestonGreensFct::Gaussian,
            FdmSchemeDesc::HundsdorferType
        },
	    {
			100.0, 0.01, 0.02,
            // Feller constraint violated, high vol-of-vol case
            // \frac{2\kappa\theta}{\sigma^2} = 2.0 * 1.0 * 0.05 / 0.2 = 0.5 < 1
			0.05, 1.0, 0.05, -0.75, std::sqrt(0.2),
			201, 501, 25, 25,
			0.01, 0.03,
			FdmSquareRootFwdOp::Log,
			FdmHestonGreensFct::ZeroCorrelation,
			FdmSchemeDesc::HundsdorferType
	    },
        {
            100.0, 0.01, 0.02,
            // Feller constraint fulfilled, low vol-of-vol case
            // \frac{2\kappa\theta}{\sigma^2} = 2.0 * 1.0 * 0.05 / 0.05 = 2.0 > 1
            0.05, 1.0, 0.05, -0.75, std::sqrt(0.05),
            401, 501, 5, 5,
			0.01, 0.02,
            FdmSquareRootFwdOp::Plain,
            FdmHestonGreensFct::Gaussian,
            FdmSchemeDesc::HundsdorferType
        }
    };

    for (Size i=0; i < LENGTH(testCases); ++i) {
        hestonFokkerPlanckFwdEquationTest(testCases[i]);
    }
}


namespace {
    boost::shared_ptr<Matrix> createLocalVolMatrixFromProcess(
        boost::shared_ptr<BlackScholesMertonProcess> lvProcess,
        const std::vector<Real>& strikes,
        const std::vector<Date>& dates,
        std::vector<Time>& times) {

        const boost::shared_ptr<LocalVolTermStructure> localVol =
            lvProcess->localVolatility().currentLink();

        const DayCounter dc = localVol->dayCounter();
        const Date todaysDate = Settings::instance().evaluationDate();

        QL_REQUIRE(times.size() == dates.size(), "mismatch");

        for (Size i=0; i < times.size(); ++i) {
            times[i] = dc.yearFraction(todaysDate, dates[i]);
        }

        boost::shared_ptr<Matrix> surface(
            new Matrix(strikes.size(), dates.size()));

        for (Size i=0; i < strikes.size(); ++i) {
            for (Size j=0; j < dates.size(); ++j) {
                try {
                    (*surface)[i][j] = localVol->localVol(dates[j], strikes[i], true);
                } catch (Error&) {
                    (*surface)[i][j] = 0.2;
                }
            }
        }

        return surface;
    }

    boost::tuple<std::vector<Real>, std::vector<Date>,
                 boost::shared_ptr<BlackVarianceSurface> >
        createSmoothImpliedVol(const DayCounter& dc, const Calendar& cal) {

        const Date todaysDate = Settings::instance().evaluationDate();

        Integer times[] = { 13, 41, 75, 165, 256, 345, 524, 703 };
        std::vector<Date> dates;
        for (Size i = 0; i < 8; ++i) {
            Date date = todaysDate + times[i];
            dates.push_back(date);
        }

        Real tmp[] = { 2.222222222, 11.11111111, 44.44444444, 75.55555556, 80, 84.44444444, 88.88888889, 93.33333333, 97.77777778, 100,
                       102.2222222, 106.6666667, 111.1111111, 115.5555556, 120, 124.4444444, 166.6666667, 222.2222222, 444.4444444, 666.6666667
             };
        const std::vector<Real> surfaceStrikes(tmp, tmp+LENGTH(tmp));

        Volatility v[] =
          { 1.015873, 1.015873, 1.015873, 0.89729, 0.796493, 0.730914, 0.631335, 0.568895,
            0.711309, 0.711309, 0.711309, 0.641309, 0.635593, 0.583653, 0.508045, 0.463182,
            0.516034, 0.500534, 0.500534, 0.500534, 0.448706, 0.416661, 0.375470, 0.353442,
            0.516034, 0.482263, 0.447713, 0.387703, 0.355064, 0.337438, 0.316966, 0.306859,
            0.497587, 0.464373, 0.430764, 0.374052, 0.344336, 0.328607, 0.310619, 0.301865,
            0.479511, 0.446815, 0.414194, 0.361010, 0.334204, 0.320301, 0.304664, 0.297180,
            0.461866, 0.429645, 0.398092, 0.348638, 0.324680, 0.312512, 0.299082, 0.292785,
            0.444801, 0.413014, 0.382634, 0.337026, 0.315788, 0.305239, 0.293855, 0.288660,
            0.428604, 0.397219, 0.368109, 0.326282, 0.307555, 0.298483, 0.288972, 0.284791,
            0.420971, 0.389782, 0.361317, 0.321274, 0.303697, 0.295302, 0.286655, 0.282948,
            0.413749, 0.382754, 0.354917, 0.316532, 0.300016, 0.292251, 0.284420, 0.281164,
            0.400889, 0.370272, 0.343525, 0.307904, 0.293204, 0.286549, 0.280189, 0.277767,
            0.390685, 0.360399, 0.334344, 0.300507, 0.287149, 0.281380, 0.276271, 0.274588,
            0.383477, 0.353434, 0.327580, 0.294408, 0.281867, 0.276746, 0.272655, 0.271617,
            0.379106, 0.349214, 0.323160, 0.289618, 0.277362, 0.272641, 0.269332, 0.268846,
            0.377073, 0.347258, 0.320776, 0.286077, 0.273617, 0.269057, 0.266293, 0.266265,
            0.399925, 0.369232, 0.338895, 0.289042, 0.265509, 0.255589, 0.249308, 0.249665,
            0.423432, 0.406891, 0.373720, 0.314667, 0.281009, 0.263281, 0.246451, 0.242166,
            0.453704, 0.453704, 0.453704, 0.381255, 0.334578, 0.305527, 0.268909, 0.251367,
            0.517748, 0.517748, 0.517748, 0.416577, 0.364770, 0.331595, 0.287423, 0.264285 };

        Matrix blackVolMatrix(surfaceStrikes.size(), dates.size());
        for (Size i=0; i < surfaceStrikes.size(); ++i)
            for (Size j=0; j < dates.size(); ++j) {
                blackVolMatrix[i][j] = v[i*(dates.size())+j];
            }

        const boost::shared_ptr<BlackVarianceSurface> volTS(
            new BlackVarianceSurface(todaysDate, cal,
                                     dates,
                                     surfaceStrikes, blackVolMatrix,
                                     dc));
        volTS->setInterpolation<Bicubic>();

        return boost::make_tuple(surfaceStrikes, dates, volTS);
    }
}

void HestonSLVModelTest::testHestonFokkerPlanckFwdEquationLogLVLeverage() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the Heston process Log Transformation with leverage LV limiting case...");

    SavedSettings backup;

    const DayCounter dc = ActualActual();
    const Date todaysDate = Date(28, Dec, 2012);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturityDate = todaysDate + Period(1, Years);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = 100;
    const Real x0 = std::log(s0);
    const Rate r = 0.0;
    const Rate q = 0.0;

    const Real kappa =  1.0;
    const Real theta =  1.0;
    const Real rho   = -0.75;
    const Real sigma =  0.02;
    const Real v0    =  theta;

    const FdmSquareRootFwdOp::TransformationType transform
        = FdmSquareRootFwdOp::Plain;

    const DayCounter dayCounter = Actual365Fixed();
    const Calendar calendar = TARGET();

    const Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(s0)));
    const Handle<YieldTermStructure> rTS(flatRate(todaysDate, r, dayCounter));
    const Handle<YieldTermStructure> qTS(flatRate(todaysDate, q, dayCounter));

    boost::shared_ptr<HestonProcess> hestonProcess(
        new HestonProcess(rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const Size xGrid = 201;
    const Size vGrid = 401;
    const Size tGrid = 25;

    const SquareRootProcessRNDCalculator rnd(v0, kappa, theta, sigma);

    const Real upperBound = rnd.stationary_invcdf(0.99);
    const Real lowerBound = rnd.stationary_invcdf(0.01);

    const Real beta = 10.0;
    std::vector<boost::tuple<Real, Real, bool> > critPoints;
    critPoints.push_back(boost::tuple<Real, Real, bool>(lowerBound, beta, true));
    critPoints.push_back(boost::tuple<Real, Real, bool>(v0, beta/100, true));
    critPoints.push_back(boost::tuple<Real, Real, bool>(upperBound, beta, true));
    const boost::shared_ptr<Fdm1dMesher> varianceMesher(
            new Concentrating1dMesher(lowerBound, upperBound, vGrid, critPoints));

    const boost::shared_ptr<Fdm1dMesher> equityMesher(
        new Concentrating1dMesher(std::log(2), std::log(600.0), xGrid,
            std::make_pair(x0+0.005, 0.1), true));

    const boost::shared_ptr<FdmMesherComposite>
        mesher(new FdmMesherComposite(equityMesher, varianceMesher));

    const boost::tuple<std::vector<Real>, std::vector<Date>,
                 boost::shared_ptr<BlackVarianceSurface> > smoothSurface =
        createSmoothImpliedVol(dayCounter, calendar);
    const boost::shared_ptr<BlackScholesMertonProcess> lvProcess(
        new BlackScholesMertonProcess(spot, qTS, rTS,
            Handle<BlackVolTermStructure>(smoothSurface.get<2>())));

    // step two days using non-correlated process
    const Time eT = 2.0/365;

    Real v=-Null<Real>(), p_v;
    Array p(mesher->layout()->size(), 0.0);
    const Real bsV0 = square<Real>()(
        lvProcess->blackVolatility()->blackVol(0.0, s0, true));

    SquareRootProcessRNDCalculator rndCalculator(v0, kappa, theta, sigma);
    const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
    for (FdmLinearOpIterator iter = layout->begin(); iter != layout->end();
         ++iter) {
        const Real x = mesher->location(iter, 0);
        if (v != mesher->location(iter, 1)) {
            v = mesher->location(iter, 1);
            p_v = rndCalculator.pdf(v, eT);
        }
        const Real p_x = 1.0/(std::sqrt(M_TWOPI*bsV0*eT))
            * std::exp(-0.5*square<Real>()(x - x0)/(bsV0*eT));
        p[iter.index()] = p_v*p_x;
    }
    const Time dt = (maturity-eT)/tGrid;


    Real denseStrikes[] =
        { 2.222222222, 11.11111111, 20, 25, 30, 35, 40,
          44.44444444, 50, 55, 60, 65, 70, 75.55555556,
          80, 84.44444444, 88.88888889, 93.33333333, 97.77777778, 100,
          102.2222222, 106.6666667, 111.1111111, 115.5555556, 120,
          124.4444444, 166.6666667, 222.2222222, 444.4444444, 666.6666667 };

    const std::vector<Real> ds(denseStrikes, denseStrikes+LENGTH(denseStrikes));

    Matrix surface(ds.size(), smoothSurface.get<1>().size());
    std::vector<Time> times(surface.columns());

    const std::vector<Date> dates = smoothSurface.get<1>();
    boost::shared_ptr<Matrix> m = createLocalVolMatrixFromProcess(
        lvProcess, ds, dates, times);

    const boost::shared_ptr<FixedLocalVolSurface> leverage(
        new FixedLocalVolSurface(todaysDate, dates, ds, m, dc));

    const boost::shared_ptr<PricingEngine> lvEngine(
        new AnalyticEuropeanEngine(lvProcess));

    const boost::shared_ptr<FdmLinearOpComposite> hestonFwdOp(
        new FdmHestonFwdOp(mesher, hestonProcess, transform, leverage));

    HundsdorferScheme evolver(FdmSchemeDesc::Hundsdorfer().theta,
                              FdmSchemeDesc::Hundsdorfer().mu,
                              hestonFwdOp);

    Time t=dt;
    evolver.setStep(dt);

    for (Size i=0; i < tGrid; ++i, t+=dt) {
        evolver.step(p, t);
    }

    const boost::shared_ptr<Exercise> exercise(
        new EuropeanExercise(maturityDate));

    const boost::shared_ptr<PricingEngine> fdmEngine(
        new FdBlackScholesVanillaEngine(lvProcess, 50, 201, 0,
                                        FdmSchemeDesc::Douglas(), true,0.2));

    for (Size strike=5; strike < 200.0; ++strike) {
        const boost::shared_ptr<StrikedTypePayoff> payoff(
            new CashOrNothingPayoff(Option::Put, strike, 1.0));

        Array pd(p.size());
        for (FdmLinearOpIterator iter = layout->begin();
            iter != layout->end(); ++iter) {
            const Size idx = iter.index();
            const Real s = std::exp(mesher->location(iter, 0));

            pd[idx] = payoff->operator()(s)*p[idx];
        }

        const Real calculated
            = fokkerPlanckPrice2D(pd, mesher)*rTS->discount(maturityDate);

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(fdmEngine);
        const Real expected = option.NPV();

        const Real tol = 0.015;
        if (std::fabs(expected - calculated ) > tol) {
            BOOST_FAIL("failed to reproduce Heston prices at"
                       << "\n   strike      " << strike
                       << QL_FIXED << std::setprecision(5)
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
    }
}


void HestonSLVModelTest::testBlackScholesFokkerPlanckFwdEquationLocalVol() {
	BOOST_TEST_MESSAGE(
			"Testing Fokker-Planck forward equation for BS Local Vol process...");

	SavedSettings backup;

	const DayCounter dc = ActualActual();
	const Date todaysDate(5, July, 2014);
	Settings::instance().evaluationDate() = todaysDate;

	const Real s0 = 100;
	const Real x0 = std::log(s0);
	const Rate r = 0.035;
	const Rate q = 0.01;

	const Calendar calendar = TARGET();
	const DayCounter dayCounter = Actual365Fixed();

	const boost::shared_ptr<YieldTermStructure> rTS(
			flatRate(todaysDate, r, dayCounter));
	const boost::shared_ptr<YieldTermStructure> qTS(
			flatRate(todaysDate, q, dayCounter));

	boost::tuple<std::vector<Real>, std::vector<Date>,
			boost::shared_ptr<BlackVarianceSurface> > smoothImpliedVol =
			createSmoothImpliedVol(dayCounter, calendar);

	const std::vector<Real>& strikes = smoothImpliedVol.get<0>();
	const std::vector<Date>& dates = smoothImpliedVol.get<1>();
	const Handle<BlackVolTermStructure> vTS = Handle<BlackVolTermStructure>(
			createSmoothImpliedVol(dayCounter, calendar).get<2>());

	const Size xGrid = 2 * 100 + 1;
	const Size tGrid = 400;

	const boost::shared_ptr<Quote> spot(new SimpleQuote(s0));
	const boost::shared_ptr<BlackScholesMertonProcess> process(
		new BlackScholesMertonProcess(
			Handle<Quote>(spot),
			Handle<YieldTermStructure>(qTS),
			Handle<YieldTermStructure>(rTS), vTS));

	const boost::shared_ptr<LocalVolTermStructure> localVol(
		process->localVolatility().currentLink());

	const boost::shared_ptr<PricingEngine> engine(
			new AnalyticEuropeanEngine(process));

	for (Size i = 1; i < dates.size(); ++i) {
		for (Size j = 3; j < strikes.size() - 5; j += 5) {

			const Date& exDate = dates[i];
			const Date maturityDate = exDate;
			const Time maturity = dc.yearFraction(todaysDate, maturityDate);
			const boost::shared_ptr<Exercise> exercise(
					new EuropeanExercise(exDate));

			const boost::shared_ptr<FdmMesher> uniformMesher(
				new FdmMesherComposite(
					boost::shared_ptr<Fdm1dMesher>(
						new FdmBlackScholesMesher(xGrid, process,
							maturity, s0))));

			//-- operator --
			const boost::shared_ptr<FdmLinearOpComposite> uniformBSFwdOp(
				new FdmLocalVolFwdOp(
					uniformMesher, spot, rTS, qTS, localVol, 0.2));

			const boost::shared_ptr<FdmMesher> concentratedMesher(
				new FdmMesherComposite(
					boost::shared_ptr<Fdm1dMesher>(
						new FdmBlackScholesMesher(xGrid, process,
								maturity, s0, Null<Real>(),
								Null<Real>(), 0.0001, 1.5,
								std::pair<Real, Real>(s0, 0.1)))));

			//-- operator --
			const boost::shared_ptr<FdmLinearOpComposite> concentratedBSFwdOp(
				new FdmLocalVolFwdOp(
					concentratedMesher, spot, rTS, qTS, localVol, 0.2));

			const boost::shared_ptr<FdmMesher> shiftedMesher(
				new FdmMesherComposite(
					boost::shared_ptr<Fdm1dMesher>(
						new FdmBlackScholesMesher(xGrid, process,
							maturity, s0, Null<Real>(),
							Null<Real>(), 0.0001, 1.5,
							std::pair<Real, Real>(s0 * 1.1,
									0.2)))));

			//-- operator --
			const boost::shared_ptr<FdmLinearOpComposite> shiftedBSFwdOp(
				new FdmLocalVolFwdOp(
					shiftedMesher, spot, rTS, qTS, localVol, 0.2));

			const boost::shared_ptr<StrikedTypePayoff> payoff(
					new PlainVanillaPayoff(Option::Call, strikes[j]));

			VanillaOption option(payoff, exercise);
			option.setPricingEngine(engine);

			const Real expected = option.NPV();
			const Real calcUniform = fokkerPlanckPrice1D(uniformMesher,
					uniformBSFwdOp, payoff, x0, maturity, tGrid)
					* rTS->discount(maturityDate);
			const Real calcConcentrated = fokkerPlanckPrice1D(
					concentratedMesher, concentratedBSFwdOp, payoff, x0,
					maturity, tGrid) * rTS->discount(maturityDate);
			const Real calcShifted = fokkerPlanckPrice1D(shiftedMesher,
					shiftedBSFwdOp, payoff, x0, maturity, tGrid)
					* rTS->discount(maturityDate);
			const Real tol = 0.05;

			if (std::fabs(expected - calcUniform) > tol) {
				BOOST_FAIL(
						"failed to reproduce european option price " << "with an uniform mesher" << "\n   strike:     " << strikes[i] << QL_FIXED << std::setprecision(8) << "\n   calculated: " << calcUniform << "\n   expected:   " << expected << "\n   tolerance:  " << tol);
			}
			if (std::fabs(expected - calcConcentrated) > tol) {
				BOOST_FAIL(
						"failed to reproduce european option price " << "with a concentrated mesher" << "\n   strike:     " << strikes[i] << QL_FIXED << std::setprecision(8) << "\n   calculated: " << calcConcentrated << "\n   expected:   " << expected << "\n   tolerance:  " << tol);
			}
			if (std::fabs(expected - calcShifted) > tol) {
				BOOST_FAIL(
						"failed to reproduce european option price " << "with a shifted mesher" << "\n   strike:     " << strikes[i] << QL_FIXED << std::setprecision(8) << "\n   calculated: " << calcShifted << "\n   expected:   " << expected << "\n   tolerance:  " << tol);
			}
		}
	}
}


namespace {
    struct HestonModelParams {
        const Rate r, q;
        const Real kappa, theta, rho, sigma, v0;
    };

    struct HestonSLVTestCase {
        const HestonModelParams hestonParams;
        const HestonSLVFokkerPlanckFdmParams fdmParams;
    };


    void lsvCalibrationTest(const HestonSLVTestCase& testCase) {
        const Date todaysDate(2, June, 2015);
        Settings::instance().evaluationDate() = todaysDate;
        const Date finalDate(2, June, 2020);

        const Calendar calendar = TARGET();
        const DayCounter dc = Actual365Fixed();

        const Real s0 = 100;
        const Handle<Quote> spot(boost::make_shared<SimpleQuote>(s0));

        const Rate r = testCase.hestonParams.r;
        const Rate q = testCase.hestonParams.q;

        const Real kappa = testCase.hestonParams.kappa;
        const Real theta = testCase.hestonParams.theta;
        const Real rho   = testCase.hestonParams.rho;
        const Real sigma = testCase.hestonParams.sigma;
        const Real v0    = testCase.hestonParams.v0;
        const Volatility lv = 0.3;

        const Handle<YieldTermStructure> rTS(flatRate(r, dc));
        const Handle<YieldTermStructure> qTS(flatRate(q, dc));

        const boost::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, spot, v0, kappa, theta, sigma, rho));

        const Handle<HestonModel> hestonModel(
            boost::make_shared<HestonModel>(hestonProcess));

        const Handle<LocalVolTermStructure> localVol(
            boost::make_shared<LocalConstantVol>(todaysDate, lv, dc));

        const HestonSLVModel slvModel(
             localVol, hestonModel, finalDate, testCase.fdmParams);

        // this includes a calibration of the leverage function!
        boost::shared_ptr<LocalVolTermStructure>
            l = slvModel.leverageFunction();

        const boost::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
            new GeneralizedBlackScholesProcess(spot, qTS, rTS,
                Handle<BlackVolTermStructure>(flatVol(lv, dc))));

        const boost::shared_ptr<PricingEngine> analyticEngine(
            new AnalyticEuropeanEngine(bsProcess));

        const boost::shared_ptr<PricingEngine> hestonEngine(
            new AnalyticHestonEngine(*hestonModel, 164));

        const Real strikes[] = { 50, 75, 80, 90, 100, 110, 125, 150 };
        const Real times[] = { 3, 6, 9, 12, 24, 36, 60 };

        std::cout << "strike\tmaturity\texpected calculated pureHeston "
                  << "hestonImplied vega slvImpl diff(bp)" << std::endl;

        for (Size t=0; t < LENGTH(times); ++t) {
            const Date expiry = todaysDate +  Period(times[t], Months);
            const boost::shared_ptr<Exercise> exercise(
                new EuropeanExercise(expiry));

            const boost::shared_ptr<PricingEngine> slvEngine(
                (times[t] <= 3) ?
                    new FdHestonVanillaEngine(*hestonModel,
                        std::max(101.0, 51*times[t]/12.0), 401, 101, 0,
                            FdmSchemeDesc::ModifiedCraigSneyd(), l)
                :   new FdHestonVanillaEngine(*hestonModel,
                        std::max(51.0, 51*times[t]/12.0), 201, 101, 0,
                            FdmSchemeDesc::ModifiedCraigSneyd(), l));

            for (Size s=0; s < LENGTH(strikes); ++s) {
                const Real strike = strikes[s];

                const boost::shared_ptr<StrikedTypePayoff> payoff(
                    new PlainVanillaPayoff(
                        (strike > s0) ? Option::Call : Option::Put, strike));

                VanillaOption option(payoff, exercise);

                option.setPricingEngine(slvEngine);
                const Real calculated = option.NPV();

                option.setPricingEngine(analyticEngine);
                const Real expected = option.NPV();
                const Real vega = option.vega();

                //---------------------------------------
                    option.setPricingEngine(hestonEngine);
                    const Real pureHeston = option.NPV();

                    const boost::shared_ptr<GeneralizedBlackScholesProcess> bp(
                        new GeneralizedBlackScholesProcess(spot, qTS, rTS,
                            Handle<BlackVolTermStructure>(flatVol(lv,
                                                          dc))));

                    std::cout << QL_FIXED << std::setprecision(0)
                              << strike << "\t "
                              << std::setprecision(0) << times[t] << "\t "
                              << std::setprecision(8) << expected << " "
                              << std::setprecision(8) << calculated << " "
                              << std::setprecision(8) << pureHeston << " "
                              << std::setprecision(8) << option.impliedVolatility(pureHeston, bp)
                              << " "
                              << std::setprecision(8) << vega << " "
                              << std::setprecision(8) << lv + (calculated-expected)/vega << " "
                              << std::setprecision(1) << (calculated-expected)/vega*1e4
                              << std::endl;

                const Real tol = 0.0005;//testCase.eps;
                if (std::fabs((calculated-expected)/vega) > tol) {
                    std::cout << "failed to reproduce round trip vola "
                              << "\n   strike      " << strike
                              << "\n   time        " << times[t]
                              << QL_FIXED << std::setprecision(5)
                              << "\n   calculated: " << lv + (calculated-expected)/vega
                              << "\n   expected:   " << lv
                              << "\n   tolerance:  " << tol << std::endl;
    //                BOOST_FAIL("failed to reproduce round trip vola "
    //                          << "\n   strike      " << strike
    //                          << "\n   time        " << times[t]
    //                          << QL_FIXED << std::setprecision(5)
    //                          << "\n   calculated: " << lv + (calculated-expected)/vega
    //                          << "\n   expected:   " << lv
    //                          << "\n   tolerance:  " << tol);
                }
            }
        }
    }
}



void HestonSLVModelTest::testHestonSLVModel() {
    SavedSettings backup;

    const HestonSLVFokkerPlanckFdmParams plainParams =
        { 201, 301, 1000, 25, 3.0, 2,
          0.1, 1e-4, -Null<Real>(), 10000,
          1e-8, 1e-8, 0.0, 1.0, 1.0, 1.0, 1e-6,
          FdmHestonGreensFct::Gaussian,
          FdmSquareRootFwdOp::Plain,
          FdmSchemeDesc::ModifiedCraigSneyd()
        };

    const HestonSLVFokkerPlanckFdmParams logParams =
        { 301, 601, 2000, 30, 2.0, 2,
          0.1, 1e-4, -Null<Real>(), 10000,
          1e-5, 1e-5, 0.0000025, 1.0, 0.1, 0.9, 1e-5,
          FdmHestonGreensFct::Gaussian,
          FdmSquareRootFwdOp::Log,
          FdmSchemeDesc::ModifiedCraigSneyd()
        };

    const HestonSLVFokkerPlanckFdmParams powerParams =
        { 401, 801, 2000, 30, 2.0, 2,
          0.1, 1e-3, -Null<Real>(), 10000,
          1e-6, 1e-6, 0.001, 1.0, 0.001, 1.0, 1e-5,
          FdmHestonGreensFct::Gaussian,
          FdmSquareRootFwdOp::Power,
          FdmSchemeDesc::ModifiedCraigSneyd()
        };


    const HestonSLVTestCase testCases[] = {
        { {0.035, 0.01, 1.0, 0.06, -0.75, 0.1, 0.09}, plainParams},
        { {0.035, 0.01, 1.0, 0.06, -0.75, std::sqrt(0.2), 0.09}, logParams },
        { {0.035, 0.01, 1.0, 0.09, -0.75, std::sqrt(0.2), 0.06}, logParams },
        { {0.035, 0.01, 1.0, 0.06, -0.75, 0.2, 0.09}, powerParams }
    };


    for (Size i=0; i < LENGTH(testCases); ++i) {
        BOOST_TEST_MESSAGE("Testing stochastic local volatility calibration case " << i << " ...");
        lsvCalibrationTest(testCases[i]);
    }

}


test_suite* HestonSLVModelTest::experimental() {
    test_suite* suite = BOOST_TEST_SUITE(
    	"Heston Stochastic Local Volatility tests");

//    suite->add(QUANTLIB_TEST_CASE(
//        &HestonSLVModelTest::testBlackScholesFokkerPlanckFwdEquation));
//    suite->add(QUANTLIB_TEST_CASE(&HestonSLVModelTest::testSquareRootZeroFlowBC));
//    suite->add(QUANTLIB_TEST_CASE(&HestonSLVModelTest::testTransformedZeroFlowBC));
//    suite->add(QUANTLIB_TEST_CASE(
//        &HestonSLVModelTest::testSquareRootEvolveWithStationaryDensity));
//    suite->add(QUANTLIB_TEST_CASE(
//        &HestonSLVModelTest::testSquareRootLogEvolveWithStationaryDensity));
//    suite->add(QUANTLIB_TEST_CASE(
//        &HestonSLVModelTest::testSquareRootFokkerPlanckFwdEquation));
//    suite->add(QUANTLIB_TEST_CASE(
//        &HestonSLVModelTest::testHestonFokkerPlanckFwdEquation));
//    suite->add(QUANTLIB_TEST_CASE(
//        &HestonSLVModelTest::testHestonFokkerPlanckFwdEquationLogLVLeverage));
//    suite->add(QUANTLIB_TEST_CASE(
//        &HestonSLVModelTest::testBlackScholesFokkerPlanckFwdEquationLocalVol));
//    suite->add(QUANTLIB_TEST_CASE(&HestonSLVModelTest::testSLVCalibration));

    suite->add(QUANTLIB_TEST_CASE(&HestonSLVModelTest::testHestonSLVModel));


    return suite;
}
