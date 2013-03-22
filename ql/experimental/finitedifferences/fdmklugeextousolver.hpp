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

/*! \file fdmklugeextousolver.hpp
    \brief Kluge/extended Ornstein-Uhlenbeck FDM solver
*/

#ifndef quantlib_fdm_kluge_ou_solver_hpp
#define quantlib_fdm_kluge_ou_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/experimental/processes/klugeextouprocess.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/experimental/finitedifferences/fdmklugeextouop.hpp>
#include <ql/methods/finitedifferences/solvers/fdmndimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class ExtOUWithJumpsProcess;
    class ExtendedOrnsteinUhlenbeckProcess;

    template <Size N=3>
    class FdmKlugeExtOUSolver : public LazyObject {
      public:
        FdmKlugeExtOUSolver(
          const Handle<KlugeExtOUProcess>& klugeOUProcess,
          const boost::shared_ptr<YieldTermStructure>& rTS,
          const FdmSolverDesc& solverDesc,
          const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer())
        : klugeOUProcess_(klugeOUProcess),
          rTS_           (rTS),
          solverDesc_    (solverDesc),
          schemeDesc_    (schemeDesc) {
            registerWith(klugeOUProcess_);
        }

        Real valueAt(const std::vector<Real>& x) const {
            calculate();
            return solver_->interpolateAt(x);
        }

      protected:
        void performCalculations() const {
            boost::shared_ptr<FdmLinearOpComposite>op(
                new FdmKlugeExtOUOp(solverDesc_.mesher,
                                    klugeOUProcess_.currentLink(),
                                    rTS_, solverDesc_.bcSet, 16));

            solver_ = boost::shared_ptr<FdmNdimSolver<N> >(
                          new FdmNdimSolver<N>(solverDesc_, schemeDesc_, op));
        }

      private:
        const Handle<KlugeExtOUProcess> klugeOUProcess_;
        const boost::shared_ptr<YieldTermStructure> rTS_;

        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;

        mutable boost::shared_ptr<FdmNdimSolver<N> > solver_;
        BOOST_STATIC_ASSERT(N >= 3); // KlugeExtOU solver can't be applied on meshes
                                     // with less than three dimensions
    };
}

#endif
