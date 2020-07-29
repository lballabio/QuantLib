/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

#include <ql/experimental/volatility/zabr.hpp>
#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/errors.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/ode/adaptiverungekutta.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/experimental/finitedifferences/glued1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operatortraits.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmdupire1dop.hpp>
#include <ql/experimental/finitedifferences/fdmzabrop.hpp>
#include <ql/functional.hpp>

using std::pow;

namespace QuantLib {

ZabrModel::ZabrModel(const Real expiryTime, const Real forward,
                     const Real alpha, const Real beta, const Real nu,
                     const Real rho, const Real gamma)
    : expiryTime_(expiryTime), forward_(forward), alpha_(alpha), beta_(beta),
      nu_(nu * std::pow(alpha_, 1.0 - gamma)), rho_(rho), gamma_(gamma) {

    validateSabrParameters(alpha, beta, nu, rho);
    QL_REQUIRE(gamma >= 0.0 /*&& gamma<=1.0*/,
               "gamma must be non negative: " << gamma << " not allowed");
    QL_REQUIRE(forward >= 0.0,
               "forward must be non negative: " << forward << " not allowed");
    QL_REQUIRE(expiryTime > 0.0, "expiry time must be positive: "
                                     << expiryTime << " not allowed");
}

Real ZabrModel::lognormalVolatilityHelper(const Real strike,
                                          const Real x) const {
    if (close(strike, forward_))
        return std::pow(forward_, beta_ - 1.0) * alpha_;
    else
        return std::log(forward_ / strike) / x;
}

Real ZabrModel::lognormalVolatility(const Real strike) const {
    return lognormalVolatility(std::vector<Real>(1, strike))[0];
}

Disposable<std::vector<Real> >
ZabrModel::lognormalVolatility(const std::vector<Real> &strikes) const {
    using namespace ext::placeholders;
    std::vector<Real> x_ = x(strikes);
    std::vector<Real> result(strikes.size());
    std::transform(strikes.begin(), strikes.end(), x_.begin(), result.begin(),
                   ext::bind(&ZabrModel::lognormalVolatilityHelper,
                               this, _1, _2));
    return result;
}

Real ZabrModel::normalVolatilityHelper(const Real strike, const Real x) const {
    if (close(strike, forward_))
        return std::pow(forward_, beta_) * alpha_;
    else
        return (forward_ - strike) / x;
}

Real ZabrModel::normalVolatility(const Real strike) const {
    return normalVolatility(std::vector<Real>(1, strike))[0];
}

Disposable<std::vector<Real> >
ZabrModel::normalVolatility(const std::vector<Real> &strikes) const {
    using namespace ext::placeholders;
    std::vector<Real> x_ = x(strikes);
    std::vector<Real> result(strikes.size());
    std::transform(strikes.begin(), strikes.end(), x_.begin(), result.begin(),
                   ext::bind(&ZabrModel::normalVolatilityHelper, this,
                               _1, _2));
    return result;
}

Real ZabrModel::localVolatilityHelper(const Real f, const Real x) const {
    return alpha_ * std::pow(std::fabs(f), beta_) /
           F(y(f), std::pow(alpha_, gamma_ - 1.0) *
                       x); // TODO optimize this, y is comoputed together
                           // with x already
}

Real ZabrModel::localVolatility(const Real f) const {
    return localVolatility(std::vector<Real>(1, f))[0];
}

Disposable<std::vector<Real> >
ZabrModel::localVolatility(const std::vector<Real> &f) const {
    using namespace ext::placeholders;
    std::vector<Real> x_ = x(f);
    std::vector<Real> result(f.size());
    std::transform(f.begin(), f.end(), x_.begin(), result.begin(),
                   ext::bind(&ZabrModel::localVolatilityHelper, this,
                               _1, _2));
    return result;
}

Real ZabrModel::fdPrice(const Real strike) const {
    return fdPrice(std::vector<Real>(1, strike))[0];
}

Disposable<std::vector<Real> >
ZabrModel::fdPrice(const std::vector<Real> &strikes) const {

    // TODO check strikes to be increasing
    // TODO put these parameters somewhere
    const Real start =
        std::min(0.00001, strikes.front() * 0.5); // lowest strike for grid
    const Real end =
        std::max(0.10, strikes.back() * 1.5); // highest strike for grid
    const Size size = 500;                    // grid points
    const Real density = 0.1; // density for concentrating mesher
    const Size steps =
        (Size)std::ceil(expiryTime_ * 24); // number of steps in dimension t
    const Size dampingSteps = 5;           // thereof damping steps

    // Layout
    std::vector<Size> dim(1, size);
    const ext::shared_ptr<FdmLinearOpLayout> layout(
        new FdmLinearOpLayout(dim));

    // Mesher
    const ext::shared_ptr<Fdm1dMesher> m1(new Concentrating1dMesher(
        start, end, size, std::pair<Real, Real>(forward_, density), true));
    // const ext::shared_ptr<Fdm1dMesher> m1(new
    // Uniform1dMesher(start,end,size));
    // const ext::shared_ptr<Fdm1dMesher> m1a(new
    // Uniform1dMesher(start,0.03,101));
    // const ext::shared_ptr<Fdm1dMesher> m1b(new
    // Uniform1dMesher(0.03,end,100));
    // const ext::shared_ptr<Fdm1dMesher> m1(new Glued1dMesher(*m1a,*m1b));
    const std::vector<ext::shared_ptr<Fdm1dMesher> > meshers(1, m1);
    const ext::shared_ptr<FdmMesher> mesher(
        new FdmMesherComposite(layout, meshers));

    // Boundary conditions
    FdmBoundaryConditionSet boundaries;

    // initial values
    Array rhs(mesher->layout()->size());
    for (FdmLinearOpIterator iter = layout->begin(); iter != layout->end();
         ++iter) {
        Real k = mesher->location(iter, 0);
        rhs[iter.index()] = std::max(forward_ - k, 0.0);
    }

    // local vols (TODO how can we avoid these Array / vector copying?)
    Array k = mesher->locations(0);
    std::vector<Real> kv(k.size());
    std::copy(k.begin(), k.end(), kv.begin());
    std::vector<Real> locVolv = localVolatility(kv);
    Array locVol(locVolv.size());
    std::copy(locVolv.begin(), locVolv.end(), locVol.begin());

    // solver
    ext::shared_ptr<FdmDupire1dOp> map(new FdmDupire1dOp(mesher, locVol));
    FdmBackwardSolver solver(map, boundaries,
                             ext::shared_ptr<FdmStepConditionComposite>(),
                             FdmSchemeDesc::Douglas());
    solver.rollback(rhs, expiryTime_, 0.0, steps, dampingSteps);

    // interpolate solution
    ext::shared_ptr<Interpolation> solution(new CubicInterpolation(
        k.begin(), k.end(), rhs.begin(), CubicInterpolation::Spline, true,
        CubicInterpolation::SecondDerivative, 0.0,
        CubicInterpolation::SecondDerivative, 0.0));
    // ext::shared_ptr<Interpolation> solution(new
    // LinearInterpolation(k.begin(),k.end(),rhs.begin()));
    solution->disableExtrapolation();
    std::vector<Real> result(strikes.size());
    std::transform(strikes.begin(), strikes.end(), result.begin(), *solution);
    return result;
}

Real ZabrModel::fullFdPrice(const Real strike) const {

    // TODO what are good values here, still experimenting with them
    Real eps = 0.01;
    Real scaleFactor = 1.5;
    Real normInvEps = InverseCumulativeNormal()(1.0 - eps);
    Real alphaI = alpha_ * std::pow(forward_, beta_ - 1.0);
    // nu is already standardized within this class ...
    Real v0 = alpha_ * std::exp(-scaleFactor * normInvEps *
                                std::sqrt(expiryTime_) * nu_);
    Real v1 = alpha_ *
              std::exp(scaleFactor * normInvEps * std::sqrt(expiryTime_) * nu_);
    Real f0 = forward_ * std::exp(-scaleFactor * normInvEps *
                                  std::sqrt(expiryTime_) * alphaI);
    Real f1 = forward_ * std::exp(scaleFactor * normInvEps *
                                  std::sqrt(expiryTime_) * alphaI);
    v1 = std::min(v1, 2.0);
    f0 = std::min(strike / 2.0, f0);
    f1 = std::max(strike * 1.5, std::min(f1, std::max(2.0, strike * 1.5)));

    const Size sizef = 100;
    const Size sizev = 100;
    const Size steps = Size(24 * expiryTime_ + 1);
    const Size dampingSteps = 5;
    const Real densityf = 0.1;
    const Real densityv = 0.1;

    QL_REQUIRE(strike >= f0 && strike <= f1,
               "strike (" << strike << ") must be inside pde grid [" << f0
                          << ";" << f1 << "]");

    // Layout
    std::vector<Size> dim;
    dim.push_back(sizef);
    dim.push_back(sizev);
    const ext::shared_ptr<FdmLinearOpLayout> layout(
        new FdmLinearOpLayout(dim));

    // Mesher
    // two concentrating mesher around f and k to get the mesher for f
    const Real x0 = std::min(forward_, strike);
    const Real x1 = std::max(forward_, strike);
    const Size sizefa = std::max<Size>(
        4, (Size)std::ceil(((x0 + x1) / 2.0 - f0) / (f1 - f0) * (Real)sizef));
    const Size sizefb = sizef - sizefa + 1; // common point, so we can spend
    // one more here
    const ext::shared_ptr<Fdm1dMesher> mfa(
        new Concentrating1dMesher(f0, (x0 + x1) / 2.0, sizefa,
                                  std::pair<Real, Real>(x0, densityf), true));
    const ext::shared_ptr<Fdm1dMesher> mfb(
        new Concentrating1dMesher((x0 + x1) / 2.0, f1, sizefb,
                                  std::pair<Real, Real>(x1, densityf), true));
    const ext::shared_ptr<Fdm1dMesher> mf(new Glued1dMesher(*mfa, *mfb));

    // concentraing mesher around f to get the forward mesher
    // const ext::shared_ptr<Fdm1dMesher> mf(new Concentrating1dMesher(
    //     f0, f1, sizef, std::pair<Real, Real>(forward_, densityf), true));

    // Volatility mesher
    const ext::shared_ptr<Fdm1dMesher> mv(new Concentrating1dMesher(
        v0, v1, sizev, std::pair<Real, Real>(alpha_, densityv), true));

    // uniform meshers
    // const ext::shared_ptr<Fdm1dMesher> mf(new
    // Uniform1dMesher(f0,f1,sizef));
    // const ext::shared_ptr<Fdm1dMesher> mv(new
    // Uniform1dMesher(v0,v1,sizev));

    std::vector<ext::shared_ptr<Fdm1dMesher> > meshers;
    meshers.push_back(mf);
    meshers.push_back(mv);
    const ext::shared_ptr<FdmMesher> mesher(
        new FdmMesherComposite(layout, meshers));

    // initial values
    Array rhs(mesher->layout()->size());
    std::vector<Real> f_;
    std::vector<Real> v_;
    for (FdmLinearOpIterator iter = layout->begin(); iter != layout->end();
         ++iter) {
        Real f = mesher->location(iter, 0);
        // Real v = mesher->location(iter, 0);
        rhs[iter.index()] = std::max(f - strike, 0.0);
        if (iter.coordinates()[1] == 0U)
            f_.push_back(mesher->location(iter, 0));
        if (iter.coordinates()[0] == 0U)
            v_.push_back(mesher->location(iter, 1));
    }

    // Boundary conditions
    FdmBoundaryConditionSet boundaries;

    ext::shared_ptr<FdmZabrOp> map(
        new FdmZabrOp(mesher, beta_, nu_, rho_, gamma_));
    FdmBackwardSolver solver(map, boundaries,
                             ext::shared_ptr<FdmStepConditionComposite>(),
                             FdmSchemeDesc::/*CraigSneyd()*/ Hundsdorfer());

    solver.rollback(rhs, expiryTime_, 0.0, steps, dampingSteps);

    // interpolate solution (this is not necessary when using concentrating
    // meshers with required point)
    Matrix result(f_.size(), v_.size());
    for (Size j = 0; j < v_.size(); ++j)
        std::copy(rhs.begin() + j * f_.size(),
                  rhs.begin() + (j + 1) * f_.size(), result.row_begin(j));
    ext::shared_ptr<BicubicSpline> interpolation =
        ext::make_shared<BicubicSpline>(
            f_.begin(), f_.end(), v_.begin(), v_.end(), result);
    interpolation->disableExtrapolation();
    return (*interpolation)(forward_, alpha_);
}

Real ZabrModel::x(const Real strike) const {
    return x(std::vector<Real>(1, strike))[0];
}

Disposable<std::vector<Real> >
ZabrModel::x(const std::vector<Real> &strikes) const {
    using namespace ext::placeholders;

    QL_REQUIRE(strikes[0] > 0.0 || beta_ < 1.0,
               "strikes must be positive (" << strikes[0] << ") if beta = 1");
    for (std::vector<Real>::const_iterator i = strikes.begin() + 1;
         i != strikes.end(); ++i)
        QL_REQUIRE(*i > *(i - 1), "strikes must be strictly ascending ("
                                      << *(i - 1) << "," << *i << ")");

    AdaptiveRungeKutta<Real> rk(1.0E-8, 1.0E-5,
                                0.0); // TODO move the parameters here as
                                      // parameters with default values to
                                      // the constructor
    std::vector<Real> y(strikes.size()), result(strikes.size());
    std::transform(strikes.rbegin(), strikes.rend(), y.begin(),
                   ext::bind(&ZabrModel::y, this, _1));

    if (close(gamma_, 1.0)) {
        for (Size m = 0; m < y.size(); m++) {
            Real J = std::sqrt(1.0 + nu_ * nu_ * y[m] * y[m] -
                               2.0 * rho_ * nu_ * y[m]);
            result[y.size() - 1 - m] =
                std::log((J + nu_ * y[m] - rho_) / (1.0 - rho_)) / nu_;
        }
    } else {
        Size ynz = std::upper_bound(y.begin(), y.end(), 0.0) - y.begin();
        if (ynz > 0)
            if (close(y[ynz - 1], 0.0))
                ynz--;
        if (ynz == y.size())
            ynz--;

        for (int dir = 1; dir >= -1; dir -= 2) {
            Real y0 = 0.0, u0 = 0.0;
            for (int m = ynz + (dir == -1 ? -1 : 0);
                 dir == -1 ? m >= 0 : m < (int)y.size(); m += dir) {
                Real u = rk(ext::bind(&ZabrModel::F, this, _1, _2),
                            u0, y0, y[m]);
                result[y.size() - 1 - m] = u * pow(alpha_, 1.0 - gamma_);
                u0 = u;
                y0 = y[m];
            }
        }
    }

    return result;
}

Real ZabrModel::y(const Real strike) const {

    if (close(beta_, 1.0)) {
        return std::log(forward_ / strike) * std::pow(alpha_, gamma_ - 2.0);
    } else {
        return (strike < 0.0
                    ? std::pow(forward_, 1.0 - beta_) +
                          std::pow(-strike, 1.0 - beta_)
                    : std::pow(forward_, 1.0 - beta_) -
                          std::pow(strike, 1.0 - beta_)) *
               std::pow(alpha_, gamma_ - 2.0) / (1.0 - beta_);
    }
}

Real ZabrModel::F(const Real y, const Real u) const {
    Real A = 1.0 + (gamma_ - 2.0) * (gamma_ - 2.0) * nu_ * nu_ * y * y +
             2.0 * rho_ * (gamma_ - 2.0) * nu_ * y;
    Real B = 2.0 * rho_ * (1.0 - gamma_) * nu_ +
             2.0 * (1.0 - gamma_) * (gamma_ - 2.0) * nu_ * nu_ * y;
    Real C = (1.0 - gamma_) * (1.0 - gamma_) * nu_ * nu_;
    return (-B * u + std::sqrt(B * B * u * u - 4.0 * A * (C * u * u - 1.0))) /
           (2.0 * A);
}
}
