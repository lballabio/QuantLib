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

/*! \file fdmsimple3dextoujumpsolver.hpp
    \brief solver for simple swing options based on ext OU-Jump (Kluge) Model
*/


#ifndef quantlib_fdm_3d_ext_ou_jump_solver_hpp
#define quantlib_fdm_3d_ext_ou_jump_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdm3dimsolver.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpop.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class FdmSimple3dExtOUJumpSolver : public LazyObject {
      public:
        FdmSimple3dExtOUJumpSolver(
            const Handle<ExtOUWithJumpsProcess>& process,
            const boost::shared_ptr<YieldTermStructure>& rTS,
            const FdmSolverDesc& solverDesc,
            const FdmSchemeDesc& schemeDesc  = FdmSchemeDesc::Hundsdorfer())
        : process_(process), rTS_(rTS),
          solverDesc_(solverDesc), schemeDesc_(schemeDesc) {
            registerWith(process);
        }

        Real valueAt(Real x, Real y, Real z) const {
            calculate();
            return solver_->interpolateAt(x, y, z);
        }

      protected:
        void performCalculations() const {
            boost::shared_ptr<FdmLinearOpComposite>op(
                new FdmExtOUJumpOp(solverDesc_.mesher,
                                   process_.currentLink(),
                                   rTS_, solverDesc_.bcSet, 32));

            solver_ = boost::shared_ptr<Fdm3DimSolver>(
                          new Fdm3DimSolver(solverDesc_, schemeDesc_, op));
        }

      private:
        const Handle<ExtOUWithJumpsProcess> process_;
        const boost::shared_ptr<YieldTermStructure> rTS_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;

        mutable boost::shared_ptr<Fdm3DimSolver> solver_;
    };
}

#endif
