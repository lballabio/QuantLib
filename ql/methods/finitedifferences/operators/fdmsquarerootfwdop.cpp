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

/*! \file fdmsquarerootfwdop.cpp
    \brief Fokker-Planck forward operator for an square root process
*/

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/fdmsquarerootfwdop.hpp>
#include <ql/methods/finitedifferences/operators/modtriplebandlinearop.hpp>

namespace QuantLib {

    FdmSquareRootFwdOp::FdmSquareRootFwdOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        Real kappa, Real theta, Real sigma,
        Size direction, TransformationType transform)
    : direction_(direction),
      kappa_(kappa),
      theta_(theta),
      sigma_(sigma),
      transform_(transform),
      mapX_(transform == Plain ?
          new ModTripleBandLinearOp(FirstDerivativeOp(direction_, mesher)
              .mult(kappa*(mesher->locations(direction_)-theta) + sigma*sigma)
              .add(SecondDerivativeOp(direction_, mesher)
                   .mult(0.5*sigma*sigma*mesher->locations(direction_)))
                .add(Array(mesher->layout()->size(), kappa)))

        : transform == Power ? new ModTripleBandLinearOp(
            SecondDerivativeOp(direction_, mesher)
               .mult(0.5*sigma*sigma*mesher->locations(direction_))
               .add(FirstDerivativeOp(direction_, mesher)
                     .mult(kappa*(mesher->locations(direction_)+theta)))
               .add(Array(mesher->layout()->size(),
                          2*kappa*kappa*theta/(sigma*sigma))))

            : new ModTripleBandLinearOp(FirstDerivativeOp(direction_, mesher)
                    .mult(Exp(-mesher->locations(direction))
                        *( -0.5*sigma*sigma - kappa*theta) + kappa)
                    .add(SecondDerivativeOp(direction_, mesher)
                    .mult(0.5*sigma*sigma*Exp(-mesher->locations(direction))))
                    .add(kappa*theta*Exp(-mesher->locations(direction))))
            ),
      v_  (mesher->layout()->dim()[direction_]) {

        const FdmLinearOpIterator endIter = mesher->layout()->end();
        for (FdmLinearOpIterator iter = mesher->layout()->begin();
            iter != endIter; ++iter) {
            const Real v = mesher->location(iter, direction_);
            v_[iter.coordinates()[direction_]] = v;
        }

        // zero flux boundary condition
        setLowerBC(mesher);
        setUpperBC(mesher);
    }

    void FdmSquareRootFwdOp::setLowerBC(
        const ext::shared_ptr<FdmMesher>& mesher) {
        const Size n = 1;
        Real alpha, beta, gamma;

        getCoeff(alpha, beta, gamma, n);
        const Real f = lowerBoundaryFactor(transform_);

        const Real b = -(h(n-1)+h(n))/zeta(n);
        const Real c =  h(n-1)/zetap(n);

        const FdmLinearOpIterator endIter = mesher->layout()->end();
        for (FdmLinearOpIterator iter = mesher->layout()->begin();
            iter != endIter; ++iter) {
            if (iter.coordinates()[direction_] == 0) {
                const Size idx = iter.index();
                mapX_->diag(idx)  = beta  + f*b; //*v(n-1);
                mapX_->upper(idx) = gamma + f*c; //*v(n-1);
            }
        }
    }

    void FdmSquareRootFwdOp::setUpperBC(
        const ext::shared_ptr<FdmMesher>& mesher) {
        const Size n = v_.size();
        Real alpha, beta, gamma;

        getCoeff(alpha, beta, gamma, n);
        const Real f = upperBoundaryFactor(transform_);

        const Real b = (h(n)+h(n-1))/zeta(n);
        const Real c = -h(n)/zetam(n);

        const FdmLinearOpIterator endIter = mesher->layout()->end();
        for (FdmLinearOpIterator iter = mesher->layout()->begin();
            iter != endIter; ++iter) {
            if (iter.coordinates()[direction_] == n-1) {
                const Size idx = iter.index();
                mapX_->diag(idx) = beta   + f*b; //*v(n+1);
                mapX_->lower(idx) = alpha + f*c; //*v(n+1);
            }
        }
    }

    Real FdmSquareRootFwdOp::lowerBoundaryFactor(TransformationType transform) const {
        if (transform == Plain) {
            return f0Plain();
        }
        else if (transform == Power) {
            return f0Power();
        }
        else if (transform == Log) {
            return f0Log();
        }
        else
            QL_FAIL("unknown transform");
    }

    Real FdmSquareRootFwdOp::upperBoundaryFactor(TransformationType transform) const {
        if (transform == Plain) {
            return f1Plain();
        }
        else if (transform == Power) {
            return f1Power();
        }
        else if (transform == Log) {
            return f1Log();
        }
        else
            QL_FAIL("unknown transform");
    }

    Real FdmSquareRootFwdOp::f0Plain() const {
        const Size n = 1;
        const Real a = -(2*h(n-1)+h(n))/zetam(n);
        const Real alpha = sigma_*sigma_*v(n)/zetam(n) - mu(n)*h(n)/zetam(n);
        const Real nu = a*v(n-1) + (2*kappa_*(v(n-1)-theta_) + sigma_*sigma_)
                                        /(sigma_*sigma_);

        return alpha/nu*v(n-1);
    }

    Real FdmSquareRootFwdOp::f1Plain() const {
        const Size n = v_.size();
        const Real a =  (2*h(n)+h(n-1))/zetap(n);
        const Real gamma = sigma_*sigma_*v(n)/zetap(n) + mu(n)*h(n-1)/zetap(n);
        const Real nu = a*v(n+1) + (2*kappa_*(v(n+1)-theta_) + sigma_*sigma_)
                        /(sigma_*sigma_);

        return gamma/nu*v(n+1);
    }

    Real FdmSquareRootFwdOp::f0Power() const {
        const Size n = 1;
        const Real mu = kappa_*(v(n)+theta_);
        const Real a = -(2*h(n-1)+h(n))/zetam(n);
        const Real alpha = sigma_*sigma_*v(n)/zetam(n) - mu*h(n)/zetam(n);
        const Real nu  = a*v(n-1) +2*(kappa_*v(n-1)/(sigma_*sigma_));

        return alpha/nu*v(n-1);
    }

    Real FdmSquareRootFwdOp::f1Power() const {
        const Size n = v_.size();
        const Real mu = kappa_*(v(n)+theta_);
        const Real a =  (2*h(n)+h(n-1))/zetap(n);
        const Real gamma = sigma_*sigma_*v(n)/zetap(n) + mu*h(n-1)/zetap(n);
        const Real nu = a*v(n+1) +2*(kappa_*v(n+1)/(sigma_*sigma_));

        return gamma/nu*v(n+1); 
    }

    Real FdmSquareRootFwdOp::f0Log() const {
        const Size n = 1;
        const Real mu = ((-kappa_*theta_-sigma_*sigma_/2.0)*exp(-v(1))+kappa_);
        const Real a = -(2*h(n-1)+h(n))/zetam(n);
        const Real alpha = sigma_*sigma_*exp(-v(n))/zetam(n) - mu*h(n)/zetam(n);
        const Real nu = a*exp(-v(n-1)) + 2*kappa_*(1-theta_*exp(-v(n-1)))
                        /(sigma_*sigma_);

        return alpha/nu*exp(-v(n-1));
    }

    Real FdmSquareRootFwdOp::f1Log() const {
        const Size n = v_.size();
        const Real mu = ((-kappa_*theta_-sigma_*sigma_/2.0)*exp(-v(n))+kappa_);
        const Real a =  (2*h(n)+h(n-1))/zetap(n);
        const Real gamma = sigma_*sigma_*exp(-v(n))/zetap(n) + mu*h(n-1)/zetap(n);
        const Real nu = a*exp(-v(n+1)) + 2*kappa_*(1-theta_*exp(-v(n+1)))
                        /(sigma_*sigma_);

        return gamma/nu*exp(-v(n+1));
    }

    Real FdmSquareRootFwdOp::v(Size i) const {
        if (i > 0 && i <= v_.size()) {
            return v_[i-1];
        }
        else if (i == 0) {
            if (transform_ == Log) {
                return 2*v_[0] - v_[1];
//              log(std::max(0.5*exp(v_[0]), exp(v_[0] - 0.01 * (v_[1] - v_[0]))));
            } else {
                return std::max(0.5*v_[0], v_[0] - 0.01 * (v_[1] - v_[0]));
            }
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

    void FdmSquareRootFwdOp::getCoeff(Real& alpha, Real& beta,
                                               Real& gamma, Size n) const {
        if (transform_ == Plain) {
            getCoeffPlain(alpha, beta, gamma, n);
        }
        else if (transform_ == Power) {
            getCoeffPower(alpha, beta, gamma, n);
        }
        else if (transform_ == Log) {
            getCoeffLog(alpha, beta, gamma, n);
        } 
    }

    void FdmSquareRootFwdOp::getCoeffPlain(Real& alpha, Real& beta,
                                               Real& gamma, Size n) const {
        alpha =   sigma_*sigma_*v(n)/zetam(n) - mu(n)*h(n)/zetam(n);
        beta  = - sigma_*sigma_*v(n)/zeta(n)
                    + mu(n)*(h(n)-h(n-1))/zeta(n) + kappa_;
        gamma =   sigma_*sigma_*v(n)/zetap(n) + mu(n)*h(n-1)/zetap(n);

    }

    void FdmSquareRootFwdOp::getCoeffLog(Real& alpha, Real& beta,
                                               Real& gamma, Size n) const {
        const Real mu = ((-kappa_*theta_-sigma_*sigma_/2.0)*exp(-v(n))+kappa_);
        alpha =   sigma_*sigma_*exp(-v(n))/zetam(n) - mu*h(n)/zetam(n);
        beta  = - sigma_*sigma_*exp(-v(n))/zeta(n)
                          + mu*(h(n)-h(n-1))/zeta(n) + kappa_*theta_*exp(-v(n));
        gamma =   sigma_*sigma_*exp(-v(n))/zetap(n) + mu*h(n-1)/zetap(n);
    }

    void FdmSquareRootFwdOp::getCoeffPower(Real& alpha, Real& beta,
                                               Real& gamma, Size n) const {
        const Real mu = kappa_*(theta_+v(n));
        alpha = (sigma_*sigma_*v(n) - mu*h(n))/zetam(n);
        beta = (-sigma_*sigma_*v(n) + mu*(h(n)-h(n-1)))/zeta(n)
                                + 2*kappa_*kappa_*theta_/(sigma_*sigma_);
        gamma=  (sigma_*sigma_*v(n) + mu*h(n-1))/zetap(n);
    }

    Array FdmSquareRootFwdOp::apply(const Array& p) const {
        return mapX_->apply(p);
    }

    Array FdmSquareRootFwdOp::apply_mixed(const Array& r) const {
        return Array(r.size(), 0.0);
    }

    Array FdmSquareRootFwdOp::apply_direction(
        Size direction, const Array& r) const {
        if (direction == direction_) {
            return mapX_->apply(r);
        }
        else {
            return Array(r.size(), 0.0);
        }
    }

    Array FdmSquareRootFwdOp::solve_splitting(
        Size direction, const Array& r, Real dt) const {
        if (direction == direction_) {
            return mapX_->solve_splitting(r, dt, 1.0);
        }
        else {
            return r;
        }
    }

    Array FdmSquareRootFwdOp::preconditioner(
        const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

    std::vector<SparseMatrix> FdmSquareRootFwdOp::toMatrixDecomp() const {
        return std::vector<SparseMatrix>(1, mapX_->toMatrix());
    }

}
