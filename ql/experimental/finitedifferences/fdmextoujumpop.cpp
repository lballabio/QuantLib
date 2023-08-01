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

/*! \file fdmexpoujumpop.cpp
    \brief Ornstein Uhlenbeck process plus jumps (Kluge Model)
*/

#include <ql/experimental/finitedifferences/fdmextendedornsteinuhlenbeckop.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpop.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4180)
#endif

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/operation.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

namespace QuantLib {

    FdmExtOUJumpOp::FdmExtOUJumpOp(
        const ext::shared_ptr<FdmMesher>& mesher,
        const ext::shared_ptr<ExtOUWithJumpsProcess>& process,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        const FdmBoundaryConditionSet& bcSet,
        Size integroIntegrationOrder)
    : mesher_ (mesher),
      process_(process),
      rTS_    (rTS),
      bcSet_  (bcSet),
      gaussLaguerreIntegration_(integroIntegrationOrder),
      x_      (mesher->locations(0)),
      ouOp_   (new FdmExtendedOrnsteinUhlenbeckOp(
                   mesher,
                   process->getExtendedOrnsteinUhlenbeckProcess(), rTS, bcSet)),
      dyMap_  (FirstDerivativeOp(1, mesher)
                .mult(-process->beta()*mesher->locations(1)))
    {
        const Real eta     = process_->eta();
        const Real lambda  = process_->jumpIntensity();

        const Array yInt   = gaussLaguerreIntegration_.x();
        const Array weights= gaussLaguerreIntegration_.weights();

        integroPart_ = SparseMatrix(mesher_->layout()->size(),
                                    mesher_->layout()->size());

        Array yLoc(mesher_->layout()->dim()[1]);
        for (const auto& iter : *mesher_->layout()) {
            yLoc[iter.coordinates()[1]] = mesher_->location(iter, 1);
        }

        for (const auto& iter : *mesher_->layout()) {
            const Size diag = iter.index();
            integroPart_(diag, diag) -= lambda;

            const Real y = mesher_->location(iter, 1);
            const Integer yIndex = iter.coordinates()[1];

            for (Size i=0; i < yInt.size(); ++i) {
                const Real weight = std::exp(-yInt[i])*weights[i];

                const Real ys = y + yInt[i]/eta;
                const Integer l = (ys > yLoc.back()) ? yLoc.size()-2
                    : std::upper_bound(yLoc.begin(),
                                       yLoc.end()-1, ys) - yLoc.begin()-1;

                const Real s = (ys-yLoc[l])/(yLoc[l+1]-yLoc[l]);
                integroPart_(diag, mesher_->layout()->neighbourhood(iter, 1, l-yIndex))
                    += weight*lambda*(1-s);
                integroPart_(diag, mesher_->layout()->neighbourhood(iter, 1, l+1-yIndex))
                    += weight*lambda*s;
            }
        }
    }

    Size FdmExtOUJumpOp::size() const {
        return mesher_->layout()->dim().size();;
    }

    void FdmExtOUJumpOp::setTime(Time t1, Time t2) {
        ouOp_->setTime(t1, t2);
    }

    Array FdmExtOUJumpOp::apply(const Array& r) const {
        return ouOp_->apply(r) + dyMap_.apply(r) + integro(r);
    }

    Array FdmExtOUJumpOp::apply_mixed(const Array& r) const {
        return integro(r);
    }

    Array FdmExtOUJumpOp::apply_direction(Size direction,
                                          const Array& r) const {
        if (direction == 0)
            return ouOp_->apply_direction(direction, r);
        else if (direction == 1)
            return dyMap_.apply(r);
        else {
            return Array(r.size(), 0.0);
        }
    }

    Array FdmExtOUJumpOp::solve_splitting(Size direction,
                                          const Array& r, Real a) const {
        if (direction == 0) {
            return ouOp_->solve_splitting(direction, r, a);
        }
        else if (direction == 1) {
            return dyMap_.solve_splitting(r, a, 1.0);
        }
        else {
            return r;
        }
    }

    Array FdmExtOUJumpOp::preconditioner(const Array& r, Real dt) const {
        return ouOp_->solve_splitting(0, r, dt);
    }

    Array FdmExtOUJumpOp::integro(const Array& r) const {
        return prod(integroPart_, r);
    }

    std::vector<SparseMatrix> FdmExtOUJumpOp::toMatrixDecomp() const {
        QL_REQUIRE(bcSet_.empty(), "boundary conditions are not supported");

        std::vector<SparseMatrix> retVal(1, ouOp_->toMatrixDecomp().front());
        retVal.push_back(dyMap_.toMatrix());
        retVal.push_back(integroPart_);

        return retVal;
    }

}
