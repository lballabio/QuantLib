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

#include <ql/experimental/finitedifferences/fdblackscholesbarrierengine.hpp>
#include <ql/exercise.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/experimental/finitedifferences/fdblackscholesbarrierengine.hpp>
#include <ql/experimental/finitedifferences/fdmdividendhandler.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholessolver.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesmesher.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdblackscholesrebateengine.hpp>
#include <ql/experimental/finitedifferences/fdblackscholesvanillaengine.hpp>

namespace QuantLib {

    FdBlackScholesBarrierEngine::FdBlackScholesBarrierEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Size tGrid, Size xGrid, Size dampingSteps, 
            const FdmSchemeDesc& schemeDesc,
            bool localVol, Real illegalLocalVolOverwrite)
    : GenericEngine<DividendBarrierOption::arguments,
                    DividendBarrierOption::results>(),
      process_(process), tGrid_(tGrid), xGrid_(xGrid), 
      dampingSteps_(dampingSteps), schemeDesc_(schemeDesc),
      localVol_(localVol), illegalLocalVolOverwrite_(illegalLocalVolOverwrite){
    }

    void FdBlackScholesBarrierEngine::calculate() const {

        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        const boost::shared_ptr<FdmLinearOpLayout> layout(
                                              new FdmLinearOpLayout(dim));

        // 2. Mesher
        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
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

        const boost::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(xGrid_, process_, maturity,
                                      payoff->strike(), xMin, xMax));
        
        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(equityMesher);
        boost::shared_ptr<FdmMesher> mesher (
                                     new FdmMesherComposite(layout, meshers));

        // 3. Calculator
        boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                new FdmLogInnerValue(payoff, mesher, 0));

        // 4. Step conditions
        std::list<boost::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 4.1 Step condition if discrete dividends
        boost::shared_ptr<FdmDividendHandler> dividendCondition(
            new FdmDividendHandler(arguments_.cashFlow, mesher,
                                   process_->riskFreeRate()->referenceDate(),
                                   process_->riskFreeRate()->dayCounter(), 0));

        if(!arguments_.cashFlow.empty()) {
            stepConditions.push_back(dividendCondition);
            stoppingTimes.push_back(dividendCondition->dividendTimes());
        }

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "only european style option are supported");

        boost::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 5. Boundary conditions
        std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::DownOut) {
            boundaries.push_back(boost::shared_ptr<FdmDirichletBoundary>(
                new FdmDirichletBoundary(mesher, arguments_.rebate, 0,
                                         FdmDirichletBoundary::Lower)));

        }

        if (   arguments_.barrierType == Barrier::UpIn
            || arguments_.barrierType == Barrier::UpOut) {
            boundaries.push_back(boost::shared_ptr<FdmDirichletBoundary>(
                new FdmDirichletBoundary(mesher, arguments_.rebate, 0,
                                         FdmDirichletBoundary::Upper)));
        }

        // 6. Solver
        boost::shared_ptr<FdmBlackScholesSolver> solver(
                new FdmBlackScholesSolver(
                                Handle<GeneralizedBlackScholesProcess>(process_),
                                mesher, boundaries, conditions, calculator,
                                payoff->strike(), maturity, tGrid_,
                                dampingSteps_, schemeDesc_, 
                                localVol_, illegalLocalVolOverwrite_));

        const Real spot = process_->x0();
        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);

        // 7. Calculate vanilla option and rebate for in-barriers
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::UpIn) {
            // Cast the payoff
            boost::shared_ptr<StrikedTypePayoff> payoff =
                    boost::dynamic_pointer_cast<StrikedTypePayoff>(
                                                            arguments_.payoff);
            // Calculate the vanilla option
            
            boost::shared_ptr<DividendVanillaOption> vanillaOption(
                new DividendVanillaOption(payoff,arguments_.exercise,
                                          dividendCondition->dividendDates(), 
                                          dividendCondition->dividends()));
            
            vanillaOption->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new FdBlackScholesVanillaEngine(
                        process_, tGrid_, xGrid_,
                        0, // dampingSteps
                        schemeDesc_, localVol_, illegalLocalVolOverwrite_)));

            // Calculate the rebate value
            boost::shared_ptr<DividendBarrierOption> rebateOption(
                new DividendBarrierOption(arguments_.barrierType,
                                          arguments_.barrier,
                                          arguments_.rebate,
                                          payoff, arguments_.exercise,
                                          dividendCondition->dividendDates(), 
                                          dividendCondition->dividends()));
            
            const Size min_grid_size = 50;
            const Size rebateDampingSteps 
                = (dampingSteps_ > 0) ? std::min(Size(1), dampingSteps_/2) : 0; 

            rebateOption->setPricingEngine(boost::shared_ptr<PricingEngine>(
                    new FdBlackScholesRebateEngine(
                            process_, tGrid_, std::max(min_grid_size, xGrid_/5), 
                            rebateDampingSteps, schemeDesc_, localVol_, 
                            illegalLocalVolOverwrite_)));

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
