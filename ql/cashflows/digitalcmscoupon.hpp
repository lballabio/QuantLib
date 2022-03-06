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

/*! \file digitalcmscoupon.hpp
    \brief Cms-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_cms_coupon_hpp
#define quantlib_digital_cms_coupon_hpp

#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Cms-rate coupon with digital digital call/put option
    class DigitalCmsCoupon : public DigitalCoupon {
      public:
        DigitalCmsCoupon(
            const ext::shared_ptr<CmsCoupon> &underlying,
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
    class DigitalCmsLeg {
      public:
        DigitalCmsLeg(Schedule schedule, ext::shared_ptr<SwapIndex> index);
        DigitalCmsLeg& withNotionals(Real notional);
        DigitalCmsLeg& withNotionals(const std::vector<Real>& notionals);
        DigitalCmsLeg& withPaymentDayCounter(const DayCounter&);
        DigitalCmsLeg& withPaymentAdjustment(BusinessDayConvention);
        DigitalCmsLeg& withFixingDays(Natural fixingDays);
        DigitalCmsLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        DigitalCmsLeg& withGearings(Real gearing);
        DigitalCmsLeg& withGearings(const std::vector<Real>& gearings);
        DigitalCmsLeg& withSpreads(Spread spread);
        DigitalCmsLeg& withSpreads(const std::vector<Spread>& spreads);
        DigitalCmsLeg& inArrears(bool flag = true);
        DigitalCmsLeg& withCallStrikes(Rate strike);
        DigitalCmsLeg& withCallStrikes(const std::vector<Rate>& strikes);
        DigitalCmsLeg& withLongCallOption(Position::Type);
        DigitalCmsLeg& withCallATM(bool flag = true);
        DigitalCmsLeg& withCallPayoffs(Rate payoff);
        DigitalCmsLeg& withCallPayoffs(const std::vector<Rate>& payoffs);
        DigitalCmsLeg& withPutStrikes(Rate strike);
        DigitalCmsLeg& withPutStrikes(const std::vector<Rate>& strikes);
        DigitalCmsLeg& withLongPutOption(Position::Type);
        DigitalCmsLeg& withPutATM(bool flag = true);
        DigitalCmsLeg& withPutPayoffs(Rate payoff);
        DigitalCmsLeg& withPutPayoffs(const std::vector<Rate>& payoffs);
        DigitalCmsLeg& withReplication(
            const ext::shared_ptr<DigitalReplication> &replication =
                ext::shared_ptr<DigitalReplication>(new DigitalReplication));
        DigitalCmsLeg& withNakedOption(bool nakedOption = true);

        operator Leg() const;
      private:
        Schedule schedule_;
        ext::shared_ptr<SwapIndex> index_;
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


#ifndef id_8ce572233c8e87b2dfee0f3984ca9357
#define id_8ce572233c8e87b2dfee0f3984ca9357
inline bool test_8ce572233c8e87b2dfee0f3984ca9357(const int* i) {
    return i != nullptr;
}
#endif
