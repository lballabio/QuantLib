/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2009, 2010, 2015 Klaus Spanderen

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

#include "fdmlinearop.hpp"
#include "utilities.hpp"

#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/hybridhestonhullwhiteprocess.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/mchestonhullwhiteengine.hpp>
#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/math/matrixutilities/gmres.hpp>
#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/methods/finitedifferences/schemes/douglasscheme.hpp>
#include <ql/methods/finitedifferences/schemes/hundsdorferscheme.hpp>
#include <ql/methods/finitedifferences/schemes/craigsneydscheme.hpp>
#include <ql/methods/finitedifferences/meshers/uniformgridmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/operators/fdmblackscholesop.hpp>
#include <ql/methods/finitedifferences/utilities/fdmmesherintegral.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/operators/numericaldifferentiation.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmhestonhullwhiteop.hpp>
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmhestonop.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhestonsolver.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/solvers/fdmndimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdm3dimsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmamericanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdividendhandler.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <ql/math/matrixutilities/sparseilupreconditioner.hpp>
#include <ql/functional.hpp>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/operation.hpp>

#include <numeric>
#include <utility>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    class FdmHestonExpressCondition : public StepCondition<Array> {
      public:
        FdmHestonExpressCondition(std::vector<Real> redemptions,
                                  std::vector<Real> triggerLevels,
                                  std::vector<Time> exerciseTimes,
                                  ext::shared_ptr<FdmMesher> mesher)
        : redemptions_(std::move(redemptions)), triggerLevels_(std::move(triggerLevels)),
          exerciseTimes_(std::move(exerciseTimes)), mesher_(std::move(mesher)) {}

        void applyTo(Array& a, Time t) const override {
            auto iter = std::find(exerciseTimes_.begin(), exerciseTimes_.end(), t);

            if (iter != exerciseTimes_.end()) {
                Size index = std::distance(exerciseTimes_.begin(), iter);

               ext::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
                const FdmLinearOpIterator endIter = layout->end();
                for (FdmLinearOpIterator iter = layout->begin();
                     iter != endIter; ++iter) {
                    const Real s = std::exp(mesher_->location(iter, 0));

                    if (s > triggerLevels_[index]) {
                        a[iter.index()] = redemptions_[index];
                    }
                }
            }
        }

      private:
        const std::vector<Real> redemptions_;
        const std::vector<Real> triggerLevels_;
        const std::vector<Time> exerciseTimes_;
        const ext::shared_ptr<FdmMesher> mesher_;
    };

    class ExpressPayoff : public Payoff {
      public:
        std::string name() const override { return "ExpressPayoff"; }
        std::string description() const override { return "ExpressPayoff"; }

        Real operator()(Real s) const override {
            return  ((s >= 100.0) ? 108.0 : 100.0)
                  - ((s <= 75.0) ? Real(100.0 - s) : 0.0);
        }
    };

    template <class T, class U, class V>
    struct multiplies {
        V operator()(T t, U u) { return t*u;}
    };

}

void FdmLinearOpTest::testFdmLinearOpLayout() {

    BOOST_TEST_MESSAGE("Testing indexing of a linear operator...");

    const std::vector<Size> dim = {5,7,8};

    FdmLinearOpLayout layout = FdmLinearOpLayout(dim);

    Size calculatedDim = layout.dim().size();
    Size expectedDim = dim.size();
    if (calculatedDim != expectedDim) {
        BOOST_ERROR("index.dimensions() should be " << expectedDim
                    << ", but is " << calculatedDim);
    }

    Size calculatedSize = layout.size();
    Size expectedSize = std::accumulate(dim.begin(), dim.end(),
                                        1, std::multiplies<Size>());

    if (calculatedSize != expectedSize) {
        BOOST_FAIL("index.size() should be "
                    << expectedSize << ", but is " << calculatedSize);
    }

    for (Size k=0; k < dim[0]; ++k) {
        for (Size l=0; l < dim[1]; ++l) {
            for (Size m=0; m < dim[2]; ++m) {
                std::vector<Size> tmp(3);
                tmp[0] = k; tmp[1] = l; tmp[2] = m;

                Size calculatedIndex = layout.index(tmp);
                Size expectedIndex = k + l*dim[0] + m*dim[0]*dim[1];

                if (expectedIndex != layout.index(tmp)) {
                    BOOST_FAIL("index.size() should be " << expectedIndex
                                <<", but is " << calculatedIndex);
                }
            }
        }
    }

    FdmLinearOpIterator iter = layout.begin();

    for (Size m=0; m < dim[2]; ++m) {
        for (Size l=0; l < dim[1]; ++l) {
            for (Size k=0; k < dim[0]; ++k, ++iter) {
                for (Size n=1; n < 4; ++n) {
                    Size nn = layout.neighbourhood(iter, 1, n);
                    Size calculatedIndex = k + m*dim[0]*dim[1]
                       + ((l < dim[1]-n)? l+n
                                        : dim[1]-1-(l+n-(dim[1]-1)))*dim[0];

                    if (nn != calculatedIndex) {
                        BOOST_FAIL("next neighbourhood index is " << nn
                                    << " but should be " << calculatedIndex);
                    }
                }

                for (Size n=1; n < 7; ++n) {
                    Size nn = layout.neighbourhood(iter, 2, -Integer(n));
                    Size calculatedIndex = k + l*dim[0]
                            + ((m < n) ? n-m : m-n)*dim[0]*dim[1];
                    if (nn != calculatedIndex) {
                        BOOST_FAIL("next neighbourhood index is " << nn
                            << " but should be " << calculatedIndex);
                    }
                }
            }
        }
    }
}

void FdmLinearOpTest::testUniformGridMesher() {

    BOOST_TEST_MESSAGE("Testing uniform grid mesher...");

    const std::vector<Size> dim = {5,7,8};

    ext::shared_ptr<FdmLinearOpLayout> layout(new FdmLinearOpLayout(dim));
    std::vector<std::pair<Real, Real> > boundaries = {{-5, 10}, {5, 100}, {10, 20}};

    UniformGridMesher mesher(layout, boundaries);

    const Real dx1 = 15.0/(dim[0]-1);
    const Real dx2 = 95.0/(dim[1]-1);
    const Real dx3 = 10.0/(dim[2]-1);

    constexpr double tol = 100*QL_EPSILON;
    if (   std::fabs(dx1-mesher.dminus(layout->begin(),0)) > tol
        || std::fabs(dx1-mesher.dplus(layout->begin(),0)) > tol
        || std::fabs(dx2-mesher.dminus(layout->begin(),1)) > tol
        || std::fabs(dx2-mesher.dplus(layout->begin(),1)) > tol
        || std::fabs(dx3-mesher.dminus(layout->begin(),2)) > tol
        || std::fabs(dx3-mesher.dplus(layout->begin(),2)) > tol ) {
        BOOST_FAIL("inconsistent uniform mesher object");
    }
}

