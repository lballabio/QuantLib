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

#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/douglasscheme.hpp>
#include <ql/experimental/finitedifferences/craigsneydscheme.hpp>
#include <ql/experimental/finitedifferences/hundsdorferscheme.hpp>
#include <ql/experimental/finitedifferences/impliciteulerscheme.hpp>
#include <ql/experimental/finitedifferences/expliciteulerscheme.hpp>
#include <ql/experimental/finitedifferences/modifiedcraigsneydscheme.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>

namespace QuantLib {

    FdmBackwardSolver::FdmBackwardSolver(
        const boost::shared_ptr<FdmLinearOpComposite>& map,
        const FdmBoundaryConditionSet& bcSet,
        const boost::shared_ptr<FdmStepConditionComposite> condition,
        FdmBackwardSolver::FdmSchemeType schemeType,
        Real theta, 
        Real mu)
    : map_(map), bcSet_(bcSet),
      condition_((condition) ? condition 
                             : boost::shared_ptr<FdmStepConditionComposite>(
                                 new FdmStepConditionComposite(
                                     std::list<std::vector<Time> >(),
                                     FdmStepConditionComposite::Conditions()))),
      schemeType_(schemeType),
      theta_(theta), mu_(mu) {
     }
        
    void FdmBackwardSolver::rollback(FdmBackwardSolver::array_type& rhs, 
                                     Time from, Time to,
                                     Size steps, Size dampingSteps) {

        const Time deltaT = from - to;
        const Size allSteps = steps + dampingSteps;
        const Time dampingTo = from - (deltaT*dampingSteps)/allSteps;
                    
        if (dampingSteps && schemeType_ != ImplicitEuler) {
            ImplicitEulerScheme implicitEvolver(map_, bcSet_);    
            FiniteDifferenceModel<ImplicitEulerScheme> 
                    dampingModel(implicitEvolver, condition_->stoppingTimes());
            dampingModel.rollback(rhs, from, dampingTo, 
                                  dampingSteps, *condition_);
        }
        
        switch (schemeType_) {
          case Hundsdorfer:
            {
                HundsdorferScheme hsEvolver(theta_, mu_, map_, bcSet_);
                FiniteDifferenceModel<HundsdorferScheme> 
                               hsModel(hsEvolver, condition_->stoppingTimes());
                hsModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case Douglas:
            {
                DouglasScheme dsEvolver(theta_, map_, bcSet_);
                FiniteDifferenceModel<DouglasScheme> 
                               dsModel(dsEvolver, condition_->stoppingTimes());
                dsModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case CraigSneyd:
            {
                CraigSneydScheme csEvolver(theta_, mu_, map_, bcSet_);
                FiniteDifferenceModel<CraigSneydScheme> 
                               csModel(csEvolver, condition_->stoppingTimes());
                csModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case ModifiedCraigSneyd:
            {
                ModifiedCraigSneydScheme csEvolver(theta_, mu_, map_, bcSet_);
                FiniteDifferenceModel<ModifiedCraigSneydScheme> 
                              mcsModel(csEvolver, condition_->stoppingTimes());
                mcsModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          case ImplicitEuler:
            {
                ImplicitEulerScheme implicitEvolver(map_, bcSet_);
                FiniteDifferenceModel<ImplicitEulerScheme> 
                   implicitModel(implicitEvolver, condition_->stoppingTimes());
                implicitModel.rollback(rhs, from, to, allSteps, *condition_);
            }
            break;
          case ExplicitEuler:
            {
                ExplicitEulerScheme explicitEvolver(map_, bcSet_);
                FiniteDifferenceModel<ExplicitEulerScheme> 
                   explicitModel(explicitEvolver, condition_->stoppingTimes());
                explicitModel.rollback(rhs, dampingTo, to, steps, *condition_);
            }
            break;
          default:
            QL_FAIL("Unknown scheme type");
        }
    }
}
