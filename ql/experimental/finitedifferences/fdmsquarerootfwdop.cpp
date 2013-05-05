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

/*! \file fdmsquarerootfwdop.cpp
    \brief Fokker-Planck forward operator for an square root process
*/

#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>

#include <ql/experimental/finitedifferences/fdmsquarerootfwdop.hpp>
#include <ql/experimental/finitedifferences/modtriplebandlinearop.hpp>

namespace QuantLib {

    FdmSquareRootFwdOp::FdmSquareRootFwdOp(
        const boost::shared_ptr<FdmMesher>& mesher,
        Real kappa, Real theta, Real sigma,
        Size direction, bool transform)
    : direction_(direction),
      kappa_(kappa),
      theta_(theta),
      sigma_(sigma),
      alpha_(1.0 - 2*kappa_*theta_/(sigma_*sigma_)),
      transform_(transform),
      mapX_(!transform ?

          new ModTripleBandLinearOp(FirstDerivativeOp(direction_, mesher)
              .mult(kappa*(mesher->locations(direction_)-theta) + sigma*sigma)
              .add(SecondDerivativeOp(direction_, mesher)
                   .mult(0.5*sigma*sigma*mesher->locations(direction_)))
                .add(Array(mesher->layout()->size(), kappa)))

        : new ModTripleBandLinearOp(
            SecondDerivativeOp(direction_, mesher)
               .mult(0.5*sigma*sigma*mesher->locations(direction_))
               .add(FirstDerivativeOp(direction_, mesher)
                     .mult(kappa*(mesher->locations(direction_)+theta)))
               .add(Array(mesher->layout()->size(),
                          2*kappa*kappa*theta/(sigma*sigma))))),

      v_  (mesher->layout()->dim()[direction_]),
      vq_ (mesher->layout()->size()),
      vmq_(mesher->layout()->size()) {

        const FdmLinearOpIterator endIter = mesher->layout()->end();
        for (FdmLinearOpIterator iter = mesher->layout()->begin();
            iter != endIter; ++iter) {
            const Real v = mesher->location(iter, direction_);
            v_[iter.coordinates()[direction_]] = v;
            vmq_[iter.index()] = 1.0/(vq_[iter.index()] = std::pow(v, alpha_));
        }

        // zero flux boundary condition
        if (!transform_) {
            setLowerBC(mesher);
            setUpperBC(mesher);
        }
        else {
            setTransformLowerBC(mesher);
            setTransformUpperBC(mesher);
        }
    }


    void FdmSquareRootFwdOp::setTransformLowerBC(
        const boost::shared_ptr<FdmMesher>& mesher) {

        const Size n = 1;
        Real alpha, beta, gamma;

        getTransformCoeff(alpha, beta, gamma, n);

        const Real eta = 1.0/(h(n-1) * (h(n-1)+h(n)) * h(n));
        const Real a   = -eta*(square<Real>()(h(n-1)+h(n))-h(n-1)*h(n-1));
        const Real b   =  eta*square<Real>()(h(n-1)+h(n));
        const Real c   = -eta*h(n-1)*h(n-1);
        const Real nu  = -sigma_*sigma_*v(n-1)
                            /(2*(kappa_*v(n-1)+sigma_*sigma_/2*v(n-1)*a));

        const FdmLinearOpIterator endIter = mesher->layout()->end();
        for (FdmLinearOpIterator iter = mesher->layout()->begin();
            iter != endIter; ++iter) {
            if (iter.coordinates()[direction_] == n-1) {
                const Size idx = iter.index();
                mapX_->diag()[idx]  = beta + alpha*nu*b;
                mapX_->upper()[idx] = gamma + alpha*nu*c;
            }
        }
    }

    void FdmSquareRootFwdOp::setTransformUpperBC(
        const boost::shared_ptr<FdmMesher>& mesher) {

        const Size n = v_.size();
        Real alpha, beta, gamma;

        getTransformCoeff(alpha, beta, gamma, n);

        const Real eta = 1.0/((h(n)*(h(n)+h(n-1))*h(n-1)));
        const Real a   =  eta*(square<Real>()(h(n)+h(n-1))-h(n)*h(n));
        const Real b   = -eta*square<Real>()(h(n)+h(n-1));
        const Real c   =  eta*h(n)*h(n);
        const Real nu  = -sigma_*sigma_*v(n+1)
                            /(2*(kappa_*v(n+1)+sigma_*sigma_/2*v(n+1)*a));

        const FdmLinearOpIterator endIter = mesher->layout()->end();
        for (FdmLinearOpIterator iter = mesher->layout()->begin();
            iter != endIter; ++iter) {
            if (iter.coordinates()[direction_] == n-1) {
                const Size idx = iter.index();
                mapX_->diag()[idx]  = beta + gamma*nu*b;
                mapX_->lower()[idx] = alpha + gamma*nu*c;
            }
        }
    }

    void FdmSquareRootFwdOp::setLowerBC(
       const boost::shared_ptr<FdmMesher>& mesher) {

       const Real beta  = - sigma_*sigma_*v(1)/zeta(1)
                          + mu(1)*(h(1)-h(0))/zeta(1) + kappa_;
       const Real gamma =   sigma_*sigma_*v(1)/zetap(1) + mu(1)*h(0)/zetap(1);

       const Real b = -(h(0)+h(1))/(h(0)*h(1));
       const Real c =  h(0)/(h(1)*(h(0)+h(1)));

       const Real f = f0();
       const FdmLinearOpIterator endIter = mesher->layout()->end();
       for (FdmLinearOpIterator iter = mesher->layout()->begin();
           iter != endIter; ++iter) {
           if (iter.coordinates()[direction_] == 0) {
               const Size idx = iter.index();
               mapX_->diag()[idx]  = beta  + f*b*v(0);
               mapX_->upper()[idx] = gamma + f*c*v(0);
           }
       }
    }

