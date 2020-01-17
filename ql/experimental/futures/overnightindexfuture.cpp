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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/futures/overnightindexfuture.hpp>
#include <ql/indexes/indexmanager.hpp>

namespace QuantLib {

    OvernightIndexFuture::OvernightIndexFuture(
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const ext::shared_ptr<Payoff>& payoff,
        const Date& valueDate,
        const Date& maturityDate,
        const Handle<YieldTermStructure>& discountCurve,
        const Handle<Quote>& convexityAdjustment,
        const NettingType subPeriodsNettingType)
    : Forward(overnightIndex->dayCounter(),
              overnightIndex->fixingCalendar(),
              overnightIndex->businessDayConvention(),
              0,
              payoff,
              valueDate,
              maturityDate,
              discountCurve),
      overnightIndex_(overnightIndex), convexityAdjustment_(convexityAdjustment),
      subPeriodsNettingType_(subPeriodsNettingType) {}

    Real OvernightIndexFuture::averagedSpotValue() const {
        Date today = Settings::instance().evaluationDate();
        Real avg = 0;
        Date d1 = valueDate_;
        const TimeSeries<Real>& history = IndexManager::instance()
            .getHistory(overnightIndex_->name());
        Real fwd;
        while (d1 < maturityDate_) {
            Date d2 = calendar_.advance(d1, 1, Days);
            if (d1 < today) {
                fwd = history[d1];
                QL_REQUIRE(fwd != Null<Real>(), "missing rate on " <<
                    d1 << " for index " << overnightIndex_->name());
            } else {
                fwd = discountCurve_->forwardRate(d1, d2, dayCounter_, Simple).rate();
            }
            avg += fwd * dayCounter_.yearFraction(d1, d2);
            d1 = d2;
        }

        Real convAdj = convexityAdjustment_.empty() ? 0.0 :
            convexityAdjustment_->value();
        Real R = convAdj + avg /
            dayCounter_.yearFraction(valueDate_, maturityDate_);
        return 100.0 * (1.0 - R);
    }

    Real OvernightIndexFuture::compoundedSpotValue() const {
        Date today = Settings::instance().evaluationDate();
        Real prod = 1;
        if (today > valueDate_) {
            // can't value on a weekend inside reference period because we
            // won't know the reset rate until start of next business day.
            // user can supply an estimate if they really want to do this
            today = calendar_.adjust(today, businessDayConvention_);
            // for valuations inside the reference period, index quotes
            // must have been populated in the history
            const TimeSeries<Real>& history = IndexManager::instance()
                .getHistory(overnightIndex_->name());
            Date d1 = valueDate_;
            while (d1 < today) {
                Real r = history[d1];
                QL_REQUIRE(r != Null<Real>(), "missing rate on " <<
                    d1 << " for index " << overnightIndex_->name());
                Date d2 = calendar_.advance(d1, 1, Days);
                prod *= 1 + r * dayCounter_.yearFraction(d1, d2);
                d1 = d2;
            }
        }
        DiscountFactor forwardDiscount = discountCurve_->discount(maturityDate_);
        if (valueDate_ > today) {
            forwardDiscount /= discountCurve_->discount(valueDate_);
        }
        prod /= forwardDiscount;

        Real convAdj = convexityAdjustment_.empty() ? 0.0 :
            convexityAdjustment_->value();
        Real R = convAdj + (prod - 1) /
            dayCounter_.yearFraction(valueDate_, maturityDate_);
        return 100.0 * (1.0 - R);
    }

    Real OvernightIndexFuture::spotValue() const {
        switch (subPeriodsNettingType_) {
          case Averaging:
            underlyingSpotValue_ = averagedSpotValue();
            break;
          case Compounding:
            underlyingSpotValue_ = compoundedSpotValue();
            break;
          default:
            QL_FAIL("unknown compounding convention ("
                    << Integer(subPeriodsNettingType_) << ")");
        }
        return underlyingSpotValue_;
    }

    Real OvernightIndexFuture::spotIncome(const Handle<YieldTermStructure>&) const {
        underlyingIncome_ = 0;
        return underlyingIncome_;
    }

    Real OvernightIndexFuture::forwardValue() const {
        calculate();
        return underlyingSpotValue_;
    }

    Real OvernightIndexFuture::convexityAdjustment() const {
        return convexityAdjustment_.empty() ? 0.0 :
            convexityAdjustment_->value();
    }

}
