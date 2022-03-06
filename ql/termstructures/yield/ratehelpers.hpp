/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2008, 2009, 2015 Ferdinando Ametrano
 Copyright (C) 2007, 2009 Roland Lichters
 Copyright (C) 2015 Maddalena Zanzi
 Copyright (C) 2015 Paolo Mazzocchi
 Copyright (C) 2018 Matthias Lungwitz

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

/*! \file ratehelpers.hpp
    \brief deposit, FRA, futures, and various swap rate helpers
*/

#ifndef quantlib_ratehelpers_hpp
#define quantlib_ratehelpers_hpp

#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/instruments/bmaswap.hpp>
#include <ql/instruments/futures.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/calendars/unitedstates.hpp>

#include <boost/optional.hpp>

namespace QuantLib {

    class SwapIndex;
    class Quote;

    typedef BootstrapHelper<YieldTermStructure> RateHelper;
    typedef RelativeDateBootstrapHelper<YieldTermStructure>
                                                        RelativeDateRateHelper;

    //! Rate helper for bootstrapping over IborIndex futures prices
    class FuturesRateHelper : public RateHelper {
      public:
        FuturesRateHelper(const Handle<Quote>& price,
                          const Date& iborStartDate,
                          Natural lengthInMonths,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          bool endOfMonth,
                          const DayCounter& dayCounter,
                          Handle<Quote> convexityAdjustment = Handle<Quote>(),
                          Futures::Type type = Futures::IMM);
        FuturesRateHelper(Real price,
                          const Date& iborStartDate,
                          Natural lengthInMonths,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          bool endOfMonth,
                          const DayCounter& dayCounter,
                          Rate convexityAdjustment = 0.0,
                          Futures::Type type = Futures::IMM);
        FuturesRateHelper(const Handle<Quote>& price,
                          const Date& iborStartDate,
                          const Date& iborEndDate,
                          const DayCounter& dayCounter,
                          Handle<Quote> convexityAdjustment = Handle<Quote>(),
                          Futures::Type type = Futures::IMM);
        FuturesRateHelper(Real price,
                          const Date& iborStartDate,
                          const Date& endDate,
                          const DayCounter& dayCounter,
                          Rate convexityAdjustment = 0.0,
                          Futures::Type type = Futures::IMM);
        FuturesRateHelper(const Handle<Quote>& price,
                          const Date& iborStartDate,
                          const ext::shared_ptr<IborIndex>& iborIndex,
                          const Handle<Quote>& convexityAdjustment = Handle<Quote>(),
                          Futures::Type type = Futures::IMM);
        FuturesRateHelper(Real price,
                          const Date& iborStartDate,
                          const ext::shared_ptr<IborIndex>& iborIndex,
                          Rate convexityAdjustment = 0.0,
                          Futures::Type type = Futures::IMM);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        //@}
        //! \name FuturesRateHelper inspectors
        //@{
        Real convexityAdjustment() const;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        Time yearFraction_;
        Handle<Quote> convAdj_;
    };


    //! Rate helper for bootstrapping over deposit rates
    class DepositRateHelper : public RelativeDateRateHelper {
      public:
        DepositRateHelper(const Handle<Quote>& rate,
                          const Period& tenor,
                          Natural fixingDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          bool endOfMonth,
                          const DayCounter& dayCounter);
        DepositRateHelper(Rate rate,
                          const Period& tenor,
                          Natural fixingDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          bool endOfMonth,
                          const DayCounter& dayCounter);
        DepositRateHelper(const Handle<Quote>& rate,
                          const ext::shared_ptr<IborIndex>& iborIndex);
        DepositRateHelper(Rate rate,
                          const ext::shared_ptr<IborIndex>& iborIndex);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        void initializeDates() override;
        Date fixingDate_;
        ext::shared_ptr<IborIndex> iborIndex_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };


