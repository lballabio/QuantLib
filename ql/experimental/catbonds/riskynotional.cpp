/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Grzegorz Andruszkiewicz

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

#include <ql/experimental/catbonds/riskynotional.hpp>

namespace QuantLib
{
    NotionalPath::NotionalPath()
    {
        Rate previous = 1.0;//full notional at the beginning
        notionalRate_.push_back(std::pair<Date, Real>(Date(), previous));
    }

    Rate NotionalPath::notionalRate(const Date& date) const
    {
        Size i = 0;
        for (; i<notionalRate_.size() && notionalRate_[i].first<=date; ++i)  //TODO do we take notional after reductions or before?
        {}
        return notionalRate_[i-1].second;
    }

    void NotionalPath::reset() {
        notionalRate_.resize(1);
    }

    void NotionalPath::addReduction(const Date &date, Rate newRate) {
        notionalRate_.push_back(std::pair<Date, Real>(date, newRate));
    }

    Real NotionalPath::loss() {
        return 1.0-notionalRate_.rbegin()->second;
    }

    void DigitalNotionalRisk::updatePath(const std::vector<std::pair<Date, Real> >  &events, 
                                         NotionalPath &path) const {
        path.reset();
        for(size_t i=0; i<events.size(); ++i) {
            if(events[i].second>=threshold_) {
                path.addReduction(paymentOffset_->paymentDate(events[i].first), Rate(0.0));
            }
        }
    }
}
