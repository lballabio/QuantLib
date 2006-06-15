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

#include <ql/TermStructures/piecewiseyieldcurve.hpp>
#include <ql/Instruments/vanillaswap.hpp>

namespace QuantLib {


    //! Rate helper for bootstrapping over interest-rate futures prices
    /*! \todo convexity adjustment should be implemented. */
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
        void setTermStructure(YieldTermStructure*);
      protected:
        virtual void initializeDates() = 0;
        Date evaluationDate_;
    };


    //! Rate helper for bootstrapping over deposit rates
    class DepositRateHelper : public RelativeDateRateHelper {
      public:
        #ifndef QL_DISABLE_DEPRECATED
        //! \deprecated use the corresponding Period based constructor
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
        #endif
        DepositRateHelper(const Handle<Quote>& rate,
                          Period p,
                          Integer settlementDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter);
        DepositRateHelper(Rate rate,
                          Period p,
                          Integer settlementDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          const DayCounter& dayCounter);
        Real impliedQuote() const;
        DiscountFactor discountGuess() const;
      protected:
        void initializeDates();
      private:
        Period p_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        Time yearFraction_;
    };


    //! Rate helper for bootstrapping over %FRA rates
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
      protected:
        void initializeDates();
      private:
        Integer monthsToStart_, monthsToEnd_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        Time yearFraction_;
    };

    //! Rate helper for bootstrapping over swap rates
    class SwapRateHelper : public RelativeDateRateHelper {
      public:
        /*! \warning When calling Index::addFixing(), the swap helper
                     will be notified only if the fixing is added by
                     means of the same instance that was passed to
                     this constructor. If the fixing is added to
                     another instance, the curve will not be aware of
                     the change (even though it will use the correct
                     fixing the next time it is recalculated.)
        */
        SwapRateHelper(const Handle<Quote>& rate,
                       Period p,
                       Integer settlementDays,
                       const Calendar& calendar,
                       // fixed leg
                       Frequency fixedFrequency,
                       BusinessDayConvention fixedConvention,
                       const DayCounter& fixedDayCount,
                       // floating leg
                       const boost::shared_ptr<Xibor>& index);
        /*! \warning When calling Index::addFixing(), the swap helper
                     will be notified only if the fixing is added by
                     means of the same instance that was passed to
                     this constructor. If the fixing is added to
                     another instance, the curve will not be aware of
                     the change (even though it will use the correct
                     fixing the next time it is recalculated.)
        */
        SwapRateHelper(Rate rate,
                       Period p,
                       Integer settlementDays,
                       const Calendar& calendar,
                       // fixed leg
                       Frequency fixedFrequency,
                       BusinessDayConvention fixedConvention,
                       const DayCounter& fixedDayCount,
                       // floating leg
                       const boost::shared_ptr<Xibor>& index);
        Real impliedQuote() const;
        // implementing discountGuess() is not worthwhile,
        // and may not avoid the root-finding process
        void setTermStructure(YieldTermStructure*);
      protected:
        void initializeDates();
      private:
        Period p_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention fixedConvention_;
        Frequency fixedFrequency_;
        DayCounter fixedDayCount_;
        boost::shared_ptr<Xibor> index_;
        boost::shared_ptr<VanillaSwap> swap_;
        Handle<YieldTermStructure> termStructureHandle_;
    };

}

#endif