void FdmLinearOpTest::testFirstDerivativesMapApply() {

    BOOST_TEST_MESSAGE("Testing application of first-derivatives map...");

    const std::vector<Size> dim = {400, 100, 50};

    ext::shared_ptr<FdmLinearOpLayout> index(new FdmLinearOpLayout(dim));

    std::vector<std::pair<Real, Real> > boundaries = {{-5, 5}, {0, 10}, { 5, 15}};

    ext::shared_ptr<FdmMesher> mesher(
                                 new UniformGridMesher(index, boundaries));

    FirstDerivativeOp map(2, mesher);

    Array r(mesher->layout()->size());
    const FdmLinearOpIterator endIter = index->end();

    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        r[iter.index()] =  std::sin(mesher->location(iter, 0))
                         + std::cos(mesher->location(iter, 2));
    }

    Array t = map.apply(r);
    const Real dz = (boundaries[2].second-boundaries[2].first)/(dim[2]-1);
    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Size z = iter.coordinates()[2];

        const Size z0 = (z > 0) ? z-1 : 1;
        const Size z2 = (z < dim[2]-1) ? z+1 : dim[2]-2;
        const Real lz0 = boundaries[2].first + z0*dz;
        const Real lz2 = boundaries[2].first + z2*dz;

        Real expected;
        if (z == 0) {
            expected = (std::cos(boundaries[2].first+dz)
                        - std::cos(boundaries[2].first))/dz;
        }
        else if (z == dim[2]-1) {
            expected = (std::cos(boundaries[2].second)
                        - std::cos(boundaries[2].second-dz))/dz;
        }
        else {
            expected = (std::cos(lz2)-std::cos(lz0))/(2*dz);
        }

        const Real calculated = t[iter.index()];
        if (std::fabs(calculated - expected) > 1e-10) {
            BOOST_FAIL("first derivative calculation failed."
                        << "\n    calculated: " << calculated
                        << "\n    expected:   " << expected);
        }
    }


}

void FdmLinearOpTest::testSecondDerivativesMapApply() {

    BOOST_TEST_MESSAGE("Testing application of second-derivatives map...");

    const std::vector<Size> dim = {50, 50, 50};

    ext::shared_ptr<FdmLinearOpLayout> index(new FdmLinearOpLayout(dim));

    std::vector<std::pair<Real, Real> > boundaries = {{0, 0.5}, {0, 0.5}, {0, 0.5}};

    ext::shared_ptr<FdmMesher> mesher(
                            new UniformGridMesher(index, boundaries));
    Array r(mesher->layout()->size());
    const FdmLinearOpIterator endIter = index->end();

    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        r[iter.index()] = std::sin(x)*std::cos(y)*std::exp(z);
    }

    Array t = SecondDerivativeOp(0, mesher).apply(r);

    const Real tol = 5e-2;
    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Size i = iter.index();
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        Real d = -std::sin(x)*std::cos(y)*std::exp(z);
        if (iter.coordinates()[0] == 0 || iter.coordinates()[0] == dim[0]-1) {
            d = 0;
        }

        if (std::fabs(d - t[i]) > tol) {
            BOOST_FAIL("numerical derivative in dx^2 deviation is too big"
                << "\n  found at " << x << " " << y << " " << z);
        }
    }

    t = SecondDerivativeOp(1, mesher).apply(r);
    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Size i = iter.index();
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        Real d = -std::sin(x)*std::cos(y)*std::exp(z);
        if (iter.coordinates()[1] == 0 || iter.coordinates()[1] == dim[1]-1) {
            d = 0;
        }

        if (std::fabs(d - t[i]) > tol) {
            BOOST_FAIL("numerical derivative in dy^2 deviation is too big"
                << "\n  found at " << x << " " << y << " " << z);
        }
    }

    t = SecondDerivativeOp(2, mesher).apply(r);
    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Size i = iter.index();
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        Real d = std::sin(x)*std::cos(y)*std::exp(z);
        if (iter.coordinates()[2] == 0 || iter.coordinates()[2] == dim[2]-1) {
            d = 0;
        }

        if (std::fabs(d - t[i]) > tol) {
            BOOST_FAIL("numerical derivative in dz^2 deviation is too big"
                << "\n  found at " << x << " " << y << " " << z);
        }
    }


}

void FdmLinearOpTest::testDerivativeWeightsOnNonUniformGrids() {
    BOOST_TEST_MESSAGE("Testing finite differences coefficients...");

    const ext::shared_ptr<Fdm1dMesher> mesherX(
        new Concentrating1dMesher(-2.0, 3.0, 50, std::make_pair(0.5, 0.01)));
    const ext::shared_ptr<Fdm1dMesher> mesherY(
        new Concentrating1dMesher(0.5, 5.0, 25, std::make_pair(0.5, 0.1)));
    const ext::shared_ptr<Fdm1dMesher> mesherZ(
        new Concentrating1dMesher(-1.0, 2.0, 31, std::make_pair(1.5, 0.01)));

    const ext::shared_ptr<FdmMesher> meshers(
        new FdmMesherComposite(mesherX, mesherY, mesherZ));

    const ext::shared_ptr<FdmLinearOpLayout> layout = meshers->layout();
    const FdmLinearOpIterator endIter = layout->end();

    const Real tol = 1e-13;
    for (Size direction=0; direction < 3; ++direction) {

        const SparseMatrix dfdx
            = FirstDerivativeOp(direction, meshers).toMatrix();
        const SparseMatrix d2fdx2
            = SecondDerivativeOp(direction, meshers).toMatrix();

        const Array gridPoints = meshers->locations(direction);

        for (FdmLinearOpIterator iter=layout->begin();
            iter != endIter; ++iter) {

            const Size c = iter.coordinates()[direction];
            const Size index   = iter.index();
            const Size indexM1 = layout->neighbourhood(iter,direction,-1);
            const Size indexP1 = layout->neighbourhood(iter,direction,+1);

            // test only if not on the boundary
            if (c == 0) {
                Array twoPoints(2);
                twoPoints[0] = 0.0;
                twoPoints[1] = gridPoints.at(indexP1)-gridPoints.at(index);

                const Array ndWeights1st = NumericalDifferentiation(
                    ext::function<Real(Real)>(), 1 , twoPoints).weights();

                const Real beta1  = dfdx(index, index);
                const Real gamma1 = dfdx(index, indexP1);
                if (   std::fabs((beta1  - ndWeights1st.at(0))/beta1) > tol
                    || std::fabs((gamma1 - ndWeights1st.at(1))/gamma1) > tol) {
                    BOOST_FAIL("can not reproduce the weights of the "
                               "first order derivative operator "
                               "on the lower boundary"
                            << "\n expected beta:    " << ndWeights1st.at(0)
                            << "\n calculated beta:  " << beta1
                            << "\n difference beta:  "
                            << beta1 - ndWeights1st.at(0)
                            << "\n expected gamma:   " << ndWeights1st.at(1)
                            << "\n calculated gamma: " << gamma1
                            << "\n difference gamma: "
                            << gamma1 - ndWeights1st.at(1));
                }

                // free boundary condition by default
                const Real beta2  = d2fdx2(index, index);
                const Real gamma2 = d2fdx2(index, indexP1);

                if (   std::fabs(beta2)  > QL_EPSILON
                    || std::fabs(gamma2) > QL_EPSILON) {
                    BOOST_FAIL("can not reproduce the weights of the "
                               "second order derivative operator "
                               "on the lower boundary"
                            << "\n expected beta:    " << 0.0
                            << "\n calculated beta:  " << beta2
                            << "\n expected gamma:   " << 0.0
                            << "\n calculated gamma: " << gamma2);
                }
            }
            else if (c == layout->dim()[direction]-1) {
                Array twoPoints(2);
                twoPoints[0] = gridPoints.at(indexM1)-gridPoints.at(index);
                twoPoints[1] = 0.0;

                const Array ndWeights1st = NumericalDifferentiation(
                    ext::function<Real(Real)>(), 1 , twoPoints).weights();

                const Real alpha1 = dfdx(index, indexM1);
                const Real beta1  = dfdx(index, index);
                if (   std::fabs((alpha1 - ndWeights1st.at(0))/alpha1) > tol
                    || std::fabs((beta1  - ndWeights1st.at(1))/beta1) > tol) {
                    BOOST_FAIL("can not reproduce the weights of the "
                               "first order derivative operator "
                               "on the upper boundary"
                            << "\n expected alpha:   " << ndWeights1st.at(0)
                            << "\n calculated alpha: " << alpha1
                            << "\n difference alpha: "
                            << alpha1 - ndWeights1st.at(0)
                            << "\n expected beta:    " << ndWeights1st.at(1)
                            << "\n calculated beta:  " << beta1
                            << "\n difference beta:  "
                            << beta1 - ndWeights1st.at(1));
                }

                // free boundary condition by default
                const Real alpha2 = d2fdx2(index, indexM1);
                const Real beta2  = d2fdx2(index, index);

                if (   std::fabs(alpha2)  > QL_EPSILON
                    || std::fabs(beta2) > QL_EPSILON) {
                    BOOST_FAIL("can not reproduce the weights of the "
                               "second order derivative operator "
                               "on the upper boundary"
                            << "\n expected alpha:   " << 0.0
                            << "\n calculated alpha: " << alpha2
                            << "\n expected beta:    " << 0.0
                            << "\n calculated beta:  " << beta2);
                }
            }
            else {
                Array threePoints(3);
                threePoints[0] = gridPoints.at(indexM1)-gridPoints.at(index);
                threePoints[1] = 0.0;
                threePoints[2] = gridPoints.at(indexP1)-gridPoints.at(index);

                const Array ndWeights1st = NumericalDifferentiation(
                    ext::function<Real(Real)>(), 1 , threePoints).weights();

                const Real alpha1 = dfdx(index, indexM1);
                const Real beta1  = dfdx(index, index);
                const Real gamma1 = dfdx(index, indexP1);

                if (   std::fabs((alpha1 - ndWeights1st.at(0))/alpha1) > tol
                    || std::fabs((beta1  - ndWeights1st.at(1))/beta1) > tol
                    || std::fabs((gamma1 - ndWeights1st.at(2))/gamma1) > tol) {
                    BOOST_FAIL("can not reproduce the weights of the "
                               "first order derivative operator"
                            << "\n expected alpha:   " << ndWeights1st.at(0)
                            << "\n calculated alpha: " << alpha1
                            << "\n difference alpha: "
                            << alpha1 - ndWeights1st.at(0)
                            << "\n expected beta:    " << ndWeights1st.at(1)
                            << "\n calculated beta:  " << beta1
                            << "\n difference beta:  "
                            << beta1 - ndWeights1st.at(1)
                            << "\n expected gamma:   " << ndWeights1st.at(2)
                            << "\n calculated gamma: " << gamma1
                            << "\n difference gamma: "
                            << gamma1 - ndWeights1st.at(2));
                }

                const Array ndWeights2nd = NumericalDifferentiation(
                    ext::function<Real(Real)>(), 2 , threePoints).weights();

                const Real alpha2 = d2fdx2(index, indexM1);
                const Real beta2  = d2fdx2(index, index);
                const Real gamma2 = d2fdx2(index, indexP1);
                if (   std::fabs((alpha2 - ndWeights2nd.at(0))/alpha2) > tol
                    || std::fabs((beta2  - ndWeights2nd.at(1))/beta2) > tol
                    || std::fabs((gamma2 - ndWeights2nd.at(2))/gamma2) > tol) {
                    BOOST_FAIL("can not reproduce the weights of the "
                               "second order derivative operator"
                            << "\n expected alpha:   " << ndWeights2nd.at(0)
                            << "\n calculated alpha: " << alpha2
                            << "\n difference alpha: "
                            << alpha2 - ndWeights2nd.at(0)
                            << "\n expected beta:    " << ndWeights2nd.at(1)
                            << "\n calculated beta:  " << beta2
                            << "\n difference beta:  "
                            << beta2 - ndWeights2nd.at(1)
                            << "\n expected gamma:   " << ndWeights2nd.at(2)
                            << "\n calculated gamma: " << gamma2
                            << "\n difference gamma: "
                            << gamma2 - ndWeights2nd.at(2));
                }
            }
        }
    }
}

