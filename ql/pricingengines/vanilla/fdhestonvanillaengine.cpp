/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009, 2015 Klaus Spanderen
 Copyright (C) 2015 Johannes Göttker-Schnetmann

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
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmultistrikemesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhestonsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/processes/batesprocess.hpp>
#include <utility>

namespace QuantLib {

    FdHestonVanillaEngine::FdHestonVanillaEngine(const ext::shared_ptr<HestonModel>& model,
                                                 Size tGrid,
                                                 Size xGrid,
                                                 Size vGrid,
                                                 Size dampingSteps,
                                                 const FdmSchemeDesc& schemeDesc,
                                                 ext::shared_ptr<LocalVolTermStructure> leverageFct,
                                                 const Real mixingFactor)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      tGrid_(tGrid), xGrid_(xGrid), vGrid_(vGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), leverageFct_(std::move(leverageFct)),
      quantoHelper_(ext::shared_ptr<FdmQuantoHelper>()), mixingFactor_(mixingFactor) {}

    FdHestonVanillaEngine::FdHestonVanillaEngine(const ext::shared_ptr<HestonModel>& model,
                                                 DividendSchedule dividends,
                                                 Size tGrid,
                                                 Size xGrid,
                                                 Size vGrid,
                                                 Size dampingSteps,
                                                 const FdmSchemeDesc& schemeDesc,
                                                 ext::shared_ptr<LocalVolTermStructure> leverageFct,
                                                 const Real mixingFactor)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      dividends_(std::move(dividends)),
      tGrid_(tGrid), xGrid_(xGrid), vGrid_(vGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), leverageFct_(std::move(leverageFct)),
      quantoHelper_(ext::shared_ptr<FdmQuantoHelper>()), mixingFactor_(mixingFactor) {}

    FdHestonVanillaEngine::FdHestonVanillaEngine(const ext::shared_ptr<HestonModel>& model,
                                                 ext::shared_ptr<FdmQuantoHelper> quantoHelper,
                                                 Size tGrid,
                                                 Size xGrid,
                                                 Size vGrid,
                                                 Size dampingSteps,
                                                 const FdmSchemeDesc& schemeDesc,
                                                 ext::shared_ptr<LocalVolTermStructure> leverageFct,
                                                 const Real mixingFactor)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      tGrid_(tGrid), xGrid_(xGrid), vGrid_(vGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), leverageFct_(std::move(leverageFct)),
      quantoHelper_(std::move(quantoHelper)), mixingFactor_(mixingFactor) {}

    FdHestonVanillaEngine::FdHestonVanillaEngine(const ext::shared_ptr<HestonModel>& model,
                                                 DividendSchedule dividends,
                                                 ext::shared_ptr<FdmQuantoHelper> quantoHelper,
                                                 Size tGrid,
                                                 Size xGrid,
                                                 Size vGrid,
                                                 Size dampingSteps,
                                                 const FdmSchemeDesc& schemeDesc,
                                                 ext::shared_ptr<LocalVolTermStructure> leverageFct,
                                                 const Real mixingFactor)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      dividends_(std::move(dividends)),
      tGrid_(tGrid), xGrid_(xGrid), vGrid_(vGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), leverageFct_(std::move(leverageFct)),
      quantoHelper_(std::move(quantoHelper)), mixingFactor_(mixingFactor) {}

