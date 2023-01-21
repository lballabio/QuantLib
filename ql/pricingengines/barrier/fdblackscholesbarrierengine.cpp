/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
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

#include <ql/exercise.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdividendhandler.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdblackscholesrebateengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <utility>

namespace QuantLib {

    FdBlackScholesBarrierEngine::FdBlackScholesBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        bool localVol,
        Real illegalLocalVolOverwrite)
    : process_(std::move(process)), explicitDividends_(false),
      tGrid_(tGrid), xGrid_(xGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite) {

        registerWith(process_);
    }

    FdBlackScholesBarrierEngine::FdBlackScholesBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        DividendSchedule dividends,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        bool localVol,
        Real illegalLocalVolOverwrite)
    : process_(std::move(process)), dividends_(std::move(dividends)), explicitDividends_(true),
      tGrid_(tGrid), xGrid_(xGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite) {

        registerWith(process_);
    }

    void FdBlackScholesBarrierEngine::calculate() const {

        // dividends will eventually be moved out of arguments, but for now we need the switch
        const DividendSchedule& dividendSchedule = explicitDividends_ ? dividends_ : arguments_.cashFlow;

        // 1. Mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        QL_REQUIRE(payoff, "non-striked type payoff given");
        QL_REQUIRE(payoff->strike() > 0.0, "strike must be positive");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "only european style option are supported");

        const auto spot = process_->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        QL_REQUIRE(!triggered(spot), "barrier touched");

        const Time maturity = process_->time(arguments_.exercise->lastDate());

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
                xGrid_, process_, maturity, payoff->strike(),
                xMin, xMax, 0.0001, 1.5,
                std::make_pair(Null<Real>(), Null<Real>()),
                dividendSchedule));
        
        const ext::shared_ptr<FdmMesher> mesher (
            ext::make_shared<FdmMesherComposite>(equityMesher));

        // 2. Calculator
        ext::shared_ptr<FdmInnerValueCalculator> calculator(
            ext::make_shared<FdmLogInnerValue>(payoff, mesher, 0));

        // 3. Step conditions
        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 3.1 Step condition if discrete dividends
        ext::shared_ptr<FdmDividendHandler> dividendCondition(
            ext::make_shared<FdmDividendHandler>(dividendSchedule, mesher,
                                   process_->riskFreeRate()->referenceDate(),
                                   process_->riskFreeRate()->dayCounter(), 0));

        if (!dividendSchedule.empty()) {
            stepConditions.push_back(dividendCondition);
            std::vector<Time> dividendTimes = dividendCondition->dividendTimes();
            // this effectively excludes times after maturity
            for (auto& t: dividendTimes)
                t = std::min(maturity, t);
            stoppingTimes.push_back(dividendTimes);
        }

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
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions, calculator,
                                     maturity, tGrid_, dampingSteps_ };

        ext::shared_ptr<FdmBlackScholesSolver> solver(
            ext::make_shared<FdmBlackScholesSolver>(
                               Handle<GeneralizedBlackScholesProcess>(process_),
                               payoff->strike(), solverDesc, schemeDesc_,
                               localVol_, illegalLocalVolOverwrite_));

        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);

        // 6. Calculate vanilla option and rebate for in-barriers
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::UpIn) {
            // Cast the payoff
            ext::shared_ptr<StrikedTypePayoff> payoff =
                    ext::dynamic_pointer_cast<StrikedTypePayoff>(
                                                            arguments_.payoff);
            // Calculate the vanilla option
            
            ext::shared_ptr<DividendVanillaOption> vanillaOption(
                ext::make_shared<DividendVanillaOption>(payoff,arguments_.exercise,
                                          dividendCondition->dividendDates(), 
                                          dividendCondition->dividends()));
            
            vanillaOption->setPricingEngine(
                ext::make_shared<FdBlackScholesVanillaEngine>(
                        process_, tGrid_, xGrid_,
                        0, // dampingSteps
                        schemeDesc_, localVol_, illegalLocalVolOverwrite_));

            // Calculate the rebate value
            ext::shared_ptr<DividendBarrierOption> rebateOption(
                ext::make_shared<DividendBarrierOption>(arguments_.barrierType,
                                          arguments_.barrier,
                                          arguments_.rebate,
                                          payoff, arguments_.exercise,
                                          dividendCondition->dividendDates(), 
                                          dividendCondition->dividends()));
            
            const Size min_grid_size = 50;
            const Size rebateDampingSteps 
                = (dampingSteps_ > 0) ? std::min(Size(1), dampingSteps_/2) : 0; 

            rebateOption->setPricingEngine(ext::make_shared<FdBlackScholesRebateEngine>(
                            process_, tGrid_, std::max(min_grid_size, xGrid_/5), 
                            rebateDampingSteps, schemeDesc_, localVol_, 
                            illegalLocalVolOverwrite_));

            results_.value = vanillaOption->NPV()   + rebateOption->NPV()
                                                    - results_.value;
            results_.delta = vanillaOption->delta() + rebateOption->delta()
                                                    - results_.delta;
            results_.gamma = vanillaOption->gamma() + rebateOption->gamma()
                                                    - results_.gamma;
            results_.theta = vanillaOption->theta() + rebateOption->theta()
                                                    - results_.theta;
        }
    }
}
