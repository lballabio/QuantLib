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
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmamericanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmbermudanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmpointvaluerecorder.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdividendhandler.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/fdmtimedepdirichletboundary.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdblackscholesrebateengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

namespace QuantLib {

    namespace {

        std::vector<Real> spotsOn(const ext::shared_ptr<FdmMesher>& mesher) {
            const Array& locations = mesher->locations(0);
            std::vector<Real> spots(locations.size());
            std::transform(locations.begin(), locations.end(), spots.begin(),
                           [](Real x) { return std::exp(x); });
            return spots;
        }

        /* Applied after the dividend handler on the mesher of the option
           not yet knocked in, which ends at the barrier.

           A down-and-in node that a dividend takes through the barrier is
           knocked in by the drop, so it takes the active option's value at
           its ex-dividend spot.

           On the barrier itself the value jumps at ex-dividend times.  The
           solve ending at such a time holds the barrier node at the active
           option's value before the dividend.  This condition then sets it
           to the active option's value after the dividend, which the next
           step of the rollback starts from.

           At maturity, the inactive option's payoff is the rebate on every
           node, the barrier included.  This condition replaces it there with
           the active option's payoff.
        */
        class FdmKnockInBarrierCondition : public StepCondition<Array> {
          public:
            FdmKnockInBarrierCondition(const ext::shared_ptr<FdmMesher>& mesher,
                                       const ext::shared_ptr<FdmMesher>& activeMesher,
                                       ext::shared_ptr<FdmPointValueRecorder> exDividendValues,
                                       ext::shared_ptr<FdmPointValueRecorder> cumDividendValues,
                                       std::map<Time, Real> dividends,
                                       Real barrier,
                                       bool down,
                                       Time maturity,
                                       Real terminalValue)
            : spots_(spotsOn(mesher)), activeSpots_(spotsOn(activeMesher)),
              exDividendValues_(std::move(exDividendValues)),
              cumDividendValues_(std::move(cumDividendValues)),
              dividends_(std::move(dividends)), barrier_(barrier), down_(down),
              maturity_(maturity), terminalValue_(terminalValue) {}

            void applyTo(Array& a, Time t) const override {
                const auto dividend = dividends_.find(t);
                const bool exDividend = dividend != dividends_.end();

                if (exDividend && down_) {
                    const Array& active = exDividendValues_->solutionAt(t);
                    const LinearInterpolation interpolation(
                        activeSpots_.begin(), activeSpots_.end(), active.begin());
                    for (Size i = 0; i < spots_.size(); ++i) {
                        const Real exSpot = spots_[i] - dividend->second;
                        if (exSpot < barrier_)
                            a[i] = interpolation(std::max(activeSpots_.front(), exSpot), true);
                    }
                }

                const Size barrierNode = down_ ? 0 : a.size() - 1;
                // the barrier node takes the active option's value after its dividend jump
                if (exDividend)
                    a[barrierNode] = cumDividendValues_->valueAt(t);
                else if (t == maturity_)
                    a[barrierNode] = terminalValue_;
            }

          private:
            std::vector<Real> spots_, activeSpots_;
            ext::shared_ptr<FdmPointValueRecorder> exDividendValues_, cumDividendValues_;
            std::map<Time, Real> dividends_;
            Real barrier_;
            bool down_;
            Time maturity_;
            Real terminalValue_;
        };

    }

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

        const bool knockIn =    arguments_.barrierType == Barrier::DownIn
                             || arguments_.barrierType == Barrier::UpIn;

        const auto spot = process_->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        QL_REQUIRE(!triggered(spot), "barrier touched");

        const Date settlementDate = process_->riskFreeRate()->referenceDate();
        bool exercisableToday = false;
        if (exerciseType == Exercise::American) {
            exercisableToday = arguments_.exercise->date(0) <= settlementDate
                               && settlementDate <= arguments_.exercise->lastDate();
        } else if (exerciseType == Exercise::Bermudan) {
            for (const Date& exerciseDate : arguments_.exercise->dates()) {
                if (exerciseDate == settlementDate) {
                    exercisableToday = true;
                    break;
                }
            }
        }

        // on the barrier a knock-out is knocked out before it can be
        // exercised; when the intrinsic value there exceeds the rebate,
        // the holder would exercise just inside it instead, so the value
        // jumps at the barrier and the greeks are undefined
        if (   !knockIn && exercisableToday
            && close_enough(spot, arguments_.barrier)
            && (*payoff)(arguments_.barrier) > arguments_.rebate) {
            results_.value = arguments_.rebate;
            return;
        }

