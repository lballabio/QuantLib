/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano

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

/*! \file makecms.hpp
    \brief Helper class to instantiate standard market CMS.
*/

#ifndef quantlib_makecms_hpp
#define quantlib_makecms_hpp

#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/pricingengine.hpp>

namespace QuantLib {

    class Swap;
    class IborIndex;

    //! helper class for instantiating CMS
    /*! This class provides a more comfortable way
        to instantiate standard market constant maturity swap.
    */
    class MakeCms {
      public:
        MakeCms(const Period& swapTenor,
                const ext::shared_ptr<SwapIndex>& swapIndex,
                const ext::shared_ptr<IborIndex>& iborIndex,
                Spread iborSpread = 0.0,
                const Period& forwardStart = 0*Days);

        MakeCms(const Period& swapTenor,
                const ext::shared_ptr<SwapIndex>& swapIndex,
                Spread iborSpread = 0.0,
                const Period& forwardStart = 0*Days);

        operator Swap() const;
        operator ext::shared_ptr<Swap>() const ;

        MakeCms& receiveCms(bool flag = true);
        MakeCms& withNominal(Real n);
        MakeCms& withEffectiveDate(const Date&);

        MakeCms& withCmsLegTenor(const Period& t);
        MakeCms& withCmsLegCalendar(const Calendar& cal);
        MakeCms& withCmsLegConvention(BusinessDayConvention bdc);
        MakeCms& withCmsLegTerminationDateConvention(BusinessDayConvention);
        MakeCms& withCmsLegRule(DateGeneration::Rule r);
        MakeCms& withCmsLegEndOfMonth(bool flag = true);
        MakeCms& withCmsLegFirstDate(const Date& d);
        MakeCms& withCmsLegNextToLastDate(const Date& d);
        MakeCms& withCmsLegDayCount(const DayCounter& dc);

        MakeCms& withFloatingLegTenor(const Period& t);
        MakeCms& withFloatingLegCalendar(const Calendar& cal);
        MakeCms& withFloatingLegConvention(BusinessDayConvention bdc);
        MakeCms& withFloatingLegTerminationDateConvention(
                                                    BusinessDayConvention bdc);
        MakeCms& withFloatingLegRule(DateGeneration::Rule r);
        MakeCms& withFloatingLegEndOfMonth(bool flag = true);
        MakeCms& withFloatingLegFirstDate(const Date& d);
        MakeCms& withFloatingLegNextToLastDate(const Date& d);
        MakeCms& withFloatingLegDayCount(const DayCounter& dc);

        MakeCms& withAtmSpread(bool flag = true);

        MakeCms& withDiscountingTermStructure(
            const Handle<YieldTermStructure>& discountingTermStructure);
        MakeCms& withCmsCouponPricer(
            const ext::shared_ptr<CmsCouponPricer>& couponPricer);

      private:
        Period swapTenor_;
        ext::shared_ptr<SwapIndex> swapIndex_;
        ext::shared_ptr<IborIndex> iborIndex_;
        Spread iborSpread_;
        bool useAtmSpread_;
        Period forwardStart_;

        Spread cmsSpread_;
        Real cmsGearing_;
        Rate cmsCap_, cmsFloor_;

        Date effectiveDate_;
        Calendar cmsCalendar_, floatCalendar_;

        bool payCms_;
        Real nominal_;
        Period cmsTenor_, floatTenor_;
        BusinessDayConvention cmsConvention_, cmsTerminationDateConvention_;
        BusinessDayConvention floatConvention_, floatTerminationDateConvention_;
        DateGeneration::Rule cmsRule_, floatRule_;
        bool cmsEndOfMonth_, floatEndOfMonth_;
        Date cmsFirstDate_, cmsNextToLastDate_;
        Date floatFirstDate_, floatNextToLastDate_;
        DayCounter cmsDayCount_, floatDayCount_;

        ext::shared_ptr<PricingEngine> engine_;
        ext::shared_ptr<CmsCouponPricer> couponPricer_;
    };

}

#endif


#ifndef id_d85df14e7d9d784771fc9dd2b43a1afb
#define id_d85df14e7d9d784771fc9dd2b43a1afb
inline bool test_d85df14e7d9d784771fc9dd2b43a1afb(const int* i) {
    return i != nullptr;
}
#endif