void FdmLinearOpTest::testSecondOrderMixedDerivativesMapApply() {

    BOOST_TEST_MESSAGE(
        "Testing application of second-order mixed-derivatives map...");

    const std::vector<Size> dim = {50, 50, 50};

    ext::shared_ptr<FdmLinearOpLayout> index(new FdmLinearOpLayout(dim));

    std::vector<std::pair<Real, Real> > boundaries = {{0, 0.5}, {0, 0.5}, {0, 0.5}};

    ext::shared_ptr<FdmMesher> mesher(
        new UniformGridMesher(index, boundaries));

    Array r(mesher->layout()->size());
    const FdmLinearOpIterator endIter = index->end();

    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        r[iter.index()] = std::sin(x)*std::cos(y)*std::exp(z);
    }

    Array t = SecondOrderMixedDerivativeOp(0, 1, mesher).apply(r);
    Array u = SecondOrderMixedDerivativeOp(1, 0, mesher).apply(r);

    const Real tol = 5e-2;
    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Size i = iter.index();
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        const Real d = -std::cos(x)*std::sin(y)*std::exp(z);

        if (std::fabs(d - t[i]) > tol) {
            BOOST_FAIL("numerical derivative in dxdy deviation is too big"
                        << "\n  found at " << x << " " << y << " " << z);
        }

        if (std::fabs(t[i]-u[i]) > 1e5*QL_EPSILON) {
            BOOST_FAIL("numerical derivative in dxdy not equal to dydx"
                    << "\n  found at " << x << " " << y << " " << z
                    << "\n  value    " << std::fabs(t[i]-u[i]));
        }
    }

    t = SecondOrderMixedDerivativeOp(0, 2, mesher).apply(r);
    u = SecondOrderMixedDerivativeOp(2, 0, mesher).apply(r);
    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Size i = iter.index();
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        const Real d = std::cos(x)*std::cos(y)*std::exp(z);

        if (std::fabs(d - t[i]) > tol) {
            BOOST_FAIL("numerical derivative in dxdy deviation is too big"
                << "\n  found at " << x << " " << y << " " << z);
        }

        if (std::fabs(t[i]-u[i]) > 1e5*QL_EPSILON) {
            BOOST_FAIL("numerical derivative in dxdz not equal to dzdx"
                << "\n  found at " << x << " " << y << " " << z
                << "\n  value    " << std::fabs(t[i]-u[i]));
        }
    }

    t = SecondOrderMixedDerivativeOp(1, 2, mesher).apply(r);
    u = SecondOrderMixedDerivativeOp(2, 1, mesher).apply(r);
    for (FdmLinearOpIterator iter = index->begin(); iter != endIter; ++iter) {
        const Size i = iter.index();
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        const Real d = -std::sin(x)*std::sin(y)*std::exp(z);

        if (std::fabs(d - t[i]) > tol) {
            BOOST_FAIL("numerical derivative in dydz deviation is too big"
                << "\n  found at " << x << " " << y << " " << z);
        }

        if (std::fabs(t[i]-u[i]) > 1e5*QL_EPSILON) {
            BOOST_FAIL("numerical derivative in dydz not equal to dzdy"
                << "\n  found at " << x << " " << y << " " << z
                << "\n  value    " << std::fabs(t[i]-u[i]));
        }
    }


}

