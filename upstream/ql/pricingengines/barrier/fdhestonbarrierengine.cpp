/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdividendhandler.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdhestonrebateengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <utility>

namespace QuantLib {

    FdHestonBarrierEngine::FdHestonBarrierEngine(const ext::shared_ptr<HestonModel>& model,
                                                 Size tGrid,
                                                 Size xGrid,
                                                 Size vGrid,
                                                 Size dampingSteps,
                                                 const FdmSchemeDesc& schemeDesc,
                                                 ext::shared_ptr<LocalVolTermStructure> leverageFct,
                                                 const Real mixingFactor)
    : GenericModelEngine<HestonModel,
                         BarrierOption::arguments,
                         BarrierOption::results>(model),
      tGrid_(tGrid), xGrid_(xGrid), vGrid_(vGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), leverageFct_(std::move(leverageFct)), mixingFactor_(mixingFactor) {}

    FdHestonBarrierEngine::FdHestonBarrierEngine(const ext::shared_ptr<HestonModel>& model,
                                                 DividendSchedule dividends,
                                                 Size tGrid,
                                                 Size xGrid,
                                                 Size vGrid,
                                                 Size dampingSteps,
                                                 const FdmSchemeDesc& schemeDesc,
                                                 ext::shared_ptr<LocalVolTermStructure> leverageFct,
                                                 const Real mixingFactor)
    : GenericModelEngine<HestonModel,
                         BarrierOption::arguments,
                         BarrierOption::results>(model),
      dividends_(std::move(dividends)),
      tGrid_(tGrid), xGrid_(xGrid), vGrid_(vGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), leverageFct_(std::move(leverageFct)), mixingFactor_(mixingFactor) {}

    void FdHestonBarrierEngine::calculate() const {

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

        Real xMin=Null<Real>();
        Real xMax=Null<Real>();
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::DownOut) {
            xMin = std::log(arguments_.barrier);
        }
        if (   arguments_.barrierType == Barrier::UpIn
            || arguments_.barrierType == Barrier::UpOut) {
            xMax = std::log(arguments_.barrier);
        }

        const ext::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(
                xGrid_,
                FdmBlackScholesMesher::processHelper(
                    process->s0(), process->dividendYield(),
                    process->riskFreeRate(), vMesher->volaEstimate()),
                maturity, payoff->strike(),
                xMin, xMax, 0.0001, 1.5,
                std::make_pair(Null<Real>(), Null<Real>()),
                dividends_));

        const ext::shared_ptr<FdmMesher> mesher (
			ext::make_shared<FdmMesherComposite>(equityMesher, vMesher));

        // 2. Calculator
        ext::shared_ptr<FdmInnerValueCalculator> calculator(
			ext::make_shared<FdmLogInnerValue>(payoff, mesher, 0));

        // 3. Step conditions
        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 3.1 Step condition if discrete dividends
        ext::shared_ptr<FdmDividendHandler> dividendCondition(
			ext::make_shared<FdmDividendHandler>(dividends_, mesher,
                                   process->riskFreeRate()->referenceDate(),
                                   process->riskFreeRate()->dayCounter(), 0));

        if (!dividends_.empty()) {
            stepConditions.push_back(dividendCondition);
            std::vector<Time> dividendTimes = dividendCondition->dividendTimes();
            // this effectively excludes times after maturity
            for (auto& t: dividendTimes)
                t = std::min(maturity, t);
            stoppingTimes.push_back(dividendTimes);
        }

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "only european style option are supported");

        ext::shared_ptr<FdmStepConditionComposite> conditions(
			ext::make_shared<FdmStepConditionComposite>(stoppingTimes, stepConditions));

        // 4. Boundary conditions
        FdmBoundaryConditionSet boundaries;
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::DownOut) {
            boundaries.push_back(
				ext::make_shared<FdmDirichletBoundary>(mesher, arguments_.rebate, 0,
                                         FdmDirichletBoundary::Lower));

        }
        if (   arguments_.barrierType == Barrier::UpIn
            || arguments_.barrierType == Barrier::UpOut) {
            boundaries.push_back(
				ext::make_shared<FdmDirichletBoundary>(mesher, arguments_.rebate, 0,
                                         FdmDirichletBoundary::Upper));
        }

        // 5. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     calculator, maturity,
                                     tGrid_, dampingSteps_ };

        ext::shared_ptr<FdmHestonSolver> solver(ext::make_shared<FdmHestonSolver>(
                    Handle<HestonProcess>(process), solverDesc, schemeDesc_,
                    Handle<FdmQuantoHelper>(), leverageFct_, mixingFactor_));

        const Real spot = process->s0()->value();
        results_.value = solver->valueAt(spot, process->v0());
        results_.delta = solver->deltaAt(spot, process->v0());
        results_.gamma = solver->gammaAt(spot, process->v0());
        results_.theta = solver->thetaAt(spot, process->v0());

        // 6. Calculate vanilla option and rebate for in-barriers
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::UpIn) {
            // Cast the payoff
            ext::shared_ptr<StrikedTypePayoff> payoff =
                    ext::dynamic_pointer_cast<StrikedTypePayoff>(
                                                            arguments_.payoff);
            // Calculate the vanilla option
            VanillaOption vanillaOption(payoff, arguments_.exercise);
            vanillaOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
				ext::make_shared<FdHestonVanillaEngine>(*model_, dividends_,
                                                        tGrid_, xGrid_,
                                                        vGrid_, dampingSteps_,
                                                        schemeDesc_)));
            // Calculate the rebate value
            BarrierOption rebateOption(arguments_.barrierType,
                                       arguments_.barrier,
                                       arguments_.rebate,
                                       payoff, arguments_.exercise);
            const Size xGridMin = 20;
            const Size vGridMin = 10;
            const Size rebateDampingSteps 
                = (dampingSteps_ > 0) ? std::min(Size(1), dampingSteps_/2) : 0; 
            rebateOption.setPricingEngine(
                ext::make_shared<FdHestonRebateEngine>(*model_, dividends_,
                                                       tGrid_,
                                                       std::max(xGridMin, xGrid_/4), 
                                                       std::max(vGridMin, vGrid_/4),
                                                       rebateDampingSteps,
                                                       schemeDesc_));

            results_.value = vanillaOption.NPV()   + rebateOption.NPV()   - results_.value;
            results_.delta = vanillaOption.delta() + rebateOption.delta() - results_.delta;
            results_.gamma = vanillaOption.gamma() + rebateOption.gamma() - results_.gamma;
            results_.theta = vanillaOption.theta() + rebateOption.theta() - results_.theta;
        }
    }
}
