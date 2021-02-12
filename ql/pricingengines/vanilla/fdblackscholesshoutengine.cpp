/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

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

/*! \file fdblackscholesshoutengine.cpp
*/

#include <ql/exercise.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmshoutloginnervaluecalculator.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesshoutengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

namespace QuantLib {

    FdBlackScholesShoutEngine::FdBlackScholesShoutEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc)
    : process_(std::move(process)), tGrid_(tGrid), xGrid_(xGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc) {

        registerWith(process_);
    }

    void FdBlackScholesShoutEngine::calculate() const {
        const Date exerciseDate = arguments_.exercise->lastDate();
        const Time maturity = process_->time(exerciseDate);

        const auto payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);

        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        const auto mesher = ext::make_shared<FdmMesherComposite>(
            ext::make_shared<FdmBlackScholesMesher>(
                xGrid_, process_, maturity, payoff->strike(),
                Null<Real>(), Null<Real>(), 0.0001, 1.5,
                std::pair<Real, Real>(payoff->strike(), 0.1)));

        const auto innerValuecalculator =
            ext::make_shared<FdmShoutLogInnerValueCalculator>(
                process_, maturity, payoff, mesher, 0);

        const auto conditions =
            FdmStepConditionComposite::vanillaComposite(
                DividendSchedule(), arguments_.exercise, mesher,
                innerValuecalculator,
                process_->riskFreeRate()->referenceDate(),
                process_->riskFreeRate()->dayCounter());

        const FdmSolverDesc solverDesc = {
            mesher, FdmBoundaryConditionSet(),
            conditions, innerValuecalculator,
            maturity, tGrid_, dampingSteps_ };

        const auto solver =
            ext::make_shared<FdmBlackScholesSolver>(
                Handle<GeneralizedBlackScholesProcess>(process_),
                payoff->strike(), solverDesc, schemeDesc_);

        const Real spot = process_->x0();

        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);
    }
}
