/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Klaus Spanderen

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

#include <ql/stochasticprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/meshers/fdmsimpleprocess1dmesher.hpp>

namespace QuantLib {

    FdmSimpleProcess1dMesher::FdmSimpleProcess1dMesher(
        Size size,
        const ext::shared_ptr<StochasticProcess1D>& process,
        Time maturity, Size tAvgSteps, Real eps, Real mandatoryPoint)
        : Fdm1dMesher(size) {
            
        std::fill(locations_.begin(), locations_.end(), 0.0);    
        for (Size l=1; l<=tAvgSteps; ++l) {
            const Real t = (maturity*l)/tAvgSteps;
            
            const Real mp = (mandatoryPoint != Null<Real>()) ? mandatoryPoint
                                                             : process->x0();

            const Real qMin = std::min(std::min(mp, process->x0()),
                process->evolve(0, process->x0(), t, 
                                InverseCumulativeNormal()(eps)));
            const Real qMax = std::max(std::max(mp, process->x0()),
                process->evolve(0, process->x0(), t,
                                InverseCumulativeNormal()(1-eps)));
            
            const Real dp = (1-2*eps)/(size-1);
            Real p = eps;
            locations_[0] += qMin;
            
            for (Size i=1; i < size-1; ++i) {
                p += dp;
                locations_[i] += process->evolve(0, process->x0(), t, 
                                                 InverseCumulativeNormal()(p));
            }
            locations_.back() += qMax;
        }
        std::transform(locations_.begin(), locations_.end(), locations_.begin(),
                       [=](Real x) -> Real { return x / tAvgSteps; });
        for (Size i=0; i < size-1; ++i) {
            dminus_[i+1] = dplus_[i] = locations_[i+1] - locations_[i];
        }
        dplus_.back() = dminus_.front() = Null<Real>();        
    }
}
