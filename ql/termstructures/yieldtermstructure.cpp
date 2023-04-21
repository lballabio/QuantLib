/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2009 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    namespace {
        // time interval used in finite differences
        const Time dt = 0.0001;
    }

    YieldTermStructure::YieldTermStructure(const DayCounter& dc,
                                           bool extrapolate)
    : TermStructure(dc, extrapolate) {}

    YieldTermStructure::YieldTermStructure(const Date& referenceDate,
                                           const Calendar& cal,
                                           const DayCounter& dc,
                                           std::vector<Handle<Quote> > jumps,
                                           const std::vector<Date>& jumpDates,
                                           bool extrapolate)
    : TermStructure(referenceDate, cal, dc, extrapolate), jumps_(std::move(jumps)), jumpDates_(jumpDates),
      jumpTimes_(jumpDates.size()), nJumps_(jumps_.size()) {
        setJumps(YieldTermStructure::referenceDate());
        for (Size i=0; i<nJumps_; ++i)
            registerWith(jumps_[i]);
    }

    YieldTermStructure::YieldTermStructure(Natural settlementDays,
                                           const Calendar& cal,
                                           const DayCounter& dc,
                                           std::vector<Handle<Quote> > jumps,
                                           const std::vector<Date>& jumpDates,
                                           bool extrapolate)
    : TermStructure(settlementDays, cal, dc, extrapolate), jumps_(std::move(jumps)), jumpDates_(jumpDates),
      jumpTimes_(jumpDates.size()), nJumps_(jumps_.size()) {
        setJumps(YieldTermStructure::referenceDate());
        for (Size i=0; i<nJumps_; ++i)
            registerWith(jumps_[i]);
    }

    void YieldTermStructure::setJumps(const Date& referenceDate) {
        if (jumpDates_.empty() && !jumps_.empty()) { // turn of year dates
            jumpDates_.resize(nJumps_);
            jumpTimes_.resize(nJumps_);
            Year y = referenceDate.year();
            for (Size i=0; i<nJumps_; ++i)
                jumpDates_[i] = Date(31, December, y+i);
        } else { // fixed dates
            QL_REQUIRE(jumpDates_.size()==nJumps_,
                       "mismatch between number of jumps (" << nJumps_ <<
                       ") and jump dates (" << jumpDates_.size() << ")");
        }
        for (Size i=0; i<nJumps_; ++i)
            jumpTimes_[i] = timeFromReference(jumpDates_[i]);
        latestReference_ = referenceDate;
    }

    DiscountFactor YieldTermStructure::discount(Time t,
                                                bool extrapolate) const {
        checkRange(t, extrapolate);

        if (jumps_.empty())
            return discountImpl(t);

        DiscountFactor jumpEffect = 1.0;
        for (Size i=0; i<nJumps_; ++i) {
            if (jumpTimes_[i]>0 && jumpTimes_[i]<t) {
                QL_REQUIRE(jumps_[i]->isValid(),
                           "invalid " << io::ordinal(i+1) << " jump quote");
                DiscountFactor thisJump = jumps_[i]->value();
                QL_REQUIRE(thisJump > 0.0,
                           "invalid " << io::ordinal(i+1) << " jump value: " <<
                           thisJump);
                jumpEffect *= thisJump;
            }
        }
        return jumpEffect * discountImpl(t);
    }

    InterestRate YieldTermStructure::zeroRate(const Date& d,
                                              const DayCounter& dayCounter,
                                              Compounding comp,
                                              Frequency freq,
                                              bool extrapolate) const {
        Time t = timeFromReference(d);
        if (t == 0) {
            Real compound = 1.0/discount(dt, extrapolate);
            // t has been calculated with a possibly different daycounter
            // but the difference should not matter for very small times
            return InterestRate::impliedRate(compound,
                                             dayCounter, comp, freq,
                                             dt);
        }
        Real compound = 1.0/discount(t, extrapolate);
        return InterestRate::impliedRate(compound,
                                         dayCounter, comp, freq,
                                         referenceDate(), d);
    }

    InterestRate YieldTermStructure::zeroRate(Time t,
                                              Compounding comp,
                                              Frequency freq,
                                              bool extrapolate) const {
        if (t==0.0) t = dt;
        Real compound = 1.0/discount(t, extrapolate);
        return InterestRate::impliedRate(compound,
                                         dayCounter(), comp, freq,
                                         t);
    }

    InterestRate YieldTermStructure::forwardRate(const Date& d1,
                                                 const Date& d2,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        if (d1==d2) {
            checkRange(d1, extrapolate);
            Time t1 = std::max(timeFromReference(d1) - dt/2.0, 0.0);
            Time t2 = t1 + dt;
            Real compound =
                discount(t1, true)/discount(t2, true);
            // times have been calculated with a possibly different daycounter
            // but the difference should not matter for very small times
            return InterestRate::impliedRate(compound,
                                             dayCounter, comp, freq,
                                             dt);
        }
        QL_REQUIRE(d1 < d2,  d1 << " later than " << d2);
        Real compound = discount(d1, extrapolate)/discount(d2, extrapolate);
        return InterestRate::impliedRate(compound,
                                         dayCounter, comp, freq,
                                         d1, d2);
    }

    InterestRate YieldTermStructure::forwardRate(Time t1,
                                                 Time t2,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        Real compound;
        if (t2==t1) {
            checkRange(t1, extrapolate);
            t1 = std::max(t1 - dt/2.0, 0.0);
            t2 = t1 + dt;
            compound = discount(t1, true)/discount(t2, true);
        } else {
            QL_REQUIRE(t2>t1, "t2 (" << t2 << ") < t1 (" << t2 << ")");
            compound = discount(t1, extrapolate)/discount(t2, extrapolate);
        }
        return InterestRate::impliedRate(compound,
                                         dayCounter(), comp, freq,
                                         t2-t1);
    }

    void YieldTermStructure::update() {
        TermStructure::update();
        Date newReference = Date();
        try {
            newReference = referenceDate();
            if (newReference != latestReference_)
                setJumps(newReference);
        } catch (Error&) {
            if (newReference == Date()) {
                // the curve couldn't calculate the reference
                // date. Most of the times, this is because some
                // underlying handle wasn't set, so we can just absorb
                // the exception and continue; the jumps will be set
                // correctly when a valid underlying is set.
                return;
            } else {
                // something else happened during the call to
                // setJumps(), so we let the exception bubble up.
                throw;
            }
        }
    }

}
