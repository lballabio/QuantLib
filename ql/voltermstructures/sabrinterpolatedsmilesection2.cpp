/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

#include <ql/voltermstructures/sabrinterpolatedsmilesection2.hpp>
#include <ql/quote.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    SabrInterpolatedSmileSection2::SabrInterpolatedSmileSection2(
                       const Date& optionDate,
                       const std::vector<Rate>& strikeSpreads,
                       const Handle<Quote>& atmVolatility,
                       const std::vector<Handle<Quote> >& volSpreads,
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
    : SabrInterpolatedSmileSection(optionDate,
                       strikeSpreads,
                       volSpreads,
                       forward,
                       alpha, beta, nu, rho,
                       isAlphaFixed, isBetaFixed,
                       isNuFixed, isRhoFixed, vegaWeighted,
                       endCriteria, method, dc),
      strikeSpreads_(strikeSpreads),
      volSpreads_(volSpreads),
      atmVolatility_(atmVolatility)
    {

        LazyObject::registerWith(atmVolatility_);
        for (Size i=0; i<volSpreads_.size(); ++i)
            LazyObject::registerWith(volSpreads_[i]);
    }

    void SabrInterpolatedSmileSection2::performCalculations() const {
        forwardValue_ = forward_->value();
        vols_.clear();
        actualStrikes_.clear();
        for (Size i=0; i<strikeSpreads_.size(); ++i) {
                vols_.push_back(atmVolatility_->value()+ volSpreads_[i]->value());
                actualStrikes_.push_back(forwardValue_+ strikeSpreads_[i]);
        }

        // we are recreating the sabrinterpolation object unconditionnaly to
        // avoid iterator invalidation
        createInterpolation();

        sabrInterpolation_->update();
    }

}
