/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2014, 2015 Klaus Spanderen
 Copyright (C) 2015 Johannes Göttker-Schnetmann

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

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmhestonop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <utility>

namespace QuantLib {

    FdmHestonEquityPart::FdmHestonEquityPart(const ext::shared_ptr<FdmMesher>& mesher,
                                             ext::shared_ptr<YieldTermStructure> rTS,
                                             ext::shared_ptr<YieldTermStructure> qTS,
                                             ext::shared_ptr<FdmQuantoHelper> quantoHelper,
                                             ext::shared_ptr<LocalVolTermStructure> leverageFct)
    : varianceValues_(0.5 * mesher->locations(1)), dxMap_(FirstDerivativeOp(0, mesher)),
      dxxMap_(SecondDerivativeOp(0, mesher).mult(0.5 * mesher->locations(1))), mapT_(0, mesher),
      mesher_(mesher), rTS_(std::move(rTS)), qTS_(std::move(qTS)),
      quantoHelper_(std::move(quantoHelper)), leverageFct_(std::move(leverageFct)) {

        // on the boundary s_min and s_max the second derivative
        // d^2V/dS^2 is zero and due to Ito's Lemma the variance term
        // in the drift should vanish.
        for (const auto& iter : *mesher_->layout()) {
            if (   iter.coordinates()[0] == 0
                || iter.coordinates()[0] == mesher_->layout()->dim()[0]-1) {
                varianceValues_[iter.index()] = 0.0;
            }
        }
        volatilityValues_ = Sqrt(2*varianceValues_);
    }

    void FdmHestonEquityPart::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        L_ = getLeverageFctSlice(t1, t2);
        const Array Lsquare = L_*L_;

        if (quantoHelper_ != nullptr) {
            mapT_.axpyb(r - q - varianceValues_*Lsquare
                - quantoHelper_->quantoAdjustment(
                    volatilityValues_*L_, t1, t2),
                dxMap_, dxxMap_.mult(Lsquare), Array(1, -0.5*r));
        } else {
            mapT_.axpyb(r - q - varianceValues_*Lsquare, dxMap_,
                        dxxMap_.mult(Lsquare), Array(1, -0.5*r));
        }
    }

    Array FdmHestonEquityPart::getLeverageFctSlice(Time t1, Time t2) const {

        Array v(mesher_->layout()->size(), 1.0);

        if (!leverageFct_) {
            return v;
        }
        const Real t = 0.5*(t1+t2);
        const Time time = std::min(leverageFct_->maxTime(), t);

        for (const auto& iter : *mesher_->layout()) {
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


    const TripleBandLinearOp& FdmHestonEquityPart::getMap() const {
        return mapT_;
    }

    FdmHestonVariancePart::FdmHestonVariancePart(const ext::shared_ptr<FdmMesher>& mesher,
                                                 ext::shared_ptr<YieldTermStructure> rTS,
                                                 Real mixedSigma,
                                                 Real kappa,
                                                 Real theta)
    : dyMap_(SecondDerivativeOp(1, mesher)
                 .mult(0.5 * mixedSigma * mixedSigma * mesher->locations(1))
                 .add(FirstDerivativeOp(1, mesher).mult(kappa * (theta - mesher->locations(1))))),
      mapT_(1, mesher), rTS_(std::move(rTS)) {}

    void FdmHestonVariancePart::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        mapT_.axpyb(Array(), dyMap_, dyMap_, Array(1,-0.5*r));
    }

    const TripleBandLinearOp& FdmHestonVariancePart::getMap() const {
        return mapT_;
    }

    FdmHestonOp::FdmHestonOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<HestonProcess> & hestonProcess,
        const ext::shared_ptr<FdmQuantoHelper>& quantoHelper,
        const ext::shared_ptr<LocalVolTermStructure>& leverageFct,
        const Real mixingFactor)
    : correlationMap_(SecondOrderMixedDerivativeOp(0, 1, mesher)
                        .mult(hestonProcess->rho()*hestonProcess->sigma()
                                *mixingFactor
                                *mesher->locations(1))),
      dyMap_(mesher, hestonProcess->riskFreeRate().currentLink(),
              hestonProcess->sigma()*mixingFactor,
              hestonProcess->kappa(), 
              hestonProcess->theta()),
      dxMap_(mesher,
             hestonProcess->riskFreeRate().currentLink(), 
             hestonProcess->dividendYield().currentLink(),
             quantoHelper, leverageFct) {
    }


    void FdmHestonOp::setTime(Time t1, Time t2) {
        dxMap_.setTime(t1, t2);
        dyMap_.setTime(t1, t2);
    }

    Size FdmHestonOp::size() const {
        return 2;
    }

    Array FdmHestonOp::apply(const Array& u) const {
        return dyMap_.getMap().apply(u) + dxMap_.getMap().apply(u)
              + dxMap_.getL()*correlationMap_.apply(u);
    }

    Array FdmHestonOp::apply_direction(Size direction,
                                       const Array& r) const {
        if (direction == 0)
            return dxMap_.getMap().apply(r);
        else if (direction == 1)
            return dyMap_.getMap().apply(r);
        else
            QL_FAIL("direction too large");
    }

    Array FdmHestonOp::apply_mixed(const Array& r) const {
        return dxMap_.getL()*correlationMap_.apply(r);
    }

    Array FdmHestonOp::solve_splitting(Size direction,
                                       const Array& r, Real a) const {

        if (direction == 0) {
            return dxMap_.getMap().solve_splitting(r, a, 1.0);
        }
        else if (direction == 1) {
            return dyMap_.getMap().solve_splitting(r, a, 1.0);
        }
        else
            QL_FAIL("direction too large");
    }

    Array FdmHestonOp::preconditioner(const Array& r, Real dt) const {
        return solve_splitting(1, solve_splitting(0, r, dt), dt) ;
    }

    std::vector<SparseMatrix> FdmHestonOp::toMatrixDecomp() const {
        return {
            dxMap_.getMap().toMatrix(),
            dyMap_.getMap().toMatrix(),
            correlationMap_.toMatrix()
        };
    }

}
