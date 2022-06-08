/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

#include "nthorderderivativeop.hpp"
#include "utilities.hpp"

#include <ql/math/comparison.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/richardsonextrapolation.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/nthorderderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>

#include <boost/numeric/ublas/banded.hpp>
#include <boost/numeric/ublas/operation_sparse.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <numeric>
#include <utility>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void NthOrderDerivativeOpTest::testSparseMatrixApply() {
    BOOST_TEST_MESSAGE("Testing sparse matrix apply...");

    SparseMatrix sm(5,7);

    BOOST_CHECK(sm.size1() == SparseMatrix::size_type(5));
    BOOST_CHECK(sm.size2() == SparseMatrix::size_type(7));

    sm(1,3) = 3.0;

    const Array x(7,0.0,1.0);
    const Array y = prod(sm, x);

    BOOST_CHECK(close_enough(y[0], 0.0));
    BOOST_CHECK(close_enough(y[1], 3.0*3.0));
    BOOST_CHECK(close_enough(y[2], 0.0));
    BOOST_CHECK(close_enough(y[3], 0.0));
    BOOST_CHECK(close_enough(y[4], 0.0));
}

void NthOrderDerivativeOpTest::testFirstOrder2PointsApply() {
    BOOST_TEST_MESSAGE("Testing two points first order "
            "derivative operator apply on an uniform grid...");

    const Real dx = 1/5.0;

    const NthOrderDerivativeOp op(0, 1, 3,
        ext::make_shared<FdmMesherComposite>(
            ext::make_shared<Uniform1dMesher>(0.0, 1.0, 6)));

    const Array x(6,0.0, 1.0);
    const Array y = op.apply(x);

    for (Size i=0; i < x.size(); ++i)
        BOOST_CHECK(close_enough(y[i], 1/dx));
}

void NthOrderDerivativeOpTest::testFirstOrder3PointsOnUniformGrid() {
    BOOST_TEST_MESSAGE("Testing three points first order "
            "derivative operator on an uniform grid...");

    const Real ddx = 1.0/0.2;

    const SparseMatrix m =
        NthOrderDerivativeOp(0, 1, 3,
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Uniform1dMesher>(0.0, 1.0, 6))).toMatrix();

    // to reproduce the reference results use
    // http://web.media.mit.edu/~crtaylor/calculator.html

    BOOST_CHECK(close_enough(m(2,0), 0.0));
    BOOST_CHECK(close_enough(m(2,1), -0.5*ddx));
    BOOST_CHECK(m(2,2) < 42*QL_EPSILON);
    BOOST_CHECK(close_enough(m(2,3), 0.5*ddx));
    BOOST_CHECK(close_enough(m(2,4), 0.0));
    BOOST_CHECK(close_enough(m(2,5), 0.0));

    BOOST_CHECK(close_enough(m(0,0), -3.0/2.0*ddx));
    BOOST_CHECK(close_enough(m(0,1), 2.0*ddx));
    BOOST_CHECK(close_enough(m(0,2), -0.5*ddx));
    BOOST_CHECK(close_enough(m(0,3), 0.0));
    BOOST_CHECK(close_enough(m(0,4), 0.0));
    BOOST_CHECK(close_enough(m(0,5), 0.0));

    BOOST_CHECK(close_enough(m(5,0), 0.0));
    BOOST_CHECK(close_enough(m(5,1), 0.0));
    BOOST_CHECK(close_enough(m(5,2), 0.0));
    BOOST_CHECK(close_enough(m(5,3), 0.5*ddx));
    BOOST_CHECK(close_enough(m(5,4), -2.0*ddx));
    BOOST_CHECK(close_enough(m(5,5), 3.0/2.0*ddx));
}

