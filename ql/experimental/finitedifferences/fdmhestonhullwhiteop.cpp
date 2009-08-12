/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

#include <ql/experimental/finitedifferences/fdmhestonhullwhiteop.hpp>
#include <ql/experimental/finitedifferences/secondderivativeop.hpp>
#include <ql/experimental/finitedifferences/secondordermixedderivativeop.hpp>

namespace QuantLib {

    FdmHestonHullWhiteEquityPart::FdmHestonHullWhiteEquityPart(
        const boost::shared_ptr<FdmMesher>& mesher,
        const boost::shared_ptr<YieldTermStructure>& qTS)
    : rates_(mesher->locations(2)),
      varianceValues_(0.5*mesher->locations(1)),
      dxMap_ (FirstDerivativeOp(0, mesher)),
      dxxMap_(SecondDerivativeOp(0, mesher).mult(0.5*mesher->locations(1))),
      mapT_   (0, mesher),
      mesher_ (mesher),
      qTS_(qTS) {
        // on the boundary s_min and s_max the second derivative
        // d²V/dS² is zero and due to Ito's Lemma the variance term
        // in the drift should vanish.
        boost::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
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

    void FdmHestonHullWhiteEquityPart::setTime(Time t1, Time t2) {
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        mapT_.axpyb(rates_-varianceValues_-q, dxMap_, dxxMap_, Array());
    }

    const TripleBandLinearOp& FdmHestonHullWhiteEquityPart::getMap() const {
        return mapT_;
    }

    FdmHestonHullWhiteVariancePart::FdmHestonHullWhiteVariancePart(
        const boost::shared_ptr<FdmMesher>& mesher,
        Real sigma, Real kappa, Real theta)
    : dyMap_(SecondDerivativeOp(1, mesher)
               .mult(0.5*sigma*sigma*mesher->locations(1))
             .add(FirstDerivativeOp(1, mesher)
               .mult(kappa*(theta - mesher->locations(1))))) {
    }

    const TripleBandLinearOp& FdmHestonHullWhiteVariancePart::getMap() const {
        return dyMap_;
    }

    FdmHestonHullWhiteRatesPart::FdmHestonHullWhiteRatesPart(
        const boost::shared_ptr<FdmMesher>& mesher,
        const boost::shared_ptr<HullWhiteProcess>& hwProcess)
    : rates_(mesher->locations(2)),
      dzMap_(FirstDerivativeOp(2, mesher)),
      dzzMap_(SecondDerivativeOp(2, mesher)
                 .mult(0.5*hwProcess->sigma()*hwProcess->sigma()
                       *Array(mesher->layout()->size(), 1.))
                 .add(-mesher->locations(2))),
      mapT_(2, mesher),
      hwProcess_(hwProcess) {
    }

    void FdmHestonHullWhiteRatesPart::setTime(Time t1, Time t2) {
        const Time dt = t2-t1;

        const Array drift = (rates_*(std::exp(-hwProcess_->a()*dt)-1.0)
                           + hwProcess_->expectation(t1, 0.0, dt))/dt;

        mapT_.axpyb(drift, dzMap_, dzzMap_, Array());
    }

    const TripleBandLinearOp& FdmHestonHullWhiteRatesPart::getMap() const {
        return mapT_;
    }

    FdmHestonHullWhiteOp::FdmHestonHullWhiteOp(
                    const boost::shared_ptr<FdmMesher>& mesher,
                    const boost::shared_ptr<HestonProcess>& hestonProcess,
                    const boost::shared_ptr<HullWhiteProcess>& hwProcess,
                    Real equityShortRateCorrelation)
    : rates_(mesher->locations(2)),
      v0_(hestonProcess->v0()),
      kappa_(hestonProcess->kappa()),
      theta_(hestonProcess->theta()),
      sigma_(hestonProcess->sigma()),
      rho_(hestonProcess->rho()),
      hwProcess_(hwProcess),
      hestonCorrMap_(SecondOrderMixedDerivativeOp(0, 1, mesher)
                     .mult(rho_*sigma_*mesher->locations(1))),
      equityIrCorrMap_(SecondOrderMixedDerivativeOp(0, 2, mesher)
                       .mult(Sqrt(mesher->locations(1))
                              * hwProcess->sigma()
                              * equityShortRateCorrelation)),
      dyMap_(mesher, sigma_, kappa_, theta_),
      dxMap_(mesher, hestonProcess->dividendYield().currentLink()),
      dzMap_(mesher, hwProcess) {

        QL_REQUIRE(  equityShortRateCorrelation*equityShortRateCorrelation
                    +hestonProcess->rho()*hestonProcess->rho() <= 1.0,
                    "correlation matrix has negative eigenvalues");
    }

    void FdmHestonHullWhiteOp::setTime(Time t1, Time t2) {
        dxMap_.setTime(t1, t2);
        dzMap_.setTime(t1, t2);
    }

    Size FdmHestonHullWhiteOp::size() const {
        return 3;
    }

    Disposable<Array> FdmHestonHullWhiteOp::apply(const Array& u) const {
        return  dyMap_.getMap().apply(u) + dxMap_.getMap().apply(u)
              + dzMap_.getMap().apply(u)
              + hestonCorrMap_.apply(u) + equityIrCorrMap_.apply(u);
    }

    Disposable<Array>
    FdmHestonHullWhiteOp::apply_direction(Size direction,
                                          const Array& r) const {
        if (direction == 0)
            return dxMap_.getMap().apply(r);
        else if (direction == 1)
            return dyMap_.getMap().apply(r);
        else if (direction == 2)
            return dzMap_.getMap().apply(r);
        else
            QL_FAIL("direction too large");
    }

    Disposable<Array> FdmHestonHullWhiteOp::apply_mixed(const Array& r) const {
        return hestonCorrMap_.apply(r) + equityIrCorrMap_.apply(r);
    }

    Disposable<Array>
    FdmHestonHullWhiteOp::solve_splitting(Size direction, const Array& r,
                                          Real a) const {
        if (direction == 0) {
            return dxMap_.getMap().solve_splitting(r, a, 1.0);
        }
        else if (direction == 1) {
            return dyMap_.getMap().solve_splitting(r, a, 1.0);
        }
        else if (direction == 2) {
            return dzMap_.getMap().solve_splitting(r, a, 1.0);
        }
        else
            QL_FAIL("direction too large");
    }
    
    Disposable<Array> FdmHestonHullWhiteOp::preconditioner(const Array& r, 
                                                           Real dt) const {
        return solve_splitting(0, r, dt);
    }
}
