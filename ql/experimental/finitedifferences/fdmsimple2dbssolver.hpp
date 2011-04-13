/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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

/*!
 * \file fdmsimple2dbssolver.hpp
*/

#ifndef quantlib_fdm_simple_2d_bs_solver_hpp
#define quantlib_fdm_simple_2d_bs_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/experimental/finitedifferences/fdmdirichletboundary.hpp>
#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>

namespace QuantLib {

    class FdmInnerValueCalculator;
    class FdmMesher;
    class FdmSnapshotCondition;
    class FdmStepConditionComposite;
    class BicubicSpline;

    class FdmSimple2dBSSolver : LazyObject {
      public:
        typedef std::vector<boost::shared_ptr<FdmDirichletBoundary> >
            BoundaryConditionSet;

        FdmSimple2dBSSolver(
            const Handle<GeneralizedBlackScholesProcess>& process,
            const boost::shared_ptr<FdmMesher>& mesher,
            const BoundaryConditionSet & bcSet,
            const boost::shared_ptr<FdmStepConditionComposite> & condition,
            const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
            Real strike,
            Time maturity,
            Size timeSteps,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        Real valueAt(Real s, Real a) const;
        Real deltaAt(Real s, Real a, Real eps) const;
        Real gammaAt(Real s, Real a, Real eps) const;
        Real thetaAt(Real s, Real a) const;

      protected:
        void performCalculations() const;

      private:
        Handle<GeneralizedBlackScholesProcess> process_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const BoundaryConditionSet bcSet_;
        const boost::shared_ptr<FdmSnapshotCondition> thetaCondition_;
        const boost::shared_ptr<FdmStepConditionComposite> condition_;
        const Real strike_;
        const Time maturity_;
        const Size timeSteps_;

        const FdmSchemeDesc schemeDesc_;

        std::vector<Real> x_, a_, initialValues_;
        mutable Matrix resultValues_;
        mutable boost::shared_ptr<BicubicSpline> interpolation_;
    };
}

#endif /* quantlib_fdm_simple_2d_bs_solver_hpp */
