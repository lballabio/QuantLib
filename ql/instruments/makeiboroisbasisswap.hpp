/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Yue Tian

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

/*! \file makeiboroisbasisSwap.hpp
    \brief Helper class to instantiate ibor vs. overnight indexed swaps.
*/

#ifndef quantlib_makeiboroisbasisswap_hpp
#define quantlib_makeiboroisbasisswap_hpp

#include <ql/instruments/iboroisbasisswap.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate ibor vs. overnight indexed swaps.
    */
    class MakeIBOROISBasisSwap {
      public:
        MakeIBOROISBasisSwap(const Period& swapTenor,
				const boost::shared_ptr<IborIndex>& iborIndex,
                const boost::shared_ptr<OvernightIndex>& overnightIndex,
                Rate spread = Null<Rate>(),
                const Period& fwdStart = 0*Days);

        operator IBOROISBasisSwap() const;
        operator boost::shared_ptr<IBOROISBasisSwap>() const ;

        MakeIBOROISBasisSwap& withType(IBOROISBasisSwap::Type type);
        MakeIBOROISBasisSwap& withNominal(Real n);
        MakeIBOROISBasisSwap& withSettlementDays(Natural fixingDays);
        MakeIBOROISBasisSwap& withEffectiveDate(const Date&);
        MakeIBOROISBasisSwap& withTerminationDate(const Date&);
		MakeIBOROISBasisSwap& withEndOfMonth(bool flag = true);
        MakeIBOROISBasisSwap& withPaymentConvention(BusinessDayConvention bc);

        MakeIBOROISBasisSwap& withFloatingLegTenor(const Period& t);
        MakeIBOROISBasisSwap& withFloatingLegCalendar(const Calendar& cal);
        MakeIBOROISBasisSwap& withFloatingLegConvention(BusinessDayConvention bdc);
        MakeIBOROISBasisSwap& withFloatingLegTerminationDateConvention(
                                                   BusinessDayConvention bdc);
        MakeIBOROISBasisSwap& withFloatingLegRule(DateGeneration::Rule r);
        MakeIBOROISBasisSwap& withFloatingLegDayCount(const DayCounter& dc);

		MakeIBOROISBasisSwap& withOvernightLegTenor(const Period& t);
        MakeIBOROISBasisSwap& withOvernightLegCalendar(const Calendar& cal);
        MakeIBOROISBasisSwap& withOvernightLegConvention(BusinessDayConvention bdc);
        MakeIBOROISBasisSwap& withOvernightLegTerminationDateConvention(
                                                   BusinessDayConvention bdc);
        MakeIBOROISBasisSwap& withOvernightLegRule(DateGeneration::Rule r);
        MakeIBOROISBasisSwap& withOvernightLegDayCount(const DayCounter& dc);
        MakeIBOROISBasisSwap& withOvernightLegSpread(Spread sp);

        MakeIBOROISBasisSwap& withDiscountingTermStructure(
                  const Handle<YieldTermStructure>& discountingTermStructure);

      private:
        Period swapTenor_;
		boost::shared_ptr<IborIndex> floatingIndex_;
		boost::shared_ptr<OvernightIndex> overnightIndex_;
		Spread overnightSpread_;
		Period forwardStart_;
		bool endOfMonth_;

		IBOROISBasisSwap::Type type_;
        Real nominal_;
		Natural fixingDays_;
		Date effectiveDate_, terminationDate_;
		BusinessDayConvention paymentConvention_;

        Period floatingLegTenor_;
        Calendar floatingLegCalendar_;
        BusinessDayConvention floatingLegConvention_;
        BusinessDayConvention floatingLegTerminationDateConvention_;
        DateGeneration::Rule floatingLegRule_;      
        DayCounter floatingLegDayCount_;

		Period overnightLegTenor_;
        Calendar overnightLegCalendar_;
        BusinessDayConvention overnightLegConvention_;
        BusinessDayConvention overnightLegTerminationDateConvention_;
        DateGeneration::Rule overnightLegRule_;
        DayCounter overnightLegDayCount_;

        boost::shared_ptr<PricingEngine> engine_;
    };

}

#endif
