
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
        RateHelper(const RelinkableHandle<Quote>& quote);
        RateHelper(double quote);
        virtual ~RateHelper() {}
        //! \name RateHelper interface
        //@{
        double quoteError() const;
        double referenceQuote() const { return quote_->value(); }
        virtual double impliedQuote() const = 0;
        virtual DiscountFactor discountGuess() const {
            return Null<double>();
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
        virtual void setTermStructure(TermStructure*);
        //! maturity date
        virtual Date maturity() const = 0;
        //@}
        //! \name Observer interface
        //@{
        void update() { notifyObservers(); }
        //@}
      protected:
        RelinkableHandle<Quote> quote_;
        TermStructure* termStructure_;
    };


    //! Deposit rate
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
    */
    class DepositRateHelper : public RateHelper {
      public:
        DepositRateHelper(const RelinkableHandle<Quote>& rate,
                          int n, TimeUnit units,
                          int settlementDays,
                          const Calendar& calendar,
                          RollingConvention convention,
                          const DayCounter& dayCounter);
        DepositRateHelper(double rate,
                          int n, TimeUnit units,
                          int settlementDays,
                          const Calendar& calendar,
                          RollingConvention convention,
                          const DayCounter& dayCounter);
        double impliedQuote() const;
        DiscountFactor discountGuess() const;
        void setTermStructure(TermStructure*);
        Date maturity() const;
      private:
        int n_;
        TimeUnit units_;
        int settlementDays_;
        Calendar calendar_;
        RollingConvention convention_;
        DayCounter dayCounter_;
        Date settlement_, maturity_;
        double yearFraction_;
    };


    //! Forward rate agreement
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().

        \todo convexity adjustment should be implemented.
    */
    class FraRateHelper : public RateHelper {
      public:
        FraRateHelper(const RelinkableHandle<Quote>& rate,
                      int monthsToStart, int monthsToEnd,
                      int settlementDays,
                      const Calendar& calendar,
                      RollingConvention convention,
                      const DayCounter& dayCounter);
        FraRateHelper(double rate,
                      int monthsToStart, int monthsToEnd,
                      int settlementDays,
                      const Calendar& calendar,
                      RollingConvention convention,
                      const DayCounter& dayCounter);
        double impliedQuote() const;
        DiscountFactor discountGuess() const;
        void setTermStructure(TermStructure*);
        Date maturity() const;
      private:
        int monthsToStart_, monthsToEnd_;
        int settlementDays_;
        Calendar calendar_;
        RollingConvention convention_;
        DayCounter dayCounter_;
        Date settlement_, start_, maturity_;
        double yearFraction_;
    };


    //! Interest-rate futures
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
    */
    class FuturesRateHelper : public RateHelper {
      public:
        FuturesRateHelper(const RelinkableHandle<Quote>& price,
                          const Date& ImmDate,
                          int nMonths,
                          const Calendar& calendar,
                          RollingConvention convention,
                          const DayCounter& dayCounter);
	    FuturesRateHelper(const RelinkableHandle<Quote>& price,
                          const Date& ImmDate,
                          const Date& MatDate,
                          const Calendar& calendar,
                          RollingConvention convention,
                          const DayCounter& dayCounter);
        FuturesRateHelper(double price,
                          const Date& ImmDate,
                          int nMonths,
                          const Calendar& calendar,
                          RollingConvention convention,
                          const DayCounter& dayCounter);
        double impliedQuote() const;
        DiscountFactor discountGuess() const;
        Date maturity() const;
      private:
        Date ImmDate_;
        int nMonths_;
        Calendar calendar_;
        RollingConvention convention_;
        DayCounter dayCounter_;
        Date maturity_;
        double yearFraction_;
    };


    //! %Swap rate
    /*! \warning This class assumes that the settlement date
                 does not change between calls of setTermStructure().
    */
    class SwapRateHelper : public RateHelper {
      public:
        SwapRateHelper(const RelinkableHandle<Quote>& rate,
                       int n, TimeUnit units,
                       int settlementDays,
                       const Calendar& calendar,
                       RollingConvention convention,
                       // fixed leg
                       int fixedFrequency,
                       bool fixedIsAdjusted,
                       const DayCounter& fixedDayCount,
                       // floating leg
                       int floatingFrequency);
        SwapRateHelper(double rate,
                       int n, TimeUnit units,
                       int settlementDays,
                       const Calendar& calendar,
                       RollingConvention convention,
                       // fixed leg
                       int fixedFrequency,
                       bool fixedIsAdjusted,
                       const DayCounter& fixedDayCount,
                       // floating leg
                       int floatingFrequency);
        double impliedQuote() const;
        // implementing discountGuess() is not worthwhile,
        // and may not avoid the root-finding process
        Date maturity() const;
        void setTermStructure(TermStructure*);
      protected:
        int n_;
        TimeUnit units_;
        int settlementDays_;
        Calendar calendar_;
        RollingConvention convention_;
        int fixedFrequency_, floatingFrequency_;
        bool fixedIsAdjusted_;
        DayCounter fixedDayCount_;
        Date settlement_;
        boost::shared_ptr<SimpleSwap> swap_;
        RelinkableHandle<TermStructure> termStructureHandle_;
    };

}


#endif
