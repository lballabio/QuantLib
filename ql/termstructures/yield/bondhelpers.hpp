/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Toyin Akin
 Copyright (C) 2007, 2009 StatPro Italia srl
 Copyright (C) 2008 Ferdinando Ametrano

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

/*! \file bondhelpers.hpp
    \brief bond rate helpers
*/

#ifndef quantlib_bond_helpers_hpp
#define quantlib_bond_helpers_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/instruments/bonds/cpibond.hpp>
#include <ql/cashflows/cpicoupon.hpp>

namespace QuantLib {

    //! Bond helper for curve bootstrap
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
    */
    class BondHelper : public RateHelper {
      public:
        /*! \warning Setting a pricing engine to the passed bond from
                     external code will cause the bootstrap to fail or
                     to give wrong results. It is advised to discard
                     the bond after creating the helper, so that the
                     helper has sole ownership of it.
        */
        BondHelper(const Handle<Quote>& price,
                   const ext::shared_ptr<Bond>& bond,
                   bool useCleanPrice = true);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        void setTermStructure(YieldTermStructure*);
        //@}
        //! \name Additional inspectors
        //@{
        ext::shared_ptr<Bond> bond() const;
        bool useCleanPrice() const;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
      protected:
        ext::shared_ptr<Bond> bond_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
        bool useCleanPrice_;        
    };

    //! Fixed-coupon bond helper for curve bootstrap
    class FixedRateBondHelper : public BondHelper {
      public:
        FixedRateBondHelper(const Handle<Quote>& price,
                            Natural settlementDays,
                            Real faceAmount,
                            const Schedule& schedule,
                            const std::vector<Rate>& coupons,
                            const DayCounter& dayCounter,
                            BusinessDayConvention paymentConv = Following,
                            Real redemption = 100.0,
                            const Date& issueDate = Date(),
                            const Calendar& paymentCalendar = Calendar(),
                            const Period& exCouponPeriod = Period(),
                            const Calendar& exCouponCalendar = Calendar(),
                            const BusinessDayConvention exCouponConvention = Unadjusted,
                            bool exCouponEndOfMonth = false,
                            const bool useCleanPrice = true);
        //! \name Additional inspectors
        //@{
        ext::shared_ptr<FixedRateBond> fixedRateBond() const;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
      protected:
        ext::shared_ptr<FixedRateBond> fixedRateBond_;
    };


    // inline

    inline ext::shared_ptr<Bond> BondHelper::bond() const {
        return bond_;
    }

    inline bool BondHelper::useCleanPrice() const {
        return useCleanPrice_;
    }

    inline ext::shared_ptr<FixedRateBond>
    FixedRateBondHelper::fixedRateBond() const {
        return fixedRateBond_;
    }

    //! CPI bond helper for curve bootstrap
    class CPIBondHelper : public BondHelper {
      public:
        CPIBondHelper(const Handle<Quote>& price,
                            Natural settlementDays,
                            Real faceAmount,
                            const bool growthOnly,
                            Real baseCPI,
                            const Period& observationLag,
                            const ext::shared_ptr<ZeroInflationIndex>& cpiIndex,
                            CPI::InterpolationType observationInterpolation,
                            const Schedule& schedule,
                            const std::vector<Rate>& fixedRate,
                            const DayCounter& accrualDayCounter,
                            BusinessDayConvention paymentConvention = Following,
                            const Date& issueDate = Date(),
                            const Calendar& paymentCalendar = Calendar(),
                            const Period& exCouponPeriod = Period(),
                            const Calendar& exCouponCalendar = Calendar(),
                            const BusinessDayConvention exCouponConvention = Unadjusted,
                            bool exCouponEndOfMonth = false,
                            const bool useCleanPrice = true);
        //! \name Additional inspectors
        //@{
        ext::shared_ptr<CPIBond> cpiBond() const;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
      protected:
        ext::shared_ptr<CPIBond> cpiBond_;
    };


    inline ext::shared_ptr<CPIBond>
    CPIBondHelper::cpiBond() const {
        return cpiBond_;
    }
}

#endif
