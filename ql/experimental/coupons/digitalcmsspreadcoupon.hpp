/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file digitalcmsspreadcoupon.hpp
    \brief Cms-spread-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_cmsspread_coupon_hpp
#define quantlib_digital_cmsspread_coupon_hpp

#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/experimental/coupons/cmsspreadcoupon.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Cms-spread-rate coupon with digital digital call/put option
    class DigitalCmsSpreadCoupon : public DigitalCoupon {
      public:
        explicit DigitalCmsSpreadCoupon(
            const ext::shared_ptr<CmsSpreadCoupon> &underlying,
            Rate callStrike = Null<Rate>(),
            Position::Type callPosition = Position::Long,
            bool isCallATMIncluded = false,
            Rate callDigitalPayoff = Null<Rate>(),
            Rate putStrike = Null<Rate>(),
            Position::Type putPosition = Position::Long,
            bool isPutATMIncluded = false,
            Rate putDigitalPayoff = Null<Rate>(),
            const ext::shared_ptr<DigitalReplication> &replication =
                ext::shared_ptr<DigitalReplication>(new DigitalReplication),
            bool nakedOption = false);

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
    };


    //! helper class building a sequence of digital ibor-rate coupons
    class DigitalCmsSpreadLeg {
      public:
        DigitalCmsSpreadLeg(Schedule schedule,
                            ext::shared_ptr<SwapSpreadIndex> index,
                            ext::shared_ptr<FloatingRateCouponPricer> pricer);
        DigitalCmsSpreadLeg& withNotionals(Real notional);
        DigitalCmsSpreadLeg& withNotionals(const std::vector<Real>& notionals);
        DigitalCmsSpreadLeg& withPaymentDayCounter(const DayCounter&);
        DigitalCmsSpreadLeg& withPaymentAdjustment(BusinessDayConvention);
        DigitalCmsSpreadLeg& withFixingDays(Natural fixingDays);
        DigitalCmsSpreadLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        DigitalCmsSpreadLeg& withGearings(Real gearing);
        DigitalCmsSpreadLeg& withGearings(const std::vector<Real>& gearings);
        DigitalCmsSpreadLeg& withSpreads(Spread spread);
        DigitalCmsSpreadLeg& withSpreads(const std::vector<Spread>& spreads);
        DigitalCmsSpreadLeg& inArrears(bool flag = true);
        DigitalCmsSpreadLeg& withCallStrikes(Rate strike);
        DigitalCmsSpreadLeg& withCallStrikes(const std::vector<Rate>& strikes);
        DigitalCmsSpreadLeg& withLongCallOption(Position::Type);
        DigitalCmsSpreadLeg& withCallATM(bool flag = true);
        DigitalCmsSpreadLeg& withCallPayoffs(Rate payoff);
        DigitalCmsSpreadLeg& withCallPayoffs(const std::vector<Rate>& payoffs);
        DigitalCmsSpreadLeg& withPutStrikes(Rate strike);
        DigitalCmsSpreadLeg& withPutStrikes(const std::vector<Rate>& strikes);
        DigitalCmsSpreadLeg& withLongPutOption(Position::Type);
        DigitalCmsSpreadLeg& withPutATM(bool flag = true);
        DigitalCmsSpreadLeg& withPutPayoffs(Rate payoff);
        DigitalCmsSpreadLeg& withPutPayoffs(const std::vector<Rate>& payoffs);
        DigitalCmsSpreadLeg& withReplication(
            const ext::shared_ptr<DigitalReplication> &replication =
                ext::shared_ptr<DigitalReplication>(new DigitalReplication));
        DigitalCmsSpreadLeg& withNakedOption(bool nakedOption = true);

        operator Leg() const;
      private:
        Schedule schedule_;
        ext::shared_ptr<SwapSpreadIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_ = Following;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        bool inArrears_ = false;
        std::vector<Rate> callStrikes_, callPayoffs_;
        Position::Type longCallOption_ = Position::Long;
        bool callATM_ = false;
        std::vector<Rate> putStrikes_, putPayoffs_;
        Position::Type longPutOption_ = Position::Long;
        bool putATM_ = false;
        ext::shared_ptr<DigitalReplication> replication_;
        bool nakedOption_;
        ext::shared_ptr<FloatingRateCouponPricer> pricer_;
    };

}


#endif
