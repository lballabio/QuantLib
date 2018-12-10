/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009, 2011, 2014, 2015 Klaus Spanderen
 Copyright (C) 2015 Johannes Goettker-Schnetmann

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

/*! \file fdmhestonsolver.hpp
*/

#ifndef quantlib_fdm_heston_solver_hpp
#define quantlib_fdm_heston_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    class HestonProcess;
    class Fdm2DimSolver;

    class FdmHestonSolver : public LazyObject {
      public:
        FdmHestonSolver(
            const Handle<HestonProcess>& process,
            const FdmSolverDesc& solverDesc,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer(),
            const Handle<FdmQuantoHelper>& quantoHelper
                                                = Handle<FdmQuantoHelper>(),
            const ext::shared_ptr<LocalVolTermStructure>& leverageFct
                = ext::shared_ptr<LocalVolTermStructure>());

        Real valueAt(Real s, Real v) const;
        Real thetaAt(Real s, Real v) const;

        // First and second order derivative with respect to S_t.
        // Please note that this is not the "model implied" delta or gamma.
        // E.g. see Fabio Mercurio, Massimo Morini
        // "A Note on Hedging with Local and Stochastic Volatility Models",
        // http://papers.ssrn.com/sol3/papers.cfm?abstract_id=1294284
        Real deltaAt(Real s, Real v) const;
        Real gammaAt(Real s, Real v) const;

        Real meanVarianceDeltaAt(Real s, Real v) const;
        Real meanVarianceGammaAt(Real s, Real v) const;

      protected:
        void performCalculations() const;
        
      private:
        const Handle<HestonProcess> process_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;
        const Handle<FdmQuantoHelper> quantoHelper_;
        const ext::shared_ptr<LocalVolTermStructure> leverageFct_;

        mutable ext::shared_ptr<Fdm2DimSolver> solver_;
    };
}

#endif
