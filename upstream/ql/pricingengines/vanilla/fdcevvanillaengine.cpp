/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file fdcevvanillaengine.hpp
    \brief Finite-Differences pricing engine for the CEV model
*/

#include <ql/exercise.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmcev1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmcevop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/solvers/fdm1dimsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdiscountdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/fdmtimedepdirichletboundary.hpp>
#include <ql/pricingengines/vanilla/analyticcevengine.hpp>
#include <ql/pricingengines/vanilla/fdcevvanillaengine.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    namespace {
        class PriceAtBoundary {
          public:
            PriceAtBoundary(Time maturityTime,
                            ext::shared_ptr<StrikedTypePayoff> payoff,
                            ext::shared_ptr<YieldTermStructure> rTS,
                            ext::shared_ptr<CEVCalculator> calculator)
            : maturityTime_(maturityTime), payoff_(std::move(payoff)),
              calculator_(std::move(calculator)), rTS_(std::move(rTS)) {}

            Real operator()(Real t) const {
                const Time time2Expiry = std::max(1/365., maturityTime_ - t);
                const DiscountFactor df =
                    rTS_->discount(maturityTime_) / rTS_->discount(t);

                return  df * calculator_->value(
                    payoff_->optionType(), payoff_->strike(), time2Expiry);
            }

          private:
            const Time maturityTime_;
            const ext::shared_ptr<StrikedTypePayoff> payoff_;
            const ext::shared_ptr<CEVCalculator> calculator_;
            const ext::shared_ptr<YieldTermStructure> rTS_;
        };
    }

    FdCEVVanillaEngine::FdCEVVanillaEngine(Real f0,
                                           Real alpha,
                                           Real beta,
                                           Handle<YieldTermStructure> discountCurve,
                                           Size tGrid,
                                           Size xGrid,
                                           Size dampingSteps,
                                           Real scalingFactor,
                                           Real eps,
                                           const FdmSchemeDesc& schemeDesc)
    : f0_(f0), alpha_(alpha), beta_(beta), discountCurve_(std::move(discountCurve)), tGrid_(tGrid),
      xGrid_(xGrid), dampingSteps_(dampingSteps), scalingFactor_(scalingFactor), eps_(eps),
      schemeDesc_(schemeDesc) {
        registerWith(discountCurve_);
    }

    void FdCEVVanillaEngine::calculate() const {

        // 1. Mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const ext::shared_ptr<YieldTermStructure> rTS =
            discountCurve_.currentLink();

        const DayCounter dc = rTS->dayCounter();

        const Date referenceDate = rTS->referenceDate();
        const Date maturityDate = arguments_.exercise->lastDate();
        const Time maturityTime = dc.yearFraction(referenceDate, maturityDate);

        const ext::shared_ptr<Fdm1dMesher> cevMesher =
            ext::make_shared<FdmCEV1dMesher>(
                xGrid_,
                f0_, alpha_, beta_,
                maturityTime, eps_, scalingFactor_,
                std::make_pair(payoff->strike(), 0.1));

        const Real lowerBound = cevMesher->locations().front();
        const Real upperBound = cevMesher->locations().back();

        const ext::shared_ptr<FdmMesher> mesher =
           ext::make_shared<FdmMesherComposite>(cevMesher);

        // 2. Calculator
        const ext::shared_ptr<FdmInnerValueCalculator> calculator =
            ext::make_shared<FdmCellAveragingInnerValue>(payoff, mesher, 0);

        // 3. Step conditions
        const ext::shared_ptr<FdmStepConditionComposite> conditions =
            FdmStepConditionComposite::vanillaComposite(
                DividendSchedule(), arguments_.exercise,
                mesher, calculator,
                referenceDate, dc);

        // 4. Boundary conditions
        FdmBoundaryConditionSet boundaries;

        const PriceAtBoundary upperBoundPrice(
            maturityTime, payoff, rTS,
            ext::make_shared<CEVCalculator>(upperBound, alpha_, beta_));

        boundaries.push_back(ext::make_shared<FdmTimeDepDirichletBoundary>(
            mesher, std::function<Real (Real)>(upperBoundPrice),
            0, FdmTimeDepDirichletBoundary::Upper));

        const Real delta = (1-2*beta_)/(1-beta_);
        if (delta < 2.0) {
            const Real terminalCashFlow = (*payoff)(lowerBound);

            boundaries.push_back(
                ext::make_shared<FdmDiscountDirichletBoundary>(
                    mesher, rTS, maturityTime, terminalCashFlow,
                    0, FdmTimeDepDirichletBoundary::Lower));
        }

        // 5. Solver
        const FdmSolverDesc solverDesc = {
            mesher, boundaries, conditions,
            calculator, maturityTime, tGrid_, dampingSteps_
        };

        const ext::shared_ptr<FdmLinearOpComposite> op =
            ext::make_shared<FdmCEVOp>(
               mesher, discountCurve_.currentLink(), f0_, alpha_, beta_, 0);

        const ext::shared_ptr<Fdm1DimSolver> solver =
            ext::make_shared<Fdm1DimSolver>(solverDesc, schemeDesc_, op);

        results_.value = solver->interpolateAt(f0_);
        results_.delta = solver->derivativeX(f0_);
        results_.gamma = solver->derivativeXX(f0_);
        results_.theta = solver->thetaAt(f0_);
    }
}
