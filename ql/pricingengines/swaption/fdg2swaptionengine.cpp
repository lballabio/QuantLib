/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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
#include <ql/indexes/iborindex.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/pricingengines/swaption/fdg2swaptionengine.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/meshers/fdmsimpleprocess1dmesher.hpp>
#include <ql/methods/finitedifferences/solvers/fdmg2solver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmaffinemodelswapinnervalue.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>

namespace QuantLib {

    FdG2SwaptionEngine::FdG2SwaptionEngine(
        const ext::shared_ptr<G2>& model,
        Size tGrid, Size xGrid, Size yGrid,
        Size dampingSteps, Real invEps,
        const FdmSchemeDesc& schemeDesc)
    : GenericModelEngine<G2, Swaption::arguments, Swaption::results>(model),
      tGrid_(tGrid),
      xGrid_(xGrid),
      yGrid_(yGrid),
      dampingSteps_(dampingSteps),
      invEps_(invEps),
      schemeDesc_(schemeDesc) {
    }

    void FdG2SwaptionEngine::calculate() const {

        // 1. Term structure
        const Handle<YieldTermStructure> ts = model_->termStructure();

        // 2. Mesher
        const DayCounter dc = ts->dayCounter();
        const Date referenceDate = ts->referenceDate();
        const Time maturity = dc.yearFraction(referenceDate,
                                              arguments_.exercise->lastDate());

        const ext::shared_ptr<OrnsteinUhlenbeckProcess> process1(
            new OrnsteinUhlenbeckProcess(model_->a(), model_->sigma()));

        const ext::shared_ptr<OrnsteinUhlenbeckProcess> process2(
            new OrnsteinUhlenbeckProcess(model_->b(), model_->eta()));

        const ext::shared_ptr<Fdm1dMesher> xMesher(
            new FdmSimpleProcess1dMesher(xGrid_,process1,maturity,1,invEps_));

        const ext::shared_ptr<Fdm1dMesher> yMesher(
            new FdmSimpleProcess1dMesher(yGrid_,process2,maturity,1,invEps_));

        const ext::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(xMesher, yMesher));

        // 3. Inner Value Calculator
        const std::vector<Date>& exerciseDates = arguments_.exercise->dates();
        std::map<Time, Date> t2d;

        for (auto exerciseDate : exerciseDates) {
            const Time t = dc.yearFraction(referenceDate, exerciseDate);
            QL_REQUIRE(t >= 0, "exercise dates must not contain past date");

            t2d[t] = exerciseDate;
        }

        const Handle<YieldTermStructure> disTs = model_->termStructure();
        const Handle<YieldTermStructure> fwdTs
            = arguments_.swap->iborIndex()->forwardingTermStructure();

        QL_REQUIRE(fwdTs->dayCounter() == disTs->dayCounter(),
                "day counter of forward and discount curve must match");
        QL_REQUIRE(fwdTs->referenceDate() == disTs->referenceDate(),
                "reference date of forward and discount curve must match");

        const ext::shared_ptr<G2> fwdModel(
            new G2(fwdTs, model_->a(), model_->sigma(),
                   model_->b(), model_->eta(), model_->rho()));

        const ext::shared_ptr<FdmInnerValueCalculator> calculator(
             new FdmAffineModelSwapInnerValue<G2>(
                 model_.currentLink(), fwdModel,
                 arguments_.swap, t2d, mesher, 0));

        // 4. Step conditions
        const ext::shared_ptr<FdmStepConditionComposite> conditions =
             FdmStepConditionComposite::vanillaComposite(
                 DividendSchedule(), arguments_.exercise,
                 mesher, calculator, referenceDate, dc);

        // 5. Boundary conditions
        const FdmBoundaryConditionSet boundaries;

        // 6. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     calculator, maturity,
                                     tGrid_, dampingSteps_ };

        FdmG2Solver solver(model_, solverDesc, schemeDesc_);

        results_.value = solver.valueAt(0.0, 0.0);
    }
}
