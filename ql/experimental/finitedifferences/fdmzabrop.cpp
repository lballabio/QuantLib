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

#include <ql/experimental/finitedifferences/fdmzabrop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>

namespace QuantLib {

FdmZabrUnderlyingPart::FdmZabrUnderlyingPart(
    const std::shared_ptr<FdmMesher> &mesher, const Real beta, const Real nu,
    const Real rho, const Real gamma)
    : volatilityValues_(mesher->locations(1)),
      forwardValues_(mesher->locations(0)),
      mapT_(SecondDerivativeOp(0, mesher)
                .mult(0.5 * volatilityValues_ * volatilityValues_ *
                      Pow(forwardValues_, 2.0 * beta))),
      mesher_(mesher) {}

void FdmZabrUnderlyingPart::setTime(Time t1, Time t2) {}

const TripleBandLinearOp &FdmZabrUnderlyingPart::getMap() const {
    return mapT_;
}

FdmZabrVolatilityPart::FdmZabrVolatilityPart(
    const std::shared_ptr<FdmMesher> &mesher, const Real beta, const Real nu,
    const Real rho, const Real gamma)
    : volatilityValues_(mesher->locations(1)),
      forwardValues_(mesher->locations(0)),
      mapT_(SecondDerivativeOp(1, mesher).mult(
          0.5 * nu * nu * Pow(volatilityValues_, 2.0 * gamma))),
      mesher_(mesher) {}

void FdmZabrVolatilityPart::setTime(Time t1, Time t2) {}

const TripleBandLinearOp &FdmZabrVolatilityPart::getMap() const {
    return mapT_;
}

FdmZabrOp::FdmZabrOp(const std::shared_ptr<FdmMesher> &mesher,
                     const Real beta, const Real nu, const Real rho,
                     const Real gamma)
    : volatilityValues_(mesher->locations(1)),
      forwardValues_(mesher->locations(0)),
      dxyMap_(SecondOrderMixedDerivativeOp(0, 1, mesher)
                  .mult(nu * rho * Pow(Abs(volatilityValues_), gamma + 1.0) *
                        Pow(forwardValues_, beta))),
      dxMap_(FdmZabrUnderlyingPart(mesher, beta, nu, rho, gamma)),
      dyMap_(FdmZabrVolatilityPart(mesher, beta, nu, rho, gamma)) {}

void FdmZabrOp::setTime(Time t1, Time t2) {
    dxMap_.setTime(t1, t2);
    dyMap_.setTime(t1, t2);
}

Size FdmZabrOp::size() const { return 2; }

Array FdmZabrOp::apply(const Array &u) const {
    return dyMap_.getMap().apply(u) + dxMap_.getMap().apply(u) +
           dxyMap_.apply(u);
}

Array FdmZabrOp::apply_direction(Size direction,
                                 const Array &r) const {
    if (direction == 0)
        return dxMap_.getMap().apply(r);
    else if (direction == 1)
        return dyMap_.getMap().apply(r);
    else
        QL_FAIL("direction too large");
}

Array FdmZabrOp::apply_mixed(const Array &r) const {
    return dxyMap_.apply(r);
}

Array FdmZabrOp::solve_splitting(Size direction, const Array &r,
                                 Real a) const {

    if (direction == 0) {
        return dxMap_.getMap().solve_splitting(r, a, 1.0);
    } else if (direction == 1) {
        return dyMap_.getMap().solve_splitting(r, a, 1.0);
    } else
        QL_FAIL("direction too large");
}

Array FdmZabrOp::preconditioner(const Array &r, Real dt) const {
    return solve_splitting(0, r, dt);
}

std::vector<SparseMatrix> FdmZabrOp::toMatrixDecomp() const {
    return {
        dxMap_.getMap().toMatrix(),
        dyMap_.getMap().toMatrix(),
        dxyMap_.toMatrix()
    };
}

}
