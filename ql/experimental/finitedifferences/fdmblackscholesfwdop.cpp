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

#include <ql/math/functional.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesfwdop.hpp>

namespace QuantLib {

    FdmBlackScholesFwdOp::FdmBlackScholesFwdOp(
        const boost::shared_ptr<FdmMesher>& mesher,
        const boost::shared_ptr<GeneralizedBlackScholesProcess> & bsProcess,
        Real strike,
        Size direction)
    : mesher_(mesher),
      rTS_   (bsProcess->riskFreeRate().currentLink()),
      qTS_   (bsProcess->dividendYield().currentLink()),
      volTS_ (bsProcess->blackVolatility().currentLink()),
      dxMap_ (FirstDerivativeOp(direction, mesher)),
      dxxMap_(SecondDerivativeOp(direction, mesher)),
      mapT_  (direction, mesher),
      strike_(strike),
      direction_(direction) {
    }

    void FdmBlackScholesFwdOp::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        const Real v
            = volTS_->blackForwardVariance(t1, t2, strike_)/(t2-t1);
        mapT_.axpyb(Array(1, - r + q + 0.5*v), dxMap_,
                    dxxMap_.mult(0.5*Array(mesher_->layout()->size(), v)),
                    Array(1, 0.0));
    }

    Size FdmBlackScholesFwdOp::size() const {
        return 1u;
    }

    Disposable<Array> FdmBlackScholesFwdOp::apply(const Array& u) const {
        return mapT_.apply(u);
    }

    Disposable<Array> FdmBlackScholesFwdOp::apply_direction(
        Size direction, const Array& r) const {
        if (direction == direction_)
            return mapT_.apply(r);
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }

    Disposable<Array> FdmBlackScholesFwdOp::apply_mixed(const Array& r) const {
        Array retVal(r.size(), 0.0);
        return retVal;
    }

    Disposable<Array> FdmBlackScholesFwdOp::solve_splitting(
        Size direction, const Array& r, Real dt) const {
        if (direction == direction_)
            return mapT_.solve_splitting(r, dt, 1.0);
        else {
            Array retVal(r);
            return retVal;
        }
    }

    Disposable<Array> FdmBlackScholesFwdOp::preconditioner(
        const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

#if !defined(QL_NO_UBLAS_SUPPORT)
    Disposable<std::vector<SparseMatrix> >
    FdmBlackScholesFwdOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(1, mapT_.toMatrix());
        return retVal;
    }
#endif
}
