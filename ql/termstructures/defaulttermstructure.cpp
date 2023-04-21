/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
        const DayCounter& dc, std::vector<Handle<Quote> > jumps, const std::vector<Date>& jumpDates, bool extrapolate)
    : TermStructure(dc, extrapolate), jumps_(std::move(jumps)), jumpDates_(jumpDates),
      jumpTimes_(jumpDates.size()), nJumps_(jumps_.size()) {
        setJumps();
        for (Size i=0; i<nJumps_; ++i)
            registerWith(jumps_[i]);
    }

    DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
        const Date& referenceDate,
        const Calendar& cal,
        const DayCounter& dc,
        std::vector<Handle<Quote> > jumps,
        const std::vector<Date>& jumpDates,
        bool extrapolate)
    : TermStructure(referenceDate, cal, dc, extrapolate), jumps_(std::move(jumps)), jumpDates_(jumpDates),
      jumpTimes_(jumpDates.size()), nJumps_(jumps_.size()) {
        setJumps();
        for (Size i=0; i<nJumps_; ++i)
            registerWith(jumps_[i]);
    }

    DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
        Natural settlementDays,
        const Calendar& cal,
        const DayCounter& dc,
        std::vector<Handle<Quote> > jumps,
        const std::vector<Date>& jumpDates,
        bool extrapolate)
    : TermStructure(settlementDays, cal, dc, extrapolate), jumps_(std::move(jumps)), jumpDates_(jumpDates),
      jumpTimes_(jumpDates.size()), nJumps_(jumps_.size()) {
        setJumps();
        for (Size i=0; i<nJumps_; ++i)
            registerWith(jumps_[i]);
    }

    void DefaultProbabilityTermStructure::setJumps() {
        if (jumpDates_.empty() && !jumps_.empty()) { // turn of year dates
            jumpDates_.resize(nJumps_);
            jumpTimes_.resize(nJumps_);
            Year y = referenceDate().year();
            for (Size i=0; i<nJumps_; ++i)
                jumpDates_[i] = Date(31, December, y+i);
        } else { // fixed dats
            QL_REQUIRE(jumpDates_.size()==nJumps_,
                       "mismatch between number of jumps (" << nJumps_ <<
                       ") and jump dates (" << jumpDates_.size() << ")");
        }
        for (Size i=0; i<nJumps_; ++i)
            jumpTimes_[i] = timeFromReference(jumpDates_[i]);
        latestReference_ = referenceDate();
    }

    Probability DefaultProbabilityTermStructure::survivalProbability(
                                                     Time t,
                                                     bool extrapolate) const {
        checkRange(t, extrapolate);

        if (!jumps_.empty()) {
            Probability jumpEffect = 1.0;
            for (Size i=0; i<nJumps_ && jumpTimes_[i]<t; ++i) {
                QL_REQUIRE(jumps_[i]->isValid(),
                           "invalid " << io::ordinal(i+1) << " jump quote");
                DiscountFactor thisJump = jumps_[i]->value();
                QL_REQUIRE(thisJump > 0.0 && thisJump <= 1.0,
                           "invalid " << io::ordinal(i+1) << " jump value: " <<
                           thisJump);
                jumpEffect *= thisJump;
            }
            return jumpEffect * survivalProbabilityImpl(t);
        }

        return survivalProbabilityImpl(t);
    }

    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     const Date& d1,
                                                     const Date& d2,
                                                     bool extrapolate) const {
        QL_REQUIRE(d1 <= d2,
                   "initial date (" << d1 << ") "
                   "later than final date (" << d2 << ")");
        Probability p1 = d1 < referenceDate() ? 0.0 :
                                           defaultProbability(d1,extrapolate),
                    p2 = defaultProbability(d2,extrapolate);
        return p2 - p1;
    }

    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     Time t1,
                                                     Time t2,
                                                     bool extrapolate) const {
        QL_REQUIRE(t1 <= t2,
                   "initial time (" << t1 << ") "
                   "later than final time (" << t2 << ")");
        Probability p1 = t1 < 0.0 ? 0.0 : defaultProbability(t1,extrapolate),
                    p2 = defaultProbability(t2,extrapolate);
        return p2 - p1;
    }

}
