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

using std::vector;

namespace QuantLib {

    namespace {

        class OvernightIndexedCouponPricer : public FloatingRateCouponPricer {
          public:
            void initialize(const FloatingRateCoupon& coupon) override {
                coupon_ = dynamic_cast<const OvernightIndexedCoupon*>(&coupon);
                QL_ENSURE(coupon_, "wrong coupon type");
            }
            Rate swapletRate(const Date& date) const override {

                const Date today = Settings::instance().evaluationDate();

                const ext::shared_ptr<OvernightIndex> index =
                    ext::dynamic_pointer_cast<OvernightIndex>(coupon_->index());

                const vector<Date>& fixingDates = coupon_->fixingDates();
                const vector<Date>& valueDates = coupon_->valueDates();
                const vector<Time>& dt = coupon_->dt();

                const Size n = dt.size();
                Size i = 0;

                Real compoundFactor = 1.0;

                const auto minDate = std::min(today, date);

                // already fixed part
                while (i < n && fixingDates[i] < minDate) {
                    // rate must have been fixed
                    const Rate pastFixing = IndexManager::instance().getHistory(
                                                index->name())[fixingDates[i]];
                    QL_REQUIRE(pastFixing != Null<Real>(),
                               "Missing " << index->name() <<
                               " fixing for " << fixingDates[i]);
                    compoundFactor *= (1.0 + pastFixing*dt[i]);
                    ++i;
                }

                // today is a border case
                if (i < n && fixingDates[i] == minDate) {
                    // might have been fixed
                    try {
                        Rate pastFixing = IndexManager::instance().getHistory(
                                                index->name())[fixingDates[i]];
                        if (pastFixing != Null<Real>()) {
                            compoundFactor *= (1.0 + pastFixing*dt[i]);
                            ++i;
                        } else {
                            ;   // fall through and forecast
                        }
                    } catch (Error&) {
                        ;       // fall through and forecast
                    }
                }

                auto endIndex = i;

                if (minDate < valueDates[n]) {
                    while (endIndex < n && minDate <= valueDates[endIndex])
                        endIndex++;
                } else {
                    endIndex = n;
                }

                // forward part using telescopic property in order
                // to avoid the evaluation of multiple forward fixings
                if (i < endIndex) {
                    const Handle<YieldTermStructure> curve =
                        index->forwardingTermStructure();
                    QL_REQUIRE(!curve.empty(),
                               "null term structure set to this instance of "<<
                               index->name());

                    const DiscountFactor startDiscount = curve->discount(valueDates[i]);
                    const DiscountFactor endDiscount = curve->discount(valueDates[endIndex]);

                    compoundFactor *= startDiscount/endDiscount;
                }

                const Rate rate = (compoundFactor - 1.0) / coupon_->accrualPeriod();
                return coupon_->gearing() * rate + coupon_->spread();
            }
            Rate swapletRate() const override {

                const Date accrualEndDate = coupon_->valueDates()[coupon_->dt().size()];
                return swapletRate(accrualEndDate);
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
                         overnightIndex->fixingDays(), overnightIndex,
                         gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, false) {

        // value dates
        Date tmpEndDate = endDate;

        /* For the coupon's valuation only the first and last future valuation
           dates matter, therefore we can avoid to construct the whole series
           of valuation dates, a front and back stub will do. However notice
           that if the global evaluation date moves forward it might run past
           the front stub of valuation dates we build here (which incorporates
           a grace period of 7 business after the evluation date). This will
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
                                             valueDates_.end()-1);
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
            return -nominal() * rate(d) *
                   dayCounter().yearFraction(d, std::max(d, accrualEndDate_), refPeriodStart_,
                                             refPeriodEnd_);
        } else {
            // usual case
            return nominal() * rate(d) *
                   dayCounter().yearFraction(accrualStartDate_, std::min(d, accrualEndDate_),
                                             refPeriodStart_, refPeriodEnd_);
        }
    }

    Rate OvernightIndexedCoupon::rate(const Date& d) const {
        prepareRate();
        return pricer_->swapletRate(d);
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
    : schedule_(schedule), overnightIndex_(std::move(i)), paymentCalendar_(schedule.calendar()),
      paymentAdjustment_(Following), paymentLag_(0), telescopicValueDates_(false),
      averagingMethod_(RateAveraging::Compound) {}

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

    OvernightLeg& OvernightLeg::withPaymentLag(Natural lag) {
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
