/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Jose Aparicio
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file creditdefaultswap.hpp
    \brief Credit default swap
*/

#ifndef quantlib_credit_default_swap_hpp
#define quantlib_credit_default_swap_hpp

#include <ql/instrument.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/default.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class Claim;

    //! Credit default swap
    /*! \note This instrument currently assumes that the issuer did
              not default until today's date.

        \warning if <tt>Settings::includeReferenceDateCashFlows()</tt>
                 is set to <tt>true</tt>, payments occurring at the
                 settlement date of the swap might be included in the
                 NPV and therefore affect the fair-spread
                 calculation. This might not be what you want.

        \warning conventionalSpread (and impliedHazardRate) by default
                 use the mid-point engine, which is not ISDA conform.

        \ingroup instruments
    */
    class CreditDefaultSwap : public Instrument {
      public:
        class arguments;
        class results;
        class engine;
        enum PricingModel {
            Midpoint,
            ISDA
        };
        //! \name Constructors
        //@{
        //! CDS quoted as running-spread only
        /*! @param side  Whether the protection is bought or sold.
            @param notional  Notional value
            @param spread  Running spread in fractional units.
            @param schedule  Coupon schedule.
            @param paymentConvention  Business-day convention for
                                      payment-date adjustment.
            @param dayCounter  Day-count convention for accrual.
            @param settlesAccrual  Whether or not the accrued coupon is
                                   due in the event of a default.
            @param paysAtDefaultTime  If set to true, any payments
                                      triggered by a default event are
                                      due at default time. If set to
                                      false, they are due at the end of
                                      the accrual period.
            @param protectionStart  The first date where a default event will trigger the contract. 
                                    Before the CDS Big Bang 2009, this was typically trade date (T) + 1 calendar day.
                                    After the CDS Big Bang 2009, protection is typically effective immediately i.e. on 
                                    trade date so this is what should be entered for protection start.
                                    Notice that there is no default lookback period and protection start here. 
                                    In the way it determines the dirty amount it is more like the trade execution date.
            @param lastPeriodDayCounter Day-count convention for accrual in last period
            @param rebatesAccrual  The protection seller pays the accrued 
                                    scheduled current coupon at the start 
                                    of the contract. The rebate date is not
                                    provided but computed to be two days after
                                    protection start.
            @param tradeDate  The contract's trade date. It will be used with the \p cashSettlementDays to determine 
                              the date on which the cash settlement amount is paid. If not given, the trade date is 
                              guessed from the protection start date and \p schedule date generation rule.
            @param cashSettlementDays  The number of business days from \p tradeDate to cash settlement date.
        */
        CreditDefaultSwap(Protection::Side side,
                          Real notional,
                          Rate spread,
                          const Schedule& schedule,
                          BusinessDayConvention paymentConvention,
                          const DayCounter& dayCounter,
                          bool settlesAccrual = true,
                          bool paysAtDefaultTime = true,
                          const Date& protectionStart = Date(),
                          ext::shared_ptr<Claim> = ext::shared_ptr<Claim>(),
                          const DayCounter& lastPeriodDayCounter = DayCounter(),
                          bool rebatesAccrual = true,
                          const Date& tradeDate = Date(),
                          Natural cashSettlementDays = 3);
        //! CDS quoted as upfront and running spread
        /*! @param side  Whether the protection is bought or sold.
            @param notional  Notional value
            @param upfront Upfront in fractional units.
            @param spread Running spread in fractional units.
            @param schedule  Coupon schedule.
            @param paymentConvention  Business-day convention for
                                      payment-date adjustment.
            @param dayCounter  Day-count convention for accrual.
            @param settlesAccrual Whether or not the accrued coupon is
                                  due in the event of a default.
            @param paysAtDefaultTime If set to true, any payments
                                     triggered by a default event are
                                     due at default time. If set to
                                     false, they are due at the end of
                                     the accrual period.
            @param protectionStart  The first date where a default event will trigger the contract. 
                                    Before the CDS Big Bang 2009, this was typically trade date (T) + 1 calendar day.
                                    After the CDS Big Bang 2009, protection is typically effective immediately i.e. on 
                                    trade date so this is what should be entered for protection start.
                                    Notice that there is no default lookback period and protection start here. 
                                    In the way it determines the dirty amount it is more like the trade execution date.
            @param upfrontDate Settlement date for the upfront and accrual 
                                    rebate (if any) payments.
                                    Typically T+3, this is also the default 
                                    value.
            @param lastPeriodDayCounter Day-count convention for accrual in last period
            @param rebatesAccrual  The protection seller pays the accrued 
                                    scheduled current coupon at the start 
                                    of the contract. The rebate date is not
                                    provided but computed to be two days after
                                    protection start.
            @param tradeDate  The contract's trade date. It will be used with the \p cashSettlementDays to determine 
                              the date on which the cash settlement amount is paid if \p upfrontDate is empty. If not 
                              given, the trade date is guessed from the protection start date and \p schedule date 
                              generation rule.
            @param cashSettlementDays  The number of business days from \p tradeDate to cash settlement date.
        */
        CreditDefaultSwap(Protection::Side side,
                          Real notional,
                          Rate upfront,
                          Rate spread,
                          const Schedule& schedule,
                          BusinessDayConvention paymentConvention,
                          const DayCounter& dayCounter,
                          bool settlesAccrual = true,
                          bool paysAtDefaultTime = true,
                          const Date& protectionStart = Date(),
                          const Date& upfrontDate = Date(),
                          ext::shared_ptr<Claim> = ext::shared_ptr<Claim>(),
                          const DayCounter& lastPeriodDayCounter = DayCounter(),
                          bool rebatesAccrual = true,
                          const Date& tradeDate = Date(),
                          Natural cashSettlementDays = 3);
        //@}
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results*) const override;
        //@}
        //! \name Inspectors
        //@{
        Protection::Side side() const;
        Real notional() const;
        Rate runningSpread() const;
        ext::optional<Rate> upfront() const;
        bool settlesAccrual() const;
        bool paysAtDefaultTime() const;
        const Leg& coupons() const;
        //! The first date for which defaults will trigger the contract
        const Date& protectionStartDate() const;
        //! The last date for which defaults will trigger the contract
        const Date& protectionEndDate() const;
        bool rebatesAccrual() const { return accrualRebate_ != nullptr; }
        const ext::shared_ptr<SimpleCashFlow>& upfrontPayment() const;
        const ext::shared_ptr<SimpleCashFlow>& accrualRebate() const;
        const Date& tradeDate() const;
        Natural cashSettlementDays() const;
        //@}
        //! \name Results
        //@{
        /*! Returns the upfront spread that, given the running spread
            and the quoted recovery rate, will make the instrument
            have an NPV of 0.
        */
        Rate fairUpfront() const;
        /*! Returns the running spread that, given the quoted recovery
            rate, will make the running-only CDS have an NPV of 0.

            \note This calculation does not take any upfront into
                  account, even if one was given.
        */
        Rate fairSpread() const;
        /*! Returns the variation of the fixed-leg value given a
            one-basis-point change in the running spread.
        */
        Real couponLegBPS() const;
        Real upfrontBPS() const;
        Real couponLegNPV() const;
        Real defaultLegNPV() const;
        Real upfrontNPV() const;
        Real accrualRebateNPV() const;

        //! Implied hazard rate calculation
        /*! \note This method performs the calculation with the
                  instrument characteristics. It will coincide with
                  the ISDA calculation if your object has the standard
                  characteristics. Notably:
                  - The calendar should have no bank holidays, just
                    weekends.
                  - The yield curve should be LIBOR piecewise constant
                    in fwd rates, with a discount factor of 1 on the
                    calculation date, which coincides with the trade
                    date.
                  - Convention should be Following for yield curve and
                    contract cashflows.
                  - The CDS should pay accrued and mature on standard
                    IMM dates, settle on trade date +1 and upfront
                    settle on trade date +3.
        */
        Rate impliedHazardRate(Real targetNPV,
                               const Handle<YieldTermStructure>& discountCurve,
                               const DayCounter& dayCounter,
                               Real recoveryRate = 0.4,
                               Real accuracy = 1.0e-8,
                               PricingModel model = Midpoint) const;

        //! Conventional/standard upfront-to-spread conversion
        /*! Under a standard ISDA model and a set of standardised
            instrument characteristics, it is the running only quoted
            spread that will make a CDS contract have an NPV of 0 when
            quoted for that running only spread.  Refer to: "ISDA
            Standard CDS converter specification." May 2009.

            The conventional recovery rate to apply in the calculation
            is as specified by ISDA, not necessarily equal to the
            market-quoted one.  It is typically 0.4 for SeniorSec and
            0.2 for subordinate.

            \note The conversion employs a flat hazard rate. As a result,
                  you will not recover the market quotes.

            \note This method performs the calculation with the
                  instrument characteristics. It will coincide with
                  the ISDA calculation if your object has the standard
                  characteristics. Notably:
                  - The calendar should have no bank holidays, just
                    weekends.
                  - The yield curve should be LIBOR piecewise constant
                    in fwd rates, with a discount factor of 1 on the
                    calculation date, which coincides with the trade
                    date.
                  - Convention should be Following for yield curve and
                    contract cashflows.
                  - The CDS should pay accrued and mature on standard
                    IMM dates, settle on trade date +1 and upfront
                    settle on trade date +3.
        */
        Rate conventionalSpread(Real conventionalRecovery,
                                const Handle<YieldTermStructure>& discountCurve,
                                const DayCounter& dayCounter,
                                PricingModel model = Midpoint) const;
        //@}
      protected:
        //! \name Instrument interface
        //@{
        void setupExpired() const override;
        //@}
        // data members
        Protection::Side side_;
        Real notional_;
        ext::optional<Rate> upfront_;
        Rate runningSpread_;
        bool settlesAccrual_, paysAtDefaultTime_;
        ext::shared_ptr<Claim> claim_;
        Leg leg_;
        ext::shared_ptr<SimpleCashFlow> upfrontPayment_;
        ext::shared_ptr<SimpleCashFlow> accrualRebate_;
        Date protectionStart_;
        Date tradeDate_;
        Natural cashSettlementDays_;
        Date maturity_;
        // results
        mutable Rate fairUpfront_;
        mutable Rate fairSpread_;
        mutable Real couponLegBPS_, couponLegNPV_;
        mutable Real upfrontBPS_, upfrontNPV_;
        mutable Real defaultLegNPV_;
        mutable Real accrualRebateNPV_;

      private:
        //! Shared initialisation.
        void init(const Schedule& schedule, BusinessDayConvention paymentConvention, const DayCounter& dayCounter,
            const DayCounter& lastPeriodDayCounter, bool rebatesAccrual, const Date& upfrontDate = Date());
    };


    class CreditDefaultSwap::arguments
        : public virtual PricingEngine::arguments {
      public:
        arguments();
        Protection::Side side;
        Real notional;
        ext::optional<Rate> upfront;
        Rate spread;
        Leg leg;
        // if not initialized by constructors means theres no flows.
        ext::shared_ptr<SimpleCashFlow> upfrontPayment;
        ext::shared_ptr<SimpleCashFlow> accrualRebate;
        bool settlesAccrual;
        bool paysAtDefaultTime;
        ext::shared_ptr<Claim> claim;
        Date protectionStart;
        Date maturity;
        void validate() const override;
    };

    class CreditDefaultSwap::results : public Instrument::results {
      public:
        Rate fairSpread;
        Rate fairUpfront;
        Real couponLegBPS;
        Real couponLegNPV;
        Real defaultLegNPV;
        Real upfrontBPS;
        Real upfrontNPV;
        Real accrualRebateNPV;
        void reset() override;
    };

    class CreditDefaultSwap::engine
        : public GenericEngine<CreditDefaultSwap::arguments,
                               CreditDefaultSwap::results> {};

    /*! Return the CDS maturity date given the CDS trade date, \p tradeDate, the CDS \p tenor and a CDS \p rule.

        A null date is returned when a \p rule of \c CDS2015 and a \p tenor length of zero fail to yield a valid
        CDS maturity date.

        \warning An exception will be thrown if the \p rule is not \c CDS2015, \c CDS or \c OldCDS.

        \warning An exception will be thrown if the \p rule is \c OldCDS and a \p tenor of 0 months is provided. This
                 restriction can be removed if 0M tenor was available before the CDS Big Bang 2009.

        \warning An exception will be thrown if the \p tenor is not a multiple of 3 months. For the avoidance of
                 doubt, a \p tenor of 0 months is supported.
    */
    Date cdsMaturity(const Date& tradeDate, const Period& tenor, DateGeneration::Rule rule);

}


#endif
