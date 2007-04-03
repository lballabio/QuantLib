/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

#include <ql/volatilities/sabrinterpolatedsmilesection.hpp>

namespace QuantLib {

    SabrInterpolatedSmileSection::SabrInterpolatedSmileSection(
                       const Date& optionDate,
                       const std::vector<Rate>& strikes,
                       const std::vector<Handle<Quote> >& stdDevHandles,
                       const Handle<Quote>& forward,
                       Real alpha,
                       Real beta,
                       Real nu,
                       Real rho,
                       bool isAlphaFixed,
                       bool isBetaFixed,
                       bool isNuFixed,
                       bool isRhoFixed,
                       bool vegaWeighted,
                       const boost::shared_ptr<EndCriteria>& endCriteria,
                       const boost::shared_ptr<OptimizationMethod>& method,
                       const DayCounter& dc)
    : SmileSection(optionDate, dc),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(strikes),
      stdDevHandles_(stdDevHandles), forward_(forward),
      vols_(stdDevHandles.size()),
      sabrInterpolation_(strikes_.begin(), strikes_.end(), vols_.begin(),
                         exerciseTime(), forwardValue_, alpha, beta, nu, rho,
                         isAlphaFixed, isBetaFixed,
                         isNuFixed, isRhoFixed, vegaWeighted,
                         endCriteria, method) {
        registerWith(forward_);
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            registerWith(stdDevHandles_[i]);
    }

    void SabrInterpolatedSmileSection::performCalculations() const {
        forwardValue_ = 1-forward_->value()/100;
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            vols_[i] = stdDevHandles_[i]->value()/exerciseTimeSquareRoot_;
        sabrInterpolation_.update();
    }

    Real SabrInterpolatedSmileSection::variance(Real strike) const {
        calculate();
        Real v = sabrInterpolation_(strike, true);
        return v*v*exerciseTime();
    }

}
