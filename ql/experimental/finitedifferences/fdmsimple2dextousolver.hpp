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

/*! \file fdmsimple2dextousolver.hpp
    \brief solver for simple swing options based on ext OU process
*/


#ifndef quantlib_fdm_2d_ext_ou_solver_hpp
#define quantlib_fdm_2d_ext_ou_solver_hpp

#include <ql/experimental/finitedifferences/fdmextendedornsteinuhlenbeckop.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/handle.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    class FdmSimple2dExtOUSolver : public LazyObject {
      public:
        FdmSimple2dExtOUSolver(const Handle<ExtendedOrnsteinUhlenbeckProcess>& process,
                               std::shared_ptr<YieldTermStructure> rTS,
                               FdmSolverDesc solverDesc,
                               const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer())
        : process_(process), rTS_(std::move(rTS)), solverDesc_(std::move(solverDesc)),
          schemeDesc_(schemeDesc) {
            registerWith(process);
        }

        Real valueAt(Real x, Real y) const {
            calculate();
            return solver_->interpolateAt(x, y);
        }

      protected:
        void performCalculations() const override {
            std::shared_ptr<FdmLinearOpComposite>op(
                new FdmExtendedOrnsteinUhlenbeckOp(
                                solverDesc_.mesher, process_.currentLink(),
                                rTS_, solverDesc_.bcSet));

            solver_ = std::make_shared<Fdm2DimSolver>(
                          solverDesc_, schemeDesc_, op);
        }

      private:
        const Handle<ExtendedOrnsteinUhlenbeckProcess> process_;
        const std::shared_ptr<YieldTermStructure> rTS_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;

        mutable std::shared_ptr<Fdm2DimSolver> solver_;
    };
}

#endif

