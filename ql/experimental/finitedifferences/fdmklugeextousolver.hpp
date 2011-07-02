/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmklugeousolver.hpp
*/

#ifndef quantlib_fdm_kluge_ou_solver_hpp
#define quantlib_fdm_kluge_ou_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/experimental/finitedifferences/fdmsolverdesc.hpp>
#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>

namespace QuantLib {

    template <Size N> class FdmNdimSolver;
    class ExtOUWithJumpsProcess;
    class ExtendedOrnsteinUhlenbeckProcess;

    class FdmKlugeExtOUSolver : public LazyObject {
      public:
          FdmKlugeExtOUSolver(
            Real correlation,
            const Handle<ExtOUWithJumpsProcess>& klugeProcess,
            const Handle<ExtendedOrnsteinUhlenbeckProcess>& ouProcess,
            const boost::shared_ptr<YieldTermStructure>& rTS,
            const FdmSolverDesc& solverDesc,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        Real valueAt(Real x, Real y, Real z) const;

      protected:
        void performCalculations() const;

      private:
        const Handle<ExtOUWithJumpsProcess> klugeProcess_;
        const Handle<ExtendedOrnsteinUhlenbeckProcess> ouProcess_;
        const Real correlation_;
        const boost::shared_ptr<YieldTermStructure> rTS_;

        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;

        mutable boost::shared_ptr<FdmNdimSolver<3> > solver_;
    };
}

#endif
