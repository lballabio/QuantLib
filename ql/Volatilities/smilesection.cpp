/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci

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



#include <ql/Volatilities/smilesection.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/sabrinterpolation.hpp>

namespace QuantLib {


     SmileSection::SmileSection(Time timeToExpiry,
                                const std::vector<Rate>& strikes,
                                const std::vector<Rate>& volatilities)
     : timeToExpiry_(timeToExpiry), strikes_(strikes),
       volatilities_(volatilities) {

        interpolation_ = boost::shared_ptr<Interpolation>(new
            //SABRInterpolation(strikes_.begin(), strikes_.end(), volatilities_.begin(), start, atmForward, Null<Real>(), Null<Real>(), Null<Real>(), Null<Real>())
            LinearInterpolation(strikes_.begin(), strikes_.end(), volatilities_.begin())
            //NaturalCubicSpline(strikes_.begin(), strikes_.end(), volatilities_.begin())
            );
     }


     SmileSection::SmileSection(const std::vector<Real>& sabrParameters,
                                const Time timeToExpiry)
     : timeToExpiry_(timeToExpiry) {

             //fictitious data due to SABRInterpolation redundant input needs
             for (Size i=0; i<2; i++) {
                 strikes_.push_back(0.05*i+.01);
                 volatilities_.push_back(.9);
             }

             Real alpha = sabrParameters[0];
             Real beta = sabrParameters[1];
             Real nu = sabrParameters[2];
             Real rho = sabrParameters[3];
             Real forwardValue = sabrParameters[4];

             interpolation_ = boost::shared_ptr<Interpolation>(new
                  SABRInterpolation(strikes_.begin(), strikes_.end(),
                  volatilities_.begin(),
                  timeToExpiry, forwardValue,
                  alpha, beta, nu, rho,
                  true, true, true, true,
                  boost::shared_ptr<OptimizationMethod>()));
      }

    Real SmileSection::variance(Real strike) const {
        const Real v = interpolation_->operator()(strike, true);
        return v*v*timeToExpiry_;
    }

    Volatility SmileSection::volatility(Rate strike) const {
        const Real v = interpolation_->operator()(strike, true);
        return v;
    }

}