void NthOrderDerivativeOpTest::testFirstOrder5PointsOnUniformGrid() {
    BOOST_TEST_MESSAGE("Testing five points first order "
            "derivative operator on an uniform grid...");

    const Real ddx = 1.0/0.4;

    const SparseMatrix m =
        NthOrderDerivativeOp(0, 1, 5,
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Uniform1dMesher>(0.0, 2.0, 6))).toMatrix();

    BOOST_CHECK(close_enough(m(2,0), 1.0/12.0*ddx));
    BOOST_CHECK(close_enough(m(2,1), -2.0/3.0*ddx));
    BOOST_CHECK(m(2,2) < 42*QL_EPSILON);
    BOOST_CHECK(close_enough(m(2,3), 2.0/3.0*ddx));
    BOOST_CHECK(close_enough(m(2,4), -1.0/12.0*ddx));
    BOOST_CHECK(close_enough(m(2,5), 0.0));

    BOOST_CHECK(close_enough(m(0,0), -25.0/12.0*ddx));
    BOOST_CHECK(close_enough(m(0,1), 4.0*ddx));
    BOOST_CHECK(close_enough(m(0,2), -3.0*ddx));
    BOOST_CHECK(close_enough(m(0,3), 4.0/3.0*ddx));
    BOOST_CHECK(close_enough(m(0,4), -0.25*ddx));
    BOOST_CHECK(close_enough(m(0,5), 0.0));

    BOOST_CHECK(close_enough(m(1,0), -0.25*ddx));
    BOOST_CHECK(close_enough(m(1,1), -5.0/6.0*ddx));
    BOOST_CHECK(close_enough(m(1,2), 3.0/2.0*ddx));
    BOOST_CHECK(close_enough(m(1,3), -0.5*ddx));
    BOOST_CHECK(close_enough(m(1,4), 1.0/12.0*ddx));
    BOOST_CHECK(close_enough(m(1,5), 0.0));

    BOOST_CHECK(close_enough(m(4,5), 0.25*ddx));
    BOOST_CHECK(close_enough(m(4,4), 5.0/6.0*ddx));
    BOOST_CHECK(close_enough(m(4,3), -3.0/2.0*ddx));
    BOOST_CHECK(close_enough(m(4,2), 0.5*ddx));
    BOOST_CHECK(close_enough(m(4,1), -1.0/12.0*ddx));
    BOOST_CHECK(close_enough(m(4,0), 0.0));

    BOOST_CHECK(close_enough(m(5,0), 0.0));
    BOOST_CHECK(close_enough(m(5,1), 0.25*ddx));
    BOOST_CHECK(close_enough(m(5,2), -4.0/3.0*ddx));
    BOOST_CHECK(close_enough(m(5,3), 3.0*ddx));
    BOOST_CHECK(close_enough(m(5,4), -4.0*ddx));
    BOOST_CHECK(close_enough(m(5,5), 25.0/12.0*ddx));
}

void NthOrderDerivativeOpTest::testFirstOrder2PointsOnUniformGrid() {
    BOOST_TEST_MESSAGE("Testing two points first order "
            "derivative operator on an uniform grid...");

    const Real ddx = 1.0/0.2;

    const SparseMatrix m =
        NthOrderDerivativeOp(0, 1, 2,
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Uniform1dMesher>(0.0, 0.6, 4))).toMatrix();

    BOOST_CHECK(close_enough(m(0,0), -ddx));
    BOOST_CHECK(close_enough(m(0,1), ddx));
    BOOST_CHECK(close_enough(m(0,2), 0.0));
    BOOST_CHECK(close_enough(m(0,3), 0.0));

    BOOST_CHECK(close_enough(m(1,0), -ddx));
    BOOST_CHECK(close_enough(m(1,1), ddx));
    BOOST_CHECK(close_enough(m(1,2), 0.0));
    BOOST_CHECK(close_enough(m(1,3), 0.0));

    BOOST_CHECK(close_enough(m(2,0), 0.0));
    BOOST_CHECK(close_enough(m(2,1), -ddx));
    BOOST_CHECK(close_enough(m(2,2), ddx));
    BOOST_CHECK(close_enough(m(2,3), 0.0));

    BOOST_CHECK(close_enough(m(3,0), 0.0));
    BOOST_CHECK(close_enough(m(3,1), 0.0));
    BOOST_CHECK(close_enough(m(3,2), -ddx));
    BOOST_CHECK(close_enough(m(3,3), ddx));
}

void NthOrderDerivativeOpTest::testFirstOrder4PointsOnUniformGrid() {
    BOOST_TEST_MESSAGE("Testing four points first order "
            "derivative operator on an uniform grid...");

    const Real ddx = 1.0/0.2;

    const SparseMatrix m =
        NthOrderDerivativeOp(0, 1, 4,
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Uniform1dMesher>(0.0, 0.6, 4))).toMatrix();

    BOOST_CHECK(close_enough(m(0,0), -11.0/6.0*ddx));
    BOOST_CHECK(close_enough(m(0,1), 3.0*ddx));
    BOOST_CHECK(close_enough(m(0,2), -1.5*ddx));
    BOOST_CHECK(close_enough(m(2,3), 1.0/3.0*ddx));

    BOOST_CHECK(close_enough(m(1,0), -1.0/3.0*ddx));
    BOOST_CHECK(close_enough(m(1,1), -0.5*ddx));
    BOOST_CHECK(close_enough(m(1,2), ddx));
    BOOST_CHECK(close_enough(m(1,3), -1.0/6.0*ddx));

    BOOST_CHECK(close_enough(m(2,0), 1.0/6.0*ddx));
    BOOST_CHECK(close_enough(m(2,1), -ddx));
    BOOST_CHECK(close_enough(m(2,2), 0.5*ddx));
    BOOST_CHECK(close_enough(m(2,3), 1.0/3.0*ddx));

    BOOST_CHECK(close_enough(m(3,0), -1.0/3.0*ddx));
    BOOST_CHECK(close_enough(m(3,1), 1.5*ddx));
    BOOST_CHECK(close_enough(m(3,2), -3.0*ddx));
    BOOST_CHECK(close_enough(m(3,3), 11.0/6.0*ddx));
}

