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
 <http://quantlib.org/license.shtml>.

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
#include <boost/optional.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard cds.
    */
    class MakeCreditDefaultSwap {
      public:
        MakeCreditDefaultSwap(const Period& tenor, Real couponRate);
        MakeCreditDefaultSwap(const Date& termDate, Real couponRate);

        operator CreditDefaultSwap() const;
        operator ext::shared_ptr<CreditDefaultSwap>() const;

        MakeCreditDefaultSwap& withUpfrontRate(Real);
        MakeCreditDefaultSwap& withSide(Protection::Side);
        MakeCreditDefaultSwap& withNominal(Real);
        MakeCreditDefaultSwap& withCouponTenor(Period);
        MakeCreditDefaultSwap& withDayCounter(DayCounter&);
        MakeCreditDefaultSwap& withLastPeriodDayCounter(DayCounter&);
        MakeCreditDefaultSwap& withDateGenerationRule(DateGeneration::Rule rule);
        MakeCreditDefaultSwap& withCashSettlementDays(Natural cashSettlementDays);

        MakeCreditDefaultSwap& withPricingEngine(const ext::shared_ptr<PricingEngine>&);

        MakeCreditDefaultSwap& withTradeDate(const Date& tradeDate);

      private:
        Protection::Side side_;
        Real nominal_;
        boost::optional<Period> tenor_;
        boost::optional<Date> termDate_;
        Period couponTenor_;
        Real couponRate_;
        Real upfrontRate_;
        DayCounter dayCounter_;
        DayCounter lastPeriodDayCounter_;
        DateGeneration::Rule rule_;
        Natural cashSettlementDays_;
        Date tradeDate_;

        ext::shared_ptr<PricingEngine> engine_;
    };
}

#endif
