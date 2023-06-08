/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file fdmornsteinuhlenbeckop.cpp
*/

#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmornsteinuhlenbeckop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    FdmOrnsteinUhlenbeckOp::FdmOrnsteinUhlenbeckOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        ext::shared_ptr<OrnsteinUhlenbeckProcess> process,
        ext::shared_ptr<YieldTermStructure> rTS,
        Size direction)
    : mesher_(mesher), process_(std::move(process)), rTS_(std::move(rTS)), direction_(direction),
      m_(direction, mesher), mapX_(direction, mesher) {

        Array drift(mesher_->layout()->size());
        const Array x(mesher_->locations(direction));

        for (const auto& iter : *mesher_->layout()) {
            const Size i = iter.index();
            drift[i] = process_->drift(0.0, x[i]);
        }

        m_.axpyb(drift, FirstDerivativeOp(direction, mesher),
            SecondDerivativeOp(direction, mesher)
                .mult(0.5*squared(process_->volatility())
                      *Array(mesher->layout()->size(), 1.0)), Array());
    }

    Size FdmOrnsteinUhlenbeckOp::size() const {
        return mesher_->layout()->dim().size();;
    }

    void FdmOrnsteinUhlenbeckOp::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();

        mapX_.axpyb(Array(), m_, m_, Array(1, -r));
    }

    Array FdmOrnsteinUhlenbeckOp::apply(const Array& r) const {
        return mapX_.apply(r);
    }

    Array FdmOrnsteinUhlenbeckOp::apply_mixed(const Array& r) const {
        return Array(r.size(), 0.0);
    }

    Array FdmOrnsteinUhlenbeckOp::apply_direction(Size direction, const Array& r) const {
        if (direction == direction_) {
            return mapX_.apply(r);
        }
        else {
            return Array(r.size(), 0.0);
        }
    }

    Array FdmOrnsteinUhlenbeckOp::solve_splitting(Size direction, const Array& r, Real a) const {
        if (direction == direction_) {
            return mapX_.solve_splitting(r, a, 1.0);
        }
        else {
            return r;
        }
    }

    Array FdmOrnsteinUhlenbeckOp::preconditioner(const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

    std::vector<SparseMatrix> FdmOrnsteinUhlenbeckOp::toMatrixDecomp() const {
        return std::vector<SparseMatrix>(1, mapX_.toMatrix());
    }

}
