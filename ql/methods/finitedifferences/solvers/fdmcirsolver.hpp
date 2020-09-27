/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

/*! \file fdmcirsolver.hpp
*/

#ifndef quantlib_fdm_cir_solver_hpp
#define quantlib_fdm_cir_solver_hpp

#include <ql/handle.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/models/shortrate/onefactormodels/coxingersollross.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/coxingersollrossprocess.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    class HestonProcess;
    class Fdm2DimSolver;

    class FdmCIRSolver : public LazyObject {
      public:
        FdmCIRSolver(const Handle<CoxIngersollRossProcess>& process,
                     const Handle<GeneralizedBlackScholesProcess>& bsProcess,
                     const FdmSolverDesc& solverDesc,
                     const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer(),
                     Real rho = 1.0,
                     Real strike = 1.0);

        Real valueAt(Real s, Real v) const;
        Real deltaAt(Real s, Real v) const;
        Real gammaAt(Real s, Real v) const;
        Real thetaAt(Real s, Real v) const;

      protected:
        void performCalculations() const;
        
      private:
        const Handle<GeneralizedBlackScholesProcess> bsProcess_;
        const Handle<CoxIngersollRossProcess> cirProcess_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;
        const Real rho_;
        const Real strike_;

        mutable ext::shared_ptr<Fdm2DimSolver> solver_;
    };
}

#endif
