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

#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesfwdop.hpp>

namespace QuantLib {

    FdmBlackScholesFwdOp::FdmBlackScholesFwdOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<GeneralizedBlackScholesProcess> & bsProcess,
        Real strike,
        bool localVol,
        Real illegalLocalVolOverwrite,
        Size direction)
    : mesher_(mesher),
      rTS_   (bsProcess->riskFreeRate().currentLink()),
      qTS_   (bsProcess->dividendYield().currentLink()),
      volTS_ (bsProcess->blackVolatility().currentLink()),
      localVol_((localVol) ? bsProcess->localVolatility().currentLink()
                           : ext::shared_ptr<LocalVolTermStructure>()),
      x_ ((localVol) ? Array(Exp(mesher->locations(direction))) : Array()),
      dxMap_ (FirstDerivativeOp(direction, mesher)),
      dxxMap_(SecondDerivativeOp(direction, mesher)),
      mapT_  (direction, mesher),
      strike_(strike),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite),
      direction_(direction) {
    }

    void FdmBlackScholesFwdOp::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        if (localVol_ != nullptr) {
            const ext::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
            const FdmLinearOpIterator endIter = layout->end();

            Array v(layout->size());
            for (FdmLinearOpIterator iter = layout->begin();
                iter != endIter; ++iter) {
                const Size i = iter.index();

                if (illegalLocalVolOverwrite_ < 0.0) {
                    v[i] = squared(localVol_->localVol(0.5*(t1+t2), x_[i], true));
                }
                else {
                    try {
                        v[i] = squared(localVol_->localVol(0.5*(t1+t2), x_[i], true));
                    } catch (Error&) {
                        v[i] = squared(illegalLocalVolOverwrite_);
                    }
                }
            }
            mapT_.axpyb(Array(1, 1.0), dxMap_.multR(- r + q + 0.5*v),
                        dxxMap_.multR(0.5*v), Array(1, 0.0));
        } else {
            const Real v
                = volTS_->blackForwardVariance(t1, t2, strike_)/(t2-t1);
            mapT_.axpyb(Array(1, - r + q + 0.5*v), dxMap_,
                        dxxMap_.mult(0.5*Array(mesher_->layout()->size(), v)),
                        Array(1, 0.0));
        }
    }

    Size FdmBlackScholesFwdOp::size() const { return 1U; }

    Array FdmBlackScholesFwdOp::apply(const Array& u) const {
        return mapT_.apply(u);
    }

    Array FdmBlackScholesFwdOp::apply_direction(
        Size direction, const Array& r) const {
        if (direction == direction_)
            return mapT_.apply(r);
        else {
            return Array(r.size(), 0.0);
        }
    }

    Array FdmBlackScholesFwdOp::apply_mixed(const Array& r) const {
        return Array(r.size(), 0.0);
    }

    Array FdmBlackScholesFwdOp::solve_splitting(
        Size direction, const Array& r, Real dt) const {
        if (direction == direction_)
            return mapT_.solve_splitting(r, dt, 1.0);
        else {
            return r;
        }
    }

    Array FdmBlackScholesFwdOp::preconditioner(
        const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

    std::vector<SparseMatrix> FdmBlackScholesFwdOp::toMatrixDecomp() const {
        return std::vector<SparseMatrix>(1, mapT_.toMatrix());
    }

}
