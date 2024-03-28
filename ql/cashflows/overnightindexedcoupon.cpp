/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2014 Peter Caspers
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

#include <ql/cashflows/couponpricer.hpp>
#include <ql/experimental/averageois/averageoiscouponpricer.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/vectors.hpp>
#include <utility>
#include <algorithm>

using std::vector;

namespace QuantLib {

    namespace {

        class OvernightIndexedCouponPricer : public FloatingRateCouponPricer {
          public:
            void initialize(const FloatingRateCoupon& coupon) override {
                coupon_ = dynamic_cast<const OvernightIndexedCoupon*>(&coupon);
                QL_ENSURE(coupon_, "wrong coupon type");
            }

            Rate averageRate(const Date& date) const {

                const Date today = Settings::instance().evaluationDate();

                const ext::shared_ptr<OvernightIndex> index =
                    ext::dynamic_pointer_cast<OvernightIndex>(coupon_->index());
                const auto& pastFixings = IndexManager::instance().getHistory(index->name());

                const vector<Date>& fixingDates = coupon_->fixingDates();
                const vector<Date>& valueDates = coupon_->valueDates();
                const vector<Time>& dt = coupon_->dt();

                Size i = 0;
                const size_t n = std::lower_bound(valueDates.begin(), valueDates.end(), date) - valueDates.begin();
                Real compoundFactor = 1.0;

                // already fixed part
                while (i < n && fixingDates[i] < today) {
                    // rate must have been fixed
                    const Rate fixing = pastFixings[fixingDates[i]];
                    QL_REQUIRE(fixing != Null<Real>(),
                               "Missing " << index->name() <<
                               " fixing for " << fixingDates[i]);
                    Time span = (date >= valueDates[i+1] ?
                                 dt[i] :
                                 index->dayCounter().yearFraction(valueDates[i], date));
                    compoundFactor *= (1.0 + fixing * span);
                    ++i;
                }

                // today is a border case
                if (i < n && fixingDates[i] == today) {
                    // might have been fixed
                    try {
                        Rate fixing = pastFixings[fixingDates[i]];
                        if (fixing != Null<Real>()) {
                            Time span = (date >= valueDates[i+1] ?
                                         dt[i] :
                                         index->dayCounter().yearFraction(valueDates[i], date));
                            compoundFactor *= (1.0 + fixing * span);
                            ++i;
                        } else {
                            ;   // fall through and forecast
                        }
                    } catch (Error&) {
                        ;       // fall through and forecast
                    }
                }

                // forward part using telescopic property in order
                // to avoid the evaluation of multiple forward fixings
                if (i<n) {
                    const Handle<YieldTermStructure> curve = index->forwardingTermStructure();
                    QL_REQUIRE(!curve.empty(),
                               "null term structure set to this instance of " << index->name());

                    const DiscountFactor startDiscount = curve->discount(valueDates[i]);
                    if (valueDates[n] == date) {
                        // full telescopic formula
                        const DiscountFactor endDiscount = curve->discount(valueDates[n]);
                        compoundFactor *= startDiscount / endDiscount;
                    } else {
                        // The last fixing is not used for its full period (the date is between its
                        // start and end date).  We can use the telescopic formula until the previous
                        // date, then we'll add the missing bit.
                        const DiscountFactor endDiscount = curve->discount(valueDates[n-1]);
                        compoundFactor *= startDiscount / endDiscount;

                        Rate fixing = index->fixing(fixingDates[n-1]);
                        Time span = index->dayCounter().yearFraction(valueDates[n-1], date);
                        compoundFactor *= (1.0 + fixing * span);
                    }
                }

                const Rate rate = (compoundFactor - 1.0) / coupon_->accruedPeriod(date);
                return coupon_->gearing() * rate + coupon_->spread();
            }

            Rate swapletRate() const override {
                return averageRate(coupon_->accrualEndDate());
            }

            Real swapletPrice() const override { QL_FAIL("swapletPrice not available"); }
            Real capletPrice(Rate) const override { QL_FAIL("capletPrice not available"); }
            Rate capletRate(Rate) const override { QL_FAIL("capletRate not available"); }
            Real floorletPrice(Rate) const override { QL_FAIL("floorletPrice not available"); }
            Rate floorletRate(Rate) const override { QL_FAIL("floorletRate not available"); }

