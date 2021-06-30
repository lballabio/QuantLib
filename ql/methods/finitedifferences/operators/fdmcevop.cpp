/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file fdmcevop.cpp */

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmcevop.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>


namespace QuantLib {

    FdmCEVOp::FdmCEVOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        Real f0, Real alpha, Real beta,
        Size direction)
    : rTS_(rTS),
      direction_(direction),
      dxxMap_(SecondDerivativeOp(0, mesher)
              .mult(0.5 * alpha * alpha *
                    Pow(mesher->locations(direction), 2.0 * beta))),
      mapT_(direction, mesher) {
    }

    Size FdmCEVOp::size() const { return 1U; }

    void FdmCEVOp::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        mapT_.axpyb(Array(), dxxMap_, dxxMap_, Array(1, -r));
    }

    Disposable<Array> FdmCEVOp::apply(const Array& r) const {
        return mapT_.apply(r);
    }

    Disposable<Array> FdmCEVOp::apply_mixed(const Array& r) const {
        Array retVal(r.size(), 0.0);
        return retVal;
    }

    Disposable<Array>
    FdmCEVOp::apply_direction(Size direction, const Array& r) const {
        if (direction == direction_) {
            return mapT_.apply(r);
        }
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }

    Disposable<Array>
    FdmCEVOp::solve_splitting(Size direction, const Array& r, Real a) const {
        if (direction == direction_) {
            return mapT_.solve_splitting(r, a, 1.0);
        }
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }

    Disposable<Array>
    FdmCEVOp::preconditioner(const Array& r, Real dt) const {
        return solve_splitting(direction_, r, dt);
    }

    Disposable<std::vector<SparseMatrix> > FdmCEVOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(1, mapT_.toMatrix());
        return retVal;
    }

}

