/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2011 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmhestonhullwhiteop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <utility>

namespace QuantLib {

    FdmHestonHullWhiteEquityPart::FdmHestonHullWhiteEquityPart(
        const ext::shared_ptr<FdmMesher>& mesher,
        ext::shared_ptr<HullWhite> hwModel,
        ext::shared_ptr<YieldTermStructure> qTS)
    : x_(mesher->locations(2)), varianceValues_(0.5 * mesher->locations(1)),
      dxMap_(FirstDerivativeOp(0, mesher)),
      dxxMap_(SecondDerivativeOp(0, mesher).mult(0.5 * mesher->locations(1))), mapT_(0, mesher),
      hwModel_(std::move(hwModel)), mesher_(mesher), qTS_(std::move(qTS)) {

        // on the boundary s_min and s_max the second derivative
        // d²V/dS² is zero and due to Ito's Lemma the variance term
        // in the drift should vanish.
        for (const auto& iter : *mesher_->layout()) {
            if (   iter.coordinates()[0] == 0
                || iter.coordinates()[0] == mesher_->layout()->dim()[0]-1) {
                varianceValues_[iter.index()] = 0.0;
            }
        }
        volatilityValues_ = Sqrt(2*varianceValues_);
    }

    void FdmHestonHullWhiteEquityPart::setTime(Time t1, Time t2) {
        const ext::shared_ptr<OneFactorModel::ShortRateDynamics> dynamics =
            hwModel_->dynamics();

        const Real phi = 0.5*(  dynamics->shortRate(t1, 0.0)
                              + dynamics->shortRate(t2, 0.0));

        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        mapT_.axpyb(x_+phi-varianceValues_-q, dxMap_, dxxMap_, Array());
    }

    const TripleBandLinearOp& FdmHestonHullWhiteEquityPart::getMap() const {
        return mapT_;
    }

    FdmHestonHullWhiteOp::FdmHestonHullWhiteOp(const ext::shared_ptr<FdmMesher>& mesher,
                                               const ext::shared_ptr<HestonProcess>& hestonProcess,
                                               const ext::shared_ptr<HullWhiteProcess>& hwProcess,
                                               Real equityShortRateCorrelation)
    : v0_(hestonProcess->v0()), kappa_(hestonProcess->kappa()), theta_(hestonProcess->theta()),
      sigma_(hestonProcess->sigma()), rho_(hestonProcess->rho()),
      hwModel_(ext::make_shared<HullWhite>(
          hestonProcess->riskFreeRate(), hwProcess->a(), hwProcess->sigma())),
      hestonCorrMap_(
          SecondOrderMixedDerivativeOp(0, 1, mesher).mult(rho_ * sigma_ * mesher->locations(1))),
      equityIrCorrMap_(
          SecondOrderMixedDerivativeOp(0, 2, mesher)
              .mult(Sqrt(mesher->locations(1)) * hwProcess->sigma() * equityShortRateCorrelation)),
      dyMap_(SecondDerivativeOp(1U, mesher)
                 .mult(0.5 * sigma_ * sigma_ * mesher->locations(1))
                 .add(FirstDerivativeOp(1, mesher).mult(kappa_ * (theta_ - mesher->locations(1))))),
      dxMap_(mesher, hwModel_, hestonProcess->dividendYield().currentLink()),
      hullWhiteOp_(mesher, hwModel_, 2) {

        QL_REQUIRE(  equityShortRateCorrelation*equityShortRateCorrelation
                   + hestonProcess->rho()*hestonProcess->rho() <= 1.0,
                   "correlation matrix has negative eigenvalues");
    }

    void FdmHestonHullWhiteOp::setTime(Time t1, Time t2) {
        dxMap_.setTime(t1, t2);
        hullWhiteOp_.setTime(t1, t2);
    }

    Size FdmHestonHullWhiteOp::size() const {
        return 3;
    }

    Array FdmHestonHullWhiteOp::apply(const Array& u) const {
        return  dyMap_.apply(u) + dxMap_.getMap().apply(u)
              + hullWhiteOp_.apply(u)
              + hestonCorrMap_.apply(u) + equityIrCorrMap_.apply(u);
    }

    Array FdmHestonHullWhiteOp::apply_direction(Size direction,
                                                const Array& r) const {
        if (direction == 0)
            return dxMap_.getMap().apply(r);
        else if (direction == 1)
            return dyMap_.apply(r);
        else if (direction == 2)
            return hullWhiteOp_.apply(r);
        else
            QL_FAIL("direction too large");
    }

    Array FdmHestonHullWhiteOp::apply_mixed(const Array& r) const {
        return hestonCorrMap_.apply(r) + equityIrCorrMap_.apply(r);
    }

    Array FdmHestonHullWhiteOp::solve_splitting(Size direction, const Array& r,
                                                Real a) const {
        if (direction == 0) {
            return dxMap_.getMap().solve_splitting(r, a, 1.0);
        }
        else if (direction == 1) {
            return dyMap_.solve_splitting(r, a, 1.0);
        }
        else if (direction == 2) {
            return hullWhiteOp_.solve_splitting(2, r, a);
        }
        else
            QL_FAIL("direction too large");
    }
    
    Array FdmHestonHullWhiteOp::preconditioner(const Array& r, 
                                               Real dt) const {
        return solve_splitting(0, r, dt);
    }

    std::vector<SparseMatrix> FdmHestonHullWhiteOp::toMatrixDecomp() const {
        return {
            dxMap_.getMap().toMatrix(),
            dyMap_.toMatrix(),
            hullWhiteOp_.toMatrixDecomp().front(),
            hestonCorrMap_.toMatrix() + equityIrCorrMap_.toMatrix()
        };
    }

}
