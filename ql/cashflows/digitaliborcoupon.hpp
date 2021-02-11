/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti
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

/*! \file digitaliborcoupon.hpp
    \brief Ibor-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_ibor_coupon_hpp
#define quantlib_digital_ibor_coupon_hpp

#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Ibor rate coupon with digital digital call/put option
    class DigitalIborCoupon : public DigitalCoupon {
      public:
        DigitalIborCoupon(
            const ext::shared_ptr<IborCoupon> &underlying,
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
            bool nakedOption =false);

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
    };


    //! helper class building a sequence of digital ibor-rate coupons
    class DigitalIborLeg {
      public:
        DigitalIborLeg(Schedule schedule, ext::shared_ptr<IborIndex> index);
        DigitalIborLeg& withNotionals(Real notional);
        DigitalIborLeg& withNotionals(const std::vector<Real>& notionals);
        DigitalIborLeg& withPaymentDayCounter(const DayCounter&);
        DigitalIborLeg& withPaymentAdjustment(BusinessDayConvention);
        DigitalIborLeg& withFixingDays(Natural fixingDays);
        DigitalIborLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        DigitalIborLeg& withGearings(Real gearing);
        DigitalIborLeg& withGearings(const std::vector<Real>& gearings);
        DigitalIborLeg& withSpreads(Spread spread);
        DigitalIborLeg& withSpreads(const std::vector<Spread>& spreads);
        DigitalIborLeg& inArrears(bool flag = true);
        DigitalIborLeg& withCallStrikes(Rate strike);
        DigitalIborLeg& withCallStrikes(const std::vector<Rate>& strikes);
        DigitalIborLeg& withLongCallOption(Position::Type);
        DigitalIborLeg& withCallATM(bool flag = true);
        DigitalIborLeg& withCallPayoffs(Rate payoff);
        DigitalIborLeg& withCallPayoffs(const std::vector<Rate>& payoffs);
        DigitalIborLeg& withPutStrikes(Rate strike);
        DigitalIborLeg& withPutStrikes(const std::vector<Rate>& strikes);
        DigitalIborLeg& withLongPutOption(Position::Type);
        DigitalIborLeg& withPutATM(bool flag = true);
        DigitalIborLeg& withPutPayoffs(Rate payoff);
        DigitalIborLeg& withPutPayoffs(const std::vector<Rate>& payoffs);
        DigitalIborLeg &withReplication(
            const ext::shared_ptr<DigitalReplication> &replication =
                ext::shared_ptr<DigitalReplication>(new DigitalReplication));
        DigitalIborLeg& withNakedOption(bool nakedOption = true);

        operator Leg() const;
      private:
        Schedule schedule_;
        ext::shared_ptr<IborIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        bool inArrears_;
        std::vector<Rate> callStrikes_, callPayoffs_;
        Position::Type longCallOption_;
        bool callATM_;
        std::vector<Rate> putStrikes_, putPayoffs_;
        Position::Type longPutOption_;
        bool putATM_;
        ext::shared_ptr<DigitalReplication> replication_;
        bool nakedOption_;
    };

}


#endif