void NthOrderDerivativeOpTest::testFirstOrder2PointsOn2DimUniformGrid() {
    BOOST_TEST_MESSAGE("Testing two points first order "
            "derivative operator on a 2 dimensional uniform grid...");

    const Real ddx = 1.0/0.2;

    const Size xGrid=4;
    const ext::shared_ptr<FdmMesher> mesher =
        ext::make_shared<FdmMesherComposite>(
            ext::make_shared<Uniform1dMesher>(0.0, 1, xGrid),
            ext::make_shared<Uniform1dMesher>(0.0, 0.4, 3));

    const SparseMatrix m = NthOrderDerivativeOp(1, 1, 2, mesher).toMatrix();

    const ext::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
    const FdmLinearOpIterator endIter = layout->end();

    for (FdmLinearOpIterator iter = layout->begin(); iter!=endIter; ++iter) {
        const Size i = iter.index();
        const Size ix = iter.coordinates()[1];

        switch (ix) {
          case 0:
            BOOST_CHECK(close_enough(m(i, i        ),-ddx));
            BOOST_CHECK(close_enough(m(i, i+  xGrid), ddx));
            BOOST_CHECK(close_enough(m(i, i+2*xGrid), 0.0));
            break;
          case 1:
            BOOST_CHECK(close_enough(m(i, i-  xGrid),-ddx));
            BOOST_CHECK(close_enough(m(i, i        ), ddx));
            BOOST_CHECK(close_enough(m(i, i+2*xGrid), 0.0));
            break;
          case 2:
            BOOST_CHECK(close_enough(m(i, i-2*xGrid), 0.0));
            BOOST_CHECK(close_enough(m(i, i-  xGrid),-ddx));
            BOOST_CHECK(close_enough(m(i, i        ), ddx));
            break;
          default:
            QL_FAIL("inconsistent coordinate");
        }
    }
}

void NthOrderDerivativeOpTest::testSecondOrder3PointsNonUniformGrid() {
    BOOST_TEST_MESSAGE("Testing three points second order "
            "derivative operator on a non-uniform grid...");

    std::vector<Real> xValues(4);
    xValues[0] = 0.5;
    xValues[1] = 1.0;
    xValues[2] = 2.0;
    xValues[3] = 4.0;

    const SparseMatrix m =
        NthOrderDerivativeOp(0, 2, 3,
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Predefined1dMesher>(xValues))).toMatrix();

    BOOST_CHECK(close_enough(m(0,0), 8.0/3.0));
    BOOST_CHECK(close_enough(m(0,1), -4.0));
    BOOST_CHECK(close_enough(m(0,2), 4.0/3.0));
    BOOST_CHECK(close_enough(m(0,3), 0.0));

    BOOST_CHECK(close_enough(m(1,0), 8.0/3.0));
    BOOST_CHECK(close_enough(m(1,1), -4.0));
    BOOST_CHECK(close_enough(m(1,2), 4.0/3.0));
    BOOST_CHECK(close_enough(m(1,3), 0.0));

    BOOST_CHECK(close_enough(m(2,0), 0.0));
    BOOST_CHECK(close_enough(m(2,1), 2.0/3.0));
    BOOST_CHECK(close_enough(m(2,2), -1.0));
    BOOST_CHECK(close_enough(m(2,3), 1.0/3.0));

    BOOST_CHECK(close_enough(m(3,0), 0.0));
    BOOST_CHECK(close_enough(m(3,1), 2.0/3.0));
    BOOST_CHECK(close_enough(m(3,2), -1.0));
    BOOST_CHECK(close_enough(m(3,3), 1.0/3.0));
}

