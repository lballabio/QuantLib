/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen
 Copyright (C) 2026 Jogi Sidhu

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

#include <ql/exercise.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmamericanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmbermudanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdividendhandler.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/fdmtimedepdirichletboundary.hpp>
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
    : process_(std::move(process)),
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
    : process_(std::move(process)), dividends_(std::move(dividends)),
      tGrid_(tGrid), xGrid_(xGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite) {

        registerWith(process_);
    }

    void FdBlackScholesBarrierEngine::calculate() const {

        // 1. Mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        QL_REQUIRE(payoff, "non-striked type payoff given");
        QL_REQUIRE(payoff->strike() > 0.0, "strike must be positive");

        const Exercise::Type exerciseType = arguments_.exercise->type();
        QL_REQUIRE(   exerciseType == Exercise::European
                   || exerciseType == Exercise::American
                   || exerciseType == Exercise::Bermudan,
                   "exercise type is not supported");

        // knock-in options are priced via in-out parity,
        // which does not hold for early exercise
        QL_REQUIRE(   exerciseType == Exercise::European
                   || arguments_.barrierType == Barrier::DownOut
                   || arguments_.barrierType == Barrier::UpOut,
                   "only european style knock-in options are supported");

        const auto spot = process_->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        QL_REQUIRE(!triggered(spot), "barrier touched");

        // on the barrier the option is knocked out before it can be
        // exercised, while just inside the holder would exercise: when
        // the intrinsic value exceeds the rebate, the value jumps there
        // and the greeks are undefined
        if (   exerciseType == Exercise::American
            && process_->time(arguments_.exercise->date(0)) <= 0.0
            && close_enough(spot, arguments_.barrier)
            && (*payoff)(arguments_.barrier) > arguments_.rebate) {
            results_.value = arguments_.rebate;
            return;
        }

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

        const auto equityMesher = ext::make_shared<FdmBlackScholesMesher>(
                xGrid_, process_, maturity, payoff->strike(),
                xMin, xMax, 0.0001, 1.5,
                std::make_pair(Null<Real>(), Null<Real>()),
                dividends_);
        
        const auto mesher = ext::make_shared<FdmMesherComposite>(equityMesher);

        // 2. Calculator
        auto calculator = ext::make_shared<FdmLogInnerValue>(payoff, mesher, 0);

        // 3. Step conditions
        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 3.1 Step condition if discrete dividends
        auto dividendCondition = ext::make_shared<FdmDividendHandler>(dividends_, mesher,
                                   process_->riskFreeRate()->referenceDate(),
                                   process_->riskFreeRate()->dayCounter(), 0);

        if (!dividends_.empty()) {
            stepConditions.push_back(dividendCondition);
            std::vector<Time> dividendTimes = dividendCondition->dividendTimes();
            // this effectively excludes times after maturity
            for (auto& t: dividendTimes)
                t = std::min(maturity, t);
            stoppingTimes.push_back(dividendTimes);
        }

        // 3.2 Step condition if early exercise (knock-out options only)
        Time exerciseStart = Null<Time>();
        if (exerciseType == Exercise::American) {
            exerciseStart = process_->time(arguments_.exercise->date(0));
            stepConditions.push_back(
                ext::make_shared<FdmAmericanStepCondition>(mesher, calculator, exerciseStart));
        } else if (exerciseType == Exercise::Bermudan) {
            auto bermudanCondition = ext::make_shared<FdmBermudanStepCondition>(
                arguments_.exercise->dates(),
                process_->riskFreeRate()->referenceDate(),
                process_->riskFreeRate()->dayCounter(),
                mesher, calculator);
            stepConditions.push_back(bermudanCondition);
            stoppingTimes.push_back(bermudanCondition->exerciseTimes());
        }

        auto conditions = ext::make_shared<FdmStepConditionComposite>(stoppingTimes, stepConditions);

        // 4. Boundary conditions
        const auto side =
            (   arguments_.barrierType == Barrier::DownIn
             || arguments_.barrierType == Barrier::DownOut)
            ? FdmDirichletBoundary::Lower : FdmDirichletBoundary::Upper;

        FdmBoundaryConditionSet boundaries;
        if (exerciseType == Exercise::American) {
            // once exercisable, the holder exercises rather than being
            // knocked out for less than the intrinsic value at the barrier
            const Real rebate = arguments_.rebate;
            const Real exerciseValue = std::max(rebate, (*payoff)(arguments_.barrier));
            boundaries.push_back(ext::make_shared<FdmTimeDepDirichletBoundary>(
                mesher,
                [=](Real t) { return t >= exerciseStart ? exerciseValue : rebate; },
                0, side));
        } else {
            boundaries.push_back(
                ext::make_shared<FdmDirichletBoundary>(mesher, arguments_.rebate, 0, side));
        }

        // 5. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions, calculator,
                                     maturity, tGrid_, dampingSteps_ };

        auto solver = ext::make_shared<FdmBlackScholesSolver>(
                               Handle<GeneralizedBlackScholesProcess>(process_),
                               payoff->strike(), solverDesc, schemeDesc_,
                               localVol_, illegalLocalVolOverwrite_);

        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);

        // As in FdBlackScholesVanillaEngine, the step condition enforces
        // the exercise value on grid nodes only and the interpolant can
        // undershoot it in between; floor the value when the holder can
        // exercise on the reference date and the payoff is paid then.
        // Greeks stay on the interpolant.
        const Date settlementDate = process_->riskFreeRate()->referenceDate();
        bool exercisable = false;
        if (exerciseType == Exercise::American) {
            exercisable = arguments_.exercise->date(0) <= settlementDate
                          && settlementDate <= arguments_.exercise->lastDate();
        } else if (exerciseType == Exercise::Bermudan) {
            for (const Date& exerciseDate : arguments_.exercise->dates()) {
                if (exerciseDate == settlementDate) {
                    exercisable = true;
                    break;
                }
            }
        }
        if (exercisable) {
            const auto early = ext::dynamic_pointer_cast<EarlyExercise>(arguments_.exercise);
            if (early != nullptr && early->payoffAtExpiry())
                exercisable = false;
        }
        if (exercisable) {
            const Real exerciseValue = (*payoff)(spot);
            if (results_.value < exerciseValue)
                results_.value = exerciseValue;
        }

        // 6. Calculate vanilla option and rebate for in-barriers
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::UpIn) {
            // Cast the payoff
            ext::shared_ptr<StrikedTypePayoff> payoff =
                    ext::dynamic_pointer_cast<StrikedTypePayoff>(
                                                            arguments_.payoff);
            // Calculate the vanilla option
            
            VanillaOption vanillaOption(payoff, arguments_.exercise);
            
            vanillaOption.setPricingEngine(
                ext::make_shared<FdBlackScholesVanillaEngine>(
                        process_, dividends_, tGrid_, xGrid_,
                        0, // dampingSteps
                        schemeDesc_, localVol_, illegalLocalVolOverwrite_));

            // Calculate the rebate value
            BarrierOption rebateOption(arguments_.barrierType,
                                       arguments_.barrier,
                                       arguments_.rebate,
                                       payoff, arguments_.exercise);
            
            const Size min_grid_size = 50;
            const Size rebateDampingSteps 
                = (dampingSteps_ > 0) ? std::min(Size(1), dampingSteps_/2) : 0; 

            rebateOption.setPricingEngine(ext::make_shared<FdBlackScholesRebateEngine>(
                            process_, dividends_, tGrid_, std::max(min_grid_size, xGrid_/5), 
                            rebateDampingSteps, schemeDesc_, localVol_, 
                            illegalLocalVolOverwrite_));

            results_.value = vanillaOption.NPV()   + rebateOption.NPV()   - results_.value;
            results_.delta = vanillaOption.delta() + rebateOption.delta() - results_.delta;
            results_.gamma = vanillaOption.gamma() + rebateOption.gamma() - results_.gamma;
            results_.theta = vanillaOption.theta() + rebateOption.theta() - results_.theta;
        }
    }
}
