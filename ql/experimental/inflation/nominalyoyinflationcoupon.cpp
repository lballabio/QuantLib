/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/experimental/inflation/nominalyoyinflationcoupon.hpp>
#include <ql/experimental/inflation/inflationcappedcouponpricer.hpp>

namespace QuantLib {

    NominalYoYInflationCoupon::NominalYoYInflationCoupon(
                         const Date &paymentDate,
                         const Real nominal,
                         const Date &startDate,
                         const Date &endDate,
                         const Natural nominalFixingDays,
                         const BusinessDayConvention nominalBdc,
                         const boost::shared_ptr<IborIndex> &nominalIndex,
                         const bool yoyIsRatio,
                         const Period &yoyLag,
                         const boost::shared_ptr<YoYInflationIndex> &yoyIndex,
                         const DayCounter &accrualDayCounter)
    : Coupon(nominal, paymentDate,
             startDate, endDate, startDate, endDate),
      nominalFixingDays_(nominalFixingDays), nominalIndex_(nominalIndex),
      yoyIsRatio_(yoyIsRatio),  yoyLag_(yoyLag), yoyIndex_(yoyIndex),
      accrualDayCounter_(accrualDayCounter) {

        if (accrualDayCounter_.empty())
            accrualDayCounter_ = nominalIndex_->dayCounter();

        QL_REQUIRE(yoyIsRatio_ == yoyIndex_->ratio(),
                   "Index does not have the same ratio setting as instrument: "
                   << "inst "<< yoyIsRatio_ << " vs " << yoyIndex_->ratio());

        QL_REQUIRE(yoyLag_ <= yoyIndex_->availabilityLag(),
                   "Cannot use a lag in the instrument that is less than the "
                   << " availability lag of the index: inst " << yoyLag_
                   << " vs " << yoyIndex_->availabilityLag());

        QL_REQUIRE(!yoyIndex_->interpolated(),
                   "yoy inflation index should not be interpolated.");

        // Not used yet because this is only a data holder.
        // However, descendents may have pricers.
        registerWith(nominalIndex_);
        registerWith(yoyIndex_);
        registerWith(Settings::instance().evaluationDate());
    }

    // see comments about rate()
    Real NominalYoYInflationCoupon::amount() const {
        return rate() * accrualPeriod() * nominal();
    }

    //! Since we do not have a payoff function this always
    //! returns 1.0 so that it can be used to build up
    //! accrual calculations.
    Real NominalYoYInflationCoupon::rate() const {
        return 1.0;
    }

    Real NominalYoYInflationCoupon::price(
                   const Handle<YieldTermStructure>& discountingCurve) const {
        return amount() * discountingCurve->discount(date());
    }

    DayCounter NominalYoYInflationCoupon::dayCounter() const {
        return accrualDayCounter_;
    }

    Real NominalYoYInflationCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal() * rate() *
                dayCounter().yearFraction(accrualStartDate_,
                                          std::min(d, accrualEndDate_));
        }
    }

    Date NominalYoYInflationCoupon::fixingDate() const {
        return std::max(nominalFixingDate(), yoyFixingDate());
    }

    // move is in business days
    Date NominalYoYInflationCoupon::nominalFixingDate() const {
        return nominalIndex_->fixingCalendar().advance(
                                   accrualStartDate_,
                                   - static_cast<Integer>(nominalFixingDays_),
                                   Days, nominalBdc_);
    }

    Natural NominalYoYInflationCoupon::nominalFixingDays() const {
        return nominalFixingDays_;
    }

    const boost::shared_ptr<IborIndex>
    NominalYoYInflationCoupon::nominalIndex() const {
        return nominalIndex_;
    }

    // uses inflation that is constant within a month
    Date NominalYoYInflationCoupon::yoyFixingDate() const {
        return accrualEndDate_ - yoyLag_;
    }

    Period NominalYoYInflationCoupon::yoyLag() const {
        return yoyLag_;
    }

    const boost::shared_ptr<YoYInflationIndex>
    NominalYoYInflationCoupon::yoyIndex() const {
        return yoyIndex_;
    }

    void NominalYoYInflationCoupon::update() {
        notifyObservers();
    }




    InflationCappedCoupon::InflationCappedCoupon(
                         const Real nominalSpread,
                         const Real capGearing,
                         const Real inflationSpread,
                         const Date &paymentDate,
                         const Real nominal,
                         const Date &startDate,
                         const Date &endDate,
                         const Natural nominalFixingDays,
                         const BusinessDayConvention nominalBdc,
                         const boost::shared_ptr<IborIndex> &nominalIndex,
                         const bool yoyIsRatio,
                         const Period &yoyLag,
                         const boost::shared_ptr<YoYInflationIndex> &yoyIndex,
                         const DayCounter &accrualDayCounter)
    : NominalYoYInflationCoupon(paymentDate, nominal, startDate, endDate,
                                nominalFixingDays, nominalBdc, nominalIndex,
                                yoyIsRatio, yoyLag, yoyIndex,
                                accrualDayCounter),
      nominalSpread_(nominalSpread), capGearing_(capGearing),
      inflationSpread_(inflationSpread) {
        // sanity checks
        QL_REQUIRE(capGearing > 0.0, "capGearing is not positive: "
                   << capGearing);
    }


    Real InflationCappedCoupon::amount() const {
        return rate() * accrualPeriod() * nominal();
    }


    Rate InflationCappedCoupon::rate() const {
        QL_REQUIRE(pricer_, "pricer not set");
        pricer_->initialize(*this);     // at this point the pricer
                                        // can see that it has the
                                        // right sort of inflation
                                        // coupon
        return pricer_->rate();
    }


    void InflationCappedCoupon::setPricer(
               const boost::shared_ptr<InflationCappedCouponPricer>& pricer) {

        if (pricer_)
            unregisterWith(pricer_);
        pricer_ = pricer;
        QL_REQUIRE(pricer_,
                   "no adequate pricer given in InflationCappedCouponPricer");
        registerWith(pricer_);
        update();
    }

}

