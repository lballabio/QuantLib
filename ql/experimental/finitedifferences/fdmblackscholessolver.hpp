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
#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmdirichletboundary.hpp>

namespace QuantLib {

    class FdmMesher;
    class FdmSnapshotCondition;
    class FdmInnerValueCalculator;
    class FdmStepConditionComposite;
    class CubicInterpolation;
    class GeneralizedBlackScholesProcess;

    class FdmBlackScholesSolver : public LazyObject {
      public:
        typedef std::vector<boost::shared_ptr<FdmDirichletBoundary> >
            BoundaryConditionSet;

        FdmBlackScholesSolver(
            const Handle<GeneralizedBlackScholesProcess>& process,
            const boost::shared_ptr<FdmMesher>& mesher,
            const BoundaryConditionSet & bcSet,
            const boost::shared_ptr<FdmStepConditionComposite> & condition,
            const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
            Real strike,
            Time maturity,
            Size timeSteps,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>());

        Real valueAt(Real s) const;
        Real deltaAt(Real s) const;
        Real gammaAt(Real s) const;
        Real thetaAt(Real s) const;

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
        const Size dampingSteps_;

        const FdmSchemeDesc schemeDesc_;
        const bool localVol_;
        const Real illegalLocalVolOverwrite_;

        std::vector<Real> x_, initialValues_;
        mutable Array resultValues_;
        mutable boost::shared_ptr<CubicInterpolation> interpolation_;
    };
}

#endif /* quantlib_fdm_black_scholes_solver_hpp */
