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

/*! \file fdmbatessolver.hpp
*/

#ifndef quantlib_fdm_bates_solver_hpp
#define quantlib_fdm_bates_solver_hpp

#include <ql/experimental/finitedifferences/fdmhestonsolver.hpp>

namespace QuantLib {
    
    class BatesProcess;

    class FdmBatesSolver : public FdmHestonSolver {
      public:
        FdmBatesSolver(
            const Handle<BatesProcess>& process,
            const boost::shared_ptr<FdmMesher>& mesher,
            const FdmBoundaryConditionSet & bcSet,
            const boost::shared_ptr<FdmStepConditionComposite> & condition,
            const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
            Time maturity,
            Size timeSteps,
            Size dampingSteps = 0,
            Size integroIntegrationOrder = 12,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer(),
            const Handle<FdmQuantoHelper>& quantoHelper
                                                = Handle<FdmQuantoHelper>());
        
      protected:
        void performCalculations() const;
        
      private:
        const Size integroIntegrationOrder_;
        const FdmBoundaryConditionSet bcSet_;
        const Handle<BatesProcess> batesProcess_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const Handle<FdmQuantoHelper> quantoHelper_;
    };
}

#endif
