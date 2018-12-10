/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2014, 2015 Klaus Spanderen
 Copyright (C) 2015 Johannes Goettker-Schnetmann

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

namespace QuantLib {

    FdmHestonEquityPart::FdmHestonEquityPart(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        const ext::shared_ptr<YieldTermStructure>& qTS,
        const ext::shared_ptr<FdmQuantoHelper>& quantoHelper,
        const ext::shared_ptr<LocalVolTermStructure>& leverageFct)
    : varianceValues_(0.5*mesher->locations(1)),
      dxMap_ (FirstDerivativeOp(0, mesher)),
      dxxMap_(SecondDerivativeOp(0, mesher).mult(0.5*mesher->locations(1))),
      mapT_  (0, mesher),
      mesher_(mesher),
      rTS_(rTS),
      qTS_(qTS),
      quantoHelper_(quantoHelper),
      leverageFct_(leverageFct) {

        // on the boundary s_min and s_max the second derivative
        // d^2V/dS^2 is zero and due to Ito's Lemma the variance term
        // in the drift should vanish.
        ext::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
        FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
            ++iter) {
            if (   iter.coordinates()[0] == 0
                || iter.coordinates()[0] == layout->dim()[0]-1) {
                varianceValues_[iter.index()] = 0.0;
            }
        }
        volatilityValues_ = Sqrt(2*varianceValues_);
    }

    void FdmHestonEquityPart::setTime(Time t1, Time t2) {
        const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        if (quantoHelper_) {
            mapT_.axpyb(r - q - varianceValues_
                - quantoHelper_->quantoAdjustment(
                volatilityValues_, t1, t2),
                dxMap_, dxxMap_, Array(1, -0.5*r));
        }
        else {
            L_ = getLeverageFctSlice(t1, t2);
            const Array Lsquare = L_*L_;

            mapT_.axpyb(r - q - varianceValues_*Lsquare, dxMap_,
                        dxxMap_.mult(Lsquare), Array(1, -0.5*r));
        }
    }

    Disposable<Array> FdmHestonEquityPart::getLeverageFctSlice(Time t1, Time t2)
    const {
        const ext::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
        Array v(layout->size(), 1.0);

        if (!leverageFct_) {
            return v;
        }
        const Real t = 0.5*(t1+t2);
        const Time time = std::min(leverageFct_->maxTime(), t);

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


    const TripleBandLinearOp& FdmHestonEquityPart::getMap() const {
        return mapT_;
    }

    FdmHestonVariancePart::FdmHestonVariancePart(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        Real sigma, Real kappa, Real theta)
    : dyMap_(SecondDerivativeOp(1, mesher)
                .mult(0.5*sigma*sigma*mesher->locations(1))
             .add(FirstDerivativeOp(1, mesher)
                  .mult(kappa*(theta - mesher->locations(1))))),
      mapT_(1, mesher),
      rTS_(rTS) {
    }

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
        const ext::shared_ptr<LocalVolTermStructure>& leverageFct)
    : correlationMap_(SecondOrderMixedDerivativeOp(0, 1, mesher)
                        .mult(hestonProcess->rho()*hestonProcess->sigma()
                                *mesher->locations(1))),
      dyMap_(mesher, hestonProcess->riskFreeRate().currentLink(),
              hestonProcess->sigma(), 
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

    Disposable<Array> FdmHestonOp::apply(const Array& u) const {
        return dyMap_.getMap().apply(u) + dxMap_.getMap().apply(u)
              + dxMap_.getL()*correlationMap_.apply(u);
    }

    Disposable<Array> FdmHestonOp::apply_direction(Size direction,
                                                   const Array& r) const {
        if (direction == 0)
            return dxMap_.getMap().apply(r);
        else if (direction == 1)
            return dyMap_.getMap().apply(r);
        else
            QL_FAIL("direction too large");
    }

    Disposable<Array> FdmHestonOp::apply_mixed(const Array& r) const {
        return dxMap_.getL()*correlationMap_.apply(r);
    }

    Disposable<Array>
        FdmHestonOp::solve_splitting(Size direction,
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

    Disposable<Array>
        FdmHestonOp::preconditioner(const Array& r, Real dt) const {

        return solve_splitting(1, solve_splitting(0, r, dt), dt) ;
    }

#if !defined(QL_NO_UBLAS_SUPPORT)
    Disposable<std::vector<SparseMatrix> >
    FdmHestonOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(3);

        retVal[0] = dxMap_.getMap().toMatrix();
        retVal[1] = dyMap_.getMap().toMatrix();
        retVal[2] = correlationMap_.toMatrix();

        return retVal;
    }
#endif
}