        // in-out parity does not hold with early exercise
        if (knockIn && exerciseType != Exercise::European) {
            calculateEarlyExerciseKnockIn();
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
        bool exercisable = exercisableToday;
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

    void FdBlackScholesBarrierEngine::calculateEarlyExerciseKnockIn() const {

        // Two solves: the vanilla option the holder receives on knock-in
        // (the active option), whose value on the barrier is recorded at
        // each step, and the option not yet knocked in (the inactive one),
        // which cannot be exercised and takes the recorded value as its
        // boundary condition on the barrier.  A dividend can also knock in
        // a down-and-in option: the nodes it takes through the barrier then
        // get their value from the active solution.
        const auto payoff = ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        const Exercise::Type exerciseType = arguments_.exercise->type();
        const Real spot = process_->x0();
        const Time maturity = process_->time(arguments_.exercise->lastDate());
        const Real logBarrier = std::log(arguments_.barrier);
        const bool down = arguments_.barrierType == Barrier::DownIn;

        const Date referenceDate = process_->riskFreeRate()->referenceDate();
        const DayCounter dayCounter = process_->riskFreeRate()->dayCounter();

        const Date maturityDate = arguments_.exercise->lastDate();
        DividendSchedule dividendSchedule;
        std::copy_if(dividends_.begin(), dividends_.end(), std::back_inserter(dividendSchedule),
                     [&](const ext::shared_ptr<Dividend>& dividend) {
                         return dividend->date() >= referenceDate
                             && dividend->date() <= maturityDate;
                     });

        // 1. Active option: mesher as in FdBlackScholesVanillaEngine,
        //    widened if needed to keep a margin around the barrier and
        //    the lowest spot a dividend can take a down-and-in node to
        const std::pair<Real, Real> cPoint(payoff->strike(), 0.1);
        auto activeEquityMesher = ext::make_shared<FdmBlackScholesMesher>(
                xGrid_, process_, maturity, payoff->strike(),
                Null<Real>(), Null<Real>(), 0.0001, 1.5, cPoint, dividendSchedule);
        const Real xMin = activeEquityMesher->locations().front();
        const Real xMax = activeEquityMesher->locations().back();
        const Real margin = 0.1*(xMax - xMin);
        Real lowest = logBarrier;
        if (down) {
            for (const auto& dividend : dividendSchedule) {
                if (dividend->amount() < arguments_.barrier)
                    lowest = std::min<Real>(
                        lowest, std::log(arguments_.barrier - dividend->amount()));
            }
        }
        if (lowest < xMin + margin || logBarrier > xMax - margin) {
            activeEquityMesher = ext::make_shared<FdmBlackScholesMesher>(
                xGrid_, process_, maturity, payoff->strike(),
                std::min(xMin, Real(lowest - margin)),
                std::max(xMax, Real(logBarrier + margin)),
                0.0001, 1.5, cPoint, dividendSchedule);
        }
        const auto activeMesher = ext::make_shared<FdmMesherComposite>(activeEquityMesher);
        const auto activeCalculator = ext::make_shared<FdmLogInnerValue>(payoff, activeMesher, 0);

        // both solves step through the same stopping times
        std::list<std::vector<Time> > stoppingTimes;

        ext::shared_ptr<FdmDividendHandler> dividendCondition;
        std::map<Time, Real> dividendAmounts;
        std::vector<Time> exDividendTimes;
        if (!dividendSchedule.empty()) {
            dividendCondition = ext::make_shared<FdmDividendHandler>(
                dividendSchedule, activeMesher, referenceDate, dayCounter, 0);
            exDividendTimes = dividendCondition->dividendTimes();
            for (Size i = 0; i < exDividendTimes.size(); ++i)
                dividendAmounts.emplace(exDividendTimes[i], dividendCondition->dividends()[i]);
            stoppingTimes.push_back(exDividendTimes);
        }

        ext::shared_ptr<StepCondition<Array> > exerciseCondition;
        if (exerciseType == Exercise::American) {
            exerciseCondition = ext::make_shared<FdmAmericanStepCondition>(
                activeMesher, activeCalculator, process_->time(arguments_.exercise->date(0)));
        } else {
            auto bermudanCondition = ext::make_shared<FdmBermudanStepCondition>(
                arguments_.exercise->dates(), referenceDate, dayCounter,
                activeMesher, activeCalculator);
            exerciseCondition = bermudanCondition;
            stoppingTimes.push_back(bermudanCondition->exerciseTimes());
        }

        // The active option's value on the barrier is recorded twice, each
        // time after the exercise condition.  The ex-dividend value, before
        // the rollback applies the jump, is the inactive option's boundary
        // condition on the step ending there, and the whole solution is kept
        // to value the nodes the dividend knocks in.  The cum-dividend value,
        // after the jump, starts the next step.  Away from ex-dividend times
        // the two are the same, so without dividends one recorder is enough.
        const auto exDividendValues =
            ext::make_shared<FdmPointValueRecorder>(activeMesher, logBarrier, exDividendTimes);
        const auto cumDividendValues =
            dividendCondition != nullptr
                ? ext::make_shared<FdmPointValueRecorder>(activeMesher, logBarrier)
                : exDividendValues;

        std::list<ext::shared_ptr<StepCondition<Array> > > activeConditions;
        activeConditions.push_back(exerciseCondition);
        activeConditions.push_back(exDividendValues);
        if (dividendCondition != nullptr) {
            activeConditions.push_back(dividendCondition);
            activeConditions.push_back(exerciseCondition);
            activeConditions.push_back(cumDividendValues);
        }

        FdmSolverDesc activeSolverDesc = {
            activeMesher, FdmBoundaryConditionSet(),
            ext::make_shared<FdmStepConditionComposite>(stoppingTimes, activeConditions),
            activeCalculator, maturity, tGrid_, dampingSteps_ };

        const auto activeSolver = ext::make_shared<FdmBlackScholesSolver>(
            Handle<GeneralizedBlackScholesProcess>(process_), payoff->strike(),
            activeSolverDesc, schemeDesc_, localVol_, illegalLocalVolOverwrite_);
        // rolls the active option back, recording its values on the barrier
        activeSolver->valueAt(spot);

        // 2. Inactive option: mesher bounded by the barrier, rebate paid
        //    at expiry if the barrier is never touched, no exercise
        const auto inactiveEquityMesher = ext::make_shared<FdmBlackScholesMesher>(
                xGrid_, process_, maturity, payoff->strike(),
                down ? logBarrier : Null<Real>(), down ? Null<Real>() : logBarrier,
                0.0001, 1.5, std::make_pair(Null<Real>(), Null<Real>()), dividendSchedule);
        const auto inactiveMesher = ext::make_shared<FdmMesherComposite>(inactiveEquityMesher);
        const auto rebatePayoff =
            ext::make_shared<CashOrNothingPayoff>(Option::Call, 0.0, arguments_.rebate);
        const auto inactiveCalculator =
            ext::make_shared<FdmLogInnerValue>(rebatePayoff, inactiveMesher, 0);

        std::list<ext::shared_ptr<StepCondition<Array> > > inactiveConditions;
        if (!dividendSchedule.empty()) {
            inactiveConditions.push_back(ext::make_shared<FdmDividendHandler>(
                dividendSchedule, inactiveMesher, referenceDate, dayCounter, 0));
        }
        inactiveConditions.push_back(ext::make_shared<FdmKnockInBarrierCondition>(
            inactiveMesher, activeMesher, exDividendValues, cumDividendValues,
            dividendAmounts, arguments_.barrier, down, maturity, (*payoff)(arguments_.barrier)));

        // stopping at maturity applies the condition above to the terminal values
        std::list<std::vector<Time> > inactiveStoppingTimes = stoppingTimes;
        inactiveStoppingTimes.emplace_back(1, maturity);

        FdmBoundaryConditionSet boundaries;
        boundaries.push_back(ext::make_shared<FdmTimeDepDirichletBoundary>(
            inactiveMesher,
            [exDividendValues](Real t) { return exDividendValues->valueAt(t); },
            0, down ? FdmDirichletBoundary::Lower : FdmDirichletBoundary::Upper));

        FdmSolverDesc inactiveSolverDesc = {
            inactiveMesher, boundaries,
            ext::make_shared<FdmStepConditionComposite>(inactiveStoppingTimes, inactiveConditions),
            inactiveCalculator, maturity, tGrid_, dampingSteps_ };

        const auto inactiveSolver = ext::make_shared<FdmBlackScholesSolver>(
            Handle<GeneralizedBlackScholesProcess>(process_), payoff->strike(),
            inactiveSolverDesc, schemeDesc_, localVol_, illegalLocalVolOverwrite_);

        results_.value = inactiveSolver->valueAt(spot);
        results_.delta = inactiveSolver->deltaAt(spot);
        results_.gamma = inactiveSolver->gammaAt(spot);
        results_.theta = inactiveSolver->thetaAt(spot);
    }
}