void FdmLinearOpTest::testTripleBandMapSolve() {

    BOOST_TEST_MESSAGE("Testing triple-band map solution...");

    const std::vector<Size> dim = {100, 400};

    ext::shared_ptr<FdmLinearOpLayout> layout(new FdmLinearOpLayout(dim));

    std::vector<std::pair<Real, Real> > boundaries = {{0, 1.0}, {0, 1.0}};

    ext::shared_ptr<FdmMesher> mesher(
        new UniformGridMesher(layout, boundaries));

    FirstDerivativeOp dy(1, mesher);
    dy.axpyb(Array(1, 2.0), dy, dy, Array(1, 1.0));

    // check copy constructor
    FirstDerivativeOp copyOfDy(dy);

    Array u(layout->size());
    for (Size i=0; i < layout->size(); ++i)
        u[i] = std::sin(0.1*i)+std::cos(0.35*i);

    Array t(dy.solve_splitting(copyOfDy.apply(u), 1.0, 0.0));
    for (Size i=0; i < u.size(); ++i) {
        if (std::fabs(u[i] - t[i]) > 1e-6) {
            BOOST_FAIL("solve and apply are not consistent "
                    << "\n expected      : " << u[i]
                    << "\n calculated    : " << t[i]);
        }
    }

    FirstDerivativeOp dx(0, mesher);
    dx.axpyb(Array(), dx, dx, Array(1, 1.0));

    FirstDerivativeOp copyOfDx(0, mesher);
    // check assignment
    copyOfDx = dx;

    t = dx.solve_splitting(copyOfDx.apply(u), 1.0, 0.0);
    for (Size i=0; i < u.size(); ++i) {
        if (std::fabs(u[i] - t[i]) > 1e-6) {
            BOOST_FAIL("solve and apply are not consistent "
                << "\n expected      : " << u[i]
                << "\n calculated    : " << t[i]);
        }
    }

    SecondDerivativeOp dxx(0, mesher);
    dxx.axpyb(Array(1, 0.5), dxx, dx, Array(1, 1.0));

    // check of copy constructor
    SecondDerivativeOp copyOfDxx(dxx);

    t = dxx.solve_splitting(copyOfDxx.apply(u), 1.0, 0.0);

    for (Size i=0; i < u.size(); ++i) {
        if (std::fabs(u[i] - t[i]) > 1e-6) {
            BOOST_FAIL("solve and apply are not consistent "
                << "\n expected      : " << u[i]
                << "\n calculated    : " << t[i]);
        }
    }

    //check assignment operator
    copyOfDxx.add(SecondDerivativeOp(1, mesher));
    copyOfDxx = dxx;

    t = dxx.solve_splitting(copyOfDxx.apply(u), 1.0, 0.0);

    for (Size i=0; i < u.size(); ++i) {
        if (std::fabs(u[i] - t[i]) > 1e-6) {
            BOOST_FAIL("solve and apply are not consistent "
                << "\n expected      : " << u[i]
                << "\n calculated    : " << t[i]);
        }
    }
}


