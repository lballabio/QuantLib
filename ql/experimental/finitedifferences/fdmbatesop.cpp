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

#include <ql/quotes/simplequote.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/experimental/finitedifferences/fdmbatesop.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>

namespace QuantLib {

    FdmBatesOp::FdmBatesOp(
                    const boost::shared_ptr<FdmMesher>& mesher,
                    const boost::shared_ptr<BatesProcess>& batesProcess,
                    const FdmBoundaryConditionSet& bcSet,
                    const Size integroIntegrationOrder, 
                    const boost::shared_ptr<FdmQuantoHelper>& quantoHelper)
    : lambda_(batesProcess->lambda()), 
      delta_ (batesProcess->delta()), 
      nu_    (batesProcess->nu()),
      m_(std::exp(nu_+0.5*delta_*delta_)-1.0),
      gaussHermiteIntegration_(integroIntegrationOrder),
      mesher_(mesher),
      bcSet_(bcSet),
      hestonOp_(new FdmHestonOp(mesher, 
           boost::shared_ptr<HestonProcess>(new HestonProcess(
               batesProcess->riskFreeRate(),  
               Handle<YieldTermStructure>(
                   new ZeroSpreadedTermStructure(
                           batesProcess->dividendYield(),
                           Handle<Quote>(new SimpleQuote(lambda_*m_)),
                           Continuous, NoFrequency, 
                           batesProcess->dividendYield()->dayCounter())),
               batesProcess->s0(),    batesProcess->v0(),
               batesProcess->kappa(), batesProcess->theta(),
               batesProcess->sigma(), batesProcess->rho())), 
           quantoHelper)) {                        
    }

    Size FdmBatesOp::size() const {
        return hestonOp_->size();
    }
    
    void FdmBatesOp::setTime(Time t1, Time t2) {
        hestonOp_->setTime(t1, t2);
    }
    
    Disposable<Array> FdmBatesOp::apply(const Array& r) const {
        return hestonOp_->apply(r) + integro(r);
    }
    
    Disposable<Array> FdmBatesOp::apply_mixed(const Array& r) const {
        return hestonOp_->apply_mixed(r) + integro(r);
    }

    Disposable<Array> FdmBatesOp::apply_direction(Size direction,
                                                  const Array& r) const {
        return hestonOp_->apply_direction(direction, r);
    }

    Disposable<Array> FdmBatesOp::solve_splitting(Size direction,
                                                  const Array& r, Real s) const{
        return hestonOp_->solve_splitting(direction, r, s);
    }
 
    Disposable<Array> FdmBatesOp::preconditioner(const Array& r, Real s) const {
        return hestonOp_->preconditioner(r, s);
    }
    
    
    FdmBatesOp::IntegroIntegrand::IntegroIntegrand(
                    const boost::shared_ptr<LinearInterpolation>& interpl,
                    const FdmBoundaryConditionSet& bcSet,
                    Real x, Real delta, Real nu)
    : x_(x), delta_(delta), nu_(nu), 
      bcSet_(bcSet), interpl_(interpl) { }
                    
    Real FdmBatesOp::IntegroIntegrand::operator()(Real y) const {
        const Real x = x_ + M_SQRT2*delta_*y + nu_;
        Real valueOfDerivative = interpl_->operator()(x, true);
        
        for (FdmBoundaryConditionSet::const_iterator iter=bcSet_.begin();
            iter < bcSet_.end(); ++iter) {
            valueOfDerivative=(*iter)->applyAfterApplying(x, valueOfDerivative);
        }
        
        return std::exp(-y*y)*valueOfDerivative;
    }
    
    Disposable<Array> FdmBatesOp::integro(const Array& r) const {
        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
        
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
        std::vector<boost::shared_ptr<LinearInterpolation> > interpl(f.rows());
        for (Size i=0; i < f.rows(); ++i) {
            interpl[i] = boost::shared_ptr<LinearInterpolation>(
                new LinearInterpolation(x.begin(), x.end(), f.row_begin(i)));
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
}
