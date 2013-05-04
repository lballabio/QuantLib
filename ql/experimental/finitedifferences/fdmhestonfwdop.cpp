/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Klaus Spanderen

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

/*! \file fdmhestonfwdop.cpp
*/

#include <ql/processes/hestonprocess.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <ql/experimental/finitedifferences/fdmhestonfwdop.hpp>
#include <ql/experimental/finitedifferences/modtriplebandlinearop.hpp>

namespace QuantLib {

    FdmHestonFwdOp::FdmHestonFwdOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<HestonProcess>& process)
    : kappa_(process->kappa()),
      theta_(process->theta()),
      sigma_(process->sigma()),
      rho_  (process->rho()),
      v0_   (process->v0()),
      rTS_  (process->riskFreeRate().currentLink()),
      qTS_  (process->dividendYield().currentLink()),
      varianceValues_(0.5*mesher->locations(1)),
      dxMap_ (new FirstDerivativeOp(0, mesher)),
      dxxMap_(new ModTripleBandLinearOp(TripleBandLinearOp(
          SecondDerivativeOp(0, mesher).mult(0.5*mesher->locations(1))))),
      mapX_  (new TripleBandLinearOp(0, mesher)),
      mapY_  (new FdmSquareRootFwdOp(mesher,kappa_,theta_,sigma_, 1)),
      correlation_(new NinePointLinearOp(
          SecondOrderMixedDerivativeOp(0, 1, mesher)
              .mult(rho_*sigma_*mesher->locations(1))))
    {
        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();

        // zero flux boundary condition
        const Size n = layout->dim()[1];
        const Real alpha = 2*rho_*mapY_->v(0)/sigma_*mapY_->f0();
        const Real beta  = 2*rho_*mapY_->v(n)/sigma_*mapY_->f1();
        ModTripleBandLinearOp fDx(FirstDerivativeOp(0, mesher));

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
                ++iter) {
            if (iter.coordinates()[1] == 0) {
                const Size idx = iter.index();

                dxxMap_->upper()[idx]+= alpha*fDx.upper()[idx];
                dxxMap_->diag()[idx] += alpha*fDx.diag()[idx];
                dxxMap_->lower()[idx] += alpha*fDx.lower()[idx];
            }
            else if (iter.coordinates()[1] == n-1) {
                const Size idx = iter.index();

                dxxMap_->upper()[idx]+= beta*fDx.upper()[idx];
                dxxMap_->diag()[idx] += beta*fDx.diag()[idx];
                dxxMap_->lower()[idx] += beta*fDx.lower()[idx];
            }
        }
    }

    Size FdmHestonFwdOp::size() const {
        return 2;
    }

    void FdmHestonFwdOp::setTime(Time t1, Time t2){
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        mapX_->axpyb( - r + q + rho_*sigma_ + varianceValues_, *dxMap_,
                     *dxxMap_, Array());
    }

    Disposable<Array> FdmHestonFwdOp::apply(const Array& u) const {
        return mapX_->apply(u)
                + mapY_->apply(u)
                + correlation_->apply(u);
    }

    Disposable<Array> FdmHestonFwdOp::apply_mixed(const Array& u) const{
        return correlation_->apply(u);
    }

    Disposable<Array> FdmHestonFwdOp::apply_direction(
        Size direction, const Array& u) const {

        if (direction == 0)
            return mapX_->apply(u) ;
        else if (direction == 1)
            return mapY_->apply(u) ;
        else
            QL_FAIL("direction too large");
    }

    Disposable<Array> FdmHestonFwdOp::solve_splitting(
        Size direction, const Array& u, Real s) const{
        if (direction == 0) {
            return mapX_->solve_splitting(u, s, 1.0);
        }
        else if (direction == 1) {
            return mapY_->solve_splitting(1, u, s);
        }
        else
            QL_FAIL("direction too large");
    }

    Disposable<Array> FdmHestonFwdOp::preconditioner(
        const Array& u, Real dt) const{
        return solve_splitting(0, u, dt);
    }

#if !defined(QL_NO_UBLAS_SUPPORT)
    Disposable<std::vector<SparseMatrix> > FdmHestonFwdOp::toMatrixDecomp()
        const {

        std::vector<SparseMatrix> retVal(3);

        retVal[0] = mapX_->toMatrix();
        retVal[1] = mapY_->toMatrix();
        retVal[2] = correlation_->toMatrix();

        return retVal;
    }
#endif
}
