/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file fdbatesvanillaengine.cpp
    \brief Partial Integro Finite-Differences Bates vanilla option engine
*/

#include <ql/processes/batesprocess.hpp>

#include <ql/methods/finitedifferences/solvers/fdmbatessolver.hpp>
#include <ql/pricingengines/vanilla/fdbatesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>

namespace QuantLib {

    FdBatesVanillaEngine::FdBatesVanillaEngine(
            const ext::shared_ptr<BatesModel>& model,
            Size tGrid, Size xGrid, 
            Size vGrid, Size dampingSteps,
            const FdmSchemeDesc& schemeDesc)
    : GenericModelEngine<BatesModel,
                         DividendVanillaOption::arguments,
                         DividendVanillaOption::results>(model),
       tGrid_(tGrid), xGrid_(xGrid),
       vGrid_(vGrid), dampingSteps_(dampingSteps),
       schemeDesc_(schemeDesc) {
    }

    void FdBatesVanillaEngine::calculate() const {
        FdHestonVanillaEngine helperEngine(model_.currentLink(),
                                           arguments_.cashFlow,
                                           tGrid_, xGrid_, vGrid_,
                                           dampingSteps_, schemeDesc_);

        *dynamic_cast<DividendVanillaOption::arguments*>(
                               helperEngine.getArguments()) = arguments_;

        FdmSolverDesc solverDesc = helperEngine.getSolverDesc(2.0);

        const ext::shared_ptr<BatesProcess> process =
                ext::dynamic_pointer_cast<BatesProcess>(model_->process());

        ext::shared_ptr<FdmBatesSolver> solver(
            new FdmBatesSolver(Handle<BatesProcess>(process),
                               solverDesc, schemeDesc_));

        const Real v0   = process->v0();
        const Real spot = process->s0()->value();

        results_.value = solver->valueAt(spot, v0);
        results_.delta = solver->deltaAt(spot, v0);
        results_.gamma = solver->gammaAt(spot, v0);
        results_.theta = solver->thetaAt(spot, v0);
    }
}
