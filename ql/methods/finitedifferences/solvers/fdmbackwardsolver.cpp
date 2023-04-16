/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Andreas Gaida
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file fdmbackwardsolver.cpp
*/

#include <ql/mathconstants.hpp>
#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/methods/finitedifferences/schemes/craigsneydscheme.hpp>
#include <ql/methods/finitedifferences/schemes/cranknicolsonscheme.hpp>
#include <ql/methods/finitedifferences/schemes/douglasscheme.hpp>
#include <ql/methods/finitedifferences/schemes/expliciteulerscheme.hpp>
#include <ql/methods/finitedifferences/schemes/hundsdorferscheme.hpp>
#include <ql/methods/finitedifferences/schemes/impliciteulerscheme.hpp>
#include <ql/methods/finitedifferences/schemes/methodoflinesscheme.hpp>
#include <ql/methods/finitedifferences/schemes/modifiedcraigsneydscheme.hpp>
#include <ql/methods/finitedifferences/schemes/trbdf2scheme.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <utility>


namespace QuantLib {
    
    FdmSchemeDesc::FdmSchemeDesc(FdmSchemeType aType, Real aTheta, Real aMu)
    : type(aType), theta(aTheta), mu(aMu) { }

    FdmSchemeDesc FdmSchemeDesc::Douglas() { return {FdmSchemeDesc::DouglasType, 0.5, 0.0}; }

    FdmSchemeDesc FdmSchemeDesc::CrankNicolson() {
        return {FdmSchemeDesc::CrankNicolsonType, 0.5, 0.0};
    }

    FdmSchemeDesc FdmSchemeDesc::CraigSneyd() { return {FdmSchemeDesc::CraigSneydType, 0.5, 0.5}; }

    FdmSchemeDesc FdmSchemeDesc::ModifiedCraigSneyd() {
        return {FdmSchemeDesc::ModifiedCraigSneydType, 1.0 / 3.0, 1.0 / 3.0};
    }
    
    FdmSchemeDesc FdmSchemeDesc::Hundsdorfer() {
        return {FdmSchemeDesc::HundsdorferType, 0.5 + std::sqrt(3.0) / 6, 0.5};
    }
    
    FdmSchemeDesc FdmSchemeDesc::ModifiedHundsdorfer() {
        return {FdmSchemeDesc::HundsdorferType, 1.0 - std::sqrt(2.0) / 2, 0.5};
    }
    
    FdmSchemeDesc FdmSchemeDesc::ExplicitEuler() {
        return {FdmSchemeDesc::ExplicitEulerType, 0.0, 0.0};
    }

    FdmSchemeDesc FdmSchemeDesc::ImplicitEuler() {
        return {FdmSchemeDesc::ImplicitEulerType, 0.0, 0.0};
    }

    FdmSchemeDesc FdmSchemeDesc::MethodOfLines(Real eps, Real relInitStepSize) {
        return {FdmSchemeDesc::MethodOfLinesType, eps, relInitStepSize};
    }

    FdmSchemeDesc FdmSchemeDesc::TrBDF2() { return {FdmSchemeDesc::TrBDF2Type, 2 - M_SQRT2, 1e-8}; }

    FdmBackwardSolver::FdmBackwardSolver(
        std::shared_ptr<FdmLinearOpComposite> map,
        FdmBoundaryConditionSet bcSet,
        const std::shared_ptr<FdmStepConditionComposite>& condition,
        const FdmSchemeDesc& schemeDesc)
    : map_(std::move(map)), bcSet_(std::move(bcSet)),
      condition_((condition) != nullptr ?
                     condition :
                     std::make_shared<FdmStepConditionComposite>(
                         std::list<std::vector<Time> >(), FdmStepConditionComposite::Conditions())),
      schemeDesc_(schemeDesc) {}

