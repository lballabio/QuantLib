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


/*! \file fdmndimblackscholesop.cpp
*/

#include <ql/methods/finitedifferences/operators/fdmndimblackscholesop.hpp>

#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmblackscholesop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <boost/numeric/ublas/matrix.hpp>


namespace QuantLib {

    FdmndimBlackScholesOp::FdmndimBlackScholesOp(
            ext::shared_ptr<FdmMesher> mesher,
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
            Matrix rho,
            Time maturity)
    : mesher_(std::move(mesher)),
      processes_(std::move(processes)),
      currentForwardRate_(Null<Real>()) {

        QL_REQUIRE(!processes_.empty(), "no Black-Scholes process is given.");
        QL_REQUIRE(rho.size1() == rho.size2()
                && rho.size1() == processes_.size(),
                "correlation matrix has the wrong size.");

        for (Size direction = 0; direction < processes_.size(); ++direction) {
            const auto process = processes_[direction];
            ops_.push_back(
                ext::make_shared<FdmBlackScholesOp>(
                    mesher_, process, process->x0(), false, -Null<Real>(), direction
                )
            );
        }

        for (Size i=1; i < processes_.size(); ++i) {
            const auto p1 = processes_[i];
            const Volatility v1
                = p1->blackVolatility()->blackVol(maturity, p1->x0(), true);

            for (Size j=0; j < i; ++j) {
                const auto p2 = processes_[j];
                const Volatility v2
                    = p2->blackVolatility()->blackVol(maturity, p2->x0(), true);

                corrMaps_.emplace_back(
                    new NinePointLinearOp(
                        SecondOrderMixedDerivativeOp(i, j, mesher_)
                            .mult(Array(mesher_->layout()->size(), v1*v2*rho[i][j]))
                    )
                );
            }
        }
    }

    Size FdmndimBlackScholesOp::size() const {
        return processes_.size();
    }

    void FdmndimBlackScholesOp::setTime(Time t1, Time t2) {
        for (auto& op: ops_)
            op->setTime(t1, t2);

        currentForwardRate_ = 0.0;
        for (Size i=1; i < processes_.size(); ++i)
            currentForwardRate_ +=
                processes_[i]->riskFreeRate()->forwardRate(t1, t2, Continuous).rate();
    }

    Array FdmndimBlackScholesOp::apply(const Array& x) const {
        Array y = apply_mixed(x);
        for (const auto& op: ops_)
            y += op->apply(x);

        return y;
    }

    Array FdmndimBlackScholesOp::apply_mixed(const Array& x) const {
        Array y = currentForwardRate_*x;
        for (const auto& m: corrMaps_)
            y += m->apply(x);

        return y;
    }

    Array FdmndimBlackScholesOp::apply_direction(Size direction, const Array& x) const {
        return ops_[direction]->apply(x);
    }

    Array FdmndimBlackScholesOp::solve_splitting(
        Size direction, const Array& x, Real s) const {

        return ops_[direction]->solve_splitting(direction, x, s);
    }

    Array FdmndimBlackScholesOp::preconditioner(const Array& r, Real dt) const {
        return solve_splitting(0, r, dt);
    }

    std::vector<SparseMatrix> FdmndimBlackScholesOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal;

        for (const auto& op: ops_)
            retVal.push_back(op->toMatrix());

        SparseMatrix mixed =
             currentForwardRate_*boost::numeric::ublas::identity_matrix<Real>(
                 mesher_->layout()->size());
        for (const auto& m: corrMaps_)
            mixed += m->toMatrix();

        retVal.push_back(mixed);

        return retVal;
    }
}
