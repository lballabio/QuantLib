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

/*! \file fdmhestonsolver.hpp
*/

#ifndef quantlib_fdm_heston_solver_hpp
#define quantlib_fdm_heston_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/math/matrix.hpp>
#include <ql/experimental/finitedifferences/fdmquantohelper.hpp>
#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmdirichletboundary.hpp>

namespace QuantLib {

    class FdmInnerValueCalculator;
    class FdmMesher;
    class FdmSnapshotCondition;
    class FdmStepConditionComposite;
    class HestonProcess;
    class BicubicSpline;

    class FdmHestonSolver : public LazyObject {
      public:
        FdmHestonSolver(
            const Handle<HestonProcess>& process,
            const boost::shared_ptr<FdmMesher>& mesher,
            const FdmBoundaryConditionSet & bcSet,
            const boost::shared_ptr<FdmStepConditionComposite> & condition,
            const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
            Time maturity,
            Size timeSteps,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer(),
            const Handle<FdmQuantoHelper>& quantoHelper
                                                = Handle<FdmQuantoHelper>());

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
        void backwardSolve(boost::shared_ptr<FdmLinearOpComposite>& map) const;
        
      private:
        Handle<HestonProcess> process_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const FdmBoundaryConditionSet bcSet_;
        const boost::shared_ptr<FdmSnapshotCondition> thetaCondition_;
        const boost::shared_ptr<FdmStepConditionComposite> condition_;
        const Time maturity_;
        const Size timeSteps_;
        const Size dampingSteps_;

        const FdmSchemeDesc schemeDesc_;
        const Handle<FdmQuantoHelper> quantoHelper_;

        std::vector<Real> x_, v_, initialValues_;
        mutable Matrix resultValues_;
        mutable boost::shared_ptr<BicubicSpline> interpolation_;
    };
}

#endif
