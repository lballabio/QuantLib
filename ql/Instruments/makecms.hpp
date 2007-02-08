/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

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
    \brief Simple cms-rate vs Libor swap
*/

#ifndef quantlib_makecms_hpp
#define quantlib_makecms_hpp

#include <ql/CashFlows/cmscoupon.hpp>
#include <ql/Indexes/swapindex.hpp>
#include <ql/Instruments/swap.hpp>
#include <ql/CashFlows/conundrumpricer.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard market constant maturity swap.
    */
    class MakeCms {
      public:
        MakeCms(const Period& swapTenor,
                const boost::shared_ptr<SwapIndex>& swapIndex,
                Spread iborSpread,
                const boost::shared_ptr<CmsCouponPricer>& pricer,
                const Period& forwardStart = 0*Days);

        operator Swap() const;
        operator boost::shared_ptr<Swap>() const ;

        MakeCms& receiveCms(bool flag = true);
        MakeCms& withNominal(Real n);
        MakeCms& withEffectiveDate(const Date&);
        MakeCms& withDiscountingTermStructure(
            const Handle<YieldTermStructure>& discountingTermStructure);

        MakeCms& withCmsLegTenor(const Period& t);
        MakeCms& withCmsLegCalendar(const Calendar& cal);
        MakeCms& withCmsLegConvention(BusinessDayConvention bdc);
        MakeCms& withCmsLegTerminationDateConvention(BusinessDayConvention bdc);
        MakeCms& withCmsLegForward(bool flag = true);
        MakeCms& withCmsLegEndOfMonth(bool flag = true);
        MakeCms& withCmsLegFirstDate(const Date& d);
        MakeCms& withCmsLegNextToLastDate(const Date& d);
        MakeCms& withCmsLegDayCount(const DayCounter& dc);

        MakeCms& withFloatingLegTenor(const Period& t);
        MakeCms& withFloatingLegCalendar(const Calendar& cal);
        MakeCms& withFloatingLegConvention(BusinessDayConvention bdc);
        MakeCms& withFloatingLegTerminationDateConvention(BusinessDayConvention bdc);
        MakeCms& withFloatingLegForward(bool flag = true);
        MakeCms& withFloatingLegEndOfMonth(bool flag = true);
        MakeCms& withFloatingLegFirstDate(const Date& d);
        MakeCms& withFloatingLegNextToLastDate(const Date& d);
        MakeCms& withFloatingLegDayCount(const DayCounter& dc);
        
      private:
        Period swapTenor_;
        boost::shared_ptr<SwapIndex> swapIndex_;
        Spread iborSpread_;
        boost::shared_ptr<CmsCouponPricer> cmsVanillapricer_;

        Period forwardStart_;

        Spread cmsSpread_;
        Real cmsGearing_;
        Rate cmsCap_, cmsFloor_;

        Date effectiveDate_;
        Calendar cmsCalendar_, floatCalendar_;
        Handle<YieldTermStructure> discountingTermStructure_;
        boost::shared_ptr<IborIndex> iborIndex_;

        bool payCms_;
        Real nominal_;
        Period cmsTenor_, floatTenor_; 
        BusinessDayConvention cmsConvention_, cmsTerminationDateConvention_;
        BusinessDayConvention floatConvention_, floatTerminationDateConvention_;
        bool cmsBackward_, floatBackward_;
        bool cmsEndOfMonth_, floatEndOfMonth_;
        Date cmsFirstDate_, cmsNextToLastDate_;
        Date floatFirstDate_, floatNextToLastDate_;
        DayCounter cmsDayCount_, floatDayCount_;
    };

}

#endif
