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
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmornsteinuhlenbeckop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>

namespace QuantLib {

    FdmOrnsteinUhlenbackOp::FdmOrnsteinUhlenbackOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<OrnsteinUhlenbeckProcess>& process,
            const boost::shared_ptr<YieldTermStructure>& rTS,
            const FdmBoundaryConditionSet& bcSet,
            Size direction)
    : mesher_   (mesher),
      process_  (process),
      rTS_      (rTS),
      bcSet_    (bcSet),
      direction_(direction),
      m_        (direction, mesher),
      mapX_     (direction, mesher)  {

        const boost::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();

        Array drift(layout->size());
        const Array x(mesher_->locations(direction));

        for (FdmLinearOpIterator iter=layout->begin(), endIter=layout->end();
             iter!=endIter; ++iter) {
            const Size i = iter.index();
            drift[i] = process_->drift(0.0, x[i]);
        }

        m_.axpyb(drift, FirstDerivativeOp(direction, mesher),
            SecondDerivativeOp(direction, mesher)
                .mult(0.5*square<Real>()(process_->volatility())
                      *Array(mesher->layout()->size(), 1.0)), Array());
    }

    Size FdmOrnsteinUhlenbackOp::size() const {
        return mesher_->layout()->dim().size();;
    }

    void FdmOrnsteinUhlenbackOp::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();

        mapX_.axpyb(Array(), m_, m_, Array(1, -r));
    }

    Disposable<Array> FdmOrnsteinUhlenbackOp::apply(const Array& r) const {
        return mapX_.apply(r);
    }

    Disposable<Array> FdmOrnsteinUhlenbackOp::apply_mixed(
        const Array& r) const {

        Array retVal(r.size(), 0.0);
        return retVal;
    }

    Disposable<Array> FdmOrnsteinUhlenbackOp::apply_direction(
        Size direction, const Array& r) const {

        if (direction == direction_) {
            return mapX_.apply(r);
        }
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }

    Disposable<Array> FdmOrnsteinUhlenbackOp::solve_splitting(
        Size direction, const Array& r, Real a) const {

        if (direction == direction_) {
            return mapX_.solve_splitting(r, a, 1.0);
        }
        else {
            Array retVal(r);
            return retVal;
        }
    }

    Disposable<Array> FdmOrnsteinUhlenbackOp::preconditioner(
        const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

#if !defined(QL_NO_UBLAS_SUPPORT)
    Disposable<std::vector<SparseMatrix> >
    FdmOrnsteinUhlenbackOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(1, mapX_.toMatrix());
        return retVal;
    }
#endif

}
