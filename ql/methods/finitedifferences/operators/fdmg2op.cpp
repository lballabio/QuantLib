/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmg2op.cpp */


#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmg2op.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>


namespace QuantLib {

    FdmG2Op::FdmG2Op(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<G2>& model,
        Size direction1, Size direction2)
    : direction1_(direction1),
      direction2_(direction2),
      x_(mesher->locations(direction1)),
      y_(mesher->locations(direction2)),
      dxMap_(FirstDerivativeOp(direction1, mesher).mult(-x_*model->a()).add(
                SecondDerivativeOp(direction1, mesher)
                    .mult(0.5*model->sigma()*model->sigma()
                          *Array(mesher->layout()->size(), 1.0)))),
      dyMap_(FirstDerivativeOp(direction2, mesher).mult(-y_*model->b()).add(
                SecondDerivativeOp(direction2, mesher)
                    .mult(0.5*model->eta()*model->eta()
                          *Array(mesher->layout()->size(), 1.0)))),
      corrMap_(SecondOrderMixedDerivativeOp(direction1, direction2, mesher)
              .mult(Array(mesher->layout()->size(),
                          model->rho()*model->sigma()*model->eta()))),
      mapX_(direction1, mesher),
      mapY_(direction2, mesher),
      model_(model) {
    }

    Size FdmG2Op::size() const { return 2U; }

    void FdmG2Op::setTime(Time t1, Time t2) {

        const ext::shared_ptr<TwoFactorModel::ShortRateDynamics> dynamics =
            model_->dynamics();

        const Real phi = 0.5*(  dynamics->shortRate(t1, 0.0, 0.0)
                              + dynamics->shortRate(t2, 0.0, 0.0));

        const Array hr = -0.5*(x_ + y_ + phi);
        mapX_.axpyb(Array(), dxMap_, dxMap_, hr);
        mapY_.axpyb(Array(), dyMap_, dyMap_, hr);
    }

    Array FdmG2Op::apply(const Array& r) const {
        return mapX_.apply(r) + mapY_.apply(r) + apply_mixed(r);
    }

    Array FdmG2Op::apply_mixed(const Array& r) const {
        return corrMap_.apply(r);
    }

    Array FdmG2Op::apply_direction(Size direction, const Array& r) const {
        if (direction == direction1_) {
            return mapX_.apply(r);
        }
        else if (direction == direction2_) {
            return mapY_.apply(r);
        }
        else {
            return Array(r.size(), 0.0);
        }
    }

    Array FdmG2Op::solve_splitting(Size direction, const Array& r, Real a) const {
        if (direction == direction1_) {
            return mapX_.solve_splitting(r, a, 1.0);
        }
        else if (direction == direction2_) {
            return mapY_.solve_splitting(r, a, 1.0);
        }
        else {
            return Array(r.size(), 0.0);
        }
    }

    Array FdmG2Op::preconditioner(const Array& r, Real dt) const {
        return solve_splitting(direction1_, r, dt);
    }

    std::vector<SparseMatrix> FdmG2Op::toMatrixDecomp() const {
        return {
            mapX_.toMatrix(),
            mapY_.toMatrix(),
            corrMap_.toMatrix()
        };
    }

}

