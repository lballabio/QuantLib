/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2011 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
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

/*! \file iborcoupon.hpp
    \brief Coupon paying a Libor-type index
*/

#ifndef quantlib_ibor_coupon_hpp
#define quantlib_ibor_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
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
                   const boost::shared_ptr<IborIndex>& index,
                   Real gearing = 1.0,
                   Spread spread = 0.0,
                   const Date& refPeriodStart = Date(),
                   const Date& refPeriodEnd = Date(),
                   const DayCounter& dayCounter = DayCounter(),
                   bool isInArrears = false);
        //! \name Inspectors
        //@{
        const boost::shared_ptr<IborIndex>& iborIndex() const {
            return iborIndex_;
        }
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        //! Implemented in order to manage the case of par coupon
        Rate indexFixing() const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::shared_ptr<IborIndex> iborIndex_;
        Date fixingDate_, fixingValueDate_, fixingEndDate_;
        Time spanningTime_;
    };


    //! helper class building a sequence of capped/floored ibor-rate coupons
    class IborLeg {
      public:
        IborLeg(const Schedule& schedule,
                const boost::shared_ptr<IborIndex>& index);
        IborLeg& withNotionals(Real notional);
        IborLeg& withNotionals(const std::vector<Real>& notionals);
        IborLeg& withPaymentDayCounter(const DayCounter&);
        IborLeg& withPaymentAdjustment(BusinessDayConvention);
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
        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<IborIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
        bool inArrears_, zeroPayments_;
    };

}

#endif
