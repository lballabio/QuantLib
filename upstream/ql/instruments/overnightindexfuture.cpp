/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Roy Zywina
 Copyright (C) 2019 Eisuke Tani

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/overnightindexfuture.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/event.hpp>
#include <utility>

namespace QuantLib {

    OvernightIndexFuture::OvernightIndexFuture(ext::shared_ptr<OvernightIndex> overnightIndex,
                                               const Date& valueDate,
                                               const Date& maturityDate,
                                               Handle<Quote> convexityAdjustment,
                                               RateAveraging::Type averagingMethod)
    : overnightIndex_(std::move(overnightIndex)), valueDate_(valueDate),
      maturityDate_(maturityDate), convexityAdjustment_(std::move(convexityAdjustment)),
      averagingMethod_(averagingMethod) {
        QL_REQUIRE(overnightIndex_, "null overnight index");
        registerWith(overnightIndex_);
        registerWith(convexityAdjustment_);
        registerWith(Settings::instance().evaluationDate());
    }

    Real OvernightIndexFuture::averagedRate() const {
        Date today = Settings::instance().evaluationDate();
        Calendar calendar = overnightIndex_->fixingCalendar();
        DayCounter dayCounter = overnightIndex_->dayCounter();
        Handle<YieldTermStructure> forwardCurve = overnightIndex_->forwardingTermStructure();
        Real avg = 0;
        Date d1 = valueDate_;
        // d1 could be a holiday
        Date fixingDate = calendar.adjust(d1, Preceding);
        const auto& history = overnightIndex_->timeSeries();
        Real fwd;
        while (d1 < maturityDate_) {
            Date d2 = calendar.advance(d1, 1, Days);
            if (fixingDate < today) {
                fwd = history[fixingDate];
                QL_REQUIRE(fwd != Null<Real>(),
                           "missing rate on " << fixingDate << " for index " << overnightIndex_->name());
            } else if (fixingDate == today) {
                fwd = history[fixingDate];
                if (fwd == Null<Real>())
                    fwd = forwardCurve->forwardRate(fixingDate, d2, dayCounter, Simple).rate();
            } else {
                fwd = forwardCurve->forwardRate(fixingDate, d2, dayCounter, Simple).rate();
            }
            // The rate is accrued starting from d1 even when the fixing date is earlier.
            // d2 might be beyond the maturity date if the latter is a holiday.
            avg += fwd * dayCounter.yearFraction(d1, std::min(d2, maturityDate_));
            fixingDate = d1 = d2;
        }

        return avg / dayCounter.yearFraction(valueDate_, maturityDate_);
    }

    Real OvernightIndexFuture::compoundedRate() const {
        Date today = Settings::instance().evaluationDate();
        Calendar calendar = overnightIndex_->fixingCalendar();
        DayCounter dayCounter = overnightIndex_->dayCounter();
        Handle<YieldTermStructure> forwardCurve = overnightIndex_->forwardingTermStructure();
        Real prod = 1;
        Date forwardDiscountStart = valueDate_;
        if (today > valueDate_) {
            // can't value on a weekend inside reference period because we
            // won't know the reset rate until start of next business day.
            // user can supply an estimate if they really want to do this
            today = calendar.adjust(today);
            forwardDiscountStart = today;
            // for valuations inside the reference period, index quotes
            // must have been populated in the history
            const auto& history = overnightIndex_->timeSeries();
            Date d1 = valueDate_;
            // d1 could be a holiday
            Date fixingDate = calendar.adjust(d1, Preceding);
            while (d1 < today) {
                Real r = history[fixingDate];
                QL_REQUIRE(r != Null<Real>(),
                           "missing rate on " << fixingDate << " for index " << overnightIndex_->name());
                Date d2 = calendar.advance(d1, 1, Days);
                // The rate is accrued starting from d1 even when the fixing date is earlier.
                // We can't get to the maturity date inside this loop,
                // so we don't need to cap d2 like we do in averagedRate above.
                prod *= 1 + r * dayCounter.yearFraction(d1, d2);
                fixingDate = d1 = d2;
            }
            // here d1 == today, and we might have today's fixing already
            if (today < maturityDate_) {
                Real r = history[today];
                if (r != Null<Real>()) {
                    Date tomorrow = calendar.advance(today, 1, Days);
                    prod *= 1 + r * dayCounter.yearFraction(today, tomorrow);
                    forwardDiscountStart = tomorrow;
                }
            }
        }
        // the telescopic part goes from the end of the last known fixing to the maturity
        DiscountFactor forwardDiscount =
            forwardCurve->discount(maturityDate_) / forwardCurve->discount(forwardDiscountStart);
        prod /= forwardDiscount;

        return (prod - 1) / dayCounter.yearFraction(valueDate_, maturityDate_);
    }

    Real OvernightIndexFuture::rate() const {
        switch (averagingMethod_) {
          case RateAveraging::Simple:
            return averagedRate();
          case RateAveraging::Compound:
            return compoundedRate();
          default:
              QL_FAIL("unknown compounding convention (" << Integer(averagingMethod_) << ")");
        }
    }

    bool OvernightIndexFuture::isExpired() const {
        return detail::simple_event(maturityDate_).hasOccurred();
    }

    Real OvernightIndexFuture::convexityAdjustment() const {
        return convexityAdjustment_.empty() ? 0.0 : convexityAdjustment_->value();
    }

    void OvernightIndexFuture::performCalculations() const {
        Rate R = convexityAdjustment() + rate();
        NPV_ = 100.0 * (1.0 - R);
    }

}
