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

#include "utilities.hpp"
#include "nthorderderivativeop.hpp"

#include <ql/math/comparison.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/initializers.hpp>
#include <ql/math/richardsonextrapolation.hpp>
#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/operators/nthorderderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/functional.hpp>
#include <numeric>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#ifndef QL_NO_UBLAS_SUPPORT

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
        FdmSchemeDesc scheme;
    };

    class FdmHeatEquationOp : public FdmLinearOpComposite {
      public:
        FdmHeatEquationOp(
            Size nPoints,
            Volatility vol,
            const ext::shared_ptr<FdmMesher>& mesher,
            Size direction = 0)
        : vol2_(0.5*vol*vol),
          direction_(direction),
          map_(ext::make_shared<NthOrderDerivativeOp>(
              direction, 2, nPoints, mesher)),
          preconditioner_(SecondDerivativeOp(direction, mesher)
              .mult(Array(mesher->layout()->size(), vol2_))) { }

        Size size() const { return 1; }
        void setTime(Time t1, Time t2) { }

        Disposable<Array> apply(const Array& r) const {
            return vol2_*map_->apply(r);
        }

        Disposable<Array> apply_mixed(const Array& r) const {
            Array retVal(r.size(), 0.0);
            return retVal;
        }

        Disposable<Array> apply_direction(
            Size direction, const Array& r) const {
            if (direction == direction_)
                return apply(r);
            else
                return apply_mixed(r);
        }

        Disposable<Array> solve_splitting(
            Size direction, const Array& r, Real dt) const {

            using namespace ext::placeholders;
            if (direction == direction_) {
                BiCGStabResult result =
                    QuantLib::BiCGstab(
                        ext::function<Disposable<Array>(const Array&)>(
                            ext::bind(
                                &FdmHeatEquationOp::solve_apply,
                                this, _1, -dt)),
                        std::max(Size(10), r.size()), 1e-14,
                        ext::function<Disposable<Array>(const Array&)>(
                            ext::bind(&FdmLinearOpComposite::preconditioner,
                                        this, _1, dt))
                    ).solve(r, r);

                return result.x;
            }
            else {
                Array retVal(r);
                return retVal;
            }
        }

        Disposable<Array> preconditioner(const Array& r, Real dt) const  {
            return preconditioner_.solve_splitting(r, dt, 1.0);
        }

      private:
        Disposable<Array> solve_apply(const Array& r, Real dt) const {
            return r - dt*apply(r);
        }


        const Volatility vol2_;
        const Size direction_;
        const ext::shared_ptr<FdmLinearOp> map_;
        const TripleBandLinearOp preconditioner_;
    };


    class AvgPayoffFct {
      public:
        AvgPayoffFct(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                     Volatility vol, Time T, Real growthFactor)
        : payoff_(payoff),
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


    Disposable<Array> priceReport(
        const GridSetup& setup, const Array& strikes) {

        const Date today(2, May, 2018);
        const DayCounter dc = Actual365Fixed();
        const Date maturity = today + Period(1, Years);
        const Time T = dc.yearFraction(today, maturity);

        const ext::shared_ptr<YieldTermStructure> rTS
            = flatRate(today, 0.05, dc);
        const ext::shared_ptr<YieldTermStructure> qTS
            = flatRate(today, 0.025, dc);

        const Real S = 100.0;

        const Volatility vol = 0.2;
        const Real stdDev = vol * std::sqrt(T);
        const DiscountFactor df
            = qTS->discount(maturity)/rTS->discount(maturity);

        const Real y = std::log(S);

        const Real ymin = y - setup.alpha*stdDev;
        const Real ymax = y + setup.alpha*stdDev;

        const Size yGrid = setup.yGrid;

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

                    for (Size l = 0; l < loc.size(); ++l)
                        loc[l] += offset;

                    break;
                }

            const ext::shared_ptr<FdmMesher> mesher =
                ext::make_shared<FdmMesherComposite>(
                    ext::make_shared<Predefined1dMesher>(loc));

            const ext::shared_ptr<FdmLinearOpLayout> layout =
                mesher->layout();

            const Array g = mesher->locations(0);
            const Array sT = Exp(g - 0.5*vol*vol*T)*df;

            Array rhs(setup.yGrid);

            const ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::make_shared<PlainVanillaPayoff>(Option::Call, strike);

            rhs[0] = (*payoff)(sT[0]);
            rhs[yGrid-1] = (*payoff)(sT[yGrid-1]);

            for (Size j=1; j < yGrid-1; ++j)
                if (setup.cellAvg && (
                         (sT[j] < strike && sT[j+1] >= strike)
                      || (sT[j-1] < strike && sT[j] >= strike))) {

                    const Real gMin = 0.5*(g[j-1]+g[j]);
                    const Real gMax = 0.5*(g[j+1]+g[j]);

                    const AvgPayoffFct f(payoff, vol, T, df);

                    rhs[j] = GaussLobattoIntegral(1000, 1e-12)(
                        f, gMin, gMax)/(gMax - gMin);
                }
                else
                    rhs[j] = (*payoff)(sT[j]);

            const ext::shared_ptr<FdmHeatEquationOp> heatEqn =
                ext::make_shared<FdmHeatEquationOp>(
                    setup.nPoints, vol, mesher);

            FdmBackwardSolver solver(
              heatEqn,
              FdmBoundaryConditionSet(),
              ext::shared_ptr<FdmStepConditionComposite>(),
              setup.scheme);

            solver.rollback(rhs, T, 0.0, setup.tGrid, 0);

            rhs *= rTS->discount(maturity);

            const Real fdmPrice = MonotonicCubicNaturalSpline(
                g.begin(), g.end(), rhs.begin())(y);

            const Real npv =
               blackFormula(payoff, S*df, stdDev, rTS->discount(maturity));

            diffs[k] = npv-fdmPrice;
        }

        return diffs;
    }

    class FdmMispricingCostFunction : public CostFunction {
      public:
        FdmMispricingCostFunction(
            const GridSetup& setup, const Array& strikes)
        : setup_(setup), strikes_(strikes) { }

        Disposable<Array> values(const Array& x) const {
            const GridSetup g = {
                x[0], x[1],
                setup_.cellAvg, setup_.midPoint,
                setup_.nPoints, setup_.tGrid, setup_.yGrid, setup_.scheme
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

void NthOrderDerivativeOpTest::testHigerOrderBSOptionPricing() {
    BOOST_TEST_MESSAGE("Testing Black-Scholes option pricing convergence with "
            "higher order finite difference operators...");

    SavedSettings backup;

    Array strikes(8);
    strikes << 50, 75, 90, 100, 110, 125, 150, 200;

    const GridSetup initSetup = {
        5.2, 0.1, true, false, 5, 31, 51, FdmSchemeDesc::Douglas()
    };

    Array initialValues(2);
    initialValues << initSetup.alpha, initSetup.density;

    FdmMispricingCostFunction costFct(initSetup, strikes);
    NoConstraint noConstraint;

    Problem p(costFct, noConstraint, initialValues);

    LevenbergMarquardt().minimize(
        p, EndCriteria(400, 40, 1.0e-6, 1.0e-6, 1.0e-6));

    const GridSetup optimalSetup = {
        p.currentValue()[0], p.currentValue()[1],
        initSetup.cellAvg, initSetup.midPoint,
        initSetup.nPoints,
        initSetup.tGrid, initSetup.yGrid/2,
        initSetup.scheme
    };

    const Array q = priceReport(optimalSetup, strikes);
    const Real ac = std::sqrt(DotProduct(q,q)/q.size());

    const Real convergence = std::log(ac/p.functionValue())*M_LOG2E;

    if (convergence < 3.6) {
        BOOST_ERROR("convergence order is too low"
                << "\n expected convergence: 4.0"
                << "\n measured convergence: " << convergence
                << "\n tolerance           : 0.4");
    }
}


namespace {
    Real priceQuality(Real h) {

        Array strikes(1);
        strikes << 100;

        const Size yGrid = Size(1/h);
        const GridSetup setup = {
             5.20966, 0.00130581,
             true, false, 5, 801, yGrid, FdmSchemeDesc::Douglas()
        };

        return  std::fabs(priceReport(setup, strikes)[0]);
    }
}

void NthOrderDerivativeOpTest::testHigerOrderAndRichardsonExtrapolationg() {
    BOOST_TEST_MESSAGE(
            "Testing Black-Scholes option pricing convergence with "
            "higher order FDM operators and Richardson Extrapolation...");

    SavedSettings backup;

    const Real n1 = priceQuality(1.0/50);
    const Real n3
        = std::fabs(RichardsonExtrapolation(priceQuality, 1.0/50, 4.0)(2.0));

    const Real r2 = std::log(n1/n3)*M_LOG2E;

    if (r2 < 4.9) {
        BOOST_ERROR("convergence order is too low using Richardson extrapolation"
                << "\n expected convergence: 5.0"
                << "\n measured convergence: " << r2
                << "\n tolerance           : 0.1");
    }
}

#endif

test_suite* NthOrderDerivativeOpTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("NthOrderDerivativeOp tests");

#ifndef QL_NO_UBLAS_SUPPORT

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
        &NthOrderDerivativeOpTest::testHigerOrderBSOptionPricing));
    suite->add(QUANTLIB_TEST_CASE(
        &NthOrderDerivativeOpTest::testHigerOrderAndRichardsonExtrapolationg));

#endif

    return suite;
}

