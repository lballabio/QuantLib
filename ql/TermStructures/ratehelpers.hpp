
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file ratehelpers.hpp
    \brief rate helpers base class
*/

#ifndef quantlib_ratehelper_h
#define quantlib_ratehelper_h

#include <ql/Instruments/simpleswap.hpp>

namespace QuantLib {

    //! Base class for rate helpers
    /*! This class provides an abstraction for the instruments used to
        bootstrap a term structure.
        It is advised that a rate helper for an instrument contains an
        instance of the actual instrument class to ensure consistancy
        between the algorithms used during bootstrapping and later
        instrument pricing. This is not yet fully enforced in the
        available rate helpers, though - only SwapRateHelper contains a
        Swap instrument for the time being.
    */

    class RateHelper : public Observer, public Observable {
      public:
        RateHelper(const Handle<Quote>& quote);
        RateHelper(Real quote);
        virtual ~RateHelper() {}
        //! \name RateHelper interface
        //@{
        Real quoteError() const;
        Real referenceQuote() const { return quote_->value(); }
        virtual Real impliedQuote() const = 0;
        virtual DiscountFactor discountGuess() const {
            return Null<Real>();
        }
        //! sets the term structure to be used for pricing
        /*! \warning Being a pointer and not a shared_ptr, the term
                     structure is not guaranteed to remain allocated
                     for the whole life of the rate helper. It is
                     responsibility of the programmer to ensure that
                     the pointer remains valid. It is advised that
                     rate helpers be used only in term structure
                     constructors, setting the term structure to
                     <b>this</b>, i.e., the one being constructed.
        */
        virtual void setTermStructure(YieldTermStructure*);
        //! latest relevant date
        /*! The latest date at which discounts are needed by the
            helper in order to provide a quote. It does not
            necessarily equal the maturity of the underlying
            instrument.
        */
        virtual Date latestDate() const = 0;
        #ifndef QL_DISABLE_DEPRECATED
        //! maturity date
        /*! \deprecated renamed to latestDate() */
        Date maturity() const { return latestDate(); }
        #endif
        //@}
        //! \name Observer interface
        //@{
        void update() { notifyObservers(); }
        //@}
      protected:
        Handle<Quote> quote_;
        YieldTermStructure* termStructure_;
    };


    //! Deposit rate
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
    */
    class DepositRateHelper : public RateHelper {
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
        Date latestDate() const;
      private:
        Integer n_;
        TimeUnit units_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        Date settlement_, maturity_;
        Time yearFraction_;
    };


    //! Forward rate agreement
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().

        \todo convexity adjustment should be implemented.
    */
    class FraRateHelper : public RateHelper {
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
        Date latestDate() const;
      private:
        Integer monthsToStart_, monthsToEnd_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        Date settlement_, start_, maturity_;
        Time yearFraction_;
    };


    //! Interest-rate futures
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
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
        Date latestDate() const;
      private:
        Date immDate_;
        Integer nMonths_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        Date maturity_;
        Time yearFraction_;
    };


    //! %Swap rate
    /*! \todo Currency and dayCounter of Xibor should be added to obtain well define SwapRateHelper
		\warning This class assumes that the settlement date
                 does not change between calls of setTermStructure().
    */
    class SwapRateHelper : public RateHelper {
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
                       BusinessDayConvention floatingConvention);
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
                       BusinessDayConvention floatingConvention);
        Real impliedQuote() const;
        // implementing discountGuess() is not worthwhile,
        // and may not avoid the root-finding process
        Date latestDate() const;
        void setTermStructure(YieldTermStructure*);
      protected:
        Integer n_;
        TimeUnit units_;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention fixedConvention_, floatingConvention_;
        Frequency fixedFrequency_, floatingFrequency_;
        DayCounter fixedDayCount_;
        Date settlement_, latestDate_;
        boost::shared_ptr<SimpleSwap> swap_;
        Handle<YieldTermStructure> termStructureHandle_;
    };

}


#endif
