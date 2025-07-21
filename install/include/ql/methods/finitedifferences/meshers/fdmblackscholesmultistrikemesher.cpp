/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file fdmblackscholesmultistrikemesher.cpp
    \brief 1-d mesher for the Black-Scholes process (in ln(S))
*/

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmultistrikemesher.hpp>

namespace QuantLib {

    FdmBlackScholesMultiStrikeMesher::FdmBlackScholesMultiStrikeMesher(
            Size size,
            const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Time maturity, const std::vector<Real>& strikes,
            Real eps, Real scaleFactor,
            const std::pair<Real, Real>& cPoint)
    : Fdm1dMesher(size) {

        const Real spot = process->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        const DiscountFactor d =   process->dividendYield()->discount(maturity)
                                 / process->riskFreeRate()->discount(maturity);
        const Real minStrike= *std::min_element(strikes.begin(), strikes.end());
        const Real maxStrike= *std::max_element(strikes.begin(), strikes.end());
                
        const Real Fmin = spot*spot/maxStrike*d;
        const Real Fmax = spot*spot/minStrike*d;
        
        QL_REQUIRE(Fmin > 0.0, "negative forward given");

        // Set the grid boundaries
        const Real normInvEps = InverseCumulativeNormal()(1-eps);
        const Real sigmaSqrtTmin 
            = process->blackVolatility()->blackVol(maturity, minStrike)
                                                        *std::sqrt(maturity);
        const Real sigmaSqrtTmax 
            = process->blackVolatility()->blackVol(maturity, maxStrike)
                                                        *std::sqrt(maturity);
        
        const Real xMin
            = std::min(0.8*std::log(0.8*spot*spot/maxStrike),
                       std::log(Fmin) - sigmaSqrtTmin*normInvEps*scaleFactor
                                      - sigmaSqrtTmin*sigmaSqrtTmin/2.0);
        const Real xMax
            = std::max(1.2*std::log(0.8*spot*spot/minStrike),
                       std::log(Fmax) + sigmaSqrtTmax*normInvEps*scaleFactor
                                      - sigmaSqrtTmax*sigmaSqrtTmax/2.0);

        ext::shared_ptr<Fdm1dMesher> helper;
        if (   cPoint.first != Null<Real>() 
            && std::log(cPoint.first) >=xMin && std::log(cPoint.first) <=xMax) {
            
            helper = ext::shared_ptr<Fdm1dMesher>(
                new Concentrating1dMesher(xMin, xMax, size, 
                    std::pair<Real,Real>(std::log(cPoint.first),cPoint.second)));
        }
        else {
            helper = ext::shared_ptr<Fdm1dMesher>(
                                        new Uniform1dMesher(xMin, xMax, size));
            
        }

        locations_ = helper->locations();
        for (Size i=0; i < locations_.size(); ++i) {
            dplus_[i]  = helper->dplus(i);
            dminus_[i] = helper->dminus(i);
        }
    }            
}
