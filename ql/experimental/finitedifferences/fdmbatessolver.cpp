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

/*! \file fdmbatessolver.cpp
*/

#include <ql/processes/batesprocess.hpp>
#include <ql/experimental/finitedifferences/fdmbatesop.hpp>
#include <ql/experimental/finitedifferences/fdmbatessolver.hpp>


namespace QuantLib {
    
    FdmBatesSolver::FdmBatesSolver(
            const Handle<BatesProcess>& process,
            const boost::shared_ptr<FdmMesher>& mesher,
            const FdmBoundaryConditionSet& bcSet,
            const boost::shared_ptr<FdmStepConditionComposite> & condition,
            const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
            Time maturity,
            Size timeSteps,
            Size dampingSteps,
            Size integroIntegrationOrder,
            const FdmSchemeDesc& schemeDesc,
            const Handle<FdmQuantoHelper>& quantoHelper)
    : FdmHestonSolver(Handle<HestonProcess>(process.currentLink()),
                      mesher, bcSet, condition, 
                      calculator, maturity, timeSteps, 0,
                      schemeDesc, quantoHelper),
      integroIntegrationOrder_(integroIntegrationOrder),
      bcSet_(bcSet),
      batesProcess_(process),
      mesher_      (mesher),
      quantoHelper_(quantoHelper) {

        registerWith(batesProcess_);
        registerWith(quantoHelper_);
    }
          
    void FdmBatesSolver::performCalculations() const {
        boost::shared_ptr<FdmLinearOpComposite> map(
            new FdmBatesOp(mesher_, batesProcess_.currentLink(), bcSet_,
                           integroIntegrationOrder_, 
                           (!quantoHelper_.empty()) 
                                   ? quantoHelper_.currentLink()
                                   : boost::shared_ptr<FdmQuantoHelper>()));
        backwardSolve(map);
    }
}