void NthOrderDerivativeOpTest::testSecondOrder4PointsNonUniformGrid() {
    BOOST_TEST_MESSAGE("Testing four points second order "
            "derivative operator on a non-uniform grid...");

    std::vector<Real> xValues(5);
    xValues[0] = 0.5;
    xValues[1] = 1.0;
    xValues[2] = 2.0;
    xValues[3] = 4.0;
    xValues[4] = 8.0;

    const SparseMatrix m =
        NthOrderDerivativeOp(0, 2, 4,
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Predefined1dMesher>(xValues))).toMatrix();

    BOOST_CHECK(close_enough(m(0,0), 88.0/21.0));
    BOOST_CHECK(close_enough(m(0,1), -140.0/21.0));
    BOOST_CHECK(close_enough(m(0,2), 56.0/21.0));
    BOOST_CHECK(close_enough(m(0,3), -4.0/21.0));
    BOOST_CHECK(close_enough(m(0,4), 0.0));

    BOOST_CHECK(close_enough(m(1,0), 64.0/21.0));
    BOOST_CHECK(close_enough(m(1,1), -98.0/21.0));
    BOOST_CHECK(close_enough(m(1,2), 35.0/21.0));
    BOOST_CHECK(close_enough(m(1,3), -1.0/21.0));
    BOOST_CHECK(close_enough(m(1,4), 0.0));

    BOOST_CHECK(close_enough(m(2,0), 16.0/21.0));
    BOOST_CHECK(close_enough(m(2,1), -2.0/3.0));
    BOOST_CHECK(close_enough(m(2,2), -1.0/3.0));
    BOOST_CHECK(close_enough(m(2,3), 5.0/21.0));
    BOOST_CHECK(close_enough(m(2,4), 0.0));

    BOOST_CHECK(close_enough(m(3,0), 0.0));
    BOOST_CHECK(close_enough(m(3,1), 4.0/21.0));
    BOOST_CHECK(close_enough(m(3,2), -1.0/6.0));
    BOOST_CHECK(close_enough(m(3,3), -1.0/12.0));
    BOOST_CHECK(close_enough(m(3,4), 5.0/84.0));

    BOOST_CHECK(close_enough(m(4,0), 0.0));
    BOOST_CHECK(close_enough(m(4,1), -20.0/21.0));
    BOOST_CHECK(close_enough(m(4,2), 11.0/6.0));
    BOOST_CHECK(close_enough(m(4,3), -13.0/12.0));
    BOOST_CHECK(close_enough(m(4,4), 17.0/84.0));
}

void NthOrderDerivativeOpTest::testThirdOrder4PointsUniformGrid() {
    BOOST_TEST_MESSAGE("Testing four points third order "
            "derivative operator on a uniform grid...");

    const SparseMatrix m =
        NthOrderDerivativeOp(0, 3, 4,
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Uniform1dMesher>(0.0, 0.6, 4))).toMatrix();

    for (Size i=0; i < 4; ++i) {
        BOOST_CHECK(close_enough(m(i,0), -125.0));
        BOOST_CHECK(close_enough(m(i,1), 375.0));
        BOOST_CHECK(close_enough(m(i,2), -375.0));
        BOOST_CHECK(close_enough(m(i,3), 125));
    }
}


namespace {
    struct GridSetup {
        Real alpha;
        Real density;
        bool cellAvg;
        bool midPoint;
        Size nPoints;
        Size tGrid;
        Size yGrid;
        Size vGrid;
        FdmSchemeDesc scheme;
    };

    class FdmHestonNthOrderOp : public FdmLinearOpComposite {
      public:
        FdmHestonNthOrderOp(Size nPoints,
                            const ext::shared_ptr<HestonProcess>& hestonProcess,
                            const ext::shared_ptr<FdmMesher>& mesher,
                            Size direction = 0)
        : vol2_(0.5 * hestonProcess->theta()),
          preconditioner_(
              SecondDerivativeOp(direction, mesher).mult(Array(mesher->layout()->size(), vol2_))) {

            const Array vv(mesher->locations(1));
            Array varianceValues(0.5*vv);

            ext::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
            FdmLinearOpIterator endIter = layout->end();
            for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
                ++iter) {
                if (   iter.coordinates()[0] == 0
                    || iter.coordinates()[0] == layout->dim()[0]-1) {
                    varianceValues[iter.index()] = 0.0;
                }
            }
            const Size n = mesher->layout()->size();

            using namespace boost::numeric::ublas;
            banded_matrix<Real> v(n, n), u(n, n), rV(n, n);
            for (Size i=0; i < n; ++i) {
                v(i, i) = varianceValues[i];
                u(i, i) = vv[i];
                rV(i, i) = varianceValues[i] - 0.5*hestonProcess->theta();
            }

