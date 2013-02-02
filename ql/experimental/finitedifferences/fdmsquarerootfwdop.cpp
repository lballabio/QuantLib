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

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>

#include <ql/experimental/finitedifferences/fdmsquarerootfwdop.hpp>
#include <ql/experimental/finitedifferences/modtriplebandlinearop.hpp>

namespace QuantLib {

    FdmSquareRootFwdOp::FdmSquareRootFwdOp(
        const boost::shared_ptr<FdmMesher>& mesher,
        Real kappa, Real theta, Real sigma, Size direction)
    : direction_(direction),
      kappa_(kappa),
      theta_(theta),
      sigma_(sigma),
      mapX_(new ModTripleBandLinearOp(FirstDerivativeOp(direction_, mesher)
          .mult(kappa*(mesher->locations(direction_)-theta) + sigma*sigma)
          .add(SecondDerivativeOp(direction_, mesher)
                  .mult(0.5*sigma*sigma*mesher->locations(direction_)))
          .add(Array(mesher->layout()->size(), kappa)))),
      v_(mesher->layout()->dim()[direction_]) {

        const FdmLinearOpIterator endIter = mesher->layout()->end();
        for (FdmLinearOpIterator iter = mesher->layout()->begin();
            iter != endIter; ++iter) {
            v_[iter.coordinates()[direction_]]
               = mesher->location(iter, direction_);
        }

        // zero flux boundary condition
        setLowerBC(mesher);
        setUpperBC(mesher);
    }



    void FdmSquareRootFwdOp::setLowerBC(
       const boost::shared_ptr<FdmMesher>& mesher) {

       const Real beta  = - sigma_*sigma_*v(1)/zeta(1)
                          + mu(1)*(h(1)-h(0))/zeta(1) + kappa_;
       const Real gamma =   sigma_*sigma_*v(1)/zetap(1) + mu(1)*h(0)/zetap(1);

       const Real b = (h(0)+h(1))/(h(0)*h(1));
       const Real c = -h(0)/(h(1)*(h(0)+h(1)));

       const Real f = f0();
       const FdmLinearOpIterator endIter = mesher->layout()->end();
       for (FdmLinearOpIterator iter = mesher->layout()->begin();
           iter != endIter; ++iter) {
           if (iter.coordinates()[direction_] == 0) {
               const Size idx = iter.index();
               mapX_->diag()[idx]  = beta  + f*b*v(1);
               mapX_->upper()[idx] = gamma + f*c*v(2);
           }
       }
    }

    void FdmSquareRootFwdOp::setUpperBC(
       const boost::shared_ptr<FdmMesher>& mesher) {
       const Size n = v_.size();

       const Real alpha =   sigma_*sigma_*v(n)/zetam(n) - mu(n)*h(n)/zetam(n);
       const Real beta  = - sigma_*sigma_*v(n)/zeta(n)
                          + mu(n)*(h(n)-h(n-1))/zeta(n) + kappa_;

       const Real b = -(h(n)+h(n-1))/(h(n)*h(n-1));
       const Real c =  h(n)/(h(n-1)*(h(n)+h(n-1)));

       const Real f = f1();
       const FdmLinearOpIterator endIter = mesher->layout()->end();
       for (FdmLinearOpIterator iter = mesher->layout()->begin();
           iter != endIter; ++iter) {
           if (iter.coordinates()[direction_] == n-1) {
               const Size idx = iter.index();
               mapX_->diag()[idx] = beta  + f*b*v(n);
               mapX_->lower()[idx] = alpha + f*c*v(n-1);
           }
       }
    }

    Real FdmSquareRootFwdOp::f0() const {
        const Real a = -(2*h(0)+h(1))/(h(0)*(h(0)+h(1)));
        const Real alpha =   sigma_*sigma_*v(1)/zetam(1) - mu(1)*h(1)/zetam(1);
        const Real nu = -a*v(0) - 2*(kappa_*(v(0)-theta_))/(sigma_*sigma_);

        return alpha/nu;
    }

    Real FdmSquareRootFwdOp::f1() const {
        const Size n = v_.size();
        const Real a =  (2*h(n)+h(n-1))/(h(n)*(h(n)+h(n-1)));
        const Real gamma = sigma_*sigma_*v(n)/zetap(n) + mu(n)*h(n-1)/zetap(n);
        const Real nu = -a*v(n+1) - 2*kappa_*(v(n+1)-theta_)/(sigma_*sigma_);

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
    void FdmSquareRootFwdOp::setTime(Time t1, Time t2) {
    }

    Disposable<Array> FdmSquareRootFwdOp::apply(const Array& r) const {
        return mapX_->apply(r);
    }

    Disposable<Array> FdmSquareRootFwdOp::apply_mixed(const Array& r) const {
        Array retVal(r.size(), 0.0);
        return retVal;
    }
    Disposable<Array> FdmSquareRootFwdOp::apply_direction(
        Size direction, const Array& r) const {
        if (direction == 0)
            return mapX_->apply(r);
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }
    Disposable<Array> FdmSquareRootFwdOp::solve_splitting(
        Size direction, const Array& r, Real dt) const {
        if (direction == 0)
            return mapX_->solve_splitting(r, dt, 1.0);
        else {
            Array retVal(r);
            return retVal;
        }
    }

    Disposable<Array> FdmSquareRootFwdOp::preconditioner(
        const Array& r, Real dt) const {
        return solve_splitting(0, r, dt);
    }

    const boost::shared_ptr<TripleBandLinearOp>
    FdmSquareRootFwdOp::toLinearOp() const {
        return mapX_;
    }


    #if !defined(QL_NO_UBLAS_SUPPORT)
    Disposable<std::vector<SparseMatrix> >
    FdmSquareRootFwdOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(1, mapX_->toMatrix());
        return retVal;
    }
    #endif
}
