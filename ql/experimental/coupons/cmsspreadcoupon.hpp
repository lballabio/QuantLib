/*
 Copyright (C) 2014 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file cmsspreadcoupon.hpp
    \brief CMS spread coupon
*/

#ifndef quantlib_cmsspread_coupon_hpp
#define quantlib_cmsspread_coupon_hpp

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/experimental/coupons/swapspreadindex.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    class SwapIndex;

    //! CMS spread coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class CmsSpreadCoupon : public FloatingRateCoupon {
      public:
        CmsSpreadCoupon(const Date& paymentDate,
                        Real nominal,
                        const Date& startDate,
                        const Date& endDate,
                        Natural fixingDays,
                        const ext::shared_ptr<SwapSpreadIndex>& index,
                        Real gearing = 1.0,
                        Spread spread = 0.0,
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date(),
                        const DayCounter& dayCounter = DayCounter(),
                        bool isInArrears = false,
                        const Date& exCouponDate = Date(),
                        const ext::shared_ptr<FloatingRateCouponPricer>& pricer = nullptr);
        //! \name Inspectors
        //@{
        const ext::shared_ptr<SwapSpreadIndex>& swapSpreadIndex() const {
            return index_;
        }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        ext::shared_ptr<SwapSpreadIndex> index_;
    };

    class CappedFlooredCmsSpreadCoupon : public CappedFlooredCoupon {
      public:
        CappedFlooredCmsSpreadCoupon(
                  const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  Natural fixingDays,
                  const ext::shared_ptr<SwapSpreadIndex>& index,
                  Real gearing = 1.0,
                  Spread spread= 0.0,
                  const Rate cap = Null<Rate>(),
                  const Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false,
                  const Date& exCouponDate = Date(),
                  const ext::shared_ptr<FloatingRateCouponPricer>& pricer = nullptr)
        : CappedFlooredCoupon(ext::shared_ptr<FloatingRateCoupon>(new
            CmsSpreadCoupon(paymentDate, nominal, startDate, endDate, fixingDays,
                      index, gearing, spread, refPeriodStart, refPeriodEnd,
                            dayCounter, isInArrears, exCouponDate, pricer)), cap, floor) {}

        void accept(AcyclicVisitor& v) override {
            auto* v1 = dynamic_cast<Visitor<CappedFlooredCmsSpreadCoupon>*>(&v);
            if (v1 != nullptr)
                v1->visit(*this);
            else
                CappedFlooredCoupon::accept(v);
        }
    };

    //! helper class building a sequence of capped/floored cms-spread-rate coupons
    class CmsSpreadLeg {
      public:
        CmsSpreadLeg(Schedule schedule,
                     ext::shared_ptr<SwapSpreadIndex> swapSpreadIndex,
                     ext::shared_ptr<FloatingRateCouponPricer> pricer = nullptr);
        CmsSpreadLeg& withNotionals(Real notional);
        CmsSpreadLeg& withNotionals(const std::vector<Real>& notionals);
        CmsSpreadLeg& withPaymentDayCounter(const DayCounter&);
        CmsSpreadLeg& withPaymentAdjustment(BusinessDayConvention);
        CmsSpreadLeg& withFixingDays(Natural fixingDays);
        CmsSpreadLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        CmsSpreadLeg& withGearings(Real gearing);
        CmsSpreadLeg& withGearings(const std::vector<Real>& gearings);
        CmsSpreadLeg& withSpreads(Spread spread);
        CmsSpreadLeg& withSpreads(const std::vector<Spread>& spreads);
        CmsSpreadLeg& withCaps(Rate cap);
        CmsSpreadLeg& withCaps(const std::vector<Rate>& caps);
        CmsSpreadLeg& withFloors(Rate floor);
        CmsSpreadLeg& withFloors(const std::vector<Rate>& floors);
        CmsSpreadLeg& inArrears(bool flag = true);
        CmsSpreadLeg& withZeroPayments(bool flag = true);
        operator Leg() const;
      private:
        Schedule schedule_;
        ext::shared_ptr<SwapSpreadIndex> swapSpreadIndex_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_ = Following;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
        bool inArrears_ = false, zeroPayments_ = false;
        ext::shared_ptr<FloatingRateCouponPricer> pricer_;
    };


    //! base pricer for vanilla CMS spread coupons
    class CmsSpreadCouponPricer : public FloatingRateCouponPricer {
      public:
        explicit CmsSpreadCouponPricer(Handle<Quote> correlation = Handle<Quote>())
        : correlation_(std::move(correlation)) {
            registerWith(correlation_);
        }

        Handle<Quote> correlation() const{
            return correlation_;
        }

        void setCorrelation(
                         const Handle<Quote> &correlation = Handle<Quote>()) {
            unregisterWith(correlation_);
            correlation_ = correlation;
            registerWith(correlation_);
            update();
        }
      private:
        Handle<Quote> correlation_;
    };

}

#endif
