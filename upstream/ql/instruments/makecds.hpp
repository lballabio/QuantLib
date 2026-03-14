/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio
 Copyright (C) 2014 Peter Caspers

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

/*! \file makecds.hpp
    \brief Helper class to instantiate standard market cds.
*/

#ifndef quantlib_makecds_hpp
#define quantlib_makecds_hpp

#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard cds.
    */
    class MakeCreditDefaultSwap {
      public:
        MakeCreditDefaultSwap(const Period& tenor, Rate runningSpread);
        MakeCreditDefaultSwap(const Date& termDate, Rate runningSpread);
        MakeCreditDefaultSwap(const Schedule& schedule, Rate runningSpread);

        operator CreditDefaultSwap() const;
        operator ext::shared_ptr<CreditDefaultSwap>() const;

        MakeCreditDefaultSwap& withSide(Protection::Side);
        MakeCreditDefaultSwap& withNominal(Real);
        MakeCreditDefaultSwap& withUpfrontRate(Real);
        MakeCreditDefaultSwap& withCouponTenor(Period);
        MakeCreditDefaultSwap& withDateGenerationRule(DateGeneration::Rule);
        MakeCreditDefaultSwap& withConvention(BusinessDayConvention);
        MakeCreditDefaultSwap& withDayCounter(const DayCounter&);
        MakeCreditDefaultSwap& settleAccrual(bool b = true);
        MakeCreditDefaultSwap& payAtDefaultTime(bool b = true);
        MakeCreditDefaultSwap& withProtectionStart(Date);
        MakeCreditDefaultSwap& withUpfrontDate(Date);
        MakeCreditDefaultSwap& withClaim(ext::shared_ptr<Claim>);
        MakeCreditDefaultSwap& withLastPeriodDayCounter(const DayCounter&);
        MakeCreditDefaultSwap& rebateAccrual(bool b = true);
        MakeCreditDefaultSwap& withTradeDate(Date);
        MakeCreditDefaultSwap& withCashSettlementDays(Natural);

        MakeCreditDefaultSwap& withPricingEngine(const ext::shared_ptr<PricingEngine>&);

      private:
        ext::optional<Period> tenor_;
        ext::optional<Date> termDate_;
        ext::optional<Schedule> schedule_;
        Real runningSpread_;
        Protection::Side side_ = Protection::Buyer;
        Real nominal_ = 1.0;
        Real upfrontRate_ = 0.0;
        Period couponTenor_ = 3 * Months;
        DateGeneration::Rule rule_ = DateGeneration::CDS;
        BusinessDayConvention convention_ = Following;
        DayCounter dayCounter_ = Actual360();
        bool settlesAccrual_ = true;
        bool paysAtDefaultTime_ = true;
        Date protectionStart_;
        Date upfrontDate_;
        ext::shared_ptr<Claim> claim_;
        DayCounter lastPeriodDayCounter_ = Actual360(true);
        bool rebatesAccrual_ = true;
        Date tradeDate_;
        Natural cashSettlementDays_ = 3;

        ext::shared_ptr<PricingEngine> engine_;
    };
}

#endif
