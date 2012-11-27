/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmextendedornsteinuhlenbeckop.cpp
*/

#include <ql/math/functional.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/finitedifferences/fdmextendedornsteinuhlenbeckop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>

namespace QuantLib {

    FdmExtendedOrnsteinUhlenbackOp::FdmExtendedOrnsteinUhlenbackOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess>& process,
            const boost::shared_ptr<YieldTermStructure>& rTS,
            const FdmBoundaryConditionSet& bcSet,
            Size direction)
    : mesher_   (mesher),
      process_  (process),
      rTS_      (rTS),
      bcSet_    (bcSet),
      direction_(direction),
      x_        (mesher->locations(direction)),
      dxMap_    (direction, mesher),
      dxxMap_   (SecondDerivativeOp(direction, mesher)
                .mult(0.5*square<Real>()(process_->volatility())
                  *Array(mesher->layout()->size(), 1.))),
      mapX_     (direction, mesher) {
    }

    Size FdmExtendedOrnsteinUhlenbackOp::size() const {
        return mesher_->layout()->dim().size();;
    }

    void FdmExtendedOrnsteinUhlenbackOp::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();

        const boost::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
        const FdmLinearOpIterator endIter = layout->end();

        Array drift(layout->size());
        for (FdmLinearOpIterator iter = layout->begin();
             iter!=endIter; ++iter) {
            const Size i = iter.index();
            drift[i] = process_->drift(0.5*(t1+t2), x_[i]);
        }
        mapX_.axpyb(drift, dxMap_, dxxMap_, Array(1, -r));
    }

    Disposable<Array> FdmExtendedOrnsteinUhlenbackOp::apply(
                                                    const Array& r) const {
        return mapX_.apply(r);
    }

    Disposable<Array> FdmExtendedOrnsteinUhlenbackOp::apply_mixed(
                                                    const Array& r) const {
        Array retVal(r.size(), 0.0);
        return retVal;
    }

    Disposable<Array> FdmExtendedOrnsteinUhlenbackOp::apply_direction(
                                    Size direction, const Array& r) const {
        if (direction == direction_) {
            return mapX_.apply(r);
        }
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }

    Disposable<Array> FdmExtendedOrnsteinUhlenbackOp::solve_splitting(
                            Size direction, const Array& r, Real a) const {
        if (direction == direction_) {
            return mapX_.solve_splitting(r, a, 1.0);
        }
        else {
            Array retVal(r);
            return retVal;
        }
    }

    Disposable<Array> FdmExtendedOrnsteinUhlenbackOp::preconditioner(
                                            const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

#if !defined(QL_NO_UBLAS_SUPPORT)
    Disposable<std::vector<SparseMatrix> >
    FdmExtendedOrnsteinUhlenbackOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(1, mapX_.toMatrix());
        return retVal;
    }
#endif

}
