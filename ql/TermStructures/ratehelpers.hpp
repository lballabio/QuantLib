
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

    \fullpath
    Include/ql/TermStructures/%ratehelpers.hpp
    \brief rate helpers

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 14:08:42  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.20  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.19  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.18  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.17  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.16  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.15  2001/07/24 08:49:32  sigmud
// pruned redundant header inclusions
//
// Revision 1.14  2001/07/02 12:36:18  sigmud
// pruned redundant header inclusions
//
// Revision 1.13  2001/06/18 08:05:59  lballabio
// Reworked indexes and floating rate coupon
//
// Revision 1.12  2001/06/13 16:18:23  lballabio
// Polished rate helper interfaces
//
// Revision 1.11  2001/06/12 13:43:04  lballabio
// Today's date is back into term structures
// Instruments are now constructed with settlement days instead of settlement date
//
// Revision 1.10  2001/06/08 13:34:46  lballabio
// Typedef DepositRateHelper to FraRateHelper
//
// Revision 1.9  2001/06/01 16:50:16  lballabio
// Term structure on deposits and swaps
//

#ifndef quantlib_ratehelper_h
#define quantlib_ratehelper_h

#include "ql/Instruments/simpleswap.hpp"

namespace QuantLib {

    namespace TermStructures {

        //! base class for rate helpers
        /*! This class provides an abstraction for the instruments used to 
            bootstrap a term structure. 
            It is advised that a rate helper for an instrument contain an 
            instance of the actual instrument class to ensure consistancy 
            between the algorithms used during bootstrapping and later 
            instrument pricing. This is not yet fully enforced in the available 
            rate helpers, though - only SwapRateHelper contains a Swap 
            instrument for the time being. 
            
            \todo Futures rate helper should be implemented. */
        class RateHelper {
          public:
            RateHelper() : termStructure_(0) {}
            virtual ~RateHelper() {}
            virtual double rateError() const = 0;
            virtual double discountGuess() const { return Null<double>(); }
            //! sets the term structure to be used for pricing
            /*! \warning Being a pointer and not a Handle, the term structure is
                not guaranteed to remain allocated for the whole life of the
                rate helper. It is responsibility of the programmer to ensure
                that the pointer remains valid. It is advised that rate helpers
                be used only in term structure constructors, setting the term
                structure to <b>this</b>, i.e., the one being constructed.
            */
            virtual void setTermStructure(TermStructure*);
            //! maturity date
            virtual Date maturity() const = 0;
          protected:
            TermStructure* termStructure_;
        };


        //! Deposit rate
        /*! \warning This class assumes that today's date does not change 
            between calls of setTermStructure().
        */
        class DepositRateHelper : public RateHelper {
          public:
            DepositRateHelper(Rate rate, int settlementDays,
                int n, TimeUnit units, const Handle<Calendar>& calendar,
                RollingConvention convention, 
                const Handle<DayCounter>& dayCounter);
            double rateError() const;
            double discountGuess() const;
            void setTermStructure(TermStructure*);
            Date maturity() const;
          private:
            Rate rate_;
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
        */
        class FraRateHelper : public RateHelper {
          public:
            FraRateHelper(Rate rate, int settlementDays, 
                int monthsToStart, int monthsToEnd, 
                const Handle<Calendar>& calendar,
                RollingConvention convention, 
                const Handle<DayCounter>& dayCounter);
            double rateError() const;
            double discountGuess() const;
            void setTermStructure(TermStructure*);
            Date maturity() const;
          private:
            Rate rate_;
            int settlementDays_;
            int monthsToStart_, monthsToEnd_;
            TimeUnit units_;
            Handle<Calendar> calendar_;
            RollingConvention convention_;
            Handle<DayCounter> dayCounter_;
            Date settlement_, start_, maturity_;
            double yearFraction_;
        };


        //! swap rate
        /*! \warning This class assumes that today's date does not change 
            between calls of setTermStructure().
        */
        class SwapRateHelper : public RateHelper {
          public:
            SwapRateHelper(Rate rate, 
                int settlementDays, int lengthInYears, 
                const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention, 
                // fixed leg
                int fixedFrequency, 
                bool fixedIsAdjusted, 
                const Handle<DayCounter>& fixedDayCount, 
                // floating leg
                int floatingFrequency);
            double rateError() const;
            // double discountGuess() const; // null for the time being
            Date maturity() const;
            void setTermStructure(TermStructure*);
          private:
            Rate rate_;
            int settlementDays_;
            int lengthInYears_;
            Handle<Calendar> calendar_;
            RollingConvention rollingConvention_;
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