void FdmLinearOpTest::testFdmHestonBarrier() {

    BOOST_TEST_MESSAGE("Testing FDM with barrier option in Heston model...");

    SavedSettings backup;

    const std::vector<Size> dim = {200, 100};

    ext::shared_ptr<FdmLinearOpLayout> index(new FdmLinearOpLayout(dim));

    std::vector<std::pair<Real, Real> > boundaries = {{3.8, 4.905274778}, {0.0, 1.0}};

    ext::shared_ptr<FdmMesher> mesher(
        new UniformGridMesher(index, boundaries));

    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    ext::shared_ptr<HestonProcess> hestonProcess(
        new HestonProcess(rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(28, March, 2005);

    ext::shared_ptr<FdmLinearOpComposite> hestonOp(
                                   new FdmHestonOp(mesher, hestonProcess));

    Array rhs(mesher->layout()->size());
    const FdmLinearOpIterator endIter = mesher->layout()->end();
    for (FdmLinearOpIterator iter = mesher->layout()->begin();
         iter != endIter; ++iter) {
        rhs[iter.index()]=std::max(std::exp(mesher->location(iter,0))-100, 0.0);
    }

    FdmBoundaryConditionSet bcSet = {
        ext::make_shared<FdmDirichletBoundary>(mesher, 0.0, 0,
                                               FdmDirichletBoundary::Upper)
    };

    const Real theta=0.5+std::sqrt(3.0)/6.;
    HundsdorferScheme hsEvolver(theta, 0.5, hestonOp, bcSet);
    FiniteDifferenceModel<HundsdorferScheme> hsModel(hsEvolver);
    hsModel.rollback(rhs, 1.0, 0.0, 50);

    Matrix ret(dim[0], dim[1]);
    for (Size i=0; i < dim[0]; ++i)
        for (Size j=0; j < dim[1]; ++j)
            ret[i][j] = rhs[i+j*dim[0]];

    std::vector<Real> tx, ty;
    for (FdmLinearOpIterator iter = mesher->layout()->begin();
        iter != endIter; ++iter) {
            if (iter.coordinates()[1] == 0) {
                tx.push_back(mesher->location(iter, 0));
            }
            if (iter.coordinates()[0] == 0) {
                ty.push_back(mesher->location(iter, 1));
            }
    }

    BilinearInterpolation interpolate(ty.begin(), ty.end(),
                                      tx.begin(), tx.end(), ret);

    const Real x = 100;
    const Real v0 = 0.04;

    const Real npv = interpolate(v0, std::log(x));
    const Real delta = 0.5*(
            interpolate(v0, std::log(x+1))-interpolate(v0, std::log(x-1)));
    const Real gamma =   interpolate(v0, std::log(x+1))
                      +  interpolate(v0, std::log(x-1)) - 2*npv;

    const Real npvExpected   = 9.049016;
    const Real deltaExpected = 0.511285;
    const Real gammaExpected = -0.034296;

    if (std::fabs(npv - npvExpected) > 0.000001) {
        BOOST_FAIL("Error in calculating PV for Heston barrier option");
    }

    if (std::fabs(delta - deltaExpected) > 0.000001) {
        BOOST_FAIL("Error in calculating Delta for Heston barrier option");
    }

    if (std::fabs(gamma - gammaExpected) > 0.000001) {
            BOOST_FAIL("Error in calculating Gamma for Heston barrier option");
    }
}

void FdmLinearOpTest::testFdmHestonAmerican() {

    BOOST_TEST_MESSAGE("Testing FDM with American option in Heston model...");

    SavedSettings backup;

    const std::vector<Size> dim = {200, 100};

    ext::shared_ptr<FdmLinearOpLayout> index(new FdmLinearOpLayout(dim));

    std::vector<std::pair<Real, Real> > boundaries = {{3.8, std::log(220.0)}, {0.0, 1.0}};

    ext::shared_ptr<FdmMesher> mesher(
        new UniformGridMesher(index, boundaries));

    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    ext::shared_ptr<HestonProcess> hestonProcess(
        new HestonProcess(rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(28, March, 2005);

    ext::shared_ptr<FdmLinearOpComposite> LinearOp(
        new FdmHestonOp(mesher, hestonProcess));

    ext::shared_ptr<Payoff> payoff(new PlainVanillaPayoff(Option::Put, 100.0));
    Array rhs(mesher->layout()->size());
    const FdmLinearOpIterator endIter = mesher->layout()->end();
    for (FdmLinearOpIterator iter = mesher->layout()->begin();
        iter != endIter; ++iter) {
            rhs[iter.index()]
                = payoff->operator ()(std::exp(mesher->location(iter, 0)));
    }

    FdmAmericanStepCondition condition(mesher,
        ext::shared_ptr<FdmInnerValueCalculator>(
                                     new FdmLogInnerValue(payoff, mesher, 0)));
    const Real theta=0.5+std::sqrt(3.0)/6.;
    HundsdorferScheme hsEvolver(theta, 0.5, LinearOp);
    FiniteDifferenceModel<HundsdorferScheme> hsModel(hsEvolver);
    hsModel.rollback(rhs, 1.0, 0.0, 50, condition);

    Matrix ret(dim[0], dim[1]);
    for (Size i=0; i < dim[0]; ++i)
        for (Size j=0; j < dim[1]; ++j)
            ret[i][j] = rhs[i+j*dim[0]];

    std::vector<Real> tx, ty;
    for (FdmLinearOpIterator iter = mesher->layout()->begin();
        iter != endIter; ++iter) {
            if (iter.coordinates()[1] == 0) {
                tx.push_back(mesher->location(iter, 0));
            }
            if (iter.coordinates()[0] == 0) {
                ty.push_back(mesher->location(iter, 1));
            }
    }

    BilinearInterpolation interpolate(ty.begin(), ty.end(),
                                      tx.begin(), tx.end(), ret);

    const Real x = 100;
    const Real v0 = 0.04;

    const Real npv = interpolate(v0, std::log(x));
    const Real npvExpected = 5.641648;

    if (std::fabs(npv - npvExpected) > 0.000001) {
        BOOST_FAIL("Error in calculating PV for Heston American Option");
    }
}

void FdmLinearOpTest::testFdmHestonExpress() {

    BOOST_TEST_MESSAGE("Testing FDM with express certificate in Heston model...");

    SavedSettings backup;

    const std::vector<Size> dim = {200, 100};

    ext::shared_ptr<FdmLinearOpLayout> index(new FdmLinearOpLayout(dim));

    std::vector<std::pair<Real, Real> > boundaries = {{3.8, std::log(220.0)}, {0.0, 1.0}};

    ext::shared_ptr<FdmMesher> mesher(
                            new UniformGridMesher(index, boundaries));

    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    Handle<HestonProcess> hestonProcess(ext::make_shared<HestonProcess> (
        rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8));

    const Date exerciseDate(28, March, 2005);
    const Date evaluationDate(28, March, 2004);
    Settings::instance().evaluationDate() = evaluationDate;

    std::vector<Real> triggerLevels(2);
    triggerLevels[0] = triggerLevels[1] = 100.0;
    std::vector<Real> redemptions(2);
    redemptions[0] = redemptions[1] = 108.0;
    std::vector<Time> exerciseTimes(2);
    exerciseTimes[0] = 0.333; exerciseTimes[1] = 0.666;

    DividendSchedule dividendSchedule(1, ext::shared_ptr<Dividend>(
        new FixedDividend(2.5, evaluationDate + Period(6, Months))));
    ext::shared_ptr<FdmDividendHandler> dividendCondition(
        new FdmDividendHandler(dividendSchedule, mesher,
                               rTS->referenceDate(),
                               rTS->dayCounter(), 0));

    ext::shared_ptr<StepCondition<Array> > expressCondition(
        new FdmHestonExpressCondition(redemptions, triggerLevels,
                                      exerciseTimes, mesher));

    std::list<std::vector<Time>> stoppingTimes = {
        exerciseTimes, dividendCondition->dividendTimes()
    };

    std::list<ext::shared_ptr<StepCondition<Array>>> conditions = {
        expressCondition, dividendCondition
    };

    ext::shared_ptr<FdmStepConditionComposite> condition(
        new FdmStepConditionComposite(stoppingTimes, conditions));

    ext::shared_ptr<Payoff> payoff(new ExpressPayoff());

    ext::shared_ptr<FdmInnerValueCalculator> calculator(
                                    new FdmLogInnerValue(payoff, mesher, 0));

    const FdmBoundaryConditionSet bcSet;
    const FdmSolverDesc solverDesc = { mesher, bcSet,
                                       condition, calculator, 1.0, 50, 0 };
    FdmHestonSolver solver(hestonProcess, solverDesc);

    const Real s = s0->value();
    const Real v0 = 0.04;

    if (std::fabs(solver.valueAt(s, v0) - 101.027) > 0.01) {
        BOOST_FAIL("Error in calculating PV for Heston Express Certificate");
    }

    if (std::fabs(solver.deltaAt(s, v0) - 0.4181) > 0.001) {
        BOOST_FAIL("Error in calculating Delta for Heston Express Certificate");
    }

    if (std::fabs(solver.gammaAt(s, v0) + 0.0400) > 0.001) {
        BOOST_FAIL("Error in calculating Gamma for Heston Express Certificate");
    }

    if (std::fabs(solver.meanVarianceDeltaAt(s, v0) - 0.6602) > 0.001) {
        BOOST_FAIL("Error in calculating mean variance Delta for "
                "Heston Express Certificate");
    }

    if (std::fabs(solver.meanVarianceGammaAt(s, v0) + 0.0316) > 0.001) {
        BOOST_FAIL("Error in calculating mean variance Delta for "
                "Heston Express Certificate");
    }
}


namespace {

    ext::shared_ptr<HybridHestonHullWhiteProcess> createHestonHullWhite(
        Time maturity) {

        DayCounter dc = Actual365Fixed();
        const Date today = Settings::instance().evaluationDate();
        Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

        std::vector<Date> dates;
        std::vector<Rate> rates, divRates;

        for (Size i=0; i <= 25; ++i) {
            dates.push_back(today+Period(i, Years));
            rates.push_back(0.05);
            divRates.push_back(0.02);
        }

        const Handle<YieldTermStructure> rTS(
           ext::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
        const Handle<YieldTermStructure> qTS(
           ext::shared_ptr<YieldTermStructure>(
                                              new ZeroCurve(dates, divRates, dc)));

        const Real v0 = 0.04;
        ext::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, s0, v0, 1.0, v0*0.75, 0.4, -0.7));

        ext::shared_ptr<HullWhiteForwardProcess> hwFwdProcess(
                            new HullWhiteForwardProcess(rTS, 0.00883, 0.01));
        hwFwdProcess->setForwardMeasureTime(maturity);

        const Real equityShortRateCorr = -0.7;

        return ext::make_shared<HybridHestonHullWhiteProcess>(
                hestonProcess, hwFwdProcess,
                                                 equityShortRateCorr);
    }

    FdmSolverDesc createSolverDesc(
        const std::vector<Size>& dim,
        const ext::shared_ptr<HybridHestonHullWhiteProcess>& process) {

        const Time maturity
                    = process->hullWhiteProcess()->getForwardMeasureTime();

        ext::shared_ptr<FdmLinearOpLayout> layout(new FdmLinearOpLayout(dim));

        std::vector<ext::shared_ptr<Fdm1dMesher> > mesher1d = {
            ext::shared_ptr<Fdm1dMesher>(
                new Uniform1dMesher(std::log(22.0), std::log(440.0), dim[0])),
            ext::shared_ptr<Fdm1dMesher>(
                new FdmHestonVarianceMesher(dim[1], process->hestonProcess(),
                                            maturity)),
            ext::shared_ptr<Fdm1dMesher>(
                new Uniform1dMesher(-0.15, 0.15, dim[2]))
        };

        const ext::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(mesher1d));

        const FdmBoundaryConditionSet boundaries;

        std::list<std::vector<Time> > stoppingTimes;
        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;

        ext::shared_ptr<FdmStepConditionComposite> conditions(
            new FdmStepConditionComposite(
                                     std::list<std::vector<Time> >(),
                                     FdmStepConditionComposite::Conditions()));

        ext::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Call, 160.0));

        ext::shared_ptr<FdmInnerValueCalculator> calculator(
                                       new FdmLogInnerValue(payoff, mesher, 0));

        const Size tGrid = 100;
        const Size dampingSteps = 0;

        FdmSolverDesc desc = { mesher, boundaries,
                               conditions, calculator,
                               maturity, tGrid, dampingSteps };

        return desc;
    }
}

