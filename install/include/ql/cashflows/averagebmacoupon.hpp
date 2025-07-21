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

/*! \file averagebmacoupon.hpp
    \brief coupon paying a weighted average of BMA-index fixings
*/

#ifndef quantlib_bma_coupon_hpp
#define quantlib_bma_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/bmaindex.hpp>

namespace QuantLib {

    //! Average BMA coupon
    /*! %Coupon paying a BMA index, where the coupon rate is a
        weighted average of relevant fixings.

        The weighted average is computed based on the
        actual calendar days for which a given fixing is valid and
        contributing to the given interest period.

        Before weights are computed, the fixing schedule is adjusted
        for the index's fixing day gap. See rate() method for details.
    */
    class AverageBMACoupon : public FloatingRateCoupon {
      public:
        AverageBMACoupon(const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         const ext::shared_ptr<BMAIndex>& index,
                         Real gearing = 1.0,
                         Spread spread = 0.0,
                         const Date& refPeriodStart = Date(),
                         const Date& refPeriodEnd = Date(),
                         const DayCounter& dayCounter = DayCounter());

        //! \name FloatingRateCoupon interface
        //@{
        //! not applicable here; use fixingDates() instead
        Date fixingDate() const override;
        //! fixing dates of the rates to be averaged
        std::vector<Date> fixingDates() const;

        //! not applicable here; use indexFixings() instead
        Rate indexFixing() const override;
        //! fixings of the underlying index to be averaged
        std::vector<Rate> indexFixings() const;

        //! not applicable here
        Rate convexityAdjustment() const override;
        //@}

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        Schedule fixingSchedule_;
    };


    //! helper class building a sequence of average BMA coupons
    class AverageBMALeg {
      public:
        AverageBMALeg(Schedule schedule, ext::shared_ptr<BMAIndex> index);
        AverageBMALeg& withNotionals(Real notional);
        AverageBMALeg& withNotionals(const std::vector<Real>& notionals);
        AverageBMALeg& withPaymentDayCounter(const DayCounter&);
        AverageBMALeg& withPaymentAdjustment(BusinessDayConvention);
        AverageBMALeg& withGearings(Real gearing);
        AverageBMALeg& withGearings(const std::vector<Real>& gearings);
        AverageBMALeg& withSpreads(Spread spread);
        AverageBMALeg& withSpreads(const std::vector<Spread>& spreads);
        operator Leg() const;
      private:
        Schedule schedule_;
        ext::shared_ptr<BMAIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_ = Following;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
    };

}


#endif
