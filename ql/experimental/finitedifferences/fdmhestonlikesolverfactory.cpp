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

/*! \file fdmhestonlikesolverfactoray.cpp
    \brief factory for Heston like solvers aka. processes that are derived 
           from HestonProcess
*/


#include <ql/processes/batesprocess.hpp>
#include <ql/processes/hestonprocess.hpp>

#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdmquantohelper.hpp>
#include <ql/experimental/finitedifferences/fdmbatessolver.hpp>
#include <ql/experimental/finitedifferences/fdmhestonsolver.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmhestonlikesolverfactory.hpp>

namespace QuantLib {

    boost::shared_ptr<FdmHestonSolver> FdmHestonLikeSolverFactory::create(
              const Handle<HestonProcess>& process,
              const boost::shared_ptr<FdmMesher>& mesher,
              const FdmBoundaryConditionSet& bcSet,
              const boost::shared_ptr<FdmStepConditionComposite> & conditions,
              const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
              Time maturity,
              Size timeSteps,
              Size dampingSteps,
              const FdmSchemeDesc& schemeDesc,
              const Handle<FdmQuantoHelper>& quantoHelper) {
        
        boost::shared_ptr<FdmHestonSolver> retVal;
        
        if (boost::dynamic_pointer_cast<BatesProcess>(process.currentLink())) {
            retVal = boost::shared_ptr<FdmBatesSolver>(new FdmBatesSolver(
                Handle<BatesProcess>(boost::dynamic_pointer_cast<BatesProcess>(
                                                        process.currentLink())),
                mesher, bcSet, conditions,
                calculator, maturity, 
                timeSteps, dampingSteps, 16,
                schemeDesc, quantoHelper));
        }
        else {
            retVal = boost::shared_ptr<FdmHestonSolver>(new FdmHestonSolver(
                                        process, mesher, bcSet, conditions,
                                        calculator, maturity, 
                                        timeSteps, dampingSteps,
                                        schemeDesc, quantoHelper));
        }
        
        return retVal;
    };
}

