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
        const ext::shared_ptr<FdmHestonLocalVolatilityVarianceMesher> vMesher
            = ext::make_shared<FdmHestonLocalVolatilityVarianceMesher>(
                  vGrid_, process, leverageFct_, maturity, tGridAvgSteps, 0.0001, mixingFactor_);

        const Volatility avgVolaEstimate = vMesher->volaEstimate();

        // 1.2 The equity mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        ext::shared_ptr<Fdm1dMesher> equityMesher;
        if (strikes_.empty()) {
            equityMesher = ext::shared_ptr<Fdm1dMesher>(
                new FdmBlackScholesMesher(
                    xGrid_,
                    FdmBlackScholesMesher::processHelper(
                        process->s0(), process->dividendYield(),
                        process->riskFreeRate(), avgVolaEstimate),
                    maturity, payoff->strike(),
                    Null<Real>(), Null<Real>(), 0.0001, 2.0,
                    std::pair<Real, Real>(payoff->strike(), 0.1),
                    dividends_,
                    quantoHelper_));
        }
        else {
            QL_REQUIRE(dividends_.empty(),
                       "multiple strikes engine does not work with discrete dividends");
            equityMesher = ext::shared_ptr<Fdm1dMesher>(
                new FdmBlackScholesMultiStrikeMesher(
                    xGrid_,
                    FdmBlackScholesMesher::processHelper(
                      process->s0(), process->dividendYield(),
                      process->riskFreeRate(), avgVolaEstimate),
                    maturity, strikes_, 0.0001, 1.5,
                    std::pair<Real, Real>(payoff->strike(), 0.075)));
        }

        const ext::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(equityMesher, vMesher));

        // 2. Calculator
        const ext::shared_ptr<FdmInnerValueCalculator> calculator(
                          new FdmLogInnerValue(arguments_.payoff, mesher, 0));

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

    void FdHestonVanillaEngine::calculate() const {

        // cache lookup for precalculated results
        for (auto& cachedArgs2result : cachedArgs2results_) {
            if (cachedArgs2result.first.exercise->type() == arguments_.exercise->type() &&
                cachedArgs2result.first.exercise->dates() == arguments_.exercise->dates()) {
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

        ext::shared_ptr<FdmHestonSolver> solver(new FdmHestonSolver(
                    Handle<HestonProcess>(process),
                    getSolverDesc(1.5), schemeDesc_,
                    Handle<FdmQuantoHelper>(quantoHelper_), leverageFct_,
                    mixingFactor_));

        const Real v0   = process->v0();
        const Real spot = process->s0()->value();

        results_.value = solver->valueAt(spot, v0);
        results_.delta = solver->deltaAt(spot, v0);
        results_.gamma = solver->gammaAt(spot, v0);
        results_.theta = solver->thetaAt(spot, v0);

        cachedArgs2results_.resize(strikes_.size());
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
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
