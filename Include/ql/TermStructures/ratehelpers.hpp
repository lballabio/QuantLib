
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file ratehelpers.hpp
    \brief rate helpers

    $Id$
*/

// $Source$
// $Log$
// Revision 1.9  2001/06/01 16:50:16  lballabio
// Term structure on deposits and swaps
//
// Revision 1.8  2001/05/29 15:12:48  lballabio
// Reintroduced RollingConventions (and redisabled default extrapolation on PFF curve)
//
// Revision 1.7  2001/05/28 14:54:25  lballabio
// Deposit rates are always adjusted
//
// Revision 1.6  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_ratehelper_h
#define quantlib_ratehelper_h

#include "ql/termstructure.hpp"
#include "ql/calendar.hpp"
#include "ql/daycounter.hpp"
#include "ql/Instruments/simpleswap.hpp"
#include "ql/Indexes/xibor.hpp"

namespace QuantLib {

    namespace TermStructures {

        //! base class for rate helpers
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


        //! deposit rate
        class DepositRateHelper : public RateHelper {
          public:
            DepositRateHelper(Rate rate, const Date& settlement,
                int n, TimeUnit units, const Handle<Calendar>& calendar,
                RollingConvention convention, 
                const Handle<DayCounter>& dayCounter);
            double rateError() const;
            double discountGuess() const;
            Date maturity() const;
          private:
            Rate rate_;
            Date settlement_;
            int n_;
            TimeUnit units_;
            Handle<Calendar> calendar_;
            RollingConvention convention_;
            Handle<DayCounter> dayCounter_;
            Date maturity_;
            double yearFraction_;
        };

        //! swap rate
        class SwapRateHelper : public RateHelper {
          public:
            SwapRateHelper(Rate rate, 
                const Date& startDate, int n, TimeUnit units,
                const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention, 
                // fixed leg
                int fixedFrequency, 
                bool fixedIsAdjusted, 
                const Handle<DayCounter>& fixedDayCount, 
                // floating leg
                int floatingFrequency, 
                const Indexes::Xibor& index, 
                const Handle<DayCounter>& floatingDayCount);
            double rateError() const;
            // double discountGuess() const; // null for the time being
            Date maturity() const;
            void setTermStructure(TermStructure*);
          private:
            Rate rate_;
            Handle<Instruments::SimpleSwap> swap_;
            RelinkableHandle<TermStructure> termStructureHandle_;
        };

    }

}


#endif