    //! Rate helper for bootstrapping over %FRA rates
    class FraRateHelper : public RelativeDateRateHelper {
      public:
        FraRateHelper(const Handle<Quote>& rate,
                      Natural monthsToStart,
                      Natural monthsToEnd,
                      Natural fixingDays,
                      const Calendar& calendar,
                      BusinessDayConvention convention,
                      bool endOfMonth,
                      const DayCounter& dayCounter,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(Rate rate,
                      Natural monthsToStart,
                      Natural monthsToEnd,
                      Natural fixingDays,
                      const Calendar& calendar,
                      BusinessDayConvention convention,
                      bool endOfMonth,
                      const DayCounter& dayCounter,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(const Handle<Quote>& rate,
                      Natural monthsToStart,
                      const ext::shared_ptr<IborIndex>& iborIndex,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(Rate rate,
                      Natural monthsToStart,
                      const ext::shared_ptr<IborIndex>& iborIndex,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(const Handle<Quote>& rate,
                      Period periodToStart,
                      Natural lengthInMonths,
                      Natural fixingDays,
                      const Calendar& calendar,
                      BusinessDayConvention convention,
                      bool endOfMonth,
                      const DayCounter& dayCounter,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(Rate rate,
                      Period periodToStart,
                      Natural lengthInMonths,
                      Natural fixingDays,
                      const Calendar& calendar,
                      BusinessDayConvention convention,
                      bool endOfMonth,
                      const DayCounter& dayCounter,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(const Handle<Quote>& rate,
                      Period periodToStart,
                      const ext::shared_ptr<IborIndex>& iborIndex,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(Rate rate,
                      Period periodToStart,
                      const ext::shared_ptr<IborIndex>& iborIndex,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(const Handle<Quote>& rate,
                      Natural immOffsetStart,
                      Natural immOffsetEnd,
                      const ext::shared_ptr<IborIndex>& iborIndex,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        FraRateHelper(Rate rate,
                      Natural immOffsetStart,
                      Natural immOffsetEnd,
                      const ext::shared_ptr<IborIndex>& iborIndex,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      bool useIndexedCoupon = true);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        void initializeDates() override;
        Date fixingDate_;
        boost::optional<Period> periodToStart_;
        boost::optional<Natural> immOffsetStart_, immOffsetEnd_;
        Pillar::Choice pillarChoice_;
        ext::shared_ptr<IborIndex> iborIndex_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
        bool useIndexedCoupon_;
        Real spanningTime_;
    };


    //! Rate helper for bootstrapping over swap rates
    /*! \todo use input SwapIndex to create the swap */
    class SwapRateHelper : public RelativeDateRateHelper {
      public:
        SwapRateHelper(const Handle<Quote>& rate,
                       const ext::shared_ptr<SwapIndex>& swapIndex,
                       Handle<Quote> spread = Handle<Quote>(),
                       const Period& fwdStart = 0 * Days,
                       // exogenous discounting curve
                       Handle<YieldTermStructure> discountingCurve = Handle<YieldTermStructure>(),
                       Pillar::Choice pillar = Pillar::LastRelevantDate,
                       Date customPillarDate = Date(),
                       bool endOfMonth = false,
                       const boost::optional<bool>& useIndexedCoupons = boost::none);
        SwapRateHelper(const Handle<Quote>& rate,
                       const Period& tenor,
                       Calendar calendar,
                       // fixed leg
                       Frequency fixedFrequency,
                       BusinessDayConvention fixedConvention,
                       DayCounter fixedDayCount,
                       // floating leg
                       const ext::shared_ptr<IborIndex>& iborIndex,
                       Handle<Quote> spread = Handle<Quote>(),
                       const Period& fwdStart = 0 * Days,
                       // exogenous discounting curve
                       Handle<YieldTermStructure> discountingCurve = Handle<YieldTermStructure>(),
                       Natural settlementDays = Null<Natural>(),
                       Pillar::Choice pillar = Pillar::LastRelevantDate,
                       Date customPillarDate = Date(),
                       bool endOfMonth = false,
                       const boost::optional<bool>& useIndexedCoupons = boost::none);
        SwapRateHelper(Rate rate,
                       const ext::shared_ptr<SwapIndex>& swapIndex,
                       Handle<Quote> spread = Handle<Quote>(),
                       const Period& fwdStart = 0 * Days,
                       // exogenous discounting curve
                       Handle<YieldTermStructure> discountingCurve = Handle<YieldTermStructure>(),
                       Pillar::Choice pillar = Pillar::LastRelevantDate,
                       Date customPillarDate = Date(),
                       bool endOfMonth = false,
                       const boost::optional<bool>& useIndexedCoupons = boost::none);
        SwapRateHelper(Rate rate,
                       const Period& tenor,
                       Calendar calendar,
                       // fixed leg
                       Frequency fixedFrequency,
                       BusinessDayConvention fixedConvention,
                       DayCounter fixedDayCount,
                       // floating leg
                       const ext::shared_ptr<IborIndex>& iborIndex,
                       Handle<Quote> spread = Handle<Quote>(),
                       const Period& fwdStart = 0 * Days,
                       // exogenous discounting curve
                       Handle<YieldTermStructure> discountingCurve = Handle<YieldTermStructure>(),
                       Natural settlementDays = Null<Natural>(),
                       Pillar::Choice pillar = Pillar::LastRelevantDate,
                       Date customPillarDate = Date(),
                       bool endOfMonth = false,
                       const boost::optional<bool>& useIndexedCoupons = boost::none);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        //@}
        //! \name SwapRateHelper inspectors
        //@{
        Spread spread() const;
        ext::shared_ptr<VanillaSwap> swap() const;
        const Period& forwardStart() const;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        void initializeDates() override;
        Natural settlementDays_;
        Period tenor_;
        Pillar::Choice pillarChoice_;
        Calendar calendar_;
        BusinessDayConvention fixedConvention_;
        Frequency fixedFrequency_;
        DayCounter fixedDayCount_;
        ext::shared_ptr<IborIndex> iborIndex_;
        ext::shared_ptr<VanillaSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
        Handle<Quote> spread_;
        bool endOfMonth_;
        Period fwdStart_;
        Handle<YieldTermStructure> discountHandle_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;
        boost::optional<bool> useIndexedCoupons_;
    };


    //! Rate helper for bootstrapping over BMA swap rates
    class BMASwapRateHelper : public RelativeDateRateHelper {
      public:
        BMASwapRateHelper(const Handle<Quote>& liborFraction,
                          const Period& tenor, // swap maturity
                          Natural settlementDays,
                          Calendar calendar,
                          // bma leg
                          const Period& bmaPeriod,
                          BusinessDayConvention bmaConvention,
                          DayCounter bmaDayCount,
                          ext::shared_ptr<BMAIndex> bmaIndex,
                          // ibor leg
                          ext::shared_ptr<IborIndex> index);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
    protected:
      void initializeDates() override;
      Period tenor_;
      Natural settlementDays_;
      Calendar calendar_;
      Period bmaPeriod_;
      BusinessDayConvention bmaConvention_;
      DayCounter bmaDayCount_;
      ext::shared_ptr<BMAIndex> bmaIndex_;
      ext::shared_ptr<IborIndex> iborIndex_;

      ext::shared_ptr<BMASwap> swap_;
      RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };


    //! Rate helper for bootstrapping over Fx Swap rates
    /*! The forward is given by `fwdFx = spotFx + fwdPoint`.

        `isFxBaseCurrencyCollateralCurrency` indicates if the base
        currency of the FX currency pair is the one used as collateral.

        `calendar` is usually the joint calendar of the two currencies
        in the pair.

        `tradingCalendar` can be used when the cross pairs don't
        include the currency of the business center (usually USD; the
        corresponding calendar is `UnitedStates`).  If given, it will
        be used for adjusting the earliest settlement date and for
        setting the latest date. Due to FX spot market conventions, it
        is not sufficient to pass a JointCalendar with UnitedStates
        included as `calendar`; with regard the earliest date, this
        calendar is only used in case the spot date of the two
        currencies is not a US business day.

        \warning The ON fx swaps can be achieved by setting
                 `fixingDays` to 0 and using a tenor of '1d'. The same
                 tenor should be used for TN swaps, with `fixingDays`
                 set to 1.  However, handling ON and TN swaps for
                 cross rates without USD is not trivial and should be
                 treated with caution. If today is a US holiday, ON
                 trade is not possible. If tomorrow is a US Holiday,
                 the ON trade will be at least two business days long
                 in the other countries and the TN trade will not
                 exist. In such cases, if this helper is used for
                 curve construction, probably it is safer not to pass
                 a trading calendar to the ON and TN helpers and
                 provide fwdPoints that will yield proper level of
                 discount factors.
    */
    class FxSwapRateHelper : public RelativeDateRateHelper {
      public:
        FxSwapRateHelper(const Handle<Quote>& fwdPoint,
                         Handle<Quote> spotFx,
                         const Period& tenor,
                         Natural fixingDays,
                         Calendar calendar,
                         BusinessDayConvention convention,
                         bool endOfMonth,
                         bool isFxBaseCurrencyCollateralCurrency,
                         Handle<YieldTermStructure> collateralCurve,
                         Calendar tradingCalendar = Calendar());
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        //@}
        //! \name FxSwapRateHelper inspectors
        //@{
        Real spot() const { return spot_->value(); }
        Period tenor() const { return tenor_; }
        Natural fixingDays() const { return fixingDays_; }
        Calendar calendar() const { return cal_; }
        BusinessDayConvention businessDayConvention() const { return conv_; }
        bool endOfMonth() const { return eom_; }
        bool isFxBaseCurrencyCollateralCurrency() const {
                                return isFxBaseCurrencyCollateralCurrency_; }
        Calendar tradingCalendar() const { return tradingCalendar_; }
        Calendar adjustmentCalendar() const { return jointCalendar_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
    private:
      void initializeDates() override;
      Handle<Quote> spot_;
      Period tenor_;
      Natural fixingDays_;
      Calendar cal_;
      BusinessDayConvention conv_;
      bool eom_;
      bool isFxBaseCurrencyCollateralCurrency_;

      RelinkableHandle<YieldTermStructure> termStructureHandle_;

      Handle<YieldTermStructure> collHandle_;
      RelinkableHandle<YieldTermStructure> collRelinkableHandle_;

      Calendar tradingCalendar_;
      Calendar jointCalendar_;
    };

    // inline

    inline Spread SwapRateHelper::spread() const {
        return spread_.empty() ? 0.0 : spread_->value();
    }

    inline ext::shared_ptr<VanillaSwap> SwapRateHelper::swap() const {
        return swap_;
    }

    inline const Period& SwapRateHelper::forwardStart() const {
        return fwdStart_;
    }

}

#endif


#ifndef id_de7ba510f5d2b82afde700704872c472
#define id_de7ba510f5d2b82afde700704872c472
inline bool test_de7ba510f5d2b82afde700704872c472(const int* i) {
    return i != nullptr;
}
#endif