            const SparseMatrix dx = NthOrderDerivativeOp(0, 1, nPoints, mesher).toMatrix();
            const SparseMatrix dxx = NthOrderDerivativeOp(0, 2, nPoints, mesher).toMatrix();
            const SparseMatrix dv = NthOrderDerivativeOp(1, 1, nPoints, mesher).toMatrix();
            const SparseMatrix dvv = NthOrderDerivativeOp(1, 2, nPoints, mesher).toMatrix();

            const Real kappa = hestonProcess->kappa();
            const Real theta = hestonProcess->theta();
            const Real sigma = hestonProcess->sigma();
            const Real rho = hestonProcess->rho();

            map_ = sparse_prod<SparseMatrix>(-rV, dx)
                + sparse_prod<SparseMatrix>(v, dxx)
                + (0.5*rho*sigma)*sparse_prod<SparseMatrix>(
                    u, sparse_prod<SparseMatrix>(dx, dv)
                        + sparse_prod<SparseMatrix>(dv, dx))
                + (0.5*sigma*sigma)*sparse_prod<SparseMatrix>(u, dvv)
                + kappa*sparse_prod<SparseMatrix>(
                    theta*identity_matrix<Real>(n) - u, dv);
        }

        SparseMatrix toMatrix() const override {
            return map_;
        }

        Size size() const override { return 2; }
        void setTime(Time t1, Time t2) override { }

        Array apply(const Array& r) const override {
            return prod(map_, r);
        }

        Array apply_mixed(const Array& r) const override {
            QL_FAIL("operator splitting is not supported");
        }

        Array apply_direction(Size direction, const Array& r) const override {
            QL_FAIL("operator splitting is not supported");
        }

        Array solve_splitting(Size direction, const Array& r, Real dt) const override {
            QL_FAIL("operator splitting is not supported");
        }

        Array preconditioner(const Array& r, Real dt) const override {
            return preconditioner_.solve_splitting(r, dt, 1.0);
        }

      private:
        Array solve_apply(const Array& r, Real dt) const {
            return r - dt*apply(r);
        }

