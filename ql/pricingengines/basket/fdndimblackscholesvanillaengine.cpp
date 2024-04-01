/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/solvers/fdmndimsolver.hpp>
#include <ql/methods/finitedifferences/operators/fdmndimblackscholesop.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/pricingengines/basket/fdndimblackscholesvanillaengine.hpp>

#include <boost/preprocessor/iteration/local.hpp>


namespace QuantLib {

    FdndimBlackScholesVanillaEngine::FdndimBlackScholesVanillaEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
        Matrix correlation,
        std::vector<Size> xGrids,
        Size tGrid, Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        bool localVol,
        Real illegalLocalVolOverwrite)
    : processes_(std::move(processes)),
      correlation_(std::move(correlation)),
      xGrids_(std::move(xGrids)),
      tGrid_(tGrid),
      dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc),
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite) {

        QL_REQUIRE(!processes_.empty(), "no Black-Scholes process is given.");
        QL_REQUIRE(correlation_.size1() == correlation_.size2()
                && correlation_.size1() == processes_.size(),
                "correlation matrix has the wrong size.");
        QL_REQUIRE(xGrids_.size() == processes_.size(),
                "wrong number of xGrids is given.");
    }


    void FdndimBlackScholesVanillaEngine::calculate() const {
        const Time maturity = processes_[0]->time(arguments_.exercise->lastDate());

        std::vector<ext::shared_ptr<Fdm1dMesher> > meshers;
        for (Size i=0; i < processes_.size(); ++i) {
            const auto process = processes_[i];

            meshers.push_back(
                ext::make_shared<FdmBlackScholesMesher>(
                    xGrids_[i], process, maturity, process->x0(),
                    Null<Real>(), Null<Real>(), 0.0001, 1.5,
                    std::pair<Real, Real>(process->x0(), 0.1)
                )
            );
        }
        const auto mesher = ext::make_shared<FdmMesherComposite>(meshers);

        const auto payoff
            = ext::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff);
        const auto calculator
            = ext::make_shared<FdmLogBasketInnerValue>(payoff, mesher);

        const auto conditions
            = FdmStepConditionComposite::vanillaComposite(
                DividendSchedule(), arguments_.exercise,
                mesher, calculator,
                processes_[0]->riskFreeRate()->referenceDate(),
                processes_[0]->riskFreeRate()->dayCounter());

        const FdmBoundaryConditionSet boundaries;
        const FdmSolverDesc solverDesc
            = { mesher, boundaries, conditions, calculator,
                maturity, tGrid_, dampingSteps_ };

        const auto op = ext::make_shared<FdmndimBlackScholesOp>(
            mesher, processes_, correlation_, maturity
        );

        std::vector<Real> logX;
        for (const auto& p: processes_)
            logX.push_back(std::log(p->x0()));

        switch(processes_.size()) {
            #define PDE_MAX_SUPPORTED_DIM 6

            #define BOOST_PP_LOCAL_MACRO(n) \
                case n : \
                    results_.value = ext::make_shared<FdmNdimSolver<n>>( \
                        solverDesc, schemeDesc_, op)->interpolateAt(logX); \
                break;
            #define BOOST_PP_LOCAL_LIMITS (1, PDE_MAX_SUPPORTED_DIM)
            #include BOOST_PP_LOCAL_ITERATE()
            default:
                QL_FAIL("This engine does not support " << processes_.size() << " underlyings. "
                    << "Max number of underlyings is " << PDE_MAX_SUPPORTED_DIM << ". "
                    << "Change preprocessor constant PDE_MAX_SUPPORTED_DIM and recompile "
                    << "if a large number of underlyings is needed.");
        }
    }
}
