/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/FiniteDifferences/onefactoroperator.hpp>

#if !defined(QL_PATCH_BORLAND)

namespace QuantLib {

    OneFactorOperator::OneFactorOperator(
          const Array& grid,
          const boost::shared_ptr<OneFactorModel::ShortRateDynamics>& process)
    : TridiagonalOperator(grid.size()) {
        timeSetter_ = boost::shared_ptr<TridiagonalOperator::TimeSetter>(
                 new SpecificTimeSetter(grid[0], grid[1] - grid[0], process));
    }

    OneFactorOperator::SpecificTimeSetter::SpecificTimeSetter(
         Real x0, Real dx,
         const boost::shared_ptr<OneFactorModel::ShortRateDynamics>& dynamics)
    : x0_(x0), dx_(dx), dynamics_(dynamics) {}

    void OneFactorOperator::SpecificTimeSetter::setTime(
                                            Time t,
                                            TridiagonalOperator& op) const {
        Size length = op.size();
        for (Size i=0; i<length; i++) {
            Real x = x0_ + dx_*i;

            Rate r = dynamics_->shortRate(t, x);
            Real mu = dynamics_->process()->drift(t, x);
            Real sigma = dynamics_->process()->diffusion(t, x);

            Real sigma2 = sigma*sigma;
            Real pdown = (- sigma2/(2.0*dx_*dx_) ) + mu/(2.0*dx_);
            Real pm    = (+ sigma2/(dx_*dx_) )     + r;
            Real pup   = (- sigma2/(2.0*dx_*dx_) ) - mu/(2.0*dx_);
            if (i==0)
                op.setFirstRow(pm, pup);
            else if (i==(length - 1))
                op.setLastRow(pdown, pm);
            else
                op.setMidRow(i, pdown, pm, pup);
        }
    }

}

#endif