        const Volatility vol2_;
        SparseMatrix map_;
        TripleBandLinearOp preconditioner_;
    };


    class AvgPayoffFct {
      public:
        AvgPayoffFct(ext::shared_ptr<PlainVanillaPayoff> payoff,
                     Volatility vol, Time T, Real growthFactor)
        : payoff_(std::move(payoff)),
          vol2_(0.5*vol*vol*T),
          growthFactor_(growthFactor) { }

        Real operator()(Real x) const {
            return (*payoff_)(std::exp(x - vol2_)*growthFactor_);
        }

       private:
        const ext::shared_ptr<PlainVanillaPayoff> payoff_;
        const Volatility vol2_;
        const Real growthFactor_;
    };

   class MyInnerValueCalculator : public FdmInnerValueCalculator {
      public:
        MyInnerValueCalculator(ext::shared_ptr<Payoff> payoff,
                               ext::shared_ptr<FdmMesher> mesher,
                               ext::shared_ptr<YieldTermStructure> rTS,
                               ext::shared_ptr<YieldTermStructure> qTS,
                               Volatility vol,
                               Size direction)
        : payoff_(std::move(payoff)), mesher_(std::move(mesher)),
          rTS_(std::move(rTS)), qTS_(std::move(qTS)), vol_(vol),
          direction_(direction) {}

        Real innerValue(const FdmLinearOpIterator& iter, Time t)  override {
            const Real g = mesher_->location(iter, direction_);
            const Real sT = std::exp(g - 0.5*vol_*vol_*t);

            return (*payoff_)(sT);
        }

        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
            return innerValue(iter, t);
        }

      private:
        const ext::shared_ptr<Payoff> payoff_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<YieldTermStructure> rTS_, qTS_;
        const Volatility vol_;
        const Size direction_;
    };

    Array priceReport(
        const GridSetup& setup, const Array& strikes) {

        const Date today(2, May, 2018);
        const DayCounter dc = Actual365Fixed();
        const Date maturity = today + Period(1, Years);
        const Time T = dc.yearFraction(today, maturity);

        const ext::shared_ptr<YieldTermStructure> rTS
            = flatRate(today, 0.05, dc);
        const ext::shared_ptr<YieldTermStructure> qTS
            = flatRate(today, 0.0, dc);

        const Real S = 100.0;
        const Volatility vol = 0.2;
        const Real v0 = vol*vol;
        const Real kappa = 1.0;
        const Real theta = vol*vol;
        const Real sig = 0.2;
        const Real rho = -0.75;

        const ext::shared_ptr<HestonProcess> hestonProcess
            = ext::make_shared<HestonProcess>(
                  Handle<YieldTermStructure>(rTS),
                  Handle<YieldTermStructure>(qTS),
                  Handle<Quote>(ext::make_shared<SimpleQuote>(S)),
                  v0, kappa, theta, sig, rho);

        const Real stdDev = vol * std::sqrt(T);
        const DiscountFactor df
            = qTS->discount(maturity)/rTS->discount(maturity);

        const Real y = std::log(S);

        const Real ymin = y - setup.alpha*stdDev;
        const Real ymax = y + setup.alpha*stdDev;

        const Size yGrid = setup.yGrid;
        const Size vGrid = setup.vGrid;

        Array diffs(strikes.size()), fdmPrices(strikes.size());
        for (Size k=0; k < strikes.size(); ++k) {
            const Real strike = strikes[k];
            const Real specialPoint = std::log(strike/df) + 0.5*vol*vol*T;

            const ext::shared_ptr<Fdm1dMesher> mesher1d =
                ext::make_shared<Concentrating1dMesher>(
                    ymin, ymax, yGrid,
                    std::pair<Real, Real>(specialPoint, setup.density));

            std::vector<Real> loc = mesher1d->locations();
            for (Size i = 0; setup.midPoint && i < loc.size()-1; ++i)
                if (loc[i] < specialPoint && loc[i+1]>= specialPoint) {
                    const Real d = loc[i+1] - loc[i];

                    const Real offset = (specialPoint - 0.5*d) - loc[i];

                    for (Real& l : loc)
                        l += offset;

                    break;
                }

            const ext::shared_ptr<FdmMesherComposite> mesher =
                ext::make_shared<FdmMesherComposite>(
                    ext::make_shared<Predefined1dMesher>(loc),
                    ext::make_shared<FdmHestonVarianceMesher>(
                        vGrid, hestonProcess, 1.0)
            );

            const Array g = mesher->locations(0);
            const Array sT = Exp(g - 0.5*vol*vol*T)*df;

            Array rhs(mesher->layout()->size());

            const ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::make_shared<PlainVanillaPayoff>(Option::Put, strike);

            const ext::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
            const FdmLinearOpIterator endIter = layout->end();

            for (FdmLinearOpIterator iter = layout->begin();
                 iter!=endIter; ++iter) {
                const Size idx = iter.index();
                const Size idxm1 = layout->neighbourhood(iter,  0,-1);
                const Size idxp1 = layout->neighbourhood(iter,  0, 1);

                const Size nx = iter.coordinates()[0];

                if (nx != 0 && nx != yGrid-1
                    && setup.cellAvg && (
                        (sT[idx] < strike && sT[idxp1] >= strike)
                     || (sT[idxm1] < strike && sT[idx] >= strike))) {

                    const Real gMin = 0.5*(g[idxm1] + g[idx]);
                    const Real gMax = 0.5*(g[idxp1] + g[idx]);

                    const AvgPayoffFct f(payoff, vol, T, df);

                    rhs[idx] = GaussLobattoIntegral(1000, 1e-12)(
                        f, gMin, gMax)/(gMax - gMin);
                }
                else
                    rhs[idx] = (*payoff)(sT[idx]);
            }

            const ext::shared_ptr<FdmHestonNthOrderOp> heatEqn =
                ext::make_shared<FdmHestonNthOrderOp>(
                    setup.nPoints, hestonProcess, mesher);

            FdmBackwardSolver solver(
              heatEqn,
              FdmBoundaryConditionSet(),
              ext::shared_ptr<FdmStepConditionComposite>(),
              setup.scheme);

            solver.rollback(rhs, T, 0.0, setup.tGrid, 1);

            rhs *= rTS->discount(maturity);

            const std::vector<Real>& x =
                mesher->getFdm1dMeshers()[0]->locations();
            const std::vector<Real>& v =
                mesher->getFdm1dMeshers()[1]->locations();

            Matrix resultValues_(layout->dim()[1], layout->dim()[0]);
            std::copy(rhs.begin(), rhs.end(), resultValues_.begin());

            const ext::shared_ptr<BicubicSpline> interpolation =
                ext::make_shared<BicubicSpline>(
                    x.begin(), x.end(), v.begin(), v.end(), resultValues_);

            const Real fdmPrice = (*interpolation)(y, hestonProcess->v0());

            VanillaOption option(
                ext::make_shared<PlainVanillaPayoff>(Option::Put, strike),
                ext::make_shared<EuropeanExercise>(maturity)
            );
            option.setPricingEngine(
                ext::make_shared<AnalyticHestonEngine>(
                    ext::make_shared<HestonModel>(hestonProcess), 192)
            );
            const Real npv = option.NPV();

            diffs[k] = npv - fdmPrice;
        }

        return diffs;
    }

    class FdmMispricingCostFunction : public CostFunction {
      public:
        FdmMispricingCostFunction(
            const GridSetup& setup, Array strikes)
        : setup_(setup), strikes_(std::move(strikes)) { }

        Array values(const Array& x) const override {
            const GridSetup g = {
                x[0], x[1],
                setup_.cellAvg, setup_.midPoint,
                setup_.nPoints,
                setup_.tGrid, setup_.yGrid, setup_.vGrid,
                setup_.scheme
            };

            try {
                return priceReport(g, strikes_);
            }
            catch (std::exception const&) {
                Array q(2, 1000);
                return q;
            }
        }

      private:
        const GridSetup setup_;
        const Array strikes_;
    };
}

