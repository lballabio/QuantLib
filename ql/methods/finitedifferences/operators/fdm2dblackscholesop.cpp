/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file fdm2dblackscholesop.cpp
*/


#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdm2dblackscholesop.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace QuantLib {

    Fdm2dBlackScholesOp::Fdm2dBlackScholesOp(
            const ext::shared_ptr<FdmMesher>& mesher,
            const ext::shared_ptr<GeneralizedBlackScholesProcess>& p1,
            const ext::shared_ptr<GeneralizedBlackScholesProcess>& p2,
            Real correlation,
            Time /*maturity*/,
            bool localVol,
            Real illegalLocalVolOverwrite)
    : mesher_(mesher),
      p1_(p1),
      p2_(p2),
      
      localVol1_((localVol) ? p1->localVolatility().currentLink()
                            : ext::shared_ptr<LocalVolTermStructure>()),
      localVol2_((localVol) ? p2->localVolatility().currentLink()
                            : ext::shared_ptr<LocalVolTermStructure>()),
                            
      x_((localVol) ? Array(Exp(mesher->locations(0))) : Array()),
      y_((localVol) ? Array(Exp(mesher->locations(1))) : Array()),

      opX_(mesher, p1, p1->x0(), localVol, illegalLocalVolOverwrite, 0),
      opY_(mesher, p2, p2->x0(), localVol, illegalLocalVolOverwrite, 1),
      
      corrMapT_(0, 1, mesher),
      corrMapTemplate_(SecondOrderMixedDerivativeOp(0, 1, mesher)
                      .mult(Array(mesher->layout()->size(), correlation))),
                      
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite) { 
    }
           
    Size Fdm2dBlackScholesOp::size() const {
        return 2;
    }
    
    void Fdm2dBlackScholesOp::setTime(Time t1, Time t2) {
        opX_.setTime(t1, t2);
        opY_.setTime(t1, t2);

        if (localVol1_ != nullptr) {
            const ext::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
            const FdmLinearOpIterator endIter = layout->end();

            Array vol1(layout->size()), vol2(layout->size());
            for (FdmLinearOpIterator iter = layout->begin();
                 iter!=endIter; ++iter) {
                const Size i = iter.index();

                if (illegalLocalVolOverwrite_ < 0.0) {
                    vol1[i] = localVol1_->localVol(0.5*(t1+t2), x_[i], true);
                    vol2[i] = localVol2_->localVol(0.5*(t1+t2), y_[i], true);
                }
                else {
                    try {
                        vol1[i] = localVol1_->localVol(0.5*(t1+t2), x_[i],true);
                    } catch (Error&) {
                        vol1[i] = illegalLocalVolOverwrite_;
                    }
                    try {
                        vol2[i] = localVol2_->localVol(0.5*(t1+t2), y_[i],true);
                    } catch (Error&) {
                        vol2[i] = illegalLocalVolOverwrite_;
                    }

                }
            }
            corrMapT_ = corrMapTemplate_.mult(vol1*vol2);
        } else {
            const Real vol1 = p1_
                    ->blackVolatility()->blackForwardVol(t1, t2, p1_->x0());
    
            const Real vol2 = p2_
                    ->blackVolatility()->blackForwardVol(t1, t2, p2_->x0());
            
            corrMapT_ = corrMapTemplate_
                      .mult(Array(mesher_->layout()->size(), vol1*vol2));
        }

        currentForwardRate_ = p1_->riskFreeRate()
                                 ->forwardRate(t1, t2, Continuous).rate();
    }

    Disposable<Array> Fdm2dBlackScholesOp::apply(const Array& x) const {
        return opX_.apply(x) + opY_.apply(x) + apply_mixed(x);
    }
    
    Disposable<Array> Fdm2dBlackScholesOp::apply_mixed(const Array& x) const {
        return corrMapT_.apply(x) + currentForwardRate_*x;
    }
    
    Disposable<Array> Fdm2dBlackScholesOp::apply_direction(
                                       Size direction, const Array& x) const {
        if (direction == 0) {
            return opX_.apply(x);
        }
        else if (direction == 1) {
            return opY_.apply(x);
        }
        else {
            QL_FAIL("direction is too large");
        }
    }
    
    Disposable<Array> Fdm2dBlackScholesOp::solve_splitting(Size direction,
                                               const Array& x, Real s) const {
        if (direction == 0) {
            return opX_.solve_splitting(direction, x, s);
        }
        else if (direction == 1) {
            return opY_.solve_splitting(direction, x, s);
        }
        else
            QL_FAIL("direction is too large");
    }
    
    Disposable<Array> Fdm2dBlackScholesOp::preconditioner(const Array& r, 
                                                          Real dt) const {
        return solve_splitting(0, r, dt);
    }

    Disposable<std::vector<SparseMatrix> >
    Fdm2dBlackScholesOp::toMatrixDecomp() const {
        std::vector<SparseMatrix> retVal(3);
        retVal[0] = opX_.toMatrix();
        retVal[1] = opY_.toMatrix();
        retVal[2] = corrMapT_.toMatrix() +
            currentForwardRate_*boost::numeric::ublas::identity_matrix<Real>(
                    mesher_->layout()->size());

        return retVal;
    }

}
