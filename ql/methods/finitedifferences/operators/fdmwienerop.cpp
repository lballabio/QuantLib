/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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


/*! \file fdmwienerop.cpp
*/

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmwienerop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>

#include <boost/numeric/ublas/matrix.hpp>

namespace QuantLib {

    FdmWienerOp::FdmWienerOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        ext::shared_ptr<YieldTermStructure> rTS,
        const Array& lambdas)
    : rTS_(std::move(rTS)) {

        QL_REQUIRE(mesher->layout()->dim().size() == lambdas.size(),
            "mesher and lambdas need to be of the same dimension");

        for (Size i=0; i < lambdas.size(); ++i)
            ops_.emplace_back(ext::make_shared<TripleBandLinearOp>(
                SecondDerivativeOp(i, mesher)
                    .mult(Array(mesher->layout()->size(), 0.5*lambdas[i])))
            );
    }

    Size FdmWienerOp::size() const {
        return ops_.size();
    }

    void FdmWienerOp::setTime(Time t1, Time t2) {
        if (rTS_ != nullptr)
            r_ = rTS_->forwardRate(t1, t2, Continuous).rate();
    }

    Array FdmWienerOp::apply(const Array& x) const {
        Array y(-r_*x);
        for (const auto& op: ops_)
            y += op->apply(x);

        return y;
    }

    Array FdmWienerOp::apply_mixed(const Array& x) const {
        return Array(x.size(), 0.0);
    }

    Array FdmWienerOp::apply_direction(Size direction, const Array& x) const {
        return ops_[direction]->apply(x);
    }

    Array FdmWienerOp::solve_splitting(
        Size direction, const Array& x, Real s) const {

        return ops_[direction]->solve_splitting(x, s, 1.0);
    }

    Array FdmWienerOp::preconditioner(const Array& r, Real dt) const {
        return solve_splitting(0, r, dt);
    }

    std::vector<SparseMatrix> FdmWienerOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal;

        retVal.reserve(ops_.size());
        for (const auto& op: ops_)
            retVal.push_back(op->toMatrix());

        return retVal;
    }
}
