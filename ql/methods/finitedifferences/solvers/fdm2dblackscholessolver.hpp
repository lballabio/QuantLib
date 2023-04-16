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

/*! \file fdm2dblackscholessolver.hpp
*/

#ifndef quantlib_fdm_2d_black_scholes_solver_hpp
#define quantlib_fdm_2d_black_scholes_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class Fdm2DimSolver;
    class GeneralizedBlackScholesProcess;

    class Fdm2dBlackScholesSolver : public LazyObject {
      public:
        Fdm2dBlackScholesSolver(Handle<GeneralizedBlackScholesProcess> p1,
                                Handle<GeneralizedBlackScholesProcess> p2,
                                Real correlation,
                                FdmSolverDesc solverDesc,
                                const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer(),
                                bool localVol = false,
                                Real illegalLocalVolOverwrite = -Null<Real>());

        Real valueAt(Real x, Real y) const;
        Real thetaAt(Real x, Real y) const;

        Real deltaXat(Real x, Real y) const;
        Real deltaYat(Real x, Real y) const;
        Real gammaXat(Real x, Real y) const;
        Real gammaYat(Real x, Real y) const;
        Real gammaXYat(Real x, Real y) const;

      protected:
        void performCalculations() const override;

      private:
        const Handle<GeneralizedBlackScholesProcess> p1_;
        const Handle<GeneralizedBlackScholesProcess> p2_;
        const Real correlation_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;
        const bool localVol_;
        const Real illegalLocalVolOverwrite_;

        mutable std::shared_ptr<Fdm2DimSolver> solver_;
    };
}

#endif
