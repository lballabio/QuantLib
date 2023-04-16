/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006, 2007 StatPro Italia srl

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

/*! \file cmscoupon.hpp
    \brief CMS coupon
*/

#ifndef quantlib_cms_coupon_hpp
#define quantlib_cms_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class SwapIndex;

    //! CMS coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class CmsCoupon : public FloatingRateCoupon {
      public:
        CmsCoupon(const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  Natural fixingDays,
                  const std::shared_ptr<SwapIndex>& index,
                  Real gearing = 1.0,
                  Spread spread = 0.0,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false,
                  const Date& exCouponDate = Date());
        //! \name Inspectors
        //@{
        const std::shared_ptr<SwapIndex>& swapIndex() const {
            return swapIndex_;
        }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        std::shared_ptr<SwapIndex> swapIndex_;
    };


    //! helper class building a sequence of capped/floored cms-rate coupons
    class CmsLeg {
      public:
        CmsLeg(Schedule schedule, std::shared_ptr<SwapIndex> swapIndex);
        CmsLeg& withNotionals(Real notional);
        CmsLeg& withNotionals(const std::vector<Real>& notionals);
        CmsLeg& withPaymentDayCounter(const DayCounter&);
        CmsLeg& withPaymentAdjustment(BusinessDayConvention);
        CmsLeg& withFixingDays(Natural fixingDays);
        CmsLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        CmsLeg& withGearings(Real gearing);
        CmsLeg& withGearings(const std::vector<Real>& gearings);
        CmsLeg& withSpreads(Spread spread);
        CmsLeg& withSpreads(const std::vector<Spread>& spreads);
        CmsLeg& withCaps(Rate cap);
        CmsLeg& withCaps(const std::vector<Rate>& caps);
        CmsLeg& withFloors(Rate floor);
        CmsLeg& withFloors(const std::vector<Rate>& floors);
        CmsLeg& inArrears(bool flag = true);
        CmsLeg& withZeroPayments(bool flag = true);
        CmsLeg& withExCouponPeriod(const Period&,
                                   const Calendar&,
                                   BusinessDayConvention,
                                   bool endOfMonth);
        operator Leg() const;
      private:
        Schedule schedule_;
        std::shared_ptr<SwapIndex> swapIndex_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_ = Following;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
        bool inArrears_ = false, zeroPayments_ = false;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_ = Following;
        bool exCouponEndOfMonth_ = false;
    };

}

#endif
