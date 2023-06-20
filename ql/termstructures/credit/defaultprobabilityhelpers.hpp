/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2023 Andrea Pellegatta

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

/*! \file defaultprobabilityhelpers.hpp
    \brief bootstrap helpers for default-probability term structures
*/

#ifndef quantlib_default_probability_helpers_hpp
#define quantlib_default_probability_helpers_hpp

#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/time/schedule.hpp>
#include <ql/instruments/creditdefaultswap.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class CreditDefaultSwap;

    //! alias for default-probability bootstrap helpers
    typedef BootstrapHelper<DefaultProbabilityTermStructure>
                                                     DefaultProbabilityHelper;
    typedef RelativeDateBootstrapHelper<DefaultProbabilityTermStructure>
                                         RelativeDateDefaultProbabilityHelper;

    //! Base class for CDS helpers
    class CdsHelper : public RelativeDateDefaultProbabilityHelper {
      public:
        /*! Constructor taking CDS market quote
            @param quote  The helper's market quote.
            @param tenor  CDS tenor.
            @param settlementDays  The number of days from evaluation date to the start of the protection period.
                                   Prior to the CDS Big Bang in 2009, this was typically 1 calendar day. After the 
                                   CDS Big Bang, this is typically 0 calendar days i.e. protection starts 
                                   immediately.
            @param calendar  CDS calendar. Typically weekends only for standard non-JPY CDS and TYO for JPY.
            @param frequency  Coupon frequency. Typically 3 months for standard CDS.
            @param paymentConvention  The convention applied to coupons schedules and settlement dates.
            @param rule  The date generation rule for generating the CDS schedule. Typically, for CDS prior to the 
                         Big Bang, \c OldCDS should be used. After the Big Bang, \c CDS was typical and since 2015 
                         \c CDS2015 is standard.
            @param dayCounter  The day counter for CDS fee leg coupons. Typically it is Actual/360, excluding 
                               accrual end, for all but the final coupon period with Actual/360, including accrual 
                               end, for the final coupon. The \p lastPeriodDayCounter below allows for this 
                               distinction.
            @param recoveryRate  The recovery rate of the underlying reference entity.
            @param discountCurve  A handle to the relevant discount curve.
            @param settlesAccrual  Set to \c true if accrued fee is paid on the occurrence of a credit event and set 
                                   to \c false if it is not. Typically this is \c true.
            @param paysAtDefaultTime  Set to \c true if default payment is made at time of credit event or postponed 
                                      to the end of the coupon period. Typically this is \c true.
            @param startDate  Used to specify an explicit start date for the CDS schedule and the date from which the
                              CDS maturity is calculated via the \p tenor. Useful for off-the-run index schedules.
            @param lastPeriodDayCounter  The day counter for the last fee leg coupon. See comment on \p dayCounter.
            @param rebatesAccrual  Set to \c true if the fee leg accrual is rebated on the cash settlement date. For 
                                   CDS after the Big Bang, this is typically \c true.
            @param model  The pricing model to use for the helper.
        */
        CdsHelper(const Handle<Quote>& quote,
                  const Period& tenor,
                  Integer settlementDays,
                  Calendar calendar,
                  Frequency frequency,
                  BusinessDayConvention paymentConvention,
                  DateGeneration::Rule rule,
                  DayCounter dayCounter,
                  Real recoveryRate,
                  const Handle<YieldTermStructure>& discountCurve,
                  bool settlesAccrual = true,
                  bool paysAtDefaultTime = true,
                  const Date& startDate = Date(),
                  DayCounter lastPeriodDayCounter = DayCounter(),
                  bool rebatesAccrual = true,
                  CreditDefaultSwap::PricingModel model = CreditDefaultSwap::Midpoint);

        //! \copydoc CdsHelper::CdsHelper
        CdsHelper(Rate quote,
                  const Period& tenor,
                  Integer settlementDays,
                  Calendar calendar,
                  Frequency frequency,
                  BusinessDayConvention paymentConvention,
                  DateGeneration::Rule rule,
                  DayCounter dayCounter,
                  Real recoveryRate,
                  const Handle<YieldTermStructure>& discountCurve,
                  bool settlesAccrual = true,
                  bool paysAtDefaultTime = true,
                  const Date& startDate = Date(),
                  DayCounter lastPeriodDayCounter = DayCounter(),
                  bool rebatesAccrual = true,
                  CreditDefaultSwap::PricingModel model = CreditDefaultSwap::Midpoint);
        void setTermStructure(DefaultProbabilityTermStructure*) override;
        ext::shared_ptr<CreditDefaultSwap> swap() const {
            return swap_;
        }
        void update() override;

      protected:
        void initializeDates() override;
        virtual void resetEngine() = 0;
        Period tenor_;
        Integer settlementDays_;
        Calendar calendar_;
        Frequency frequency_;
        BusinessDayConvention paymentConvention_;
        DateGeneration::Rule rule_;
        DayCounter dayCounter_;
        Real recoveryRate_;
        Handle<YieldTermStructure> discountCurve_;
        bool settlesAccrual_;
        bool paysAtDefaultTime_;
        DayCounter lastPeriodDC_;
        bool rebatesAccrual_;
        CreditDefaultSwap::PricingModel model_;

        Schedule schedule_;
        ext::shared_ptr<CreditDefaultSwap> swap_;
        RelinkableHandle<DefaultProbabilityTermStructure> probability_;
        //! protection effective date.
        Date protectionStart_;
        Date startDate_;
    };

    //! Spread-quoted CDS hazard rate bootstrap helper.
    class SpreadCdsHelper : public CdsHelper {
      public:
        SpreadCdsHelper(const Handle<Quote>& runningSpread,
                        const Period& tenor,
                        Integer settlementDays,
                        const Calendar& calendar,
                        Frequency frequency,
                        BusinessDayConvention paymentConvention,
                        DateGeneration::Rule rule,
                        const DayCounter& dayCounter,
                        Real recoveryRate,
                        const Handle<YieldTermStructure>& discountCurve,
                        bool settlesAccrual = true,
                        bool paysAtDefaultTime = true,
                        const Date& startDate = Date(),
                        const DayCounter& lastPeriodDayCounter = DayCounter(),
                        bool rebatesAccrual = true,
                        CreditDefaultSwap::PricingModel model = CreditDefaultSwap::Midpoint);

        SpreadCdsHelper(
            Rate runningSpread,
            const Period& tenor,
            Integer settlementDays, // ISDA: 1
            const Calendar& calendar,
            Frequency frequency,                     // ISDA: Quarterly
            BusinessDayConvention paymentConvention, // ISDA:Following
            DateGeneration::Rule rule,               // ISDA: CDS
            const DayCounter& dayCounter,            // ISDA: Actual/360
            Real recoveryRate,
            const Handle<YieldTermStructure>& discountCurve,
            bool settlesAccrual = true,
            bool paysAtDefaultTime = true,
            const Date& startDate = Date(),
            const DayCounter& lastPeriodDayCounter = DayCounter(), // ISDA: Actual/360(inc)
            bool rebatesAccrual = true,                            // ISDA: true
            CreditDefaultSwap::PricingModel model = CreditDefaultSwap::Midpoint);
        Real impliedQuote() const override;

      private:
        void resetEngine() override;
    };

    //! Upfront-quoted CDS hazard rate bootstrap helper.
    class UpfrontCdsHelper : public CdsHelper {
      public:
        /*! \note the upfront must be quoted in fractional units. */
        UpfrontCdsHelper(const Handle<Quote>& upfront,
                         Rate runningSpread,
                         const Period& tenor,
                         Integer settlementDays,
                         const Calendar& calendar,
                         Frequency frequency,
                         BusinessDayConvention paymentConvention,
                         DateGeneration::Rule rule,
                         const DayCounter& dayCounter,
                         Real recoveryRate,
                         const Handle<YieldTermStructure>& discountCurve,
                         Natural upfrontSettlementDays = 3,
                         bool settlesAccrual = true,
                         bool paysAtDefaultTime = true,
                         const Date& startDate = Date(),
                         const DayCounter& lastPeriodDayCounter = DayCounter(),
                         bool rebatesAccrual = true,
                         CreditDefaultSwap::PricingModel model = CreditDefaultSwap::Midpoint);

        /*! \note the upfront must be quoted in fractional units. */
        UpfrontCdsHelper(Rate upfront,
                         Rate runningSpread,
                         const Period& tenor,
                         Integer settlementDays,
                         const Calendar& calendar,
                         Frequency frequency,
                         BusinessDayConvention paymentConvention,
                         DateGeneration::Rule rule,
                         const DayCounter& dayCounter,
                         Real recoveryRate,
                         const Handle<YieldTermStructure>& discountCurve,
                         Natural upfrontSettlementDays = 3,
                         bool settlesAccrual = true,
                         bool paysAtDefaultTime = true,
                         const Date& startDate = Date(),
                         const DayCounter& lastPeriodDayCounter = DayCounter(),
                         bool rebatesAccrual = true,
                         CreditDefaultSwap::PricingModel model = CreditDefaultSwap::Midpoint);
        Real impliedQuote() const override;

      private:
        Date upfrontDate();
        void initializeDates() override;
        void resetEngine() override;
        Natural upfrontSettlementDays_;
        Date upfrontDate_;
        Rate runningSpread_;
    };

}


#endif
