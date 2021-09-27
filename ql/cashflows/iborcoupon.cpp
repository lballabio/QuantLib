/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2010, 2011 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    IborCoupon::IborCoupon(const Date& paymentDate,
                           Real nominal,
                           const Date& startDate,
                           const Date& endDate,
                           Natural fixingDays,
                           const ext::shared_ptr<IborIndex>& iborIndex,
                           Real gearing,
                           Spread spread,
                           const Date& refPeriodStart,
                           const Date& refPeriodEnd,
                           const DayCounter& dayCounter,
                           bool isInArrears,
                           const Date& exCouponDate)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, iborIndex, gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, isInArrears, exCouponDate),
      iborIndex_(iborIndex) {
        fixingDate_ = fixingDate();
    }

    void IborCoupon::initializeCachedData() const {
        auto p = ext::dynamic_pointer_cast<IborCouponPricer>(pricer_);
        QL_REQUIRE(p, "IborCoupon: pricer not set or not derived from IborCouponPricer");
        p->initializeCachedData(*this);
    }

    const Date& IborCoupon::fixingEndDate() const {
        initializeCachedData();
        return fixingEndDate_;
    }

    Rate IborCoupon::indexFixing() const {

        initializeCachedData();

        /* instead of just returning index_->fixing(fixingValueDate_)
           its logic is duplicated here using a specialized iborIndex
           forecastFixing overload which
           1) allows to save date/time recalculations, and
           2) takes into account par coupon needs
        */
        Date today = Settings::instance().evaluationDate();

        if (fixingDate_>today)
            return iborIndex_->forecastFixing(fixingValueDate_,
                                              fixingEndDate_,
                                              spanningTime_);

        if (fixingDate_<today ||
            Settings::instance().enforcesTodaysHistoricFixings()) {
            // do not catch exceptions
            Rate result = index_->pastFixing(fixingDate_);
            QL_REQUIRE(result != Null<Real>(),
                       "Missing " << index_->name() << " fixing for " << fixingDate_);
            return result;
        }

        try {
            Rate result = index_->pastFixing(fixingDate_);
            if (result!=Null<Real>())
                return result;
            else
                ;   // fall through and forecast
        } catch (Error&) {
                ;   // fall through and forecast
        }
        return iborIndex_->forecastFixing(fixingValueDate_,
                                          fixingEndDate_,
                                          spanningTime_);
    }

    void IborCoupon::setPricer(const ext::shared_ptr<FloatingRateCouponPricer>& pricer) {
        cachedDataIsInitialized_ = false;
        FloatingRateCoupon::setPricer(pricer);
    }

    void IborCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<IborCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }


    IborLeg::IborLeg(Schedule schedule, ext::shared_ptr<IborIndex> index)
    : schedule_(std::move(schedule)), index_(std::move(index)), paymentAdjustment_(Following),
      paymentLag_(0), paymentCalendar_(Calendar()), inArrears_(false), zeroPayments_(false),
      exCouponPeriod_(Period()), exCouponCalendar_(Calendar()), exCouponAdjustment_(Unadjusted),
      exCouponEndOfMonth_(false) {}

    IborLeg& IborLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    IborLeg& IborLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    IborLeg& IborLeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    IborLeg& IborLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    IborLeg& IborLeg::withPaymentLag(Natural lag) {
        paymentLag_ = lag;
        return *this;
    }

    IborLeg& IborLeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    IborLeg& IborLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    IborLeg& IborLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    IborLeg& IborLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    IborLeg& IborLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    IborLeg& IborLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    IborLeg& IborLeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    IborLeg& IborLeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1,cap);
        return *this;
    }

    IborLeg& IborLeg::withCaps(const std::vector<Rate>& caps) {
        caps_ = caps;
        return *this;
    }

    IborLeg& IborLeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1,floor);
        return *this;
    }

    IborLeg& IborLeg::withFloors(const std::vector<Rate>& floors) {
        floors_ = floors;
        return *this;
    }

    IborLeg& IborLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    IborLeg& IborLeg::withZeroPayments(bool flag) {
        zeroPayments_ = flag;
        return *this;
    }

	IborLeg& IborLeg::withExCouponPeriod(const Period& period,
                                         const Calendar& cal,
                                         BusinessDayConvention convention,
                                         bool endOfMonth) {
        exCouponPeriod_ = period;
        exCouponCalendar_ = cal;
        exCouponAdjustment_ = convention;
        exCouponEndOfMonth_ = endOfMonth;
        return *this;
	}

    IborLeg& IborLeg::useIndexedCoupon(boost::optional<bool> useIndexedCoupon) {
        useIndexedCoupon_ = useIndexedCoupon;
        return *this;
    }

    IborLeg::operator Leg() const {

        Leg leg = FloatingLeg<IborIndex, IborCoupon, CappedFlooredIborCoupon>(
                         schedule_, notionals_, index_, paymentDayCounter_,
                         paymentAdjustment_, fixingDays_, gearings_, spreads_,
                         caps_, floors_, inArrears_, zeroPayments_, paymentLag_, paymentCalendar_, 
			             exCouponPeriod_, exCouponCalendar_, exCouponAdjustment_, exCouponEndOfMonth_);

        if (caps_.empty() && floors_.empty() && !inArrears_) {
            ext::shared_ptr<IborCouponPricer> pricer = ext::make_shared<BlackIborCouponPricer>(
                Handle<OptionletVolatilityStructure>(),
                BlackIborCouponPricer::TimingAdjustment::Black76,
                Handle<Quote>(ext::make_shared<SimpleQuote>(1.0)), useIndexedCoupon_);
            setCouponPricer(leg, pricer);
        }

        return leg;
    }

}