    FdmSolverDesc FdHestonVanillaEngine::getSolverDesc(Real) const {

        // 1. Mesher
        const ext::shared_ptr<HestonProcess> process = model_->process();
        const Time maturity = process->time(arguments_.exercise->lastDate());

        // 1.1 The variance mesher
        const Size tGridMin = 5;
        const Size tGridAvgSteps = std::max(tGridMin, tGrid_/50);
        const auto vMesher = ext::make_shared<FdmHestonLocalVolatilityVarianceMesher>(
                  vGrid_, process, leverageFct_, maturity, tGridAvgSteps, 0.0001, mixingFactor_);

        const Volatility avgVolaEstimate = vMesher->volaEstimate();

        // 1.2 The equity mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        ext::shared_ptr<Fdm1dMesher> equityMesher;
        if (strikes_.empty()) {
            equityMesher = ext::make_shared<FdmBlackScholesMesher>(
                    xGrid_,
                    FdmBlackScholesMesher::processHelper(
                        process->s0(), process->dividendYield(),
                        process->riskFreeRate(), avgVolaEstimate),
                    maturity, payoff->strike(),
                    Null<Real>(), Null<Real>(), 0.0001, 2.0,
                    std::pair<Real, Real>(payoff->strike(), 0.1),
                    dividends_,
                    quantoHelper_);
        }
        else {
            QL_REQUIRE(dividends_.empty(),
                       "multiple strikes engine does not work with discrete dividends");
            equityMesher = ext::make_shared<FdmBlackScholesMultiStrikeMesher>(
                    xGrid_,
                    FdmBlackScholesMesher::processHelper(
                      process->s0(), process->dividendYield(),
                      process->riskFreeRate(), avgVolaEstimate),
                    maturity, strikes_, 0.0001, 1.5,
                    std::pair<Real, Real>(payoff->strike(), 0.075));
        }

        const auto mesher = ext::make_shared<FdmMesherComposite>(equityMesher, vMesher);

        // 2. Calculator
        const auto calculator = ext::make_shared<FdmLogInnerValue>(arguments_.payoff, mesher, 0);

        // 3. Step conditions
        const ext::shared_ptr<FdmStepConditionComposite> conditions =
             FdmStepConditionComposite::vanillaComposite(
                                 dividends_, arguments_.exercise,
                                 mesher, calculator,
                                 process->riskFreeRate()->referenceDate(),
                                 process->riskFreeRate()->dayCounter());

        // 4. Boundary conditions
        const FdmBoundaryConditionSet boundaries;

        // 5. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     calculator, maturity,
                                     tGrid_, dampingSteps_ };

