
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file ratehelpers.hpp
    \brief rate helpers base class

    \fullpath
    ql/TermStructures/%ratehelpers.hpp
*/

// $Id$

#ifndef quantlib_ratehelper_h
#define quantlib_ratehelper_h

#include "ql/marketelement.hpp"
#include "ql/Instruments/simpleswap.hpp"

namespace QuantLib {

    namespace TermStructures {

        //! base class for rate helpers
        /*! This class provides an abstraction for the instruments used to 
            bootstrap a term structure. 
            It is advised that a rate helper for an instrument contain an 
            instance of the actual instrument class to ensure consistancy 
            between the algorithms used during bootstrapping and later 
            instrument pricing. This is not yet fully enforced in the 
            available rate helpers, though - only SwapRateHelper contains a 
            Swap instrument for the time being.
        */
            
        class RateHelper : public Patterns::Observer, 
                           public Patterns::Observable {
          public:
            RateHelper(const RelinkableHandle<MarketElement>& rate);
            virtual ~RateHelper();
            //! \name RateHelper interface
            //@{
            Rate rateError() const;
            virtual Rate impliedRate() const = 0;
            virtual DiscountFactor discountGuess() const { 
                return Null<double>(); 
            }
            //! sets the term structure to be used for pricing
            /*! \warning Being a pointer and not a Handle, the term structure 
                is not guaranteed to remain allocated for the whole life of 
                the rate helper. It is responsibility of the programmer to 
                ensure that the pointer remains valid. It is advised that 
                rate helpers be used only in term structure constructors, 
                setting the term structure to <b>this</b>, i.e., the one 
                being constructed.
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
            RelinkableHandle<MarketElement> quote_;
            TermStructure* termStructure_;
        };


        //! Deposit rate
        /*! \warning This class assumes that today's date does not change 
            between calls of setTermStructure().
        */
        class DepositRateHelper : public RateHelper {
          public:
            DepositRateHelper(const RelinkableHandle<MarketElement>& rate,
                              int settlementDays,
                              int n,
                              TimeUnit units,
                              const Handle<Calendar>& calendar,
                              RollingConvention convention,
                              const Handle<DayCounter>& dayCounter);
            Rate impliedRate() const;
            DiscountFactor discountGuess() const;
            void setTermStructure(TermStructure*);
            Date maturity() const;
          private:
            int settlementDays_;
            int n_;
            TimeUnit units_;
            Handle<Calendar> calendar_;
            RollingConvention convention_;
            Handle<DayCounter> dayCounter_;
            Date settlement_, maturity_;
            double yearFraction_;
        };


        //! Forward rate agreement
        /*! \warning This class assumes that today's date does not change 
            between calls of setTermStructure().

            \todo convexity adjustment should be implemented.
        */
        class FraRateHelper : public RateHelper {
          public:
            FraRateHelper(const RelinkableHandle<MarketElement>& rate,
                          int settlementDays,
                          int monthsToStart,
                          int monthsToEnd,
                          const Handle<Calendar>& calendar,
                          RollingConvention convention,
                          const Handle<DayCounter>& dayCounter);
            Rate impliedRate() const;
            DiscountFactor discountGuess() const;
            void setTermStructure(TermStructure*);
            Date maturity() const;
          private:
            int settlementDays_;
            int monthsToStart_, monthsToEnd_;
            TimeUnit units_;
            Handle<Calendar> calendar_;
            RollingConvention convention_;
            Handle<DayCounter> dayCounter_;
            Date settlement_, start_, maturity_;
            double yearFraction_;
        };


        //! Interest Rate Futures
        /*! \warning This class assumes that today's date does not change 
            between calls of setTermStructure().
        */
        class FuturesRateHelper : public RateHelper {
          public:
            FuturesRateHelper(const RelinkableHandle<MarketElement>& price,
                              const Date& ImmDate,
                              int settlementDays,
                              int nMonths,
                              const Handle<Calendar>& calendar,
                              RollingConvention convention,
                              const Handle<DayCounter>& dayCounter);
            Rate impliedRate() const;
            DiscountFactor discountGuess() const;
            Date maturity() const;
          private:
            Date ImmDate_;
            int settlementDays_;
            int nMonths_;
            Handle<Calendar> calendar_;
            RollingConvention convention_;
            Handle<DayCounter> dayCounter_;
            Date maturity_;
            double yearFraction_;
        };


        //! swap rate
        /*! \warning This class assumes that today's date does not change 
            between calls of setTermStructure().

            \todo discountGuess() should be implemented.
        */
        class SwapRateHelper : public RateHelper {
          public:
            SwapRateHelper(const RelinkableHandle<MarketElement>& rate,
                           int settlementDays,
                           int lengthInYears,
                           const Handle<Calendar>& calendar,
                           RollingConvention convention,
                           // fixed leg
                           int fixedFrequency,
                           bool fixedIsAdjusted,
                           const Handle<DayCounter>& fixedDayCount,
                           // floating leg
                           int floatingFrequency);
            Rate impliedRate() const;
            // double discountGuess() const; // null for the time being
            Date maturity() const;
            void setTermStructure(TermStructure*);
          private:
            int settlementDays_;
            int lengthInYears_;
            Handle<Calendar> calendar_;
            RollingConvention convention_;
            int fixedFrequency_, floatingFrequency_;
            bool fixedIsAdjusted_;
            Handle<DayCounter> fixedDayCount_;
            Date settlement_;
            Handle<Instruments::SimpleSwap> swap_;
            RelinkableHandle<TermStructure> termStructureHandle_;
        };

    }

}


#endif
