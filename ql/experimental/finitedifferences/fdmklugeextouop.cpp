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

/*! \file fdmexpoujumpop.cpp
    \brief Kluge process (power) plus Ornstein Uhlenbeck process (gas)
*/


#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/experimental/processes/klugeextouprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpop.hpp>
#include <ql/experimental/finitedifferences/fdmklugeextouop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmextendedornsteinuhlenbeckop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>

namespace QuantLib {

    FdmKlugeExtOUOp::FdmKlugeExtOUOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<KlugeExtOUProcess>& klugeOUProcess,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        const FdmBoundaryConditionSet& bcSet,
        Size integroIntegrationOrder)
    : mesher_ (mesher),
      kluge_  (klugeOUProcess->getKlugeProcess()),
      extOU_  (klugeOUProcess->getExtOUProcess()),
      rTS_    (rTS),
      bcSet_  (bcSet),
      klugeOp_(new FdmExtOUJumpOp(mesher, kluge_, rTS, bcSet,
                                  integroIntegrationOrder)),
      ouOp_   (new FdmExtendedOrnsteinUhlenbeckOp(
                  mesher, extOU_,
                  ext::shared_ptr<YieldTermStructure>(
                      new FlatForward(rTS->referenceDate(),
                              Handle<Quote>(ext::shared_ptr<Quote>(
                                      new SimpleQuote(0.0))),
                                      rTS->dayCounter())),
                  bcSet, 2)),
      corrMap_(SecondOrderMixedDerivativeOp(0, 2, mesher).mult(
          Array(mesher->layout()->size(),
                 klugeOUProcess->rho()*extOU_->volatility()
                *kluge_->getExtendedOrnsteinUhlenbeckProcess()
                       ->volatility())))
    { }

    Size FdmKlugeExtOUOp::size() const {
        return mesher_->layout()->dim().size();
    }

    void FdmKlugeExtOUOp::setTime(Time t1, Time t2) {
        ouOp_->setTime(t1, t2);
        klugeOp_->setTime(t1, t2);
    }

    Disposable<Array> FdmKlugeExtOUOp::apply(const Array& r) const {
        return ouOp_->apply(r) + klugeOp_->apply(r) + corrMap_.apply(r);
    }

    Disposable<Array> FdmKlugeExtOUOp::apply_mixed(const Array& r) const {
        return  corrMap_.apply(r) + klugeOp_->apply_mixed(r);
    }

    Disposable<Array> FdmKlugeExtOUOp::apply_direction(Size direction,
                                                       const Array& r) const {
        return klugeOp_->apply_direction(direction, r)
                + ouOp_->apply_direction(direction, r);
    }

    Disposable<Array>
        FdmKlugeExtOUOp::solve_splitting(Size direction,
                                        const Array& r, Real a) const {
        if (direction == 0 || direction == 1) {
            return klugeOp_->solve_splitting(direction, r, a);
        }
        else if (direction == 2) {
            return ouOp_->solve_splitting(direction, r, a);
        }
        else {
            Array retVal(r);
            return retVal;
        }
    }

    Disposable<Array>
        FdmKlugeExtOUOp::preconditioner(const Array& r, Real dt) const {
        return klugeOp_->solve_splitting(0, r, dt);
    }

    Disposable<std::vector<SparseMatrix> >
    FdmKlugeExtOUOp::toMatrixDecomp() const {
        const std::vector<SparseMatrix> klugeDecomp
            = klugeOp_->toMatrixDecomp();

        std::vector<SparseMatrix> retVal(4);
        retVal[0] = klugeDecomp[0];
        retVal[1] = klugeDecomp[1];
        retVal[2] = ouOp_->toMatrixDecomp().front();
        retVal[3] = corrMap_.toMatrix() + klugeDecomp[2];

        return retVal;
    }

}
