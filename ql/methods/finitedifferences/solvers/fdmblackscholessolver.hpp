/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

/*! \file fdmblackscholessolver.hpp
*/

#ifndef quantlib_fdm_black_scholes_solver_hpp
#define quantlib_fdm_black_scholes_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>

namespace QuantLib {

    class Fdm1DimSolver;
    class FdmSnapshotCondition;
    class GeneralizedBlackScholesProcess;

    class FdmBlackScholesSolver : public LazyObject {
      public:
        FdmBlackScholesSolver(Handle<GeneralizedBlackScholesProcess> process,
                              Real strike,
                              FdmSolverDesc solverDesc,
                              const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
                              bool localVol = false,
                              Real illegalLocalVolOverwrite = -Null<Real>(),
                              Handle<FdmQuantoHelper> quantoHelper = Handle<FdmQuantoHelper>());

        Real valueAt(Real s) const;
        Real deltaAt(Real s) const;
        Real gammaAt(Real s) const;
        Real thetaAt(Real s) const;

      protected:
        void performCalculations() const override;

      private:
        Handle<GeneralizedBlackScholesProcess> process_;
        const Real strike_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;
        const bool localVol_;
        const Real illegalLocalVolOverwrite_;
        const Handle<FdmQuantoHelper> quantoHelper_;

        mutable ext::shared_ptr<Fdm1DimSolver> solver_;
    };
}

#endif /* quantlib_fdm_black_scholes_solver_hpp */
