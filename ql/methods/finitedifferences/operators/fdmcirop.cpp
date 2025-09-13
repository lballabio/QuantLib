/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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
#include <ql/methods/finitedifferences/operators/fdmcirop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    FdmCIREquityPart::FdmCIREquityPart(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
        Real strike)
    : dxMap_ (FirstDerivativeOp(0, mesher)),
      dxxMap_(SecondDerivativeOp(0, mesher)),
      mapT_ (0, mesher),
      mesher_(mesher),
      qTS_(bsProcess->dividendYield().currentLink()),
      strike_(strike),
      sigma1_(bsProcess->blackVolatility().currentLink()){
    }

    void FdmCIREquityPart::setTime(Time t1, Time t2) {
        const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();

        const Real v = sigma1_->blackForwardVariance(t1, t2, strike_)/(t2-t1);

        mapT_.axpyb(mesher_->locations(1) - q - 0.5*v, dxMap_,
                        dxxMap_.mult(Array(mesher_->layout()->size(), v/2)), -0.5*mesher_->locations(1));
    }

    const TripleBandLinearOp& FdmCIREquityPart::getMap() const {
        return mapT_;
    }

    FdmCIRRatesPart::FdmCIRRatesPart(
        const ext::shared_ptr<FdmMesher>& mesher,
        Real sigma, Real kappa, Real theta)
    : dyMap_(SecondDerivativeOp(1, mesher)
                   .mult(sigma*sigma*mesher->locations(1))
                   .add(FirstDerivativeOp(1, mesher)
                   .mult(kappa*(theta - mesher->locations(1))))),
      mapT_(1, mesher),
      mesher_(mesher){
    }

    void FdmCIRRatesPart::setTime(Time t1, Time t2) {
        mapT_.axpyb(Array(), dyMap_, dyMap_, -0.5*mesher_->locations(1));
    }

    const TripleBandLinearOp& FdmCIRRatesPart::getMap() const {
        return mapT_;
    }

    FdmCIRMixedPart::FdmCIRMixedPart(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<CoxIngersollRossProcess> & cirProcess,
        const ext::shared_ptr<GeneralizedBlackScholesProcess> & bsProcess,
        const Real rho,
        const Real strike)
        : dyMap_(SecondOrderMixedDerivativeOp(0, 1, mesher)
                     .mult(Array(mesher->layout()->size(), 2*rho*cirProcess->volatility()))),
          mapT_(0, 1, mesher),
          mesher_(mesher),
          sigma1_(bsProcess->blackVolatility().currentLink()),
          strike_(strike){
    }

    void FdmCIRMixedPart::setTime(Time t1, Time t2) {
        const Real v = std::sqrt(sigma1_->blackForwardVariance(t1, t2, strike_)/(t2-t1));
        NinePointLinearOp op(dyMap_.mult(Array(mesher_->layout()->size(), v)));
        mapT_.swap(op);
    }

    const NinePointLinearOp& FdmCIRMixedPart::getMap() const {
        return mapT_;
    }

    FdmCIROp::FdmCIROp(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<CoxIngersollRossProcess> & cirProcess,
        const ext::shared_ptr<GeneralizedBlackScholesProcess> & bsProcess,
        const Real rho,
        const Real strike)
    : dxMap_(mesher,
             bsProcess,
             strike),
      dyMap_(mesher,
             cirProcess->volatility(),
             cirProcess->speed(),
             cirProcess->level()),
      dzMap_(mesher,
             cirProcess,
             bsProcess,
             rho,
             strike){
    }


    void FdmCIROp::setTime(Time t1, Time t2) {
        dxMap_.setTime(t1, t2);
        dyMap_.setTime(t1, t2);
        dzMap_.setTime(t1, t2);
    }

    Size FdmCIROp::size() const {
        return 2;
    }

    Array FdmCIROp::apply(const Array& u) const {
        Array dx = dxMap_.getMap().apply(u);
        Array dy = dyMap_.getMap().apply(u);
        Array dz = dzMap_.getMap().apply(u);

        return (dy + dx + dz);
    }

    Array FdmCIROp::apply_direction(Size direction,
                                    const Array& r) const {
        if (direction == 0)
            return dxMap_.getMap().apply(r);
        else if (direction == 1)
            return dyMap_.getMap().apply(r);
        else
            QL_FAIL("direction too large");
    }

    Array FdmCIROp::apply_mixed(const Array& r) const {
        return dzMap_.getMap().apply(r);
    }

    Array FdmCIROp::solve_splitting(Size direction,
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

    Array FdmCIROp::preconditioner(const Array& r, Real dt) const {
        return solve_splitting(1, solve_splitting(0, r, dt), dt) ;
    }

    std::vector<SparseMatrix> FdmCIROp::toMatrixDecomp() const {
        return {
            dxMap_.getMap().toMatrix(),
            dyMap_.getMap().toMatrix(),
            dzMap_.getMap().toMatrix()
        };
    }

}
