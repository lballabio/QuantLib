/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2010 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file makevanillaswap.hpp
    \brief Helper class to instantiate standard market swaps.
*/

#ifndef quantlib_makevanillaswap_hpp
#define quantlib_makevanillaswap_hpp

#include <ql/instruments/vanillaswap.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard market swap.
    */
    class MakeVanillaSwap {
      public:
        MakeVanillaSwap(const Period& swapTenor,
                        const ext::shared_ptr<IborIndex>& iborIndex,
                        Rate fixedRate = Null<Rate>(),
                        const Period& forwardStart = 0*Days);

        operator VanillaSwap() const;
        operator ext::shared_ptr<VanillaSwap>() const;

        MakeVanillaSwap& receiveFixed(bool flag = true);
        MakeVanillaSwap& withType(Swap::Type type);
        MakeVanillaSwap& withNominal(Real n);

        MakeVanillaSwap& withSettlementDays(Natural settlementDays);
        MakeVanillaSwap& withEffectiveDate(const Date&);
        MakeVanillaSwap& withTerminationDate(const Date&);
        MakeVanillaSwap& withRule(DateGeneration::Rule r);
        MakeVanillaSwap& withPaymentConvention(BusinessDayConvention bdc);

        MakeVanillaSwap& withFixedLegTenor(const Period& t);
        MakeVanillaSwap& withFixedLegCalendar(const Calendar& cal);
        MakeVanillaSwap& withFixedLegConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFixedLegTerminationDateConvention(
                                                   BusinessDayConvention bdc);
        MakeVanillaSwap& withFixedLegRule(DateGeneration::Rule r);
        MakeVanillaSwap& withFixedLegEndOfMonth(bool flag = true);
        MakeVanillaSwap& withFixedLegFirstDate(const Date& d);
        MakeVanillaSwap& withFixedLegNextToLastDate(const Date& d);
        MakeVanillaSwap& withFixedLegDayCount(const DayCounter& dc);

        MakeVanillaSwap& withFloatingLegTenor(const Period& t);
        MakeVanillaSwap& withFloatingLegCalendar(const Calendar& cal);
        MakeVanillaSwap& withFloatingLegConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFloatingLegTerminationDateConvention(
                                                   BusinessDayConvention bdc);
        MakeVanillaSwap& withFloatingLegRule(DateGeneration::Rule r);
        MakeVanillaSwap& withFloatingLegEndOfMonth(bool flag = true);
        MakeVanillaSwap& withFloatingLegFirstDate(const Date& d);
        MakeVanillaSwap& withFloatingLegNextToLastDate(const Date& d);
        MakeVanillaSwap& withFloatingLegDayCount(const DayCounter& dc);
        MakeVanillaSwap& withFloatingLegSpread(Spread sp);

        MakeVanillaSwap& withDiscountingTermStructure(
                              const Handle<YieldTermStructure>& discountCurve);
        MakeVanillaSwap& withPricingEngine(
                              const ext::shared_ptr<PricingEngine>& engine);
        MakeVanillaSwap& withIndexedCoupons(const ext::optional<bool>& b = true);
        MakeVanillaSwap& withAtParCoupons(bool b = true);
      private:
        Period swapTenor_;
        ext::shared_ptr<IborIndex> iborIndex_;
        Rate fixedRate_;
        Period forwardStart_;

        Natural settlementDays_ = Null<Natural>();
        Date effectiveDate_, terminationDate_;
        Calendar fixedCalendar_, floatCalendar_;

        Swap::Type type_ = Swap::Payer;
        Real nominal_ = 1.0;
        Period fixedTenor_, floatTenor_;
        BusinessDayConvention fixedConvention_ = ModifiedFollowing,
                              fixedTerminationDateConvention_ = ModifiedFollowing;
        BusinessDayConvention floatConvention_, floatTerminationDateConvention_;
        DateGeneration::Rule fixedRule_ = DateGeneration::Backward,
                             floatRule_ = DateGeneration::Backward;
        bool fixedEndOfMonth_ = false, floatEndOfMonth_ = false;
        Date fixedFirstDate_, fixedNextToLastDate_;
        Date floatFirstDate_, floatNextToLastDate_;
        Spread floatSpread_ = 0.0;
        DayCounter fixedDayCount_, floatDayCount_;
        ext::optional<bool> useIndexedCoupons_;
        ext::optional<BusinessDayConvention> paymentConvention_;

        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