    void FdmBackwardSolver::rollback(FdmBackwardSolver::array_type& rhs, 
                                     Time from, Time to,
                                     Size steps, Size dampingSteps) {

        const Time deltaT = from - to;
        const Size allSteps = steps + dampingSteps;
        const Time dampingTo = from - (deltaT*dampingSteps)/allSteps;

        if ((dampingSteps != 0U) && schemeDesc_.type != FdmSchemeDesc::ImplicitEulerType) {
            ImplicitEulerScheme implicitEvolver(map_, bcSet_);    
            FiniteDifferenceModel<ImplicitEulerScheme> 
                    dampingModel(implicitEvolver, condition_->stoppingTimes());
            dampingModel.rollback(rhs, from, dampingTo, 
                                  dampingSteps, *condition_);
        }

        switch (schemeDesc_.type) {
          case FdmSchemeDesc::HundsdorferType:
            {
                HundsdorferScheme hsEvolver(schemeDesc_.theta, schemeDesc_.mu, 
                                            map_, bcSet_);
                FiniteDifferenceModel<HundsdorferScheme> 
                               hsModel(hsEvolver, condition_->stoppingTimes());
                hsModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case FdmSchemeDesc::DouglasType:
            {
                DouglasScheme dsEvolver(schemeDesc_.theta, map_, bcSet_);
                FiniteDifferenceModel<DouglasScheme> 
                               dsModel(dsEvolver, condition_->stoppingTimes());
                dsModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case FdmSchemeDesc::CrankNicolsonType:
            {
              CrankNicolsonScheme cnEvolver(schemeDesc_.theta, map_, bcSet_);
              FiniteDifferenceModel<CrankNicolsonScheme>
                             cnModel(cnEvolver, condition_->stoppingTimes());
              cnModel.rollback(rhs, dampingTo, to, steps, *condition_);

            }
            break;
          case FdmSchemeDesc::CraigSneydType:
            {
                CraigSneydScheme csEvolver(schemeDesc_.theta, schemeDesc_.mu, 
                                           map_, bcSet_);
                FiniteDifferenceModel<CraigSneydScheme> 
                               csModel(csEvolver, condition_->stoppingTimes());
                csModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case FdmSchemeDesc::ModifiedCraigSneydType:
            {
                ModifiedCraigSneydScheme csEvolver(schemeDesc_.theta, 
                                                   schemeDesc_.mu,
                                                   map_, bcSet_);
                FiniteDifferenceModel<ModifiedCraigSneydScheme> 
                              mcsModel(csEvolver, condition_->stoppingTimes());
                mcsModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case FdmSchemeDesc::ImplicitEulerType:
            {
                ImplicitEulerScheme implicitEvolver(map_, bcSet_);
                FiniteDifferenceModel<ImplicitEulerScheme> 
                   implicitModel(implicitEvolver, condition_->stoppingTimes());
                implicitModel.rollback(rhs, from, to, allSteps, *condition_);
            }
            break;
          case FdmSchemeDesc::ExplicitEulerType:
            {
                ExplicitEulerScheme explicitEvolver(map_, bcSet_);
                FiniteDifferenceModel<ExplicitEulerScheme> 
                   explicitModel(explicitEvolver, condition_->stoppingTimes());
                explicitModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case FdmSchemeDesc::MethodOfLinesType:
            {
                MethodOfLinesScheme methodOfLines(
                    schemeDesc_.theta, schemeDesc_.mu, map_, bcSet_);
                FiniteDifferenceModel<MethodOfLinesScheme>
                   molModel(methodOfLines, condition_->stoppingTimes());
                molModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case FdmSchemeDesc::TrBDF2Type:
            {
                const FdmSchemeDesc trDesc
                    = FdmSchemeDesc::CraigSneyd();

                const std::shared_ptr<CraigSneydScheme> hsEvolver(
                    std::make_shared<CraigSneydScheme>(
                        trDesc.theta, trDesc.mu, map_, bcSet_));

                TrBDF2Scheme<CraigSneydScheme> trBDF2(
                    schemeDesc_.theta, map_, hsEvolver, bcSet_,schemeDesc_.mu);

                FiniteDifferenceModel<TrBDF2Scheme<CraigSneydScheme> >
                   trBDF2Model(trBDF2, condition_->stoppingTimes());
                trBDF2Model.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          default:
            QL_FAIL("Unknown scheme type");
        }
    }
}
