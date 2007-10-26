/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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
    \brief deposit, FRA, futures, and swap rate helpers
*/

#ifndef quantlib_ratehelpers_hpp
#define quantlib_ratehelpers_hpp

#include <ql/termstructures/yield/ratehelper.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over interest-rate futures prices
    /*! \todo implement/refactor constructors with:
              Index instead of (nMonths, calendar, convention, dayCounter),
              IMM code
    */
    class FuturesRateHelper : public RateHelper {
      public:
        FuturesRateHelper(const Handle<Quote>& price,
                          const Date& immDate,
                          Size nMonths,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter,
                          const Handle<Quote>& convexityAdjustment);
        FuturesRateHelper(const Handle<Quote>& price,
                          const Date& immDate,
                          Size nMonths,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter,
                          Rate convexityAdjustment = 0.0);
        FuturesRateHelper(Real price,
                          const Date& immDate,
                          Size nMonths,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter,
                          Rate convexityAdjustment = 0.0);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        DiscountFactor discountGuess() const;
        //@}
        //! \name FuturesRateHelper inspectors
        //@{
        Real convexityAdjustment() const;
        //@}
      private:
        Time yearFraction_;
        Handle<Quote> convAdj_;
    };

    //! Rate helper with date schedule relative to the global evaluation date
    /*! This class takes care of rebuilding the date schedule when the global
        evaluation date changes
    */
    class RelativeDateRateHelper : public RateHelper {
      public:
        RelativeDateRateHelper(const Handle<Quote>& quote);
        RelativeDateRateHelper(Real quote);
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        virtual void initializeDates() = 0;
        Date evaluationDate_;
    };


    //! Rate helper for bootstrapping over deposit rates
    class DepositRateHelper : public RelativeDateRateHelper {
      public:
        DepositRateHelper(const Handle<Quote>& rate,
                          const Period& tenor,
                          Natural settlementDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          bool endOfMonth,
                          Natural fixingDays,
                          const DayCounter& dayCounter);
        DepositRateHelper(Rate rate,
                          const Period& tenor,
                          Natural settlementDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          bool endOfMonth,
                          Natural fixingDays,
                          const DayCounter& dayCounter);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        DiscountFactor discountGuess() const;
        void setTermStructure(YieldTermStructure*);
        //@}
      private:
        void initializeDates();
        Date fixingDate_;
        Natural settlementDays_;
        boost::shared_ptr<IborIndex> index_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };


    //! Rate helper for bootstrapping over %FRA rates
    class FraRateHelper : public RelativeDateRateHelper {
      public:
        FraRateHelper(const Handle<Quote>& rate,
                      Natural monthsToStart,
                      Natural monthsToEnd,
                      Natural settlementDays,
                      const Calendar& calendar,
                      BusinessDayConvention convention,
                      bool endOfMonth,
                      Natural fixingDays,
                      const DayCounter& dayCounter);
        FraRateHelper(Rate rate,
                      Natural monthsToStart,
                      Natural monthsToEnd,
                      Natural settlementDays,
                      const Calendar& calendar,
                      BusinessDayConvention convention,
                      bool endOfMonth,
                      Natural fixingDays,
                      const DayCounter& dayCounter);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        DiscountFactor discountGuess() const;
        void setTermStructure(YieldTermStructure*);
        //@}
      private:
        void initializeDates();
        Date fixingDate_;
        Natural monthsToStart_;
        Natural settlementDays_;
        boost::shared_ptr<IborIndex> index_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

    //! Rate helper for bootstrapping over swap rates
    class SwapRateHelper : public RelativeDateRateHelper {
      public:
        SwapRateHelper(const Handle<Quote>& rate,
                       const Period& tenor,
                       Natural settlementDays,
                       const Calendar& calendar,
                       // fixed leg
                       Frequency fixedFrequency,
                       BusinessDayConvention fixedConvention,
                       const DayCounter& fixedDayCount,
                       // floating leg
                       const boost::shared_ptr<IborIndex>& index,
                       const Handle<Quote>& spread = Handle<Quote>());
        SwapRateHelper(Rate rate,
                       const Period& tenor,
                       Natural settlementDays,
                       const Calendar& calendar,
                       // fixed leg
                       Frequency fixedFrequency,
                       BusinessDayConvention fixedConvention,
                       const DayCounter& fixedDayCount,
                       // floating leg
                       const boost::shared_ptr<IborIndex>& index,
                       const Handle<Quote>& spread = Handle<Quote>());
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        // implementing discountGuess() is not worthwhile,
        // and may not avoid the root-finding process
        void setTermStructure(YieldTermStructure*);
        //@}
        //! \name SwapRateHelper inspectors
        //@{
        Spread spread() const;
        boost::shared_ptr<VanillaSwap> swap() const;
        //@}
      protected:
        void initializeDates();
        Period tenor_;
        Natural settlementDays_;
        Calendar calendar_;
        BusinessDayConvention fixedConvention_;
        Frequency fixedFrequency_;
        DayCounter fixedDayCount_;
        boost::shared_ptr<IborIndex> index_;
        boost::shared_ptr<VanillaSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
        Handle<Quote> spread_;
    };

}

#endif
