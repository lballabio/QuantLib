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

#include <ql/exercise.hpp>
#include <ql/experimental/finitedifferences/fdornsteinuhlenbeckvanillaengine.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmsimpleprocess1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmornsteinuhlenbeckop.hpp>
#include <ql/methods/finitedifferences/solvers/fdm1dimsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    namespace {
        class FdmOUInnerValue : public FdmInnerValueCalculator {
          public:
            FdmOUInnerValue(ext::shared_ptr<Payoff> payoff,
                            ext::shared_ptr<FdmMesher> mesher,
                            Size direction)
            : payoff_(std::move(payoff)), mesher_(std::move(mesher)), direction_(direction) {}


            Real innerValue(const FdmLinearOpIterator& iter, Time) override {
                const Real s = mesher_->location(iter, direction_);
                return (*payoff_)(s);
            }

            Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
                return innerValue(iter, t);
            }

          private:
            const ext::shared_ptr<Payoff> payoff_;
            const ext::shared_ptr<FdmMesher> mesher_;
            const Size direction_;
        };
    }

    FdOrnsteinUhlenbeckVanillaEngine::FdOrnsteinUhlenbeckVanillaEngine(
        ext::shared_ptr<OrnsteinUhlenbeckProcess> process,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        Real epsilon,
        const FdmSchemeDesc& schemeDesc)
    : process_(std::move(process)), rTS_(rTS), tGrid_(tGrid), xGrid_(xGrid),
      dampingSteps_(dampingSteps), epsilon_(epsilon), schemeDesc_(schemeDesc) {
        registerWith(process_);
        registerWith(rTS);
    }

    FdOrnsteinUhlenbeckVanillaEngine::FdOrnsteinUhlenbeckVanillaEngine(
        ext::shared_ptr<OrnsteinUhlenbeckProcess> process,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        DividendSchedule dividends,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        Real epsilon,
        const FdmSchemeDesc& schemeDesc)
    : process_(std::move(process)), rTS_(rTS),
      dividends_(std::move(dividends)), tGrid_(tGrid), xGrid_(xGrid),
      dampingSteps_(dampingSteps), epsilon_(epsilon), schemeDesc_(schemeDesc) {
        registerWith(process_);
        registerWith(rTS);
    }

    void FdOrnsteinUhlenbeckVanillaEngine::calculate() const {

        // 1. Mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        const DayCounter dc = rTS_->dayCounter();
        const Date referenceDate = rTS_->referenceDate();

        const Time maturity = dc.yearFraction(
            referenceDate, arguments_.exercise->lastDate());

        const ext::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmSimpleProcess1dMesher(
                xGrid_, process_, maturity, 1, epsilon_));

        const ext::shared_ptr<FdmMesher> mesher (
            new FdmMesherComposite(equityMesher));

        // 2. Calculator
        const ext::shared_ptr<FdmInnerValueCalculator> calculator(
            new FdmOUInnerValue(payoff, mesher, 0));

        // 3. Step conditions
        const ext::shared_ptr<FdmStepConditionComposite> conditions =
            FdmStepConditionComposite::vanillaComposite(
                                    dividends_, arguments_.exercise,
                                    mesher, calculator,
                                    referenceDate, dc);

        // 4. Boundary conditions
        const FdmBoundaryConditionSet boundaries;

        // 5. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions, calculator,
                                     maturity, tGrid_, dampingSteps_ };

        const ext::shared_ptr<FdmOrnsteinUhlenbeckOp> op(
            new FdmOrnsteinUhlenbeckOp(mesher, process_, rTS_, 0));

        const ext::shared_ptr<Fdm1DimSolver> solver(
                new Fdm1DimSolver(solverDesc, schemeDesc_, op));

        const Real spot = process_->x0();

        results_.value = solver->interpolateAt(spot);
        results_.delta = solver->derivativeX(spot);
        results_.gamma = solver->derivativeXX(spot);
        results_.theta = solver->thetaAt(spot);
    }
}