void FdmLinearOpTest::testFdmHestonHullWhiteOp() {
    BOOST_TEST_MESSAGE("Testing FDM with Heston Hull-White model...");

    SavedSettings backup;

    const Date today = Date(28, March, 2004);
    Settings::instance().evaluationDate() = today;

    Date exerciseDate(28, March, 2012);
    const Time maturity = Actual365Fixed().yearFraction(today, exerciseDate);

    const std::vector<Size> dim = {51, 31, 31};

    ext::shared_ptr<HybridHestonHullWhiteProcess> jointProcess
                                            = createHestonHullWhite(maturity);
    FdmSolverDesc desc = createSolverDesc(dim, jointProcess);
    ext::shared_ptr<FdmMesher> mesher = desc.mesher;

    ext::shared_ptr<HullWhiteForwardProcess> hwFwdProcess
                                            = jointProcess->hullWhiteProcess();

    ext::shared_ptr<HullWhiteProcess> hwProcess(
        new HullWhiteProcess(jointProcess->hestonProcess()->riskFreeRate(),
                             hwFwdProcess->a(), hwFwdProcess->sigma()));

    ext::shared_ptr<FdmLinearOpComposite> linearOp(
        new FdmHestonHullWhiteOp(mesher,
                                 jointProcess->hestonProcess(),
                                 hwProcess,
                                 jointProcess->eta()));

    Array rhs(mesher->layout()->size());
    const FdmLinearOpIterator endIter = mesher->layout()->end();
    for (FdmLinearOpIterator iter = mesher->layout()->begin();
        iter != endIter; ++iter) {
            rhs[iter.index()] = desc.calculator->avgInnerValue(iter, maturity);
    }

    const Real theta = 0.5+std::sqrt(3.0)/6.;
    HundsdorferScheme hsEvolver(theta, 0.5, linearOp);
    FiniteDifferenceModel<HundsdorferScheme> hsModel(hsEvolver);

    hsModel.rollback(rhs, maturity, 0.0, desc.timeSteps);

    std::vector<Real> tx, ty, tr, y;
    for (FdmLinearOpIterator iter = mesher->layout()->begin();
        iter != endIter; ++iter) {
            if (iter.coordinates()[1] == 0 && iter.coordinates()[2] == 0) {
                tx.push_back(mesher->location(iter, 0));
            }
            if (iter.coordinates()[0] == 0 && iter.coordinates()[2] == 0) {
                ty.push_back(mesher->location(iter, 1));
            }
            if (iter.coordinates()[0] == 0 && iter.coordinates()[1] == 0) {
                tr.push_back(mesher->location(iter, 2));
            }
    }

    const Real x0 = 100;
    const Real v0 = jointProcess->hestonProcess()->v0();
    const Real r0 = 0;
    for (Size k=0; k < dim[2]; ++k) {
        Matrix ret(dim[0], dim[1]);
        for (Size i=0; i < dim[0]; ++i)
            for (Size j=0; j < dim[1]; ++j)
                ret[i][j] = rhs[ i+j*dim[0]+k*dim[0]*dim[1] ];

        y.push_back(BicubicSpline(ty.begin(), ty.end(),
                                  tx.begin(), tx.end(), ret)(v0, std::log(x0)));
    }

    const Real directCalc
        = MonotonicCubicNaturalSpline(tr.begin(), tr.end(), y.begin())(r0);

    std::vector<Real> x(3);
    x[0] = std::log(x0); x[1] = v0; x[2] = r0;

    Fdm3DimSolver solver3d(desc, FdmSchemeDesc::Hundsdorfer(), linearOp);
    const Real solverCalc = solver3d.interpolateAt(x[0], x[1], x[2]);
    const Real solverTheta = solver3d.thetaAt(x[0], x[1], x[2]);

    if (std::fabs(directCalc - solverCalc) > 1e-4) {
        BOOST_FAIL("Error in calculating PV for Heston Hull White Option");
    }


    FdmNdimSolver<3> solverNd(desc, FdmSchemeDesc::Hundsdorfer(), linearOp);
    const Real solverNdCalc = solverNd.interpolateAt(x);
    const Real solverNdTheta = solverNd.thetaAt(x);

    if (std::fabs(solverNdCalc - solverCalc) > 1e-4) {
        BOOST_FAIL("Error in calculating PV for Heston Hull White Option");
    }
    if (std::fabs(solverNdTheta - solverTheta) > 1e-4) {
        BOOST_FAIL("Error in calculating PV for Heston Hull White Option");
    }

    VanillaOption option(
            ext::shared_ptr<StrikedTypePayoff>(
                                new PlainVanillaPayoff(Option::Call, 160.0)),
            ext::shared_ptr<Exercise>(new EuropeanExercise(exerciseDate)));

    const Real tol = 0.025;
    option.setPricingEngine(
            MakeMCHestonHullWhiteEngine<PseudoRandom>(jointProcess)
            .withSteps(200)
            .withAntitheticVariate()
            .withControlVariate()
            .withAbsoluteTolerance(tol)
            .withSeed(42));

    // the following takes far too long
    // const Real expected = option.NPV();
    // use precalculated value instead
    const Real expected = 4.73;

    if (std::fabs(directCalc - expected) > 3*tol) {
        BOOST_FAIL("Error in calculating MC PV for Heston Hull White Option");
    }
}

namespace {
    Array axpy(const boost::numeric::ublas::compressed_matrix<Real>& A,
               const Array& x) {
        
        boost::numeric::ublas::vector<Real> tmpX(x.size()), y(x.size());
        std::copy(x.begin(), x.end(), tmpX.begin());
        boost::numeric::ublas::axpy_prod(A, tmpX, y);

        return Array(y.begin(), y.end());
    }

    boost::numeric::ublas::compressed_matrix<Real> createTestMatrix(
        Size n, Size m, Real theta) {

        boost::numeric::ublas::compressed_matrix<Real> a(n*m, n*m);

        for (Size i=0; i < n; ++i) {
            for (Size j=0; j < m; ++j) {
                const Size k = i*m+j;
                a(k,k)=1.0;

                if (i > 0 && j > 0 && i <n-1 && j < m-1) {
                    const Size im1 = i-1;
                    const Size ip1 = i+1;
                    const Size jm1 = j-1;
                    const Size jp1 = j+1;
                    const Real delta = theta/((ip1-im1)*(jp1-jm1));

                    a(k,im1*m+jm1) =  delta;
                    a(k,im1*m+jp1) = -delta;
                    a(k,ip1*m+jm1) = -delta;
                    a(k,ip1*m+jp1) =  delta;
                }
            }
        }

        return a;
    }
}

void FdmLinearOpTest::testBiCGstab() {
    BOOST_TEST_MESSAGE(
        "Testing bi-conjugated gradient stabilized algorithm...");

    const Size n=41, m=21;
    const Real theta = 1.0;
    const boost::numeric::ublas::compressed_matrix<Real> a
        = createTestMatrix(n, m, theta);

    const ext::function<Array(const Array&)> matmult
        = [&](const Array& _x) { return axpy(a, _x); };

    SparseILUPreconditioner ilu(a, 4);
    ext::function<Array(const Array&)> precond
        = [&](const Array& _x) { return ilu.apply(_x); };

    Array b(n*m);
    MersenneTwisterUniformRng rng(1234);
    for (Real& i : b) {
        i = rng.next().value;
    }

    const Real tol = 1e-10;

    const BiCGstab biCGstab(matmult, n*m, tol, precond);
    const Array x = biCGstab.solve(b).x;

    const Real error = std::sqrt(DotProduct(b-axpy(a, x),
                                 b-axpy(a, x))/DotProduct(b,b));

    if (error > tol) {
        BOOST_FAIL("Error calculating the inverse using BiCGstab" <<
                "\n tolerance:  " << tol <<
                "\n error:      " << error);
    }
}

