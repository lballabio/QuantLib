/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 StatPro Italia srl

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

#include <ql/experimental/credit/issuer.hpp>

namespace QuantLib {

    Issuer::Issuer(const Handle<DefaultProbabilityTermStructure>& probability,
                   Real recoveryRate,
                   const std::vector<boost::shared_ptr<DefaultEvent> >& events)
    : probability_(probability), recoveryRate_(recoveryRate),
      events_(events) {
        std::sort(events_.begin(), events_.end(),
                  earlier_than<boost::shared_ptr<DefaultEvent> >());
    }


    const Handle<DefaultProbabilityTermStructure>&
    Issuer::defaultProbability() const {
        return probability_;
    }

    Real Issuer::recoveryRate() const {
        return recoveryRate_;
    }


    namespace {

        bool seniority_matches(const boost::shared_ptr<DefaultEvent>& e,
                               Seniority s) {
            // AnySeniority acts as a wildcard on either side
            return s == AnySeniority
                || e->seniority() == s
                || e->seniority() == AnySeniority;
        }

        bool restructuring_matches(const boost::shared_ptr<DefaultEvent>& e,
                                   Restructuring r) {
            // AnyRestructuring acts as a wildcard on either side
            return r == AnyRestructuring
                || e->restructuring() == r
                || e->restructuring() == AnyRestructuring;
        }

        bool between(const boost::shared_ptr<DefaultEvent>& e,
                     const Date& start,
                     const Date& end) {
            return !e->hasOccurred(start) && e->hasOccurred(end);
        }

    }


    boost::shared_ptr<DefaultEvent>
    Issuer::defaultedBetween(const Date& start,
                             const Date& end,
                             Seniority seniority,
                             Restructuring restructuring) const {
        for (Size i=0; i<events_.size(); ++i) {
            if (seniority_matches(events_[i], seniority) &&
                restructuring_matches(events_[i], restructuring) &&
                between(events_[i], start, end))
                return events_[i];
        }
        return boost::shared_ptr<DefaultEvent>();
    }


    Issuer::operator boost::shared_ptr<Observable>() const {
        return probability_;
    }

}

