/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2007 Cristina Duminuco
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

#include <ql/termstructures/volatility/sabrinterpolatedsmilesection.hpp>
#include <ql/settings.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    SabrInterpolatedSmileSection::SabrInterpolatedSmileSection(
                const Date& optionDate,
                const Handle<Quote>& forward,
                const std::vector<Rate>& strikes,
                bool hasFloatingStrikes,
                const Handle<Quote>& atmVolatility,
                const std::vector<Handle<Quote> >& volHandles,
                Real alpha, Real beta, Real nu, Real rho,
                bool isAlphaFixed, bool isBetaFixed,
                bool isNuFixed, bool isRhoFixed,
                bool vegaWeighted,
                const boost::shared_ptr<EndCriteria>& endCriteria,
                const boost::shared_ptr<OptimizationMethod>& method,
                const DayCounter& dc)
         : SmileSection(optionDate, dc),
           forward_(forward), atmVolatility_(atmVolatility),
           volHandles_(volHandles), strikes_(strikes), actualStrikes_(strikes),
           hasFloatingStrikes_(hasFloatingStrikes), vols_(volHandles.size()),
           alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
           isAlphaFixed_(isAlphaFixed), isBetaFixed_(isBetaFixed),
           isNuFixed_(isNuFixed), isRhoFixed_(isRhoFixed),
           vegaWeighted_(vegaWeighted),
           endCriteria_(endCriteria), method_(method),
           evaluationDate_(Settings::instance().evaluationDate()) {

            LazyObject::registerWith(forward_);
            LazyObject::registerWith(atmVolatility_);
            for (Size i=0; i<volHandles_.size(); ++i)
                LazyObject::registerWith(volHandles_[i]);
    }

    SabrInterpolatedSmileSection::SabrInterpolatedSmileSection(
               const Date& optionDate,
               const Rate& forward,
               const std::vector<Rate>& strikes,
               bool hasFloatingStrikes,
               const Volatility& atmVolatility,
               const std::vector<Volatility>& volHandles,
               Real alpha, Real beta, Real nu, Real rho,
               bool isAlphaFixed, bool isBetaFixed,
               bool isNuFixed, bool isRhoFixed,
               bool vegaWeighted,
               const boost::shared_ptr<EndCriteria>& endCriteria,
               const boost::shared_ptr<OptimizationMethod>& method,
               const DayCounter& dc)
         : SmileSection(optionDate, dc),
           forward_(Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(forward)))),
           atmVolatility_(Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(atmVolatility)))),
           volHandles_(volHandles.size()), strikes_(strikes), actualStrikes_(strikes),
           hasFloatingStrikes_(hasFloatingStrikes), vols_(volHandles.size()),
           alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
           isAlphaFixed_(isAlphaFixed), isBetaFixed_(isBetaFixed),
           isNuFixed_(isNuFixed), isRhoFixed_(isRhoFixed),
           vegaWeighted_(vegaWeighted),
           endCriteria_(endCriteria), method_(method),
           evaluationDate_(Settings::instance().evaluationDate()) {

            for (Size i=0; i<volHandles_.size(); ++i)
                volHandles_[i] = Handle<Quote>(boost::shared_ptr<Quote>(new
                                        SimpleQuote(volHandles[i])));

    }

    void SabrInterpolatedSmileSection::createInterpolation() const {
         boost::shared_ptr<SABRInterpolation> tmp(new SABRInterpolation(
                     actualStrikes_.begin(), actualStrikes_.end(), vols_.begin(),
                     exerciseTime(), forwardValue_,
                     alpha_, beta_, nu_, rho_,
                     isAlphaFixed_, isBetaFixed_, isNuFixed_, isRhoFixed_, vegaWeighted_,
                     endCriteria_, method_));
         swap(tmp, sabrInterpolation_);
    }

    void SabrInterpolatedSmileSection::performCalculations() const {
        forwardValue_ = forward_->value();
        vols_.clear();
        actualStrikes_.clear();
        // we populate the volatilities, skipping the invalid ones
        for (Size i=0; i<volHandles_.size(); ++i) {
            if (volHandles_[i]->isValid()) {
                if (hasFloatingStrikes_) {
                    actualStrikes_.push_back(forwardValue_ + strikes_[i]);
                    vols_.push_back(atmVolatility_->value() + volHandles_[i]->value());
                } else {
                    actualStrikes_.push_back(strikes_[i]);
                    vols_.push_back(volHandles_[i]->value());
                }
            }
        }
        // we are recreating the sabrinterpolation object unconditionnaly to
        // avoid iterator invalidation
        createInterpolation();
        sabrInterpolation_->update();
    }

    Real SabrInterpolatedSmileSection::varianceImpl(Real strike) const {
        calculate();
        Real v = (*sabrInterpolation_)(strike, true);
        return v*v*exerciseTime();
    }

}

