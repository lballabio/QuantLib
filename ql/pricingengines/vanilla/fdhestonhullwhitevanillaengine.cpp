/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmultistrikemesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmsimpleprocess1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonhullwhitevanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <utility>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    FdHestonHullWhiteVanillaEngine::FdHestonHullWhiteVanillaEngine(
        const ext::shared_ptr<HestonModel>& hestonModel,
        ext::shared_ptr<HullWhiteProcess> hwProcess,
        Real corrEquityShortRate,
        Size tGrid,
        Size xGrid,
        Size vGrid,
        Size rGrid,
        Size dampingSteps,
        bool controlVariate,
        const FdmSchemeDesc& schemeDesc)
    : GenericModelEngine<HestonModel,
                         DividendVanillaOption::arguments,
                         DividendVanillaOption::results>(hestonModel),
      hwProcess_(std::move(hwProcess)), explicitDividends_(false),
      corrEquityShortRate_(corrEquityShortRate), tGrid_(tGrid),
      xGrid_(xGrid), vGrid_(vGrid), rGrid_(rGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), controlVariate_(controlVariate) {}

    FdHestonHullWhiteVanillaEngine::FdHestonHullWhiteVanillaEngine(
        const ext::shared_ptr<HestonModel>& hestonModel,
        ext::shared_ptr<HullWhiteProcess> hwProcess,
        DividendSchedule dividends,
        Real corrEquityShortRate,
        Size tGrid,
        Size xGrid,
        Size vGrid,
        Size rGrid,
        Size dampingSteps,
        bool controlVariate,
        const FdmSchemeDesc& schemeDesc)
    : GenericModelEngine<HestonModel,
                         DividendVanillaOption::arguments,
                         DividendVanillaOption::results>(hestonModel),
      hwProcess_(std::move(hwProcess)), dividends_(std::move(dividends)), explicitDividends_(true),
      corrEquityShortRate_(corrEquityShortRate), tGrid_(tGrid),
      xGrid_(xGrid), vGrid_(vGrid), rGrid_(rGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), controlVariate_(controlVariate) {}

    QL_DEPRECATED_ENABLE_WARNING

    void FdHestonHullWhiteVanillaEngine::calculate() const {

        // dividends will eventually be moved out of arguments, but for now we need the switch
        QL_DEPRECATED_DISABLE_WARNING
        const DividendSchedule& passedDividends = explicitDividends_ ? dividends_ : arguments_.cashFlow;
        QL_DEPRECATED_ENABLE_WARNING
  
        // 1. cache lookup for precalculated results
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
                    QL_REQUIRE(passedDividends.empty(),
                               "multiple strikes engine does not work with discrete dividends");
                    results_ = cachedArgs2result.second;
                    return;
                }
            }
        }

        // 2. Mesher
        const ext::shared_ptr<HestonProcess> hestonProcess=model_->process();
        const Time maturity=hestonProcess->time(arguments_.exercise->lastDate());

        // 2.1 The variance mesher
        const Size tGridMin = 5;
        const ext::shared_ptr<FdmHestonVarianceMesher> varianceMesher(
            new FdmHestonVarianceMesher(vGrid_, hestonProcess,
                                        maturity,std::max(tGridMin,tGrid_/50)));

        // 2.2 The equity mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "wrong payoff type given");

        ext::shared_ptr<Fdm1dMesher> equityMesher;
        if (strikes_.empty()) {
            equityMesher = ext::shared_ptr<Fdm1dMesher>(
                new FdmBlackScholesMesher(
                    xGrid_, 
                    FdmBlackScholesMesher::processHelper(
                      hestonProcess->s0(), hestonProcess->dividendYield(), 
                      hestonProcess->riskFreeRate(), 
                      varianceMesher->volaEstimate()),
                      maturity, payoff->strike(),
                      Null<Real>(), Null<Real>(), 0.0001, 1.5, 
                      std::pair<Real, Real>(payoff->strike(), 0.1),
                      passedDividends));
        }
        else {
            QL_REQUIRE(passedDividends.empty(),
                       "multiple strikes engine does not work with discrete dividends");
            equityMesher = ext::shared_ptr<Fdm1dMesher>(
                new FdmBlackScholesMultiStrikeMesher(
                    xGrid_,
                    FdmBlackScholesMesher::processHelper(
                      hestonProcess->s0(), hestonProcess->dividendYield(), 
                      hestonProcess->riskFreeRate(), 
                      varianceMesher->volaEstimate()),
                    maturity, strikes_, 0.0001, 1.5,
                    std::pair<Real, Real>(payoff->strike(), 0.075)));            
        }
       
        //2.3 The short rate mesher        
        const ext::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess(
            new OrnsteinUhlenbeckProcess(hwProcess_->a(),hwProcess_->sigma()));
        const ext::shared_ptr<Fdm1dMesher> shortRateMesher(
                   new FdmSimpleProcess1dMesher(rGrid_, ouProcess, maturity));
        
        const ext::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(equityMesher, varianceMesher,
                                   shortRateMesher));

        // 3. Calculator
        const ext::shared_ptr<FdmInnerValueCalculator> calculator(
                            new FdmLogInnerValue(arguments_.payoff, mesher, 0));

        // 4. Step conditions
        const ext::shared_ptr<FdmStepConditionComposite> conditions = 
            FdmStepConditionComposite::vanillaComposite(
                                passedDividends, arguments_.exercise, 
                                mesher, calculator, 
                                hestonProcess->riskFreeRate()->referenceDate(),
                                hestonProcess->riskFreeRate()->dayCounter());

        // 5. Boundary conditions
        const FdmBoundaryConditionSet boundaries;

        // 6. Solver
        const FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                           calculator, maturity,
                                           tGrid_, dampingSteps_ };

        const ext::shared_ptr<FdmHestonHullWhiteSolver> solver(
            new FdmHestonHullWhiteSolver(Handle<HestonProcess>(hestonProcess),
                                         Handle<HullWhiteProcess>(hwProcess_),
                                         corrEquityShortRate_,
                                         solverDesc, schemeDesc_));

        const Real spot = hestonProcess->s0()->value();
        const Real v0   = hestonProcess->v0();
        results_.value = solver->valueAt(spot, v0, 0);
        results_.delta = solver->deltaAt(spot, v0, 0, spot*0.01);
        results_.gamma = solver->gammaAt(spot, v0, 0, spot*0.01);
        results_.theta = solver->thetaAt(spot, v0, 0);

        cachedArgs2results_.resize(strikes_.size());        
        for (Size i=0; i < strikes_.size(); ++i) {
            cachedArgs2results_[i].first.exercise = arguments_.exercise;
            cachedArgs2results_[i].first.payoff = 
                ext::make_shared<PlainVanillaPayoff>(
                    payoff->optionType(), strikes_[i]);
            const Real d = payoff->strike()/strikes_[i];

            QL_DEPRECATED_DISABLE_WARNING
            DividendVanillaOption::results& 
                                results = cachedArgs2results_[i].second;
            QL_DEPRECATED_ENABLE_WARNING
            results.value = solver->valueAt(spot*d, v0, 0)/d;
            results.delta = solver->deltaAt(spot*d, v0, 0, spot*d*0.01);
            results.gamma = solver->gammaAt(spot*d, v0, 0, spot*d*0.01)*d;
            results.theta = solver->thetaAt(spot*d, v0, 0)/d;
        }
     
        if (controlVariate_) {
            ext::shared_ptr<PricingEngine> analyticEngine(
                                       new AnalyticHestonEngine(*model_, 164));
            ext::shared_ptr<Exercise> exercise(
                        new EuropeanExercise(arguments_.exercise->lastDate()));
            
            VanillaOption option(payoff, exercise);
            option.setPricingEngine(analyticEngine);
            Real analyticNPV = option.NPV();

            ext::shared_ptr<FdHestonVanillaEngine> fdEngine(
                    new FdHestonVanillaEngine(*model_, tGrid_, xGrid_,
                                              vGrid_, dampingSteps_, 
                                              schemeDesc_));
            fdEngine->enableMultipleStrikesCaching(strikes_);
            option.setPricingEngine(fdEngine);
            
            Real fdNPV = option.NPV();
            results_.value += analyticNPV - fdNPV;
            for (Size i=0; i < strikes_.size(); ++i) {
                VanillaOption controlVariateOption(
                    ext::shared_ptr<StrikedTypePayoff>(
                        new PlainVanillaPayoff(payoff->optionType(), 
                                               strikes_[i])), exercise);
                controlVariateOption.setPricingEngine(analyticEngine);
                analyticNPV = controlVariateOption.NPV();
                
                controlVariateOption.setPricingEngine(fdEngine);
                fdNPV = controlVariateOption.NPV();
                cachedArgs2results_[i].second.value += analyticNPV - fdNPV;
            }
        }
    }
    
    void FdHestonHullWhiteVanillaEngine::update() {
        cachedArgs2results_.clear();
        QL_DEPRECATED_DISABLE_WARNING
        GenericModelEngine<HestonModel,
                           DividendVanillaOption::arguments,
                           DividendVanillaOption::results>::update();
        QL_DEPRECATED_ENABLE_WARNING
    }

    void FdHestonHullWhiteVanillaEngine::enableMultipleStrikesCaching(
                                        const std::vector<Real>& strikes) {
        strikes_ = strikes;
        update();
    }

}
