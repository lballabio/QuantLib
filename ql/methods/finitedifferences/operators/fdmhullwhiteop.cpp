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

/*! \file fdmhullwhiteop.cpp */


#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmhullwhiteop.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>

namespace QuantLib {

    FdmHullWhiteOp::FdmHullWhiteOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<HullWhite>& model,
        Size direction)
    : direction_(direction),
      x_(mesher->locations(direction)),
      dzMap_(FirstDerivativeOp(direction, mesher).mult(-x_*model->a()).add(
                SecondDerivativeOp(direction, mesher)
                    .mult(0.5*model->sigma()*model->sigma()
                          *Array(mesher->layout()->size(), 1.0)))),
      mapT_(direction, mesher),
      model_(model) {
    }

    Size FdmHullWhiteOp::size() const { return 1U; }

    void FdmHullWhiteOp::setTime(Time t1, Time t2) {

        const ext::shared_ptr<OneFactorModel::ShortRateDynamics> dynamics =
            model_->dynamics();

        const Real phi = 0.5*(  dynamics->shortRate(t1, 0.0)
                              + dynamics->shortRate(t2, 0.0));

        mapT_.axpyb(Array(), dzMap_, dzMap_, -(x_+phi));
    }

    Array FdmHullWhiteOp::apply(const Array& r) const {
        return mapT_.apply(r);
    }

    Array FdmHullWhiteOp::apply_mixed(const Array& r) const {
        return Array(r.size(), 0.0);
    }

    Array FdmHullWhiteOp::apply_direction(Size direction, const Array& r) const {
        if (direction == direction_)
            return mapT_.apply(r);
        else {
            return Array(r.size(), 0.0);
        }
    }

    Array FdmHullWhiteOp::solve_splitting(Size direction, const Array& r, Real a) const {
        if (direction == direction_) {
            return mapT_.solve_splitting(r, a, 1.0);
        }
        else {
            return Array(r.size(), 0.0);
        }
    }

    Array FdmHullWhiteOp::preconditioner(const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

    std::vector<SparseMatrix> FdmHullWhiteOp::toMatrixDecomp() const {
        return std::vector<SparseMatrix>(1, mapT_.toMatrix());
    }

}

