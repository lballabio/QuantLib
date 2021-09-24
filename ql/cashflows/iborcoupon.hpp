/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2011 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 StatPro Italia srl
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

/*! \file iborcoupon.hpp
    \brief Coupon paying a Libor-type index
*/

#ifndef quantlib_ibor_coupon_hpp
#define quantlib_ibor_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/patterns/singleton.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! %Coupon paying a Libor-type index
    class IborCoupon : public FloatingRateCoupon {
      public:
        IborCoupon(const Date& paymentDate,
                   Real nominal,
                   const Date& startDate,
                   const Date& endDate,
                   Natural fixingDays,
                   const ext::shared_ptr<IborIndex>& index,
                   Real gearing = 1.0,
                   Spread spread = 0.0,
                   const Date& refPeriodStart = Date(),
                   const Date& refPeriodEnd = Date(),
                   const DayCounter& dayCounter = DayCounter(),
                   bool isInArrears = false,
                   const Date& exCouponDate = Date());
        //! \name Inspectors
        //@{
        const ext::shared_ptr<IborIndex>& iborIndex() const { return iborIndex_; }
        //! this is dependent on usingAtParCoupons()
        const Date& fixingEndDate() const { return fixingEndDate_; }
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        //! Implemented in order to manage the case of par coupon
        Rate indexFixing() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        ext::shared_ptr<IborIndex> iborIndex_;
        Date fixingDate_, fixingValueDate_, fixingEndDate_;
        Time spanningTime_;

      public:
        // IborCoupon::Settings forward declaration
        class Settings;
        /*! \deprecated Use IborCouponSettings::Settings::instance().createAtParCoupons() instead
                        Deprecated in version 1.24.
        */
        QL_DEPRECATED static void createAtParCoupons();
        /*! \deprecated Use IborCouponSettings::Settings::instance().createIndexedCoupons() instead
                        Deprecated in version 1.24.
        */
        QL_DEPRECATED static void createIndexedCoupons();
        /*! \deprecated Use IborCouponSettings::Settings::instance().usingAtParCoupons() instead
                        Deprecated in version 1.24.
        */
        QL_DEPRECATED static bool usingAtParCoupons();
    };


    //! Per-session settings for IborCoupon class
    class IborCoupon::Settings : public Singleton<IborCoupon::Settings> {
        friend class Singleton<IborCoupon::Settings>;
      private:
        Settings();
        // disable copy/move
        Settings(const Settings &) = delete;
        Settings & operator=(const Settings &) = delete;
        Settings(Settings &&) = delete;
        Settings & operator=(Settings &&) = delete;

      public:
        //! When called, IborCoupons are created as indexed coupons instead of par coupons.
        void createAtParCoupons();

        //! When called, IborCoupons are created as par coupons instead of indexed coupons.
        void createIndexedCoupons();

        /*! If true the IborCoupons are created as par coupons and vice versa.
            The default depends on the compiler flag QL_USE_INDEXED_COUPON and can be overwritten by
            createAtParCoupons() and createIndexedCoupons() */
        bool usingAtParCoupons() const;

      private:
        bool usingAtParCoupons_;
    };

    //! helper class building a sequence of capped/floored ibor-rate coupons
    class IborLeg {
      public:
        IborLeg(Schedule schedule, ext::shared_ptr<IborIndex> index);
        IborLeg& withNotionals(Real notional);
        IborLeg& withNotionals(const std::vector<Real>& notionals);
        IborLeg& withPaymentDayCounter(const DayCounter&);
        IborLeg& withPaymentAdjustment(BusinessDayConvention);
        IborLeg& withPaymentLag(Natural lag);
        IborLeg& withPaymentCalendar(const Calendar&);
        IborLeg& withFixingDays(Natural fixingDays);
        IborLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        IborLeg& withGearings(Real gearing);
        IborLeg& withGearings(const std::vector<Real>& gearings);
        IborLeg& withSpreads(Spread spread);
        IborLeg& withSpreads(const std::vector<Spread>& spreads);
        IborLeg& withCaps(Rate cap);
        IborLeg& withCaps(const std::vector<Rate>& caps);
        IborLeg& withFloors(Rate floor);
        IborLeg& withFloors(const std::vector<Rate>& floors);
        IborLeg& inArrears(bool flag = true);
        IborLeg& withZeroPayments(bool flag = true);
        IborLeg& withExCouponPeriod(const Period&,
                                    const Calendar&,
                                    BusinessDayConvention,
                                    bool endOfMonth = false);
        operator Leg() const;

      private:
        Schedule schedule_;
        ext::shared_ptr<IborIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        Natural paymentLag_;
        Calendar paymentCalendar_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
        bool inArrears_, zeroPayments_;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_;
        bool exCouponEndOfMonth_;
    };

}

#endif