void FdmLinearOpTest::testGMRES() {
    BOOST_TEST_MESSAGE("Testing GMRES algorithm...");

    const Size n=41, m=21;
    const Real theta = 1.0;
    const boost::numeric::ublas::compressed_matrix<Real> a
        = createTestMatrix(n, m, theta);

    const ext::function<Array(const Array&)> matmult
        = [&](const Array& _x) { return axpy(a, _x); };
    
    SparseILUPreconditioner ilu(a, 4);
    ext::function<Array(const Array&)> precond
        = [&](const Array& _x) { return ilu.apply(_x); };
    
    Array b(n*m);
    MersenneTwisterUniformRng rng(1234);
    for (Real& i : b) {
        i = rng.next().value;
    }

    const Real tol = 1e-10;

    const GMRES gmres(matmult, n*m, tol, precond);
    const GMRESResult result = gmres.solve(b, b);
    const Array x = result.x;
    const Real errorCalculated = result.errors.back();

    const Real error = std::sqrt(DotProduct(b-axpy(a, x), 
                                 b-axpy(a, x))/DotProduct(b,b));

    if (error > tol) {
        BOOST_FAIL("Error calculating the inverse using GMRES" <<
                "\n tolerance:  " << tol <<
                "\n error:      " << error);
    }

    if (std::fabs(error - errorCalculated) > 10*QL_EPSILON) {
        BOOST_FAIL("Calculation if the error in GMRES went wrong" <<
                "\n calculated: " << errorCalculated <<
                "\n error:      " << error);

    }

    const GMRES gmresRestart(matmult, 5, tol, precond);
    const GMRESResult resultRestart = gmresRestart.solveWithRestart(5, b, b);
    const Real errorWithRestart = resultRestart.errors.back();

    if (errorWithRestart > tol) {
        BOOST_FAIL("Error calculating the inverse using "
                "GMRES with restarts" <<
                "\n tolerance:  " << tol <<
                "\n error:      " << errorWithRestart);
    }
}

void FdmLinearOpTest::testCrankNicolsonWithDamping() {

    BOOST_TEST_MESSAGE("Testing Crank-Nicolson with initial implicit damping steps "
                       "for a digital option...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, 0.06, dc);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, 0.06, dc);
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, 0.35, dc);

    ext::shared_ptr<StrikedTypePayoff> payoff(
                             new CashOrNothingPayoff(Option::Put, 100, 10.0));

    Time maturity = 0.75;
    Date exDate = today + timeToDays(maturity);
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    ext::shared_ptr<BlackScholesMertonProcess> process(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));
    ext::shared_ptr<PricingEngine> engine(
                                new AnalyticEuropeanEngine(process));

    VanillaOption opt(payoff, exercise);
    opt.setPricingEngine(engine);
    Real expectedPV = opt.NPV();
    Real expectedGamma = opt.gamma();

    // fd pricing using implicit damping steps and Crank Nicolson
    const Size csSteps = 25, dampingSteps = 3, xGrid = 400;
    const std::vector<Size> dim(1, xGrid);

    ext::shared_ptr<FdmLinearOpLayout> layout(new FdmLinearOpLayout(dim));
    const ext::shared_ptr<Fdm1dMesher> equityMesher(
        new FdmBlackScholesMesher(
                dim[0], process, maturity, payoff->strike(),
                Null<Real>(), Null<Real>(), 0.0001, 1.5,
                std::pair<Real, Real>(payoff->strike(), 0.01)));

    const ext::shared_ptr<FdmMesher> mesher (
        new FdmMesherComposite(equityMesher));

    ext::shared_ptr<FdmBlackScholesOp> map(
                     new FdmBlackScholesOp(mesher, process, payoff->strike()));

    ext::shared_ptr<FdmInnerValueCalculator> calculator(
                                  new FdmLogInnerValue(payoff, mesher, 0));

    Array rhs(layout->size()), x(layout->size());
    const FdmLinearOpIterator endIter = layout->end();

    for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
         ++iter) {
        rhs[iter.index()] = calculator->avgInnerValue(iter, maturity);
        x[iter.index()] = mesher->location(iter, 0);
    }

    FdmBackwardSolver solver(map, FdmBoundaryConditionSet(), 
                             ext::shared_ptr<FdmStepConditionComposite>(),
                             FdmSchemeDesc::Douglas());
    solver.rollback(rhs, maturity, 0.0, csSteps, dampingSteps);

    MonotonicCubicNaturalSpline spline(x.begin(), x.end(), rhs.begin());

    Real s = spot->value();
    Real calculatedPV = spline(std::log(s));
    Real calculatedGamma = (spline.secondDerivative(std::log(s))
                            - spline.derivative(std::log(s))    )/(s*s);

    Real relTol = 2e-3;

    if (std::fabs(calculatedPV - expectedPV) > relTol*expectedPV) {
        BOOST_FAIL("Error calculating the PV of the digital option" <<
                "\n rel. tolerance:  " << relTol <<
                "\n expected:        " << expectedPV <<
                "\n calculated:      " << calculatedPV);
    }
    if (std::fabs(calculatedGamma - expectedGamma) > relTol*expectedGamma) {
        BOOST_FAIL("Error calculating the Gamma of the digital option" <<
                "\n rel. tolerance:  " << relTol <<
                "\n expected:        " << expectedGamma <<
                "\n calculated:      " << calculatedGamma);
    }
}

void FdmLinearOpTest::testSpareMatrixReference() {
    BOOST_TEST_MESSAGE("Testing SparseMatrixReference type...");

    const Size rows    = 10;
    const Size columns = 10;
    const Size nMatrices = 5;
    const Size nElements = 50;

    PseudoRandom::urng_type rng(1234UL);

    SparseMatrix expected(rows, columns);
    std::vector<SparseMatrix> v(nMatrices, SparseMatrix(rows, columns));
    std::vector<SparseMatrixReference> refs;

    for (auto& i : v) {
        SparseMatrixReference m(i);
        for (Size j=0; j < nElements; ++j) {
            const Size row    = Size(rng.next().value*rows);
            const Size column = Size(rng.next().value*columns);

            const Real value = rng.next().value;
            m(row, column)        += value;
            expected(row, column) += value;
        }

        refs.push_back(m);
    }

    SparseMatrix calculated = std::accumulate(refs.begin()+1, refs.end(),
                                              SparseMatrix(refs.front()));

    for (Size i=0; i < rows; ++i) {
        for (Size j=0; j < columns; ++j) {
            if (std::fabs(Real(calculated(i,j)) - Real(expected(i,j))) > 100*QL_EPSILON) {
                BOOST_FAIL("Error using sparse matrix references in " <<
                           "Element (" << i << ", " << j << ")" <<
                        "\n expected  : " << Real(expected(i, j)) <<
                        "\n calculated: " << Real(calculated(i, j)));
            }
        }
    }
}

namespace {

    Size nrElementsOfSparseMatrix(const SparseMatrix& m) {
        Size retVal = 0;
        for (SparseMatrix::const_iterator1 i1 = m.begin1();
            i1 != m.end1(); ++i1) {
            retVal+=std::distance(i1.begin(), i1.end());
        }
        return retVal;
    }

}

void FdmLinearOpTest::testSparseMatrixZeroAssignment() {
    BOOST_TEST_MESSAGE("Testing assignment to zero in sparse matrix...");

    SparseMatrix m(5,5);
    if (nrElementsOfSparseMatrix(m) != 0U) {
        BOOST_FAIL("non zero return for an emtpy matrix");
    }
    m(0, 0) = 0.0; m(1, 2) = 0.0;
    if (nrElementsOfSparseMatrix(m) != 2) {
        BOOST_FAIL("two elements expected");
    }
    m(1, 3) = 1.0;
    if (nrElementsOfSparseMatrix(m) != 3) {
        BOOST_FAIL("three elements expected");
    }
    m(1, 3) = 0.0;
    if (nrElementsOfSparseMatrix(m) != 3) {
        BOOST_FAIL("three elements expected");
    }
}

