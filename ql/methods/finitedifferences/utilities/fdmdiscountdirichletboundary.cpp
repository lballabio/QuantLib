/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 Klaus Spanderen

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


/*! \file fdmdiscountdirichletboundary.cpp */

#include <ql/methods/finitedifferences/utilities/fdmdiscountdirichletboundary.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    namespace {
        class DiscountedCashflowAtBoundary {
          public:
            DiscountedCashflowAtBoundary(Time maturityTime,
                                         Real valueOnBoundary,
                                         ext::shared_ptr<YieldTermStructure> rTS)
            : maturityTime_(maturityTime), cashFlow_(valueOnBoundary), rTS_(std::move(rTS)) {}

            Real operator()(Real t) const {
                return cashFlow_
                    * rTS_->discount(maturityTime_)/rTS_->discount(t);
            }

          private:
            const Time maturityTime_;
            const Real cashFlow_;
            const ext::shared_ptr<YieldTermStructure> rTS_;
        };
    }

    FdmDiscountDirichletBoundary::FdmDiscountDirichletBoundary(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        Time maturityTime,
        Real valueOnBoundary,
        Size direction, Side side)
    : bc_(ext::make_shared<FdmTimeDepDirichletBoundary>(
            mesher,
            std::function<Real (Real)>(
                DiscountedCashflowAtBoundary(
                    maturityTime, valueOnBoundary, rTS)),
            direction, side)) {
    }

    void FdmDiscountDirichletBoundary::setTime(Time t) {
        bc_->setTime(t);
    }
    void FdmDiscountDirichletBoundary::applyBeforeApplying(
        operator_type& op) const {
        bc_->applyBeforeApplying(op);
    }
    void FdmDiscountDirichletBoundary::applyBeforeSolving(
        operator_type& op, array_type& r) const {
        bc_->applyBeforeSolving(op, r);
    }
    void FdmDiscountDirichletBoundary::applyAfterApplying(array_type& r) const {
        bc_->applyAfterApplying(r);
    }
    void FdmDiscountDirichletBoundary::applyAfterSolving(array_type& r) const {
        bc_->applyAfterSolving(r);
    }
}