          protected:
            const OvernightIndexedCoupon* coupon_;
        };
    }

    OvernightIndexedCoupon::OvernightIndexedCoupon(
                    const Date& paymentDate,
                    Real nominal,
                    const Date& startDate,
                    const Date& endDate,
                    const ext::shared_ptr<OvernightIndex>& overnightIndex,
                    Real gearing,
                    Spread spread,
                    const Date& refPeriodStart,
                    const Date& refPeriodEnd,
                    const DayCounter& dayCounter,
                    bool telescopicValueDates, 
                    RateAveraging::Type averagingMethod)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         overnightIndex ? overnightIndex->fixingDays() : 0,
                         overnightIndex,
                         gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, false), averagingMethod_(averagingMethod) {

        // value dates
        Date tmpEndDate = endDate;

        /* For the coupon's valuation only the first and last future valuation
           dates matter, therefore we can avoid to construct the whole series
           of valuation dates, a front and back stub will do. However notice
           that if the global evaluation date moves forward it might run past
           the front stub of valuation dates we build here (which incorporates
           a grace period of 7 business after the evaluation date). This will
           lead to false coupon projections (see the warning the class header). */

        if (telescopicValueDates) {
            // build optimised value dates schedule: front stub goes
            // from start date to max(evalDate,startDate) + 7bd
            Date evalDate = Settings::instance().evaluationDate();
            tmpEndDate = overnightIndex->fixingCalendar().advance(
                std::max(startDate, evalDate), 7, Days, Following);
            tmpEndDate = std::min(tmpEndDate, endDate);
        }
        Schedule sch =
            MakeSchedule()
                .from(startDate)
                // .to(endDate)
                .to(tmpEndDate)
                .withTenor(1 * Days)
                .withCalendar(overnightIndex->fixingCalendar())
                .withConvention(overnightIndex->businessDayConvention())
                .backwards();
        valueDates_ = sch.dates();

        if (telescopicValueDates) {
            // build optimised value dates schedule: back stub
            // contains at least two dates
            Date tmp = overnightIndex->fixingCalendar().advance(
                endDate, -1, Days, Preceding);
            if (tmp != valueDates_.back())
                valueDates_.push_back(tmp);
            tmp = overnightIndex->fixingCalendar().adjust(
                endDate, overnightIndex->businessDayConvention());
            if (tmp != valueDates_.back())
                valueDates_.push_back(tmp);
        }

        QL_ENSURE(valueDates_.size()>=2, "degenerate schedule");

        // fixing dates
        n_ = valueDates_.size()-1;
        if (overnightIndex->fixingDays()==0) {
            fixingDates_ = vector<Date>(valueDates_.begin(),
                                        valueDates_.end() - 1);
        } else {
            fixingDates_.resize(n_);
            for (Size i=0; i<n_; ++i)
                fixingDates_[i] = overnightIndex->fixingDate(valueDates_[i]);
        }

        // accrual (compounding) periods
        dt_.resize(n_);
        const DayCounter& dc = overnightIndex->dayCounter();
        for (Size i=0; i<n_; ++i)
            dt_[i] = dc.yearFraction(valueDates_[i], valueDates_[i+1]);

        switch (averagingMethod) {
            case RateAveraging::Simple:
                setPricer(ext::shared_ptr<FloatingRateCouponPricer>(
                    new ArithmeticAveragedOvernightIndexedCouponPricer(telescopicValueDates)));
                break;
            case RateAveraging::Compound:
                setPricer(
                    ext::shared_ptr<FloatingRateCouponPricer>(new OvernightIndexedCouponPricer));
                break;
            default:
                QL_FAIL("unknown compounding convention (" << Integer(averagingMethod) << ")");
        }
    }

    Real OvernightIndexedCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            // out of coupon range
            return 0.0;
        } else if (tradingExCoupon(d)) {
            return nominal() * averageRate(d) * accruedPeriod(d);
        } else {
            // usual case
            return nominal() * averageRate(std::min(d, accrualEndDate_)) * accruedPeriod(d);
        }
    }

    Rate OvernightIndexedCoupon::averageRate(const Date& d) const {
        QL_REQUIRE(pricer_, "pricer not set");
        pricer_->initialize(*this);
        const auto overnightIndexPricer = ext::dynamic_pointer_cast<OvernightIndexedCouponPricer>(pricer_);
        if (overnightIndexPricer)
            return overnightIndexPricer->averageRate(d);

        return pricer_->swapletRate();
    }

    const vector<Rate>& OvernightIndexedCoupon::indexFixings() const {
        fixings_.resize(n_);
        for (Size i=0; i<n_; ++i)
            fixings_[i] = index_->fixing(fixingDates_[i]);
        return fixings_;
    }

    void OvernightIndexedCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OvernightIndexedCoupon>*>(&v);
        if (v1 != nullptr) {
            v1->visit(*this);
        } else {
            FloatingRateCoupon::accept(v);
        }
    }

    OvernightLeg::OvernightLeg(const Schedule& schedule, ext::shared_ptr<OvernightIndex> i)
    : schedule_(schedule), overnightIndex_(std::move(i)), paymentCalendar_(schedule.calendar()) {
        QL_REQUIRE(overnightIndex_, "no index provided");
    }

    OvernightLeg& OvernightLeg::withNotionals(Real notional) {
        notionals_ = vector<Real>(1, notional);
        return *this;
    }

    OvernightLeg& OvernightLeg::withNotionals(const vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    OvernightLeg& OvernightLeg::withPaymentDayCounter(const DayCounter& dc) {
        paymentDayCounter_ = dc;
        return *this;
    }

    OvernightLeg&
    OvernightLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    OvernightLeg& OvernightLeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    OvernightLeg& OvernightLeg::withPaymentLag(Integer lag) {
        paymentLag_ = lag;
        return *this;
    }

    OvernightLeg& OvernightLeg::withGearings(Real gearing) {
        gearings_ = vector<Real>(1,gearing);
        return *this;
    }

    OvernightLeg& OvernightLeg::withGearings(const vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    OvernightLeg& OvernightLeg::withSpreads(Spread spread) {
        spreads_ = vector<Spread>(1,spread);
        return *this;
    }

    OvernightLeg& OvernightLeg::withSpreads(const vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    OvernightLeg& OvernightLeg::withTelescopicValueDates(bool telescopicValueDates) {
        telescopicValueDates_ = telescopicValueDates;
        return *this;
    }

    OvernightLeg& OvernightLeg::withAveragingMethod(RateAveraging::Type averagingMethod) {
        averagingMethod_ = averagingMethod;
        return *this;
    }

    OvernightLeg::operator Leg() const {

        QL_REQUIRE(!notionals_.empty(), "no notional given");

        Leg cashflows;

        // the following is not always correct
        Calendar calendar = schedule_.calendar();

        Date refStart, start, refEnd, end;
        Date paymentDate;

        Size n = schedule_.size()-1;
        for (Size i=0; i<n; ++i) {
            refStart = start = schedule_.date(i);
            refEnd   =   end = schedule_.date(i+1);
            paymentDate = paymentCalendar_.advance(end, paymentLag_, Days, paymentAdjustment_);

            if (i == 0 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1))
                refStart = calendar.adjust(end - schedule_.tenor(),
                                           paymentAdjustment_);
            if (i == n-1 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1))
                refEnd = calendar.adjust(start + schedule_.tenor(),
                                         paymentAdjustment_);

            cashflows.push_back(ext::shared_ptr<CashFlow>(new
                OvernightIndexedCoupon(paymentDate,
                                       detail::get(notionals_, i,
                                                   notionals_.back()),
                                       start, end,
                                       overnightIndex_,
                                       detail::get(gearings_, i, 1.0),
                                       detail::get(spreads_, i, 0.0),
                                       refStart, refEnd,
                                       paymentDayCounter_,
                                       telescopicValueDates_, 
                                       averagingMethod_)));
        }
        return cashflows;
    }

}