void FdmLinearOpTest::testFdmMesherIntegral() {
    BOOST_TEST_MESSAGE("Testing integrals over meshers functions...");

    const ext::shared_ptr<FdmMesherComposite> mesher(
        new FdmMesherComposite(
            ext::shared_ptr<Fdm1dMesher>(new Concentrating1dMesher(
                -1, 1.6, 21, std::pair<Real, Real>(0, 0.1))),
            ext::shared_ptr<Fdm1dMesher>(new Concentrating1dMesher(
                -3, 4, 11, std::pair<Real, Real>(1, 0.01))),
            ext::shared_ptr<Fdm1dMesher>(new Concentrating1dMesher(
                -2, 1, 5, std::pair<Real, Real>(0.5, 0.1)))));

    const ext::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();

    Array f(mesher->layout()->size());
    for (FdmLinearOpIterator iter = layout->begin();
        iter != layout->end(); ++iter) {
        const Real x = mesher->location(iter, 0);
        const Real y = mesher->location(iter, 1);
        const Real z = mesher->location(iter, 2);

        f[iter.index()] = x*x + 3*y*y - 3*z*z
                        + 2*x*y - x*z - 3*y*z
                        + 4*x - y - 3*z + 2 ;
    }

    const Real tol = 1e-12;

    // Simpson's rule has to be exact here, Mathematica code gives
    // Integrate[x*x+3*y*y-3*z*z+2*x*y-x*z-3*y*z+4*x-y-3*z+2,
    //           {x, -1, 16/10}, {y, -3, 4}, {z, -2, 1}]
    const Real expectedSimpson = 876.512;
    const Real calculatedSimpson
        = FdmMesherIntegral(mesher, DiscreteSimpsonIntegral()).integrate(f);

    if (std::fabs(calculatedSimpson - expectedSimpson) > tol*expectedSimpson) {
        BOOST_FAIL(std::setprecision(16)
            << "discrete mesher integration using Simpson's rule failed: "
            << "\n    calculated: " << calculatedSimpson
            << "\n    expected:   " << expectedSimpson);
    }

    const Real expectedTrapezoid = 917.0148209153263;
    const Real calculatedTrapezoid
        = FdmMesherIntegral(mesher, DiscreteTrapezoidIntegral()).integrate(f);

    if (std::fabs(calculatedTrapezoid - expectedTrapezoid)
            > tol*expectedTrapezoid) {
        BOOST_FAIL(std::setprecision(16)
            << "discrete mesher integration using Trapezoid rule failed: "
            << "\n    calculated: " << calculatedTrapezoid
            << "\n    expected:   " << expectedTrapezoid);
    }
}

void FdmLinearOpTest::testHighInterestRateBlackScholesMesher() {
    BOOST_TEST_MESSAGE("Testing Black-Scholes mesher in a "
            "high interest rate scenario...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(11, February, 2018);

    const Real spot = 100;
    const Rate r = 0.21;
    const Rate q = 0.02;
    const Volatility v = 0.25;

    const ext::shared_ptr<GeneralizedBlackScholesProcess> process =
        ext::make_shared<GeneralizedBlackScholesProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(spot)),
            Handle<YieldTermStructure>(flatRate(today, q, dc)),
            Handle<YieldTermStructure>(flatRate(today, r, dc)),
            Handle<BlackVolTermStructure>(flatVol(today, v, dc)));

    const Size size = 10;
    const Time maturity = 2.0;
    const Real strike = 100;
    const Real eps = 0.05;
    const Real normInvEps = 1.64485363;
    const Real scaleFactor = 2.5;

    const std::vector<Real> loc = FdmBlackScholesMesher(
        size, process, maturity, strike,
        Null<Real>(), Null<Real>(), eps, scaleFactor).locations();

    const Real calculatedMin = std::exp(loc.front());
    const Real calculatedMax = std::exp(loc.back());

    const Real minimum = spot
        * std::exp(-normInvEps*scaleFactor*v*std::sqrt(maturity));
    const Real maximum = spot
        / process->riskFreeRate()->discount(maturity)
        * process->dividendYield()->discount(maturity)
        * std::exp( normInvEps*scaleFactor*v*std::sqrt(maturity));

    const Real relTol = 1e-7;

    const Real maxDiff = std::fabs(calculatedMax - maximum);
    if (maxDiff > relTol*maximum) {
        BOOST_FAIL("Upper bound for Black-Scholes mesher failed: "
            << "\n    calculated: " << calculatedMax
            << "\n    expected:   " << maximum
            << std::scientific
            << "\n    difference: " << maxDiff
            << "\n    tolerance:  " << relTol*maximum);
    }

    const Real minDiff = std::fabs(calculatedMin - minimum);
    if (minDiff > relTol*minimum) {
        BOOST_FAIL("Lower bound for Black-Scholes mesher failed: "
            << "\n    calculated: " << calculatedMin
            << "\n    expected:   " << minimum
            << std::scientific
            << "\n    difference: " << minDiff
            << "\n    tolerance:  " << relTol*minimum);
    }
}

void FdmLinearOpTest::testLowVolatilityHighDiscreteDividendBlackScholesMesher() {
    BOOST_TEST_MESSAGE("Testing Black-Scholes mesher in a low volatility and "
            "high discrete dividend scenario...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(28, January, 2018);

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));
    const Handle<YieldTermStructure> qTS(flatRate(today, 0.07, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, 0.16, dc));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, 0.0, dc));

    const ext::shared_ptr<GeneralizedBlackScholesProcess> process =
        ext::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, volTS);

    const Date firstDivDate = today + Period(7, Months);
    const Real firstDivAmount = 10.0;
    const Date secondDivDate = today + Period(11, Months);
    const Real secondDivAmount = 5.0;

    DividendSchedule divSchedule = {
        ext::make_shared<FixedDividend>(firstDivAmount, firstDivDate),
        ext::make_shared<FixedDividend>(secondDivAmount, secondDivDate)
    };

    const Size size = 5;
    const Time maturity = 1.0;
    const Real strike = 100;
    const Real eps = 0.0001;
    const Real scaleFactor = 1.5;

    const std::vector<Real> loc = FdmBlackScholesMesher(
        size,
        process,
        maturity, strike,
        Null<Real>(), Null<Real>(),
        eps, scaleFactor,
        std::make_pair(Null<Real>(), Null<Real>()),
        divSchedule).locations();

    const Real maximum = spot->value() *
        qTS->discount(firstDivDate)/rTS->discount(firstDivDate);

    const Real minimum = (1 - firstDivAmount
        /(spot->value()*qTS->discount(firstDivDate)/rTS->discount(firstDivDate)))
        * spot->value()*qTS->discount(secondDivDate)/rTS->discount(secondDivDate)
         - secondDivAmount;

    const Real calculatedMax = std::exp(loc.back());
    const Real calculatedMin = std::exp(loc.front());


    constexpr double relTol = 1e5*QL_EPSILON;

    const Real maxDiff = std::fabs(calculatedMax - maximum);
    if (maxDiff > relTol*maximum) {
        BOOST_FAIL("Upper bound for Black-Scholes mesher failed: "
            << "\n    calculated: " << calculatedMax
            << "\n    expected:   " << maximum
            << "\n    difference: " << maxDiff
            << "\n    tolerance:  " << relTol*maximum);
    }

    const Real minDiff = std::fabs(calculatedMin - minimum);
    if (minDiff > relTol*minimum) {
        BOOST_FAIL("Lower bound for Black-Scholes mesher failed: "
            << "\n    calculated: " << calculatedMin
            << "\n    expected:   " << minimum
            << "\n    difference: " << minDiff
            << "\n    tolerance:  " << relTol*minimum);
    }
}

test_suite* FdmLinearOpTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("linear operator tests");

    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testFdmLinearOpLayout));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testUniformGridMesher));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testFirstDerivativesMapApply));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testSecondDerivativesMapApply));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testDerivativeWeightsOnNonUniformGrids));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testSecondOrderMixedDerivativesMapApply));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testTripleBandMapSolve));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testFdmHestonBarrier));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testFdmHestonAmerican));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testFdmHestonExpress));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testBiCGstab));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testGMRES));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testCrankNicolsonWithDamping));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testSpareMatrixReference));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testSparseMatrixZeroAssignment));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testFdmMesherIntegral));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testHighInterestRateBlackScholesMesher));
    suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testLowVolatilityHighDiscreteDividendBlackScholesMesher));

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&FdmLinearOpTest::testFdmHestonHullWhiteOp));
    }

    return suite;
}