    void FdmSquareRootFwdOp::setUpperBC(
       const boost::shared_ptr<FdmMesher>& mesher) {
       const Size n = v_.size();

       const Real alpha =   sigma_*sigma_*v(n)/zetam(n) - mu(n)*h(n)/zetam(n);
       const Real beta  = - sigma_*sigma_*v(n)/zeta(n)
                          + mu(n)*(h(n)-h(n-1))/zeta(n) + kappa_;

       const Real b = (h(n)+h(n-1))/(h(n)*h(n-1));
       const Real c = -h(n)/(h(n-1)*(h(n)+h(n-1)));

       const Real f = f1();
       const FdmLinearOpIterator endIter = mesher->layout()->end();
       for (FdmLinearOpIterator iter = mesher->layout()->begin();
           iter != endIter; ++iter) {
           if (iter.coordinates()[direction_] == n-1) {
               const Size idx = iter.index();
               mapX_->diag()[idx] = beta   + f*b*v(n+1);
               mapX_->lower()[idx] = alpha + f*c*v(n+1);
           }
       }
    }

    Real FdmSquareRootFwdOp::f0() const {
        const Real a = -(2*h(0)+h(1))/(h(0)*(h(0)+h(1)));
        const Real alpha = sigma_*sigma_*v(1)/zetam(1) - mu(1)*h(1)/zetam(1);
        const Real nu = a*v(0) + (2*kappa_*(v(0)-theta_) + sigma_*sigma_)
                        /(sigma_*sigma_);

        return alpha/nu;
    }

    Real FdmSquareRootFwdOp::f1() const {
        const Size n = v_.size();
        const Real a =  (2*h(n)+h(n-1))/(h(n)*(h(n)+h(n-1)));
        const Real gamma = sigma_*sigma_*v(n)/zetap(n) + mu(n)*h(n-1)/zetap(n);
        const Real nu = a*v(n+1) + (2*kappa_*(v(n+1)-theta_) + sigma_*sigma_)
                        /(sigma_*sigma_);

        return gamma/nu;
    }

    Real FdmSquareRootFwdOp::v(Size i) const {
        if (i > 0 && i <= v_.size()) {
            return v_[i-1];
        }
        else if (i == 0) {
            return std::max(0.5*v_[0], v_[0] - 0.01 * (v_[1] - v_[0]));
        }
        else if (i == v_.size()+1) {
            return v_.back() + (v_.back() - *(v_.end()-2));
        }
        else {
            QL_FAIL("unknown index");
        }
    }

    Real FdmSquareRootFwdOp::h(Size i) const {
        return v(i+1) - v(i);
    }
    Real FdmSquareRootFwdOp::mu(Size i) const {
        return kappa_*(v(i) - theta_) + sigma_*sigma_;
    }
    Real FdmSquareRootFwdOp::zetam(Size i) const {
        return h(i-1)*(h(i-1)+h(i));
    }
    Real FdmSquareRootFwdOp::zeta(Size i) const {
        return h(i-1)*h(i);
    }
    Real FdmSquareRootFwdOp::zetap(Size i) const {
        return h(i)*(h(i-1)+h(i));
    }

    Size FdmSquareRootFwdOp::size() const {
        return 1;
    }
    void FdmSquareRootFwdOp::setTime(Time, Time) {
    }

    void FdmSquareRootFwdOp::getTransformCoeff(Real& alpha, Real& beta,
                                               Real& gamma, Size n) const {
        alpha = (sigma_*sigma_*v(n) - kappa_*(theta_+v(n))*h(n))/zetam(n);

        beta = (-sigma_*sigma_*v(n) + kappa_*(theta_+v(n))*(h(n)-h(n-1)))/zeta(n)
                                + 2*kappa_*kappa_*theta_/(sigma_*sigma_);

        gamma=  (sigma_*sigma_*v(n) + kappa_*(theta_+v(n))*h(n-1))/zetap(n);
    }

    Disposable<Array> FdmSquareRootFwdOp::apply(const Array& p) const {
        if (!transform_) {
            return mapX_->apply(p);
        }
        else {
            return vmq_*mapX_->apply(vq_*p);
        }
    }

    Disposable<Array> FdmSquareRootFwdOp::apply_mixed(const Array& r) const {
        Array retVal(r.size(), 0.0);
        return retVal;
    }
    Disposable<Array> FdmSquareRootFwdOp::apply_direction(
        Size direction, const Array& r) const {
        if (direction == direction_) {
            if (!transform_) {
                return mapX_->apply(r);
            }
            else {
                return vmq_*mapX_->apply(vq_*r);
            }
        }
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }
    Disposable<Array> FdmSquareRootFwdOp::solve_splitting(
        Size direction, const Array& r, Real dt) const {
        if (direction == direction_) {
            if (!transform_) {
                return mapX_->solve_splitting(r, dt, 1.0);
            }
            else {
                return vmq_*mapX_->solve_splitting(vq_*r, dt, 1.0);
            }
        }
        else {
            Array retVal(r);
            return retVal;
        }
    }

    Disposable<Array> FdmSquareRootFwdOp::preconditioner(
        const Array& r, Real dt) const {
        if (!transform_) {
            return solve_splitting(direction_, r, dt);
        }
        else {
            return vmq_*solve_splitting(direction_, vq_*r, dt);
        }
    }

    #if !defined(QL_NO_UBLAS_SUPPORT)
    Disposable<std::vector<SparseMatrix> >
    FdmSquareRootFwdOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(1, mapX_->toMatrix());
        return retVal;
    }
    #endif
}