       return solverDesc;
    }

    namespace {
        // false for European exercise, which has no payoffAtExpiry() of its own
        bool payoffAtExpiryOf(const ext::shared_ptr<Exercise>& exercise) {
            const auto early = ext::dynamic_pointer_cast<EarlyExercise>(exercise);
            return early != nullptr && early->payoffAtExpiry();
        }
    }

    void FdHestonVanillaEngine::calculate() const {

        // cache lookup for precalculated results
        for (auto& cachedArgs2result : cachedArgs2results_) {
            if (cachedArgs2result.first.exercise->type() == arguments_.exercise->type() &&
                cachedArgs2result.first.exercise->dates() == arguments_.exercise->dates() &&
                payoffAtExpiryOf(cachedArgs2result.first.exercise) ==
                    payoffAtExpiryOf(arguments_.exercise)) {
                ext::shared_ptr<PlainVanillaPayoff> p1 =
                    ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                                                            arguments_.payoff);
                ext::shared_ptr<PlainVanillaPayoff> p2 =
                    ext::dynamic_pointer_cast<PlainVanillaPayoff>(cachedArgs2result.first.payoff);

                if ((p1 != nullptr) && p1->strike() == p2->strike() &&
                    p1->optionType() == p2->optionType()) {
                    QL_REQUIRE(dividends_.empty(),
                               "multiple strikes engine does not work with discrete dividends");
                    results_ = cachedArgs2result.second;
                    return;
                }
            }
        }

        const ext::shared_ptr<HestonProcess> process = model_->process();

        auto solver = ext::make_shared<FdmHestonSolver>(
                    Handle<HestonProcess>(process),
                    getSolverDesc(1.5), schemeDesc_,
                    Handle<FdmQuantoHelper>(quantoHelper_), leverageFct_,
                    mixingFactor_);

        const Real v0   = process->v0();
        const Real spot = process->s0()->value();

        results_.value = solver->valueAt(spot, v0);
        results_.delta = solver->deltaAt(spot, v0);
        results_.gamma = solver->gammaAt(spot, v0);
        results_.theta = solver->thetaAt(spot, v0);

        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        // The step condition enforces the exercise on grid nodes. The monotonic
        // cubic interpolant can still undershoot the payoff between nodes, and
        // VanillaOption::impliedVolatility prices Americans with this engine, so
        // a value below the payoff cannot be inverted. Floor only when the
        // holder can exercise on the reference date and the payoff is paid
        // then. Greeks stay on the interpolant: resetting them would disagree
        // with a bump wherever the undershoot is local.
        const ext::shared_ptr<Exercise>& exercise = arguments_.exercise;
        const Date settlementDate = process->riskFreeRate()->referenceDate();
        bool exercisable = false;
        switch (exercise->type()) {
          case Exercise::American:
            exercisable = exercise->date(0) <= settlementDate
                          && settlementDate <= exercise->lastDate();
            break;
          case Exercise::Bermudan:
            for (const Date& exerciseDate : exercise->dates()) {
                if (exerciseDate == settlementDate) {
                    exercisable = true;
                    break;
                }
            }
            break;
          case Exercise::European:
            break;
          default:
            QL_FAIL("exercise type is not supported");
        }
        if (exercisable) {
            const auto early = ext::dynamic_pointer_cast<EarlyExercise>(exercise);
            if (early != nullptr && early->payoffAtExpiry())
                exercisable = false;
        }
        if (exercisable) {
            const Real exerciseValue = (*payoff)(spot);
            if (results_.value < exerciseValue)
                results_.value = exerciseValue;
        }

        cachedArgs2results_.resize(strikes_.size());
        for (Size i=0; i < strikes_.size(); ++i) {
            cachedArgs2results_[i].first.exercise = arguments_.exercise;
            cachedArgs2results_[i].first.payoff =
                ext::make_shared<PlainVanillaPayoff>(
                    payoff->optionType(), strikes_[i]);
            const Real d = payoff->strike()/strikes_[i];

            VanillaOption::results& results = cachedArgs2results_[i].second;
            results.value = solver->valueAt(spot*d, v0)/d;
            results.delta = solver->deltaAt(spot*d, v0);
            results.gamma = solver->gammaAt(spot*d, v0)*d;
            results.theta = solver->thetaAt(spot*d, v0)/d;
            if (exercisable) {
                // results.value is already rescaled back to the real spot
                // and strikes_[i] by the /d above, so the floor is the
                // ordinary intrinsic value at the real spot, not the
                // scaled solver query point.
                const Real cachedExerciseValue =
                    PlainVanillaPayoff(payoff->optionType(), strikes_[i])(spot);
                if (results.value < cachedExerciseValue)
                    results.value = cachedExerciseValue;
            }
        }
    }

    void FdHestonVanillaEngine::update() {
        cachedArgs2results_.clear();
        GenericModelEngine<HestonModel,
                           VanillaOption::arguments,
                           VanillaOption::results>::update();
    }

    void FdHestonVanillaEngine::enableMultipleStrikesCaching(
                                        const std::vector<Real>& strikes) {
        strikes_ = strikes;
        cachedArgs2results_.clear();
    }


    MakeFdHestonVanillaEngine::MakeFdHestonVanillaEngine(ext::shared_ptr<HestonModel> hestonModel)
    : hestonModel_(std::move(hestonModel)),
      schemeDesc_(ext::make_shared<FdmSchemeDesc>(FdmSchemeDesc::Hundsdorfer())) {}

    MakeFdHestonVanillaEngine& MakeFdHestonVanillaEngine::withQuantoHelper(
        const ext::shared_ptr<FdmQuantoHelper>& quantoHelper) {
        quantoHelper_ = quantoHelper;
        return *this;
    }

    MakeFdHestonVanillaEngine&
    MakeFdHestonVanillaEngine::withTGrid(Size tGrid) {
        tGrid_ = tGrid;
        return *this;
    }

    MakeFdHestonVanillaEngine&
    MakeFdHestonVanillaEngine::withXGrid(Size xGrid) {
        xGrid_ = xGrid;
        return *this;
    }

    MakeFdHestonVanillaEngine&
    MakeFdHestonVanillaEngine::withVGrid(Size vGrid) {
        vGrid_ = vGrid;
        return *this;
    }

    MakeFdHestonVanillaEngine&
    MakeFdHestonVanillaEngine::withDampingSteps(Size dampingSteps) {
        dampingSteps_ = dampingSteps;
        return *this;
    }

    MakeFdHestonVanillaEngine&
    MakeFdHestonVanillaEngine::withFdmSchemeDesc(
        const FdmSchemeDesc& schemeDesc) {
        schemeDesc_ = ext::make_shared<FdmSchemeDesc>(schemeDesc);
        return *this;
    }

    MakeFdHestonVanillaEngine&
    MakeFdHestonVanillaEngine::withLeverageFunction(
        ext::shared_ptr<LocalVolTermStructure>& leverageFct) {
        leverageFct_ = leverageFct;
        return *this;
    }

    MakeFdHestonVanillaEngine&
    MakeFdHestonVanillaEngine::withCashDividends(
            const std::vector<Date>& dividendDates,
            const std::vector<Real>& dividendAmounts) {
        dividends_ = DividendVector(dividendDates, dividendAmounts);
        return *this;
    }

    MakeFdHestonVanillaEngine::operator
    ext::shared_ptr<PricingEngine>() const {
        return ext::make_shared<FdHestonVanillaEngine>(
                hestonModel_,
                dividends_,
                quantoHelper_,
                tGrid_, xGrid_, vGrid_, dampingSteps_,
                *schemeDesc_,
                leverageFct_);
    }

}
