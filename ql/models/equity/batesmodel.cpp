/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

#include <ql/models/equity/batesmodel.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    BatesModel::BatesModel(const ext::shared_ptr<BatesProcess> & process)
    : HestonModel(process) {
        arguments_.resize(8);

        arguments_[5] 
                = ConstantParameter(process->nu(),     NoConstraint());
        arguments_[6] 
                = ConstantParameter(process->delta(),  PositiveConstraint());
        arguments_[7] 
                = ConstantParameter(process->lambda(), PositiveConstraint());
        
        BatesModel::generateArguments();
    }

    void BatesModel::generateArguments() {
        process_ = ext::make_shared<BatesProcess>(
             process_->riskFreeRate(), process_->dividendYield(),
             process_->s0(), v0(), 
             kappa(), theta(), sigma(), rho(),
             lambda(), nu(), delta());
    }

    BatesDetJumpModel::BatesDetJumpModel(
            const ext::shared_ptr<BatesProcess> & process,
            Real kappaLambda, Real thetaLambda)
    : BatesModel(process) {
        arguments_.resize(10);

        arguments_[8] =
            ConstantParameter(kappaLambda, PositiveConstraint());
        arguments_[9] =
            ConstantParameter(thetaLambda, PositiveConstraint());
    }


    BatesDoubleExpModel::BatesDoubleExpModel(
        const ext::shared_ptr<HestonProcess> & process,
        Real lambda, Real nuUp, Real nuDown, Real p)
    : HestonModel(process) {
        arguments_.resize(9);

        arguments_[5] = ConstantParameter(p,
                                          BoundaryConstraint(0.0, 1.0));
        arguments_[6] = ConstantParameter(nuDown, PositiveConstraint());
        arguments_[7] = ConstantParameter(nuUp,   PositiveConstraint());
        arguments_[8] = ConstantParameter(lambda, PositiveConstraint());
    }


    BatesDoubleExpDetJumpModel::BatesDoubleExpDetJumpModel(
        const ext::shared_ptr<HestonProcess> & process,
        Real lambda, Real nuUp, Real nuDown,
        Real p, Real kappaLambda, Real thetaLambda)
    : BatesDoubleExpModel(process, lambda, nuUp, nuDown, p) {
        arguments_.resize(11);

        arguments_[9]  =
            ConstantParameter(kappaLambda, PositiveConstraint());
        arguments_[10] =
            ConstantParameter(thetaLambda, PositiveConstraint());
    }
}

