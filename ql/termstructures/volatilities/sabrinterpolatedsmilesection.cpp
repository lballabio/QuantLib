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

#include <ql/termstructures/volatilities/sabrinterpolatedsmilesection.hpp>
#include <ql/quote.hpp>
#include <ql/settings.hpp>

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
      endCriteria_(endCriteria), method_(method),
      strikes_(strikes), stdDevHandles_(stdDevHandles), forward_(forward),
      isRhoFixed_(isRhoFixed), vegaWeighted_(vegaWeighted),
      vols_(stdDevHandles.size()),
      alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
      isAlphaFixed_(isAlphaFixed), isBetaFixed_(isBetaFixed),
      isNuFixed_(isNuFixed),
      evaluationDate_(Settings::instance().evaluationDate()) {
        LazyObject::registerWith(forward_);
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            LazyObject::registerWith(stdDevHandles_[i]);
    }

    void SabrInterpolatedSmileSection::createInterpolation() const {
         boost::scoped_ptr<SABRInterpolation> tmp(new SABRInterpolation(
            strikes_.begin(), strikes_.end(), vols_.begin(),
                     exerciseTime(), forwardValue_, alpha_, beta_, nu_, rho_,
                     isAlphaFixed_, isBetaFixed_,
                     isNuFixed_, isRhoFixed_, vegaWeighted_,
                     endCriteria_, method_));
            swap(tmp, sabrInterpolation_);
    }

    void SabrInterpolatedSmileSection::performCalculations() const {
        forwardValue_ = 1-forward_->value()/100;
        Time exerciseTimeSquareRoot = std::sqrt(exerciseTime());
        
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            vols_[i] = stdDevHandles_[i]->value()/exerciseTimeSquareRoot;
        
        if(evaluationDate_ != Settings::instance().evaluationDate()) {
            createInterpolation();
            evaluationDate_ = Settings::instance().evaluationDate();
        }
        if (!sabrInterpolation_) createInterpolation();

        sabrInterpolation_->update();
    }

    Real SabrInterpolatedSmileSection::varianceImpl(Real strike) const {
        calculate();
        Real v = (*sabrInterpolation_)(strike, true);
        return v*v*exerciseTime();
    }

}