void NthOrderDerivativeOpTest::testHigherOrderHestonOptionPricing() {
    BOOST_TEST_MESSAGE("Testing Heston model option pricing convergence with "
            "higher order finite difference operators...");

    SavedSettings backup;

    const Array strikes = {50, 75, 90, 100, 110, 125, 150, 200};

    const GridSetup initSetup = {
        3.87773, 0.043847, true, false,
        5, 21, 20, 11, FdmSchemeDesc::CrankNicolson()
    };

    const Array initialValues = {initSetup.alpha, initSetup.density};

    FdmMispricingCostFunction costFct(initSetup, strikes);
    NoConstraint noConstraint;

    Problem prob(costFct, noConstraint, initialValues);

    LevenbergMarquardt().minimize(
        prob, EndCriteria(400, 40, 1.0e-4, 1.0e-4, 1.0e-4));

    const GridSetup optimalSetup = {
        prob.currentValue()[0], prob.currentValue()[1],
        initSetup.cellAvg, initSetup.midPoint,
        initSetup.nPoints,
        initSetup.tGrid,
        initSetup.yGrid/2,
        initSetup.vGrid,
        initSetup.scheme
    };

    const Array q = priceReport(optimalSetup, strikes);
    const Real ac = std::sqrt(DotProduct(q, q)/q.size());

    const Array p = priceReport(initSetup, strikes);
    const Real ap = std::sqrt(DotProduct(p, p)/p.size());

    const Real convergence = std::log(ac/ap)*M_LOG2E;

    if (convergence < 3.6) {
        BOOST_ERROR("convergence order is too low"
                << "\n expected convergence: 4.0"
                << "\n measured convergence: " << convergence
                << "\n tolerance           : 0.4");
    }
}


namespace {
    Real priceQuality(Real h) {

        const Array strikes = {100};

        const Size yGrid = Size(1/h);
        const GridSetup setup = {
              5.50966, 0.0130581,
             true, false,
             5, 401, yGrid, 21,
             FdmSchemeDesc::CrankNicolson()
        };

        return  std::fabs(priceReport(setup, strikes)[0]);
    }
}

void NthOrderDerivativeOpTest::testHigherOrderAndRichardsonExtrapolation() {
    BOOST_TEST_MESSAGE(
            "Testing Heston option pricing convergence with "
            "higher order FDM operators and Richardson Extrapolation...");

    SavedSettings backup;

    const Real n1 = priceQuality(1.0/25);
    const Real n3
        = std::fabs(RichardsonExtrapolation(priceQuality, 1.0/25, 4.0)(2.0));

    const Real r2 = std::log(n1/n3)*M_LOG2E;

    if (r2 < 4.9) {
        BOOST_ERROR("convergence order is too low using Richardson extrapolation"
                << "\n expected convergence: 5.0"
                << "\n measured convergence: " << r2
                << "\n tolerance           : 0.1");
    }
}

