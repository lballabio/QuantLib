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

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpop.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/secondderivativeop.hpp>
#include <ql/experimental/finitedifferences/fdmextendedornsteinuhlenbeckop.hpp>

namespace QuantLib {

    FdmExtOUJumpOp::FdmExtOUJumpOp(
        const boost::shared_ptr<FdmMesher>& mesher,
        const boost::shared_ptr<ExtOUWithJumpsProcess>& process,
        const boost::shared_ptr<YieldTermStructure>& rTS,
        const FdmBoundaryConditionSet& bcSet,
        Size integroIntegrationOrder)
    : mesher_ (mesher),
      process_(process),
      rTS_    (rTS),
      bcSet_  (bcSet),
      gaussLaguerreIntegration_(integroIntegrationOrder),
      x_      (mesher->locations(0)),
      ouOp_   (new FdmExtendedOrnsteinUhlenbackOp(
                   mesher,
                   process->getExtendedOrnsteinUhlenbeckProcess(), rTS, bcSet)),
      dyMap_  (FirstDerivativeOp(1, mesher)
                .mult(-process->beta()*mesher->locations(1))) {
    }

    Size FdmExtOUJumpOp::size() const {
        return mesher_->layout()->dim().size();;
    }

    void FdmExtOUJumpOp::setTime(Time t1, Time t2) {
        ouOp_->setTime(t1, t2);
    }

    Disposable<Array> FdmExtOUJumpOp::apply(const Array& r) const {
        return ouOp_->apply(r) + apply_direction(1, r) + integro(r);
    }

    Disposable<Array> FdmExtOUJumpOp::apply_mixed(const Array& r) const {
        return  integro(r);
    }

    Disposable<Array> FdmExtOUJumpOp::apply_direction(Size direction,
                                                      const Array& r) const {
        if (direction == 0)
            return ouOp_->apply_direction(direction, r);
        else if (direction == 1)
            return dyMap_.apply(r);
        else {
            Array retVal(r.size(), 0.0);
            return retVal;
        }
    }

    Disposable<Array>
        FdmExtOUJumpOp::solve_splitting(Size direction,
                                        const Array& r, Real a) const {
        if (direction == 0) {
            return ouOp_->solve_splitting(direction, r, a);
        }
        else if (direction == 1) {
            return dyMap_.solve_splitting(r, a, 1.0);
        }
        else {
            Array retVal(r);
            return retVal;
        }
    }

    Disposable<Array>
        FdmExtOUJumpOp::preconditioner(const Array& r, Real dt) const {

        return ouOp_->solve_splitting(0, r, dt);
    }

    FdmExtOUJumpOp::IntegroIntegrand::IntegroIntegrand(
                    const boost::shared_ptr<LinearInterpolation>& interpl,
                    const FdmBoundaryConditionSet& bcSet,
                    Real y, Real eta)
    : y_      (y),
      eta_    (eta),
      bcSet_  (bcSet),
      interpl_(interpl) { }

    Real FdmExtOUJumpOp::IntegroIntegrand::operator()(Real u) const {
        const Real y = y_ + u/eta_;
        Real valueOfDerivative = interpl_->operator()(y, true);

        for (FdmBoundaryConditionSet::const_iterator iter=bcSet_.begin();
            iter < bcSet_.end(); ++iter) {
            valueOfDerivative=(*iter)->applyAfterApplying(y, valueOfDerivative);
        }

        return std::exp(-u)*valueOfDerivative;
    }

    Disposable<Array> FdmExtOUJumpOp::integro(const Array& r) const {
        Array integral(r.size());

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
        const Size extraDims=layout->size()/(layout->dim()[0]*layout->dim()[1]);

        std::vector<Array>  y(extraDims, Array(layout->dim()[1]));
        std::vector<Matrix> f(extraDims,
                              Matrix(layout->dim()[1], layout->dim()[0]));

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
            ++iter) {
            const Size i = iter.coordinates()[0];
            const Size j = iter.coordinates()[1];
            const Size k = iter.index() / (layout->dim()[0]*layout->dim()[1]);

            y[k][j]    = mesher_->location(iter, 1);
            f[k][j][i] = r[iter.index()];
        }
        std::vector<std::vector<boost::shared_ptr<LinearInterpolation> > >
            interpl(extraDims, std::vector<
                     boost::shared_ptr<LinearInterpolation> >(f[0].columns()));

        for (Size k=0; k < extraDims; ++k) {
            for (Size i=0; i < f[k].columns(); ++i) {
                interpl[k][i] = boost::shared_ptr<LinearInterpolation>(
                    new LinearInterpolation(y[k].begin(), y[k].end(),
                                            f[k].column_begin(i)));
            }
        }

        const Real eta = process_->eta();

        for (FdmLinearOpIterator iter=layout->begin(); iter!=endIter; ++iter) {
            const Size i = iter.coordinates()[0];
            const Size j = iter.coordinates()[1];
            const Size k = iter.index() / (layout->dim()[0]*layout->dim()[1]);

            integral[iter.index()] = gaussLaguerreIntegration_(
                        IntegroIntegrand(interpl[k][i], bcSet_, y[k][j], eta));
        }

        return process_->jumpIntensity()*(integral-r);
    }
}
