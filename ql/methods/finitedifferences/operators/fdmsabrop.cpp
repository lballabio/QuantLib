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

/*! \file fdmsabrop.cpp
    \brief FDM operator for the SABR model
*/

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmsabrop.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {
    FdmSabrOp::FdmSabrOp(const std::shared_ptr<FdmMesher>& mesher,
                         std::shared_ptr<YieldTermStructure> rTS,
                         Real f0,
                         Real alpha,
                         Real beta,
                         Real nu,
                         Real rho)
    : rTS_(std::move(rTS)),
      dffMap_(SecondDerivativeOp(0, mesher).mult(0.5 * Exp(2.0 * mesher->locations(1)) *
                                                 Pow(mesher->locations(0), 2.0 * beta))),
      dxMap_(FirstDerivativeOp(1, mesher).mult(Array(mesher->layout()->size(), -0.5 * nu * nu))),
      dxxMap_(SecondDerivativeOp(1, mesher).mult(Array(mesher->layout()->size(), 0.5 * nu * nu))),
      correlationMap_(
          SecondOrderMixedDerivativeOp(0, 1, mesher)
              .mult(rho * nu * Exp(mesher->locations(1)) * Pow(mesher->locations(0), beta))),
      mapF_(0, mesher), mapA_(1, mesher) {}

    void FdmSabrOp::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();

        mapF_.axpyb(Array(), dffMap_, dffMap_, Array(1, -0.5*r));
        mapA_.axpyb(Array(1, 1.0), dxMap_, dxxMap_, Array(1, -0.5*r));
    }

    Size FdmSabrOp::size() const {
        return 2;
    }

    Array FdmSabrOp::apply(const Array& u) const {
        return mapF_.apply(u) + mapA_.apply(u) + correlationMap_.apply(u);
    }

    Array FdmSabrOp::apply_mixed(const Array& r) const {
        return correlationMap_.apply(r);
    }

    Array FdmSabrOp::apply_direction(
        Size direction, const Array& r) const {
        if (direction == 0)
            return mapF_.apply(r);
        else if (direction == 1)
            return mapA_.apply(r);
        else
            QL_FAIL("direction too large");
    }

    Array FdmSabrOp::solve_splitting(
       Size direction, const Array& r, Real a) const {

        if (direction == 0) {
            return mapF_.solve_splitting(r, a, 1.0);
        }
        else if (direction == 1) {
            return mapA_.solve_splitting(r, a, 1.0);
        }
        else
            QL_FAIL("direction too large");
    }

    Array FdmSabrOp::preconditioner(
        const Array& r, Real dt) const {

        return solve_splitting(1, solve_splitting(0, r, dt), dt) ;
    }

    std::vector<SparseMatrix> FdmSabrOp::toMatrixDecomp() const {
        return {
            mapA_.toMatrix(),
            mapF_.toMatrix(),
            correlationMap_.toMatrix()
        };
    }

}