void NthOrderDerivativeOpTest::testCompareFirstDerivativeOpNonUniformGrid() {
    BOOST_TEST_MESSAGE(
        "Testing with FirstDerivativeOp on a non-uniform grid...");

    Array xValues = Exp(Array(7, 0, 0.1));

    const ext::shared_ptr<Fdm1dMesher> m
        = ext::make_shared<Predefined1dMesher>(
            std::vector<Real>(xValues.begin(), xValues.end()));

    const ext::shared_ptr<FdmMesher> m1d(
        ext::make_shared<FdmMesherComposite>(m));

    const ext::shared_ptr<FirstDerivativeOp> fx
        = ext::make_shared<FirstDerivativeOp>(0, m1d);

    const ext::shared_ptr<NthOrderDerivativeOp> dx
        = ext::make_shared<NthOrderDerivativeOp>(0, 1, 3, m1d);

    const SparseMatrix fm = fx->toMatrix();
    const SparseMatrix dm = dx->toMatrix();

    for (Size i=1; i < m->size()-1; ++i) // different boundary conditions
        for (Size j=0; j < m->size(); ++j)
            BOOST_CHECK(std::fabs(fm(i, j)- dm(i, j)) < 1e-12);
}

void NthOrderDerivativeOpTest::testCompareFirstDerivativeOp2dUniformGrid() {
    BOOST_TEST_MESSAGE(
        "Testing with FirstDerivativeOp on a 2d uniform grid...");

    const ext::shared_ptr<Fdm1dMesher> m1
        = ext::make_shared<Uniform1dMesher>(0.0, 0.6, 5);
    const ext::shared_ptr<Fdm1dMesher> m2
        = ext::make_shared<Uniform1dMesher>(0.0, 1.6, 6);

    const ext::shared_ptr<FdmMesher> mc(
        ext::make_shared<FdmMesherComposite>(m1, m2));

    const ext::shared_ptr<FdmLinearOpLayout> layout = mc->layout();

    const Size n = layout->dim()[0];
    const Size m = layout->dim()[1];

    SparseMatrix fm = FirstDerivativeOp(0, mc).toMatrix();
    SparseMatrix dm = NthOrderDerivativeOp(0, 1, 3, mc).toMatrix();

    for (Size k=0; k < m; ++k) {
        const Size idx = k*n;
        for (Size i=1; i < n-1; ++i)
            for (Size j=0; j < n*m; ++j)
                BOOST_CHECK(std::fabs(fm(idx + i, j) - dm(idx + i, j)) < 1e-12);
    }

    fm = FirstDerivativeOp(1, mc).toMatrix();
    dm = NthOrderDerivativeOp(1, 1, 3, mc).toMatrix();

    for (Size i=n; i < n*(m-1); ++i)
        for (Size j=0; j < n*m; ++j)
            BOOST_CHECK(std::fabs(fm(i, j) - dm(i, j)) < 1e-12);
}

void NthOrderDerivativeOpTest::testMixedSecondOrder9PointsOnUniformGrid() {
    BOOST_TEST_MESSAGE(
            "Testing nine points mixed second order "
            "derivative operator on a uniform grid...");

    const ext::shared_ptr<Fdm1dMesher> m
        = ext::make_shared<Uniform1dMesher>(0.0, 0.6, 5);

    const ext::shared_ptr<FdmMesher> mc(
        ext::make_shared<FdmMesherComposite>(m, m));

    const SparseMatrix cc =
            prod(NthOrderDerivativeOp(0, 1, 3, mc).toMatrix(),
                 NthOrderDerivativeOp(1, 1, 3, mc).toMatrix());

    const SparseMatrix mm = SecondOrderMixedDerivativeOp(0,1,mc).toMatrix();

    const Size n = m->size();

    for (Size i=1; i < n-1; ++i)
        for (Size j=1; j < n-1; ++j) {
            const Size idx = i*n+j;
            for (Size k=1; k < n-1; ++k)
                for (Size l=1; l < n-1; ++l) {
                    const Size kdx = k*n+l;
                    BOOST_CHECK(std::fabs(mm(idx,kdx) - cc(idx,kdx)) < 1e-12);
                }
        }
}


test_suite* NthOrderDerivativeOpTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("NthOrderDerivativeOp tests");

    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testSparseMatrixApply));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testFirstOrder2PointsApply));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testFirstOrder3PointsOnUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testFirstOrder5PointsOnUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testFirstOrder2PointsOnUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testFirstOrder4PointsOnUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testFirstOrder2PointsOn2DimUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testSecondOrder3PointsNonUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testSecondOrder4PointsNonUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testThirdOrder4PointsUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testHigherOrderAndRichardsonExtrapolation));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testCompareFirstDerivativeOpNonUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testCompareFirstDerivativeOp2dUniformGrid));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testMixedSecondOrder9PointsOnUniformGrid));

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&NthOrderDerivativeOpTest::testHigherOrderHestonOptionPricing));
    }

    return suite;
}

