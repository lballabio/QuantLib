/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

#include <ql/experimental/overnightswap/eoniacoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/utilities/vectors.hpp>

#include <iostream>

namespace QuantLib {

    namespace {

        class EoniaCouponPricer : public FloatingRateCouponPricer {
          public:
            void initialize(const FloatingRateCoupon& coupon) {
                coupon_ = dynamic_cast<const EoniaCoupon*>(&coupon);
                QL_ENSURE(coupon_, "wrong coupon type");
            }
            Rate swapletRate() const {
                const std::vector<Date>& fixingDates = coupon_->fixingDates();
                const boost::shared_ptr<InterestRateIndex>& index =
                    coupon_->index();
                const DayCounter& dc = index->dayCounter();

                Natural cutoffDays = 0; // to be verified
                Date startDate = coupon_->accrualStartDate() - cutoffDays,
                     endDate = coupon_->accrualEndDate() - cutoffDays,
                     d1 = startDate,
                     d2 = startDate;

                QL_REQUIRE (fixingDates.size() > 0, "fixing date list empty");
                QL_REQUIRE (index->valueDate(fixingDates.back()) >= endDate,
                            "last fixing date valid before period end");
                
                Rate comp = 1.0;
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
                    /*
                    comp *= (1.0 
                             + (index->fixing(fixingDates[i])+coupon_->spread())
                             * dc.yearFraction(d1, d2));
                    */
                    comp *= (1.0 + index->fixing(fixingDates[i])
                             * dc.yearFraction(d1, d2));

                    days += (d2 - d1);
                    d1 = d2;
                }
                comp -= 1.0;
                Rate rate = comp / dc.yearFraction(startDate, endDate);

                QL_ENSURE(days == endDate - startDate,
                          "averaging days " << days << " differ from "
                          "interest days " << (endDate - startDate));

                return coupon_->gearing() * rate + coupon_->spread();
                //return coupon_->gearing() * rate;
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
            const EoniaCoupon* coupon_;
        };

    }

    EoniaCoupon::EoniaCoupon(const Date& paymentDate,
                 Real nominal,
                 const Date& startDate,
                 const Date& endDate,
                 const boost::shared_ptr<Eonia>& index,
                 Real gearing, Spread spread,
                 const Date& refPeriodStart,
                 const Date& refPeriodEnd,
                 const DayCounter& dayCounter)
      : FloatingRateCoupon(nominal, paymentDate, startDate, endDate,
               index->fixingDays(), index, gearing, spread,
               refPeriodStart, refPeriodEnd, dayCounter, false),
        fixingSchedule_(MakeSchedule(startDate, endDate, 1*Days,
                                     index->fixingCalendar(),
                                 index->businessDayConvention()).backwards()) {
        setPricer(boost::shared_ptr<FloatingRateCouponPricer>(
                                                 new EoniaCouponPricer));
    }

    Date EoniaCoupon::fixingDate() const {
        QL_FAIL("no single fixing date for average-BMA coupon");
    }

    std::vector<Date> EoniaCoupon::fixingDates() const {
        return fixingSchedule_.dates();
    }

    Rate EoniaCoupon::indexFixing() const {
        QL_FAIL("no single fixing for average-BMA coupon");
    }

    std::vector<Rate> EoniaCoupon::indexFixings() const {
        std::vector<Rate> fixings(fixingSchedule_.size());
        for (Size i=0; i<fixings.size(); ++i)
            fixings[i] = index_->fixing(fixingSchedule_.date(i));
        return fixings;
    }

    Rate EoniaCoupon::convexityAdjustment() const {
        QL_FAIL("not defined for Eonia coupon");
    }

    void EoniaCoupon::accept(AcyclicVisitor& v) {
        Visitor<EoniaCoupon>* v1 =
            dynamic_cast<Visitor<EoniaCoupon>*>(&v);
        if (v1 != 0) {
            v1->visit(*this);
        } else {
            FloatingRateCoupon::accept(v);
        }
    }

    EoniaLeg::EoniaLeg(const Schedule& schedule,
                       const boost::shared_ptr<Eonia>& index)
    : schedule_(schedule), index_(index), paymentAdjustment_(Following) {}

    EoniaLeg& EoniaLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    EoniaLeg& EoniaLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    EoniaLeg& EoniaLeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    EoniaLeg& EoniaLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    EoniaLeg& EoniaLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    EoniaLeg& EoniaLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    EoniaLeg& EoniaLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    EoniaLeg& EoniaLeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    EoniaLeg::operator Leg() const {

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

            cashflows.push_back(boost::shared_ptr<CashFlow>(
                          new EoniaCoupon(paymentDate,
                                          detail::get(notionals_, i,
                                                      notionals_.back()),
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

