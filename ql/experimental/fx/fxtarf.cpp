/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/fx/fxtarf.hpp>
#include <ql/event.hpp>

namespace QuantLib {

FxTarf::FxTarf(const Schedule schedule, const boost::shared_ptr<FxIndex> &index,
               const Real sourceNominal,
               const boost::shared_ptr<StrikedTypePayoff> &shortPositionPayoff,
               const boost::shared_ptr<StrikedTypePayoff> &longPositionPayoff,
               const Real target, const CouponType couponType,
               const Real shortPositionGearing, const Real longPositionGearing,
               const Handle<Quote> accumulatedAmount,
               const Handle<Quote> lastAmount)
    : schedule_(schedule), index_(index), sourceNominal_(sourceNominal),
      shortPositionPayoff_(shortPositionPayoff),
      longPositionPayoff_(longPositionPayoff), target_(target),
      couponType_(couponType), shortPositionGearing_(shortPositionGearing),
      longPositionGearing_(longPositionGearing),
      accumulatedAmount_(accumulatedAmount), lastAmount_(lastAmount) {

    QL_REQUIRE(schedule.size() >= 2,
               "FXTarf requires at least 2 schedule dates (" << schedule.size()
                                                             << ")");

    registerWith(index);
    registerWith(accumulatedAmount);
    registerWith(lastAmount);
    registerWith(Settings::instance().evaluationDate());
}

bool FxTarf::isExpired() const {
    if (detail::simple_event(schedule_.dates().back()).hasOccurred())
        return true;
    if (!accumulatedAmount_.empty())
        return accumulatedAmount_->value() >= target_;
    else {
        std::pair<Real, bool> acc = accumulatedAmountAndSettlement();
        return acc.first >= target_ && acc.second;
    }
}

std::pair<Real, bool> FxTarf::accumulatedAmountAndSettlement() const {
    Real acc = accumulatedAmount_.empty() ? 0.0 : accumulatedAmount_->value();
    int i = 1;
    while (i < schedule_.dates().size() &&
           index_->fixingDate(schedule_.date(i)) <=
               Settings::instance().evaluationDate()) {
        if (accumulatedAmount_.empty()) {
            payout(index_->fixing(index_->fixingDate(schedule_.date(i))), acc);
        }
        ++i;
    }
    bool settled = detail::simple_event(schedule_.date(i - 1)).hasOccurred();
    return std::make_pair(acc, settled);
}

Disposable<std::vector<Date> > FxTarf::fixingDates() const {
    std::vector<Date> tmp;
    for (Size i = 1; i < schedule_.size(); ++i) {
        tmp.push_back(index_->fixingDate(schedule_.date(i)));
    }
    return tmp;
}

const boost::shared_ptr<FxIndex> FxTarf::index() const { return index_; }

Real FxTarf::lastAmount() const {
    if (!accumulatedAmount_.empty()) {
        if (lastAmount_.empty())
            return 0.0;
        else
            return lastAmount_->value();
    }
    int i = 1;
    while (index_->fixingDate(schedule_.date(i)) <=
           Settings::instance().evaluationDate()) {
        ++i;
    }
    return i > 1 ? payout(index_->fixing(
                       index_->fixingDate(schedule_.date(i - 1))))
                 : 0.0;
}

Real FxTarf::nakedPayout(const Real fixing, Real &accAmount) const {
    if (accAmount >= target_)
        return 0.0;
    Real nakedPayoff = longPositionGearing_ *
                       (*longPositionPayoff_)(fixing)-shortPositionGearing_ *(
                           *shortPositionPayoff_)(fixing);
    accAmount += std::max(nakedPayoff, 0.0);
    return nakedPayoff;
}

Real FxTarf::payout(const Real fixing, Real &accAmount) const {

    Real accBefore = accAmount;
    Real nakedPayoff = nakedPayout(fixing, accAmount);

    if (accAmount < target_) {
        return nakedPayoff;
    } else {
        switch (couponType_) {
        case none:
            return 0.0;
        case capped:
            return std::max(target_ - accBefore, 0.0);
        case full:
            return nakedPayoff;
        default:
            QL_FAIL("unknown coupon type (" << couponType_ << ")");
        }
    }
}

Real FxTarf::payout(const Real fixing) const {
    Real acc = accumulatedAmount();
    return payout(fixing, acc);
}

void FxTarf::setupExpired() const { Instrument::setupExpired(); }

void FxTarf::setupArguments(PricingEngine::arguments *args) const {
    FxTarf::arguments *arguments = dynamic_cast<FxTarf::arguments *>(args);
    QL_REQUIRE(arguments != 0, "wrong argument type");
    arguments->schedule = schedule_;
    openFixingDates_.clear();
    openPaymentDates_.clear();
    for (Size i = 1; i < schedule_.size(); ++i) {
        Date fixingTmp = index_->fixingDate(schedule_.date(i));
        if (fixingTmp > Settings::instance().evaluationDate()) {
            openFixingDates_.push_back(fixingTmp);
            openPaymentDates_.push_back(schedule_.date(i));
        }
    }
    arguments->openFixingDates = openFixingDates_;
    arguments->openPaymentDates = openPaymentDates_;
    arguments->index = index_;
    arguments->target = target_;
    arguments->sourceNominal = sourceNominal_;
    arguments->longPositionType = longPositionPayoff_->optionType();
    arguments->lastAmount = lastAmount();
    std::pair<Real, bool> accSettlTmp = accumulatedAmountAndSettlement();
    arguments->accumulatedAmount = accSettlTmp.first;
    arguments->isLastAmountSettled = accSettlTmp.second;
    arguments->instrument = this;
}

void FxTarf::fetchResults(const PricingEngine::results *r) const {
    Instrument::fetchResults(r);
    const FxTarf::results *results = dynamic_cast<const FxTarf::results *>(r);
    QL_REQUIRE(results != 0, "wrong results type");
    proxy_ = results->proxy;
}

Date FxTarf::startDate() const { return schedule_.dates().front(); }

Date FxTarf::maturityDate() const { return schedule_.dates().back(); }

boost::shared_ptr<ProxyInstrument::ProxyDescription> FxTarf::proxy() const {
    calculate();
    QL_REQUIRE(proxy_ != NULL, "no proxy available");
    proxy_->validate();
    return proxy_;
}

void FxTarf::arguments::validate() const {}

void FxTarf::results::reset() {
    Instrument::results::reset();
    proxy = boost::shared_ptr<FxTarf::Proxy>();
}

} // namespace QuantLib
