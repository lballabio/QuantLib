/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Roland Lichters
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/cashflows/averagebmacoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/utilities/vectors.hpp>

namespace QuantLib {

    namespace {

        Integer bmaCutoffDays = 0; // to be verified

        class AverageBMACouponPricer : public FloatingRateCouponPricer {
          public:
            void initialize(const FloatingRateCoupon& coupon) {
                coupon_ = dynamic_cast<const AverageBMACoupon*>(&coupon);
                QL_ENSURE(coupon_, "wrong coupon type");
            }
            Rate swapletRate() const {
                const std::vector<Date>& fixingDates = coupon_->fixingDates();
                const boost::shared_ptr<InterestRateIndex>& index =
                    coupon_->index();

                Natural cutoffDays = 0; // to be verified
                Date startDate = coupon_->accrualStartDate() - cutoffDays,
                     endDate = coupon_->accrualEndDate() - cutoffDays,
                     d1 = startDate,
                     d2 = startDate;

                QL_REQUIRE (fixingDates.size() > 0, "fixing date list empty");
                QL_REQUIRE (index->valueDate(fixingDates.front()) <= startDate,
                            "first fixing date valid after period start");
                QL_REQUIRE (index->valueDate(fixingDates.back()) >= endDate,
                            "last fixing date valid before period end");

                Rate avgBMA = 0.0;
                Integer days = 0;
                for (Size i=0; i<fixingDates.size() - 1; ++i) {
                    Date valueDate = index->valueDate(fixingDates[i]);
                    Date nextValueDate = index->valueDate(fixingDates[i+1]);

                    if (fixingDates[i] >= endDate || valueDate >= endDate)
                        break;
                    if (fixingDates[i+1] < startDate
                        || nextValueDate <= startDate)
                        continue;

                    d2 = std::min(nextValueDate, endDate);

                    avgBMA += index->fixing(fixingDates[i]) * (d2 - d1);

                    days += d2 - d1;
                    d1 = d2;
                }
                avgBMA /= (endDate - startDate);

                QL_ENSURE(days == endDate - startDate,
                          "averaging days " << days << " differ from "
                          "interest days " << (endDate - startDate));

                return coupon_->gearing()*avgBMA + coupon_->spread();
            }

            Real swapletPrice() const {
                QL_FAIL("not available");
            }
            Real capletPrice(Rate) const {
                QL_FAIL("not available");
            }
            Rate capletRate(Rate) const {
                QL_FAIL("not available");
            }
            Real floorletPrice(Rate) const {
                QL_FAIL("not available");
            }
            Rate floorletRate(Rate) const {
                QL_FAIL("not available");
            }
          private:
            const AverageBMACoupon* coupon_;
        };

    }


    AverageBMACoupon::AverageBMACoupon(const Date& paymentDate,
                                       Real nominal,
                                       const Date& startDate,
                                       const Date& endDate,
                                       const boost::shared_ptr<BMAIndex>& index,
                                       Real gearing, Spread spread,
                                       const Date& refPeriodStart,
                                       const Date& refPeriodEnd,
                                       const DayCounter& dayCounter)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         index->fixingDays(), index, gearing, spread,
                         refPeriodStart, refPeriodEnd, dayCounter, false)
    {
        Calendar cal = index->fixingCalendar();
        Integer fixingDays = Integer(index->fixingDays());
        fixingDays += bmaCutoffDays;
        Date fixingStart = cal.advance(startDate, -fixingDays*Days, Preceding);
        fixingSchedule_ = index->fixingSchedule(fixingStart, endDate);

        setPricer(boost::shared_ptr<FloatingRateCouponPricer>(
                                                 new AverageBMACouponPricer));
    }

    Date AverageBMACoupon::fixingDate() const {
        QL_FAIL("no single fixing date for average-BMA coupon");
    }

    std::vector<Date> AverageBMACoupon::fixingDates() const {
        return fixingSchedule_.dates();
    }

    Rate AverageBMACoupon::indexFixing() const {
        QL_FAIL("no single fixing for average-BMA coupon");
    }

    std::vector<Rate> AverageBMACoupon::indexFixings() const {
        std::vector<Rate> fixings(fixingSchedule_.size());
        for (Size i=0; i<fixings.size(); ++i)
            fixings[i] = index_->fixing(fixingSchedule_.date(i));
        return fixings;
    }

    Rate AverageBMACoupon::convexityAdjustment() const {
        QL_FAIL("not defined for average-BMA coupon");
    }

    void AverageBMACoupon::accept(AcyclicVisitor& v) {
        Visitor<AverageBMACoupon>* v1 =
            dynamic_cast<Visitor<AverageBMACoupon>*>(&v);
        if (v1 != 0) {
            v1->visit(*this);
        } else {
            FloatingRateCoupon::accept(v);
        }
    }



    AverageBMALeg::AverageBMALeg(const Schedule& schedule,
                                 const boost::shared_ptr<BMAIndex>& index)
    : schedule_(schedule), index_(index), paymentAdjustment_(Following) {}

    AverageBMALeg& AverageBMALeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    AverageBMALeg& AverageBMALeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    AverageBMALeg& AverageBMALeg::withPaymentDayCounter(
                                               const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    AverageBMALeg& AverageBMALeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    AverageBMALeg& AverageBMALeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    AverageBMALeg& AverageBMALeg::withGearings(
                                          const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    AverageBMALeg& AverageBMALeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    AverageBMALeg& AverageBMALeg::withSpreads(
                                         const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    AverageBMALeg::operator Leg() const {

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

            cashflows.push_back(boost::shared_ptr<CashFlow>(new
                AverageBMACoupon(paymentDate,
                                 detail::get(notionals_, i, notionals_.back()),
                                 start, end,
                                 index_,
                                 detail::get(gearings_, i, 1.0),
                                 detail::get(spreads_, i, 0.0),
                                 refStart, refEnd,
                                 paymentDayCounter_)));
        }

        return cashflows;
    }

}

