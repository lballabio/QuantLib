/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmlocalvolfwdop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <utility>

namespace QuantLib {

    FdmLocalVolFwdOp::FdmLocalVolFwdOp(const ext::shared_ptr<FdmMesher>& mesher,
                                       const ext::shared_ptr<Quote>& spot,
                                       ext::shared_ptr<YieldTermStructure> rTS,
                                       ext::shared_ptr<YieldTermStructure> qTS,
                                       const ext::shared_ptr<LocalVolTermStructure>& localVol,
                                       Size direction)
    : mesher_(mesher), rTS_(std::move(rTS)), qTS_(std::move(qTS)), localVol_(localVol),
      x_((localVol) != nullptr ? Array(Exp(mesher->locations(direction))) : Array()),
      dxMap_(FirstDerivativeOp(direction, mesher)), dxxMap_(SecondDerivativeOp(direction, mesher)),
      mapT_(direction, mesher), direction_(direction) {}

    void FdmLocalVolFwdOp::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        const ext::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
        const FdmLinearOpIterator endIter = layout->end();

        Array v(layout->size());
        for (FdmLinearOpIterator iter = layout->begin();
            iter != endIter; ++iter) {
            const Size i = iter.index();

            v[i] = square<Real>()(
                localVol_->localVol(0.5*(t1+t2), x_[i], true));
        }
        mapT_.axpyb(Array(1, 1.0), dxMap_.multR(- r + q + 0.5*v),
                    dxxMap_.multR(0.5*v), Array(1, 0.0));
    }

    Size FdmLocalVolFwdOp::size() const { return 1U; }

    Disposable<Array> FdmLocalVolFwdOp::apply(const Array& u) const {
        return mapT_.apply(u);
    }

    Disposable<Array> FdmLocalVolFwdOp::apply_direction(
        Size direction, const Array& r) const {
        if (direction == direction_)
            return mapT_.apply(r);
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }

    Disposable<Array> FdmLocalVolFwdOp::apply_mixed(const Array& r) const {
        Array retVal(r.size(), 0.0);
        return retVal;
    }

    Disposable<Array> FdmLocalVolFwdOp::solve_splitting(
        Size direction, const Array& r, Real dt) const {
        if (direction == direction_)
            return mapT_.solve_splitting(r, dt, 1.0);
        else {
            Array retVal(r);
            return retVal;
        }
    }

    Disposable<Array> FdmLocalVolFwdOp::preconditioner(
        const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

    Disposable<std::vector<SparseMatrix> >
    FdmLocalVolFwdOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(1, mapT_.toMatrix());
        return retVal;
    }

}
