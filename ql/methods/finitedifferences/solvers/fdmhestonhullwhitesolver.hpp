/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file fdmhestonhullwhitesolver.hpp
*/

#ifndef quantlib_fdm_heston_hull_white_solver_hpp
#define quantlib_fdm_heston_hull_white_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>


namespace QuantLib {

    class Fdm3DimSolver;

    class FdmHestonHullWhiteSolver : public LazyObject {
      public:
        FdmHestonHullWhiteSolver(const Handle<HestonProcess>& hestonProcess,
                                 const Handle<HullWhiteProcess>& hwProcess,
                                 Rate corrEquityShortRate,
                                 FdmSolverDesc solverDesc,
                                 const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        Real valueAt(Real s, Real v, Rate r) const;
        Real thetaAt(Real s, Real v, Rate r) const;
        
        // First and second order derivative with respect to S_t. 
        // Please note that this is not the "model implied" delta or gamma.
        // E.g. see Fabio Mercurio, Massimo Morini 
        // "A Note on Hedging with Local and Stochastic Volatility Models",
        // http://papers.ssrn.com/sol3/papers.cfm?abstract_id=1294284  
        Real deltaAt(Real s, Real v, Rate r, Real eps) const;
        Real gammaAt(Real s, Real v, Rate r, Real eps) const;
        
      protected:
        void performCalculations() const override;

      private:
        const Handle<HestonProcess> hestonProcess_;  
        const Handle<HullWhiteProcess> hwProcess_;
        const Real corrEquityShortRate_;
        
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;

        mutable std::shared_ptr<Fdm3DimSolver> solver_;
    };
}

#endif
