/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file vanillaswap.hpp
    \brief Simple fixed-rate vs Libor swap
*/

#ifndef quantlib_makevanillaswap_hpp
#define quantlib_makevanillaswap_hpp

#include <ql/Instruments/vanillaswap.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard market swap.
    */
    class MakeVanillaSwap {
      public:
        MakeVanillaSwap(const Period& swapTenor, 
                        const boost::shared_ptr<Xibor>& index,
                        Rate fixedRate = Null<Rate>(),
                        const Period& forwardStart = 0*Days);
        #ifndef QL_DISABLE_DEPRECATED
        MakeVanillaSwap(
                const Date& effectiveDate,
                const Period& swapTenor, 
                const Calendar& cal,
                Rate fixedRate,
                const boost::shared_ptr<Xibor>& index,
                const Handle<YieldTermStructure>& discountingTermStructure);
        MakeVanillaSwap& withFixedLegNotEndOfMonth(bool flag = true);
        MakeVanillaSwap& withFloatingLegNotEndOfMonth(bool flag = true);
        #endif

        operator VanillaSwap() const;
        operator boost::shared_ptr<VanillaSwap>() const ;

        MakeVanillaSwap& receiveFixed(bool flag = true);
        MakeVanillaSwap& withType(VanillaSwap::Type type);
        MakeVanillaSwap& withNominal(Real n);
        MakeVanillaSwap& withEffectiveDate(const Date&);
        MakeVanillaSwap& withDiscountingTermStructure(
            const Handle<YieldTermStructure>& discountingTermStructure);

        MakeVanillaSwap& withFixedLegTenor(const Period& t);
        MakeVanillaSwap& withFixedLegCalendar(const Calendar& cal);
        MakeVanillaSwap& withFixedLegConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFixedLegTerminationDateConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFixedLegForward(bool flag = true);
        MakeVanillaSwap& withFixedLegEndOfMonth(bool flag = true);
        MakeVanillaSwap& withFixedLegFirstDate(const Date& d);
        MakeVanillaSwap& withFixedLegNextToLastDate(const Date& d);
        MakeVanillaSwap& withFixedLegDayCount(const DayCounter& dc);

        MakeVanillaSwap& withFloatingLegTenor(const Period& t);
        MakeVanillaSwap& withFloatingLegCalendar(const Calendar& cal);
        MakeVanillaSwap& withFloatingLegConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFloatingLegTerminationDateConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFloatingLegForward(bool flag = true);
        MakeVanillaSwap& withFloatingLegEndOfMonth(bool flag = true);
        MakeVanillaSwap& withFloatingLegFirstDate(const Date& d);
        MakeVanillaSwap& withFloatingLegNextToLastDate(const Date& d);
        MakeVanillaSwap& withFloatingLegDayCount(const DayCounter& dc);
        MakeVanillaSwap& withFloatingLegSpread(Spread sp);
        
      private:
        Period forwardStart_, swapTenor_;
        boost::shared_ptr<Xibor> index_;
        Rate fixedRate_;

        Date effectiveDate_;
        Calendar fixedCalendar_, floatCalendar_;
        Handle<YieldTermStructure> discountingTermStructure_;

        VanillaSwap::Type type_;
        Real nominal_;
        Period fixedTenor_, floatTenor_; 
        BusinessDayConvention fixedConvention_, fixedTerminationDateConvention_;
        BusinessDayConvention floatConvention_, floatTerminationDateConvention_;
        bool fixedBackward_, floatBackward_;
        bool fixedEndOfMonth_, floatEndOfMonth_;
        Date fixedFirstDate_, fixedNextToLastDate_;
        Date floatFirstDate_, floatNextToLastDate_;
        Spread floatSpread_;
        DayCounter fixedDayCount_, floatDayCount_;
    };

}

#endif
