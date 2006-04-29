/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2006 StatPro Italia srl

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

/*! \file ratehelpers.hpp
    \brief deposit, FRA, Futures, and Swap rate helpers
*/

#ifndef quantlib_ratehelpers_hpp
#define quantlib_ratehelpers_hpp

#include <ql/ratehelper.hpp>
#include <ql/Instruments/simpleswap.hpp>

namespace QuantLib {


    //! Interest-rate futures helper
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().

        \todo convexity adjustment should be implemented.
    */
    class FuturesRateHelper : public RateHelper {
      public:
        FuturesRateHelper(const Handle<Quote>& price,
                          const Date& immDate,
                          Integer nMonths,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter);
	    FuturesRateHelper(const Handle<Quote>& price,
                          const Date& immDate,
                          const Date& matDate,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter);
        FuturesRateHelper(Real price,
                          const Date& immDate,
                          Integer nMonths,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter);
        Real impliedQuote() const;
        DiscountFactor discountGuess() const;
      private:
        Time yearFraction_;
    };

    //! Rate helper with date schedule relative to the global evaluation date
    /*! This class takes care of rebuilding the date schedule when the global
        evaluation date changes
    */
    class RelativeDateRateHelper : public RateHelper {
      public:
        RelativeDateRateHelper(const Handle<Quote>& quote);
        RelativeDateRateHelper(Real quote);
        void update() {
            RateHelper::update();
            if (evaluationDate_ != Settings::instance().evaluationDate()) {
                evaluationDate_ = Settings::instance().evaluationDate();
                initializeDates_();
            }
        }
      protected:
        virtual void initializeDates_() = 0;
        Date evaluationDate_;
    };


    //! Deposit rate helper
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
    */
    class DepositRateHelper : public RelativeDateRateHelper {
      public:
        DepositRateHelper(const Handle<Quote>& rate,
                          Integer n, TimeUnit units,
                          Integer settlementDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter);
        DepositRateHelper(Rate rate,
                          Integer n, TimeUnit units,
                          Integer settlementDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter);
        Real impliedQuote() const;
        DiscountFactor discountGuess() const;
        void setTermStructure(YieldTermStructure*);
      private:
        void initializeDates_();
        Integer n_;
        TimeUnit units_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        Time yearFraction_;
    };


    //! Forward rate agreement helper
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
    */
    class FraRateHelper : public RelativeDateRateHelper {
      public:
        FraRateHelper(const Handle<Quote>& rate,
                      Integer monthsToStart, Integer monthsToEnd,
                      Integer settlementDays,
                      const Calendar& calendar,
                      BusinessDayConvention convention,
                      const DayCounter& dayCounter);
        FraRateHelper(Rate rate,
                      Integer monthsToStart, Integer monthsToEnd,
                      Integer settlementDays,
                      const Calendar& calendar,
                      BusinessDayConvention convention,
                      const DayCounter& dayCounter);
        Real impliedQuote() const;
        DiscountFactor discountGuess() const;
        void setTermStructure(YieldTermStructure*);
      private:
        void initializeDates_();
        Integer monthsToStart_, monthsToEnd_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        Time yearFraction_;
    };

    //! %Swap rate helper
    /*! \todo currency and day counter of Xibor should be added to
              obtain well-defined SwapRateHelper
		\warning This class assumes that the settlement date
                 does not change between calls of setTermStructure().
    */
    class SwapRateHelper : public RelativeDateRateHelper {
      public:
        SwapRateHelper(const Handle<Quote>& rate,
                       Integer n, TimeUnit units,
                       Integer settlementDays,
                       const Calendar& calendar,
                       // fixed leg
                       Frequency fixedFrequency,
                       BusinessDayConvention fixedConvention,
                       const DayCounter& fixedDayCount,
                       // floating leg
                       Frequency floatingFrequency,
                       BusinessDayConvention floatingConvention,
                       const DayCounter& floatingDayCount);
        SwapRateHelper(Rate rate,
                       Integer n, TimeUnit units,
                       Integer settlementDays,
                       const Calendar& calendar,
                       // fixed leg
                       Frequency fixedFrequency,
                       BusinessDayConvention fixedConvention,
                       const DayCounter& fixedDayCount,
                       // floating leg
                       Frequency floatingFrequency,
                       BusinessDayConvention floatingConvention,
                       const DayCounter& floatingDayCount);
        Real impliedQuote() const;
        // implementing discountGuess() is not worthwhile,
        // and may not avoid the root-finding process
        void setTermStructure(YieldTermStructure*);
      private:
        void initializeDates_();
        Integer n_;
        TimeUnit units_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention fixedConvention_, floatingConvention_;
        Frequency fixedFrequency_, floatingFrequency_;
        DayCounter fixedDayCount_, floatingDayCount_;
        boost::shared_ptr<VanillaSwap> swap_;
        Handle<YieldTermStructure> termStructureHandle_;
    };

}


#endif
