/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
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

#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/vectors.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

using std::vector;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace QuantLib {

    namespace {

        class OvernightIndexedCouponPricer : public FloatingRateCouponPricer {
          public:
            void initialize(const FloatingRateCoupon& coupon) {
                coupon_ = dynamic_cast<const OvernightIndexedCoupon*>(&coupon);
                QL_ENSURE(coupon_, "wrong coupon type");
            }
            Rate swapletRate() const {

                shared_ptr<OvernightIndex> index =
                    dynamic_pointer_cast<OvernightIndex>(coupon_->index());

                const vector<Date>& fixingDates = coupon_->fixingDates();
                const vector<Time>& dt = coupon_->dt();

                Size n = dt.size(),
                     i = 0;

                Real compoundFactor = 1.0;

                // already fixed part
                Date today = Settings::instance().evaluationDate();
                while (i<n && fixingDates[i]<today) {
                    // rate must have been fixed
                    Rate pastFixing = IndexManager::instance().getHistory(
                                                index->name())[fixingDates[i]];
                    QL_REQUIRE(pastFixing != Null<Real>(),
                               "Missing " << index->name() <<
                               " fixing for " << fixingDates[i]);
                    compoundFactor *= (1.0 + pastFixing*dt[i]);
                    ++i;
                }

                // today is a border case
                if (i<n && fixingDates[i] == today) {
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

                // forward part using telescopic property in order
                // to avoid the evaluation of multiple forward fixings
                if (i<n) {
                    Handle<YieldTermStructure> curve =
                        index->forwardingTermStructure();
                    QL_REQUIRE(!curve.empty(),
                               "null term structure set to this instance of "<<
                               index->name());

                    const vector<Date>& dates = coupon_->valueDates();
                    DiscountFactor startDiscount = curve->discount(dates[i]);
                    DiscountFactor endDiscount = curve->discount(dates[n]);

                    compoundFactor *= startDiscount/endDiscount;
                }

                Rate rate = (compoundFactor - 1.0) / coupon_->accrualPeriod();
                return coupon_->gearing() * rate + coupon_->spread();
            }

            Real swapletPrice() const { QL_FAIL("swapletPrice not available");  }
            Real capletPrice(Rate) const { QL_FAIL("capletPrice not available"); }
            Rate capletRate(Rate) const { QL_FAIL("capletRate not available"); }
            Real floorletPrice(Rate) const { QL_FAIL("floorletPrice not available"); }
            Rate floorletRate(Rate) const { QL_FAIL("floorletRate not available"); }
          protected:
            const OvernightIndexedCoupon* coupon_;
        };
    }

    OvernightIndexedCoupon::OvernightIndexedCoupon(
                    const Date& paymentDate,
                    Real nominal,
                    const Date& startDate,
                    const Date& endDate,
                    const shared_ptr<OvernightIndex>& overnightIndex,
                    Real gearing,
                    Spread spread,
                    const Date& refPeriodStart,
                    const Date& refPeriodEnd,
                    const DayCounter& dayCounter)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         overnightIndex->fixingDays(), overnightIndex,
                         gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, false) {

        // value dates
        Schedule sch = MakeSchedule()
                    .from(startDate)
                    .to(endDate)
                    .withTenor(1*Days)
                    .withCalendar(overnightIndex->fixingCalendar())
                    .withConvention(overnightIndex->businessDayConvention())
                    .backwards();
        valueDates_ = sch.dates();
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

        setPricer(shared_ptr<FloatingRateCouponPricer>(new
                                            OvernightIndexedCouponPricer));
    }

    const vector<Rate>& OvernightIndexedCoupon::indexFixings() const {
        fixings_.resize(n_);
        for (Size i=0; i<n_; ++i)
            fixings_[i] = index_->fixing(fixingDates_[i]);
        return fixings_;
    }

    void OvernightIndexedCoupon::accept(AcyclicVisitor& v) {
        Visitor<OvernightIndexedCoupon>* v1 =
            dynamic_cast<Visitor<OvernightIndexedCoupon>*>(&v);
        if (v1 != 0) {
            v1->visit(*this);
        } else {
            FloatingRateCoupon::accept(v);
        }
    }

    OvernightLeg::OvernightLeg(const Schedule& schedule,
                               const shared_ptr<OvernightIndex>& i)
    : schedule_(schedule), overnightIndex_(i), paymentAdjustment_(Following) {}

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
            paymentDate = calendar.adjust(end, paymentAdjustment_);
            if (i == 0 && !schedule_.isRegular(i+1))
                refStart = calendar.adjust(end - schedule_.tenor(),
                                           paymentAdjustment_);
            if (i == n-1 && !schedule_.isRegular(i+1))
                refEnd = calendar.adjust(start + schedule_.tenor(),
                                         paymentAdjustment_);

            cashflows.push_back(shared_ptr<CashFlow>(new
                OvernightIndexedCoupon(paymentDate,
                                       detail::get(notionals_, i,
                                                   notionals_.back()),
                                       start, end,
                                       overnightIndex_,
                                       detail::get(gearings_, i, 1.0),
                                       detail::get(spreads_, i, 0.0),
                                       refStart, refEnd,
                                       paymentDayCounter_)));
        }
        return cashflows;
    }

}
