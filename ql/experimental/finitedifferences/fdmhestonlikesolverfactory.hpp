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

/*! \file fdmhestonlikesolverfactory.hpp
    \brief factory for Heston like solvers aka. processes that are derived 
           from HestonProcess
*/

#ifndef quantlib_fdm_heston_like_solver_factory_hpp
#define quantlib_fdm_heston_like_solver_factory_hpp

#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmdirichletboundary.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    class HestonProcess;
    class FdmHestonSolver;
    class FdmMesher;
    class FdmQuantoHelper;
    class FdmInnerValueCalculator;
    class FdmStepConditionComposite;

    class FdmHestonLikeSolverFactory {
      public:
          FdmHestonLikeSolverFactory() { }
          
          boost::shared_ptr<FdmHestonSolver> create(
              const Handle<HestonProcess>& process,
              const boost::shared_ptr<FdmMesher>& mesher,
              const FdmBoundaryConditionSet& bcSet,
              const boost::shared_ptr<FdmStepConditionComposite> & condition,
              const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
              Time maturity,
              Size timeSteps,
              Size dampingSteps,
              const FdmSchemeDesc& schemeDesc,
              const Handle<FdmQuantoHelper>& quantoHelper);
    };
}

#endif
