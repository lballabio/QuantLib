/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

#include <ql/experimental/finitedifferences/fdhestondoublebarrierengine.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdividendhandler.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/pricingengines/barrier/fdhestonrebateengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <utility>

namespace QuantLib {

    FdHestonDoubleBarrierEngine::FdHestonDoubleBarrierEngine(
        const ext::shared_ptr<HestonModel>& model,
        Size tGrid,
        Size xGrid,
        Size vGrid,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        ext::shared_ptr<LocalVolTermStructure> leverageFct,
        const Real mixingFactor)
    : GenericModelEngine<HestonModel, DoubleBarrierOption::arguments, DoubleBarrierOption::results>(
          model),
      tGrid_(tGrid), xGrid_(xGrid), vGrid_(vGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), leverageFct_(std::move(leverageFct)), mixingFactor_(mixingFactor) {}

    void FdHestonDoubleBarrierEngine::calculate() const {

        QL_REQUIRE(arguments_.barrierType == DoubleBarrier::KnockOut,
                "only Knock-Out double barrier options are supported");

        // 1. Mesher
        const ext::shared_ptr<HestonProcess>& process = model_->process();
        const Time maturity = process->time(arguments_.exercise->lastDate());

        // 1.1 The variance mesher
        const Size tGridMin = 5;
        const Size tGridAvgSteps = std::max(tGridMin, tGrid_/50);

        const ext::shared_ptr<FdmHestonLocalVolatilityVarianceMesher> vMesher
            = ext::make_shared<FdmHestonLocalVolatilityVarianceMesher>(
                  vGrid_, process, leverageFct_, maturity, tGridAvgSteps, 0.0001, mixingFactor_);

        // 1.2 The equity mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        Real xMin = std::log(arguments_.barrier_lo);
        Real xMax = std::log(arguments_.barrier_hi);

        const ext::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(
                xGrid_,
                FdmBlackScholesMesher::processHelper(
                    process->s0(), process->dividendYield(),
                    process->riskFreeRate(), vMesher->volaEstimate()),
                maturity, payoff->strike(), xMin, xMax));

        const ext::shared_ptr<FdmMesher> mesher (
            new FdmMesherComposite(equityMesher, vMesher));

        // 2. Calculator
        const ext::shared_ptr<FdmInnerValueCalculator> calculator(
                                new FdmLogInnerValue(payoff, mesher, 0));

        // 3. Step conditions
        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "only european style option are supported");

        ext::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 4. Boundary conditions
        FdmBoundaryConditionSet boundaries;
        boundaries.push_back(FdmBoundaryConditionSet::value_type(
            new FdmDirichletBoundary(mesher, arguments_.rebate, 0,
                                     FdmDirichletBoundary::Lower)));

        boundaries.push_back(FdmBoundaryConditionSet::value_type(
            new FdmDirichletBoundary(mesher, arguments_.rebate, 0,
                                     FdmDirichletBoundary::Upper)));

        // 5. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     calculator, maturity,
                                     tGrid_, dampingSteps_ };

        ext::shared_ptr<FdmHestonSolver> solver(new FdmHestonSolver(
                    Handle<HestonProcess>(process), solverDesc, schemeDesc_,
                    Handle<FdmQuantoHelper>(), leverageFct_, mixingFactor_));

        const Real spot = process->s0()->value();
        results_.value = solver->valueAt(spot, process->v0());
        results_.delta = solver->deltaAt(spot, process->v0());
        results_.gamma = solver->gammaAt(spot, process->v0());
        results_.theta = solver->thetaAt(spot, process->v0());
    }
}
