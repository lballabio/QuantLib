/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano
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

/*! \file makeois.hpp
    \brief Helper class to instantiate overnight indexed swaps.
*/

#ifndef quantlib_makeois_hpp
#define quantlib_makeois_hpp

#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate overnight indexed swaps.
    */
    class MakeOIS {
      public:
        MakeOIS(const Period& swapTenor,
                const ext::shared_ptr<OvernightIndex>& overnightIndex,
                Rate fixedRate = Null<Rate>(),
                const Period& fwdStart = 0*Days);

        operator OvernightIndexedSwap() const;
        operator ext::shared_ptr<OvernightIndexedSwap>() const ;

        MakeOIS& receiveFixed(bool flag = true);
        MakeOIS& withType(Swap::Type type);
        MakeOIS& withNominal(Real n);

        MakeOIS& withSettlementDays(Natural settlementDays);
        MakeOIS& withEffectiveDate(const Date&);
        MakeOIS& withTerminationDate(const Date&);
        MakeOIS& withRule(DateGeneration::Rule r);

        MakeOIS& withPaymentFrequency(Frequency f);
        MakeOIS& withPaymentAdjustment(BusinessDayConvention convention);
        MakeOIS& withPaymentLag(Natural lag);
        MakeOIS& withPaymentCalendar(const Calendar& cal);

        MakeOIS& withEndOfMonth(bool flag = true);

        MakeOIS& withFixedLegDayCount(const DayCounter& dc);

        MakeOIS& withOvernightLegSpread(Spread sp);

        MakeOIS& withDiscountingTermStructure(
                  const Handle<YieldTermStructure>& discountingTermStructure);

        MakeOIS &withTelescopicValueDates(bool telescopicValueDates);

        MakeOIS& withAveragingMethod(RateAveraging::Type averagingMethod);

        MakeOIS& withPricingEngine(
                              const ext::shared_ptr<PricingEngine>& engine);
      private:
        Period swapTenor_;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Rate fixedRate_;
        Period forwardStart_;

        Natural settlementDays_ = 2;
        Date effectiveDate_, terminationDate_;
        Calendar calendar_;

        Frequency paymentFrequency_ = Annual;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentAdjustment_ = Following;
        Natural paymentLag_ = 0;

        DateGeneration::Rule rule_ = DateGeneration::Backward;
        bool endOfMonth_, isDefaultEOM_ = true;

        Swap::Type type_ = Swap::Payer;
        Real nominal_ = 1.0;

        Spread overnightSpread_ = 0.0;
        DayCounter fixedDayCount_;

        ext::shared_ptr<PricingEngine> engine_;

        bool telescopicValueDates_ = false;
        RateAveraging::Type averagingMethod_ = RateAveraging::Compound;
    };

}

#endif
