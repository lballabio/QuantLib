
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

    $Source$
    $Log$
    Revision 1.4  2001/05/24 11:15:57  lballabio
    Stripped conventions from Currencies

    Revision 1.3  2001/05/17 15:33:30  lballabio
    Deposit rate helpers now use conventions in Currency

    Revision 1.2  2001/05/16 15:43:38  lballabio
    Fixed typo in docs

    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#ifndef quantlib_ratehelper_h
#define quantlib_ratehelper_h

#include "ql/qldefines.hpp"
#include "ql/termstructure.hpp"
#include "ql/calendar.hpp"
#include "ql/daycounter.hpp"

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
            virtual void setTermStructure(const TermStructure*);
            //! maturity date
            virtual Date maturity() const = 0;
          protected:
            const TermStructure* termStructure_;
        };


        //! deposit rate
        class DepositRateHelper : public RateHelper {
          public:
            DepositRateHelper(Rate rate, const Date& settlement, 
                int n, TimeUnit units, const Handle<Calendar>& calendar, 
                bool isAdjusted, bool isModifiedFollowing, 
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
            bool isAdjusted_, isModified_;
            Handle<DayCounter> dayCounter_;
            Date maturity_;
            double yearFraction_;
        };

    }

}


#endif
