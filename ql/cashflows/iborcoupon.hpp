/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2011 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer
 Copyright (C) 2026 Kyrylo Protsenko

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

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
#include <ql/optional.hpp>

namespace QuantLib {

    //! Convention used to select an index for an irregular Ibor coupon
    enum class StubIndexConvention {
        ClosestIndex,
        Interpolated
    };

    //! Index selection for irregular Ibor coupons
    /*! A default-constructed configuration is empty, meaning that broken
        periods fix on the leg's own index as usual.  A non-empty one supplies
        the candidate indices used instead.

        The indices are supplied as fully constructed objects so that each one
        can use its own conventions, fixing history and forwarding curve.
        Because of this, none of them can track a curve that is still being
        bootstrapped; they must all use exogenous forwarding curves.

        For ClosestIndex, the index whose maturity is closest to the coupon end
        date is used (ties go to the shorter index).  For Interpolated, the two
        index maturities bracketing the coupon end date are used for linear
        interpolation in calendar days.  All candidate indices must resolve the
        coupon fixing date to a value date equal to the coupon accrual start
        date, and their maturities must be distinct.

        The resulting coupons always fix each candidate index over its own
        deposit period, so legs using these conventions must be built with
        indexed coupons; the par-coupon approximation is not supported.
    */
    class StubIndexConfig {
      public:
        //! empty configuration: broken periods fix on the leg's own index
        StubIndexConfig() = default;
        StubIndexConfig(StubIndexConvention convention,
                        std::vector<ext::shared_ptr<IborIndex> > indices);

        bool empty() const { return indices_.empty(); }
        StubIndexConvention convention() const { return convention_; }
        const std::vector<ext::shared_ptr<IborIndex> >& indices() const { return indices_; }

      private:
        StubIndexConvention convention_ = StubIndexConvention::ClosestIndex;
        std::vector<ext::shared_ptr<IborIndex> > indices_;
    };

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
                   const Date& exCouponDate = Date(),
                   BusinessDayConvention fixingConvention = Preceding);
        //! \name Inspectors
        //@{
        const ext::shared_ptr<IborIndex>& iborIndex() const { return iborIndex_; }
        virtual bool hasFixed() const;
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        Date fixingDate() const override;
        // implemented in order to manage the case of par coupon
        Rate indexFixing() const override;
        void setPricer(const ext::shared_ptr<FloatingRateCouponPricer>&) override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
        /*! \name Internal calculations

            You won't probably need these methods unless you're implementing
            a coupon pricer.
        */
        //@{
        //! Start of the deposit period underlying the index fixing
        const Date& fixingValueDate() const;
        //! End of the deposit period underlying the index fixing
        const Date& fixingMaturityDate() const;
        //! End of the deposit period underlying the coupon fixing
        /*! This might be not the same as fixingMaturityDate if par coupons are used. */
        const Date& fixingEndDate() const;
        //! Period underlying the index fixing, as a year fraction
        Time spanningTimeIndexMaturity() const;
        //! Period underlying the coupon fixing, as a year fraction
        /*! This might be not the same as spanningTimeIndexMaturity if par coupons are used. */
        Time spanningTime() const;
        //@}

      private:
        friend class IborCouponPricer;
        ext::shared_ptr<IborIndex> iborIndex_;
        Date fixingDate_;
        // computed by coupon pricer (depending on par coupon flag) and stored here
        void initializeCachedData() const;
        mutable bool cachedDataIsInitialized_ = false;
        mutable Date fixingValueDate_, fixingEndDate_, fixingMaturityDate_;
        mutable Time spanningTime_, spanningTimeIndexMaturity_;

      public:
        // IborCoupon::Settings forward declaration
        class Settings;
    };


    //! Irregular Ibor coupon using an explicitly supplied set of indices
    /*! This coupon is intended for broken periods.  Candidate indices are
        selected according to their maturity dates and retain their own
        forwarding curves and fixing histories.

        The selection depends on dates only, so it is performed once at
        construction; configurations invalid for this coupon (candidates whose
        value date differs from the accrual start date, duplicate maturities,
        maturities not bracketing the coupon end date) throw from the
        constructor.
    */
    class StubIborCoupon : public IborCoupon {
      public:
        StubIborCoupon(const Date& paymentDate,
                       Real nominal,
                       const Date& startDate,
                       const Date& endDate,
                       Natural fixingDays,
                       const ext::shared_ptr<IborIndex>& index,
                       StubIndexConfig stubIndexConfig,
                       Real gearing = 1.0,
                       Spread spread = 0.0,
                       const Date& refPeriodStart = Date(),
                       const Date& refPeriodEnd = Date(),
                       const DayCounter& dayCounter = DayCounter(),
                       bool isInArrears = false,
                       const Date& exCouponDate = Date(),
                       BusinessDayConvention fixingConvention = Preceding);

        const StubIndexConfig& stubIndexConfig() const {
            return stubIndexConfig_;
        }

        //! the candidate indices the coupon fixes on, with their weights
        /*! A single entry with weight 1 for ClosestIndex or an exact
            Interpolated match; the two bracketing indices otherwise.
        */
        const std::vector<std::pair<ext::shared_ptr<IborIndex>, Real> >&
        selectedIndices() const {
            return selectedIndices_;
        }

        Rate indexFixing() const override;
        bool hasFixed() const override;
        void accept(AcyclicVisitor&) override;

      private:
        StubIndexConfig stubIndexConfig_;
        std::vector<std::pair<ext::shared_ptr<IborIndex>, Real> > selectedIndices_;
    };


    //! Per-session settings for IborCoupon class
    class IborCoupon::Settings : public Singleton<IborCoupon::Settings> {
        friend class Singleton<IborCoupon::Settings>;
      private:
        Settings() = default;

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
        #ifndef QL_USE_INDEXED_COUPON
        bool usingAtParCoupons_ = true;
        #else
        bool usingAtParCoupons_ = false;
        #endif
    };

    //! helper class building a sequence of capped/floored ibor-rate coupons
    class IborLeg {
      public:
        IborLeg(Schedule schedule, ext::shared_ptr<IborIndex> index);
        IborLeg& withNotionals(Real notional);
        IborLeg& withNotionals(const std::vector<Real>& notionals);
        IborLeg& withPaymentDayCounter(const DayCounter&);
        IborLeg& withPaymentAdjustment(BusinessDayConvention);
        IborLeg& withPaymentLag(Integer lag);
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
        IborLeg& withFixingConvention(BusinessDayConvention);
        IborLeg& withIndexedCoupons(std::optional<bool> b = true);
        IborLeg& withAtParCoupons(bool b = true);
        //! sets index selection for uncapped schedule periods marked as irregular
        /*! Requires indexed coupons; see StubIndexConfig. */
        IborLeg& withStubIndexConfig(const StubIndexConfig&);
        operator Leg() const;

      private:
        Schedule schedule_;
        ext::shared_ptr<IborIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_ = Following;
        Integer paymentLag_ = 0;
        Calendar paymentCalendar_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
        bool inArrears_ = false, zeroPayments_ = false;
        BusinessDayConvention fixingConvention_ = Preceding;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_ = Unadjusted;
        bool exCouponEndOfMonth_ = false;
        std::optional<bool> useIndexedCoupons_;
        StubIndexConfig stubIndexConfig_;
    };

}

#endif
