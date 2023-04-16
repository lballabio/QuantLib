/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file fdmbatesop.cpp
    \brief Bates linear operator
*/

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmbatesop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/processes/batesprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <utility>

namespace QuantLib {

    FdmBatesOp::FdmBatesOp(const std::shared_ptr<FdmMesher>& mesher,
                           const std::shared_ptr<BatesProcess>& batesProcess,
                           FdmBoundaryConditionSet bcSet,
                           const Size integroIntegrationOrder,
                           const std::shared_ptr<FdmQuantoHelper>& quantoHelper)
    : lambda_(batesProcess->lambda()), delta_(batesProcess->delta()), nu_(batesProcess->nu()),
      m_(std::exp(nu_ + 0.5 * delta_ * delta_) - 1.0),
      gaussHermiteIntegration_(integroIntegrationOrder), mesher_(mesher), bcSet_(std::move(bcSet)),
      hestonOp_(new FdmHestonOp(
          mesher,
          std::make_shared<HestonProcess>(
              batesProcess->riskFreeRate(),
              Handle<YieldTermStructure>(std::make_shared<ZeroSpreadedTermStructure>(
                  batesProcess->dividendYield(),
                  Handle<Quote>(std::shared_ptr<Quote>(new SimpleQuote(lambda_ * m_))),
                  Continuous,
                  NoFrequency,
                  batesProcess->dividendYield()->dayCounter())),
              batesProcess->s0(),
              batesProcess->v0(),
              batesProcess->kappa(),
              batesProcess->theta(),
              batesProcess->sigma(),
              batesProcess->rho()),
          quantoHelper)) {}

    FdmBatesOp::IntegroIntegrand::IntegroIntegrand(
                    const std::shared_ptr<LinearInterpolation>& interpl,
                    const FdmBoundaryConditionSet& bcSet,
                    Real x, Real delta, Real nu)
    : x_(x), delta_(delta), nu_(nu), 
      bcSet_(bcSet), interpl_(interpl) { }
                    
    Real FdmBatesOp::IntegroIntegrand::operator()(Real y) const {
        const Real x = x_ + M_SQRT2*delta_*y + nu_;
        Real valueOfDerivative = (*interpl_)(x, true);

        for (auto iter = bcSet_.begin(); iter < bcSet_.end(); ++iter) {

            const std::shared_ptr<FdmDirichletBoundary> dirichlet
                = std::dynamic_pointer_cast<FdmDirichletBoundary>(*iter);

            QL_REQUIRE(dirichlet, "FdmBatesOp can only deal with Dirichlet "
                                  "boundary conditions.");

            valueOfDerivative
                = dirichlet->applyAfterApplying(x, valueOfDerivative);
        }

        return std::exp(-y*y)*valueOfDerivative;
    }
    
    Array FdmBatesOp::integro(const Array& r) const {
        const std::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
        
        QL_REQUIRE(layout->dim().size() == 2, "invalid layout dimension");

        Array x(layout->dim()[0]);
        Matrix f(layout->dim()[1], layout->dim()[0]);
        
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
            ++iter) {
            const Size i = iter.coordinates()[0];
            const Size j = iter.coordinates()[1];
            
            x[i]    = mesher_->location(iter, 0);
            f[j][i] = r[iter.index()];
            
        }
        std::vector<std::shared_ptr<LinearInterpolation> > interpl(f.rows());
        for (Size i=0; i < f.rows(); ++i) {
            interpl[i] = std::make_shared<LinearInterpolation>(
                x.begin(), x.end(), f.row_begin(i));
        }
        
        Array integral(r.size());
        for (FdmLinearOpIterator iter=layout->begin(); iter!=endIter; ++iter) {
            const Size i = iter.coordinates()[0];
            const Size j = iter.coordinates()[1];

            integral[iter.index()] = M_1_SQRTPI* 
                gaussHermiteIntegration_(
                      IntegroIntegrand(interpl[j], bcSet_, x[i], delta_, nu_));
        }

        return lambda_*(integral-r);
    }

    std::vector<SparseMatrix> FdmBatesOp::toMatrixDecomp() const {
        QL_FAIL("not implemented");
    }

}
