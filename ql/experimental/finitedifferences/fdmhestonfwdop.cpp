/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Klaus Spanderen
 Copyright (C) 2014 Johannes Göttker-Schnetmann

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

#include <ql/experimental/finitedifferences/fdmhestonfwdop.hpp>
#include <ql/experimental/finitedifferences/modtriplebandlinearop.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <cmath>
#include <utility>

using std::exp;

namespace QuantLib {

    FdmHestonFwdOp::FdmHestonFwdOp(const ext::shared_ptr<FdmMesher>& mesher,
                                   const ext::shared_ptr<HestonProcess>& process,
                                   FdmSquareRootFwdOp::TransformationType type,
                                   ext::shared_ptr<LocalVolTermStructure> leverageFct,
                                   const Real mixingFactor)
    : type_(type), kappa_(process->kappa()), theta_(process->theta()), sigma_(process->sigma()),
      rho_(process->rho()), v0_(process->v0()), mixedSigma_(mixingFactor * sigma_),
      rTS_(process->riskFreeRate().currentLink()), qTS_(process->dividendYield().currentLink()),
      varianceValues_(0.5 * mesher->locations(1)),
      dxMap_(ext::make_shared<FirstDerivativeOp>(0, mesher)),
      dxxMap_(ext::make_shared<ModTripleBandLinearOp>(TripleBandLinearOp(
          type == FdmSquareRootFwdOp::Log ?
              SecondDerivativeOp(0, mesher).mult(0.5 * Exp(mesher->locations(1))) :
              SecondDerivativeOp(0, mesher).mult(0.5 * mesher->locations(1))))),
      boundary_(ext::make_shared<ModTripleBandLinearOp>(TripleBandLinearOp(
          SecondDerivativeOp(0, mesher).mult(Array(mesher->locations(0).size(), 0.0))))),
      mapX_(ext::make_shared<TripleBandLinearOp>(0, mesher)),
      mapY_(ext::make_shared<FdmSquareRootFwdOp>(mesher, kappa_, theta_, mixedSigma_, 1, type)),
      correlation_(ext::make_shared<NinePointLinearOp>(
          type == FdmSquareRootFwdOp::Log ?
              SecondOrderMixedDerivativeOp(0, 1, mesher)
                  .mult(Array(mesher->layout()->size(), rho_ * mixedSigma_)) :
              SecondOrderMixedDerivativeOp(0, 1, mesher)
                  .mult(rho_ * mixedSigma_ * mesher->locations(1)))),
      leverageFct_(std::move(leverageFct)), mesher_(mesher) {
        const ext::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        // zero flux boundary condition
        const Size n = layout->dim()[1];
        const Real lowerBoundaryFactor = mapY_->lowerBoundaryFactor(type);
        const Real upperBoundaryFactor = mapY_->upperBoundaryFactor(type);

        const Real logFacLow = type == FdmSquareRootFwdOp::Log ? Real(exp(mapY_->v(0))) : 1.0;
        const Real logFacUpp = type == FdmSquareRootFwdOp::Log ? Real(exp(mapY_->v(n+1))) : 1.0;

        const Real alpha = -2*rho_/mixedSigma_*lowerBoundaryFactor*logFacLow;
        const Real beta  = -2*rho_/mixedSigma_*upperBoundaryFactor*logFacUpp;

        ModTripleBandLinearOp fDx(FirstDerivativeOp(0, mesher));

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter; ++iter) {
            if (iter.coordinates()[1] == 0) {
                const Size idx = iter.index();
                if (!leverageFct_) {
                    dxxMap_->upper(idx) += alpha*fDx.upper(idx);
                    dxxMap_->diag(idx) += alpha*fDx.diag(idx);
                    dxxMap_->lower(idx) += alpha*fDx.lower(idx);
                }
                boundary_->upper(idx)= alpha*fDx.upper(idx);
                boundary_->diag(idx) = alpha*fDx.diag(idx);
                boundary_->lower(idx) = alpha*fDx.lower(idx);
            }
            else if (iter.coordinates()[1] == n-1) {
                const Size idx = iter.index();

                if (!leverageFct_) {
                    dxxMap_->upper(idx)+= beta*fDx.upper(idx);
                    dxxMap_->diag(idx) += beta*fDx.diag(idx);
                    dxxMap_->lower(idx) += beta*fDx.lower(idx);
                }
                boundary_->upper(idx)= beta*fDx.upper(idx);
                boundary_->diag(idx) = beta*fDx.diag(idx);
                boundary_->lower(idx) = beta*fDx.lower(idx);
            }
        }
    }

    Size FdmHestonFwdOp::size() const {
        return 2;
    }

    void FdmHestonFwdOp::setTime(Time t1, Time t2){
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();
        if (leverageFct_ != nullptr) {
            L_ = getLeverageFctSlice(t1, t2);
            Array Lsquare = L_*L_;
            if (type_ == FdmSquareRootFwdOp::Plain) {
                mapX_->axpyb( Array(1, -r + q), *dxMap_,
                    dxxMap_->multR(Lsquare).add(boundary_->multR(L_))
                    .add(dxMap_->multR(rho_*mixedSigma_*L_))
                    .add(dxMap_->mult(varianceValues_).multR(Lsquare)),
                              Array());
            } else if (type_ == FdmSquareRootFwdOp::Power) {
                mapX_->axpyb( Array(1, -r + q), *dxMap_,
                    dxxMap_->multR(Lsquare).add(boundary_->multR(L_))
                    .add(dxMap_->multR(rho_*2.0*kappa_*theta_/(mixedSigma_)*L_))
                    .add(dxMap_->mult(varianceValues_).multR(Lsquare)), Array());
            } else if (type_ == FdmSquareRootFwdOp::Log) {
                mapX_->axpyb( Array(1, -r + q), *dxMap_,
                    dxxMap_->multR(Lsquare).add(boundary_->multR(L_))
                    .add(dxMap_->mult(0.5*Exp(2.0*varianceValues_)).multR(Lsquare)),
                              Array());
            }
        } else {
            if (type_ == FdmSquareRootFwdOp::Plain) {
                mapX_->axpyb( - r + q + rho_*mixedSigma_ + varianceValues_, *dxMap_,
                        *dxxMap_, Array());
            } else if (type_ == FdmSquareRootFwdOp::Power) {
                mapX_->axpyb( - r + q + rho_*2.0*kappa_*theta_/(mixedSigma_) + varianceValues_,
                              *dxMap_, *dxxMap_, Array());
            } else if (type_ == FdmSquareRootFwdOp::Log) {
                mapX_->axpyb( - r + q + 0.5*Exp(2.0*varianceValues_), *dxMap_,
                        *dxxMap_, Array());
            }
        }
    }

    Array FdmHestonFwdOp::apply(const Array& u) const {
        if (leverageFct_ != nullptr) {
            return mapX_->apply(u)
                    + mapY_->apply(u)
                    + correlation_->apply(L_*u);
        } else {
            return mapX_->apply(u)
                    + mapY_->apply(u)
                    + correlation_->apply(u);
        }
    }

    Array FdmHestonFwdOp::apply_mixed(const Array& u) const{
        if (leverageFct_ != nullptr) {
            return correlation_->apply(L_*u);
        } else {
            return correlation_->apply(u);
        }
    }

    Array FdmHestonFwdOp::apply_direction(
        Size direction, const Array& u) const {

        if (direction == 0)
            return mapX_->apply(u) ;
        else if (direction == 1)
            return mapY_->apply(u) ;
        else
            QL_FAIL("direction too large");
    }

    Array FdmHestonFwdOp::solve_splitting(
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

    Array FdmHestonFwdOp::preconditioner(
        const Array& u, Real dt) const{
        return solve_splitting(1, u, dt);
    }

    Array FdmHestonFwdOp::getLeverageFctSlice(Time t1, Time t2) const {
        const ext::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
        Array v(layout->size(), 1.0);

        if (!leverageFct_)
            return v;

        const Real t = 0.5*(t1+t2);
        const Time time = std::min(leverageFct_->maxTime(), t);
                                   //std::max(leverageFct_->minTime(), t));

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin();
             iter!=endIter; ++iter) {
            const Size nx = iter.coordinates()[0];

            if (iter.coordinates()[1] == 0) {
                const Real x = std::exp(mesher_->location(iter, 0));
                const Real spot = std::min(leverageFct_->maxStrike(),
                                           std::max(leverageFct_->minStrike(), x));
                v[nx] = std::max(0.01, leverageFct_->localVol(time, spot, true));
            }
            else {
                v[iter.index()] = v[nx];
            }
        }
        return v;
    }

    std::vector<SparseMatrix> FdmHestonFwdOp::toMatrixDecomp() const {

        std::vector<SparseMatrix> retVal(3);

        retVal[0] = mapX_->toMatrix();
        retVal[1] = mapY_->toMatrix();
        retVal[2] = correlation_->toMatrix();

        return retVal;
    }

}
