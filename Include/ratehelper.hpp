
/*
 * Copyright (C) 2001
 * QuantLib Group
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
 *
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file ratehelper.hpp
    \brief rate helpers

    $Source$
    $Log$
    Revision 1.1  2001/04/04 11:07:21  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.4  2001/03/19 18:39:27  nando
    conflict resolved

    Revision 1.3  2001/03/19 17:52:56  nando
    introduces DepositRate2.
    Later this will superseed DepositRate

    Revision 1.2  2001/03/14 14:03:44  lballabio
    Fixed Doxygen documentation and makefiles

    Revision 1.1  2001/03/07 10:34:25  nando
    added ratehelper.cpp and ratehelper.h.
    Borland, Linux and Visual C++ updated
    Also added some missing files to Visual C++


*/

#ifndef quantlib_ratehelper_h
#define quantlib_ratehelper_h

#include "qldefines.hpp"
#include "date.hpp"
#include "daycounter.hpp"
#include "calendar.hpp"
#include "rate.hpp"
#include "termstructure.hpp"
#include "dataformatters.hpp"
#include <iostream>


namespace QuantLib {

    //! %rate helper
    class RateHelper {
      public:
        RateHelper() {}
        RateHelper(const Date& maturity,
                   Rate rate,
                   const Handle<DayCounter>& dayCounter);
        virtual ~RateHelper() {}
        virtual double rateError() const = 0;
        virtual double discountGuess() const = 0;
        //! \name Modifiers
        //@{
        //! sets the term structure to be used by value() method
        void setTermStructure(const Handle<TermStructure>& termStructure);
        //@}
        //! \name Inspectors
        //@{
        Date maturity() const;
        Rate rate() const;
        Handle<DayCounter> dayCounter() const;
        //@}
      protected:
        Date maturity_;
        Time timeToMaturity_;
        Rate rate_;
        Handle<DayCounter> dayCounter_;
        Handle<TermStructure> termStructure_;
    };

    
    //! %deposit rate

    class DepositRate2 : public RateHelper {
      public:
        DepositRate2() {}
        DepositRate2(const Date& maturity,
                    Rate rate,
                    const Handle<DayCounter>& dayCounter);
        double rateError() const;
        double discountGuess() const;
    };

    //! %forward rate
    class ForwardRate : public RateHelper {
      public:
        ForwardRate() {}
        ForwardRate(const Date& maturity,
                    Rate rate,
                    const Handle<DayCounter>& dayCounter);
        double rateError() const;
        double discountGuess() const;
    };

    //! %swap rate
    class SwapRate : public RateHelper {
      public:
        SwapRate() {}
        SwapRate(const Date& maturity,
                 Rate rate,
                 const Handle<DayCounter>& dayCounter);
        double rateError() const;
        double discountGuess() const;
    };



    // inline
    inline RateHelper::RateHelper(const Date& maturity,
                           Rate rate,
                           const Handle<DayCounter>& dayCounter) 
    : maturity_(maturity), rate_(rate), dayCounter_(dayCounter) {}



    inline void RateHelper::setTermStructure(const Handle<TermStructure>& termStructure) {
        termStructure_ = termStructure;

//        std::cout << std::endl << DateFormatter::toString(maturity_) 
//            << termStructure_->discount(maturity_) << " "
//            << std::endl;

        timeToMaturity_ = dayCounter_->yearFraction(termStructure_->settlementDate(),
            maturity_);
    }

    
    inline Date RateHelper::maturity() const {
        return maturity_;
    }


    inline Rate RateHelper::rate() const {
        return rate_;
    }

    
    inline Handle<DayCounter> RateHelper::dayCounter() const {
        return dayCounter_;
    }



    inline DepositRate2::DepositRate2(const Date& maturity,
                             Rate rate,
                             const Handle<DayCounter>& dayCounter)
    : RateHelper(maturity, rate, dayCounter) {}


    inline ForwardRate::ForwardRate(const Date& maturity,
                             Rate rate,
                             const Handle<DayCounter>& dayCounter)
    : RateHelper(maturity, rate, dayCounter) {}


    inline SwapRate::SwapRate(const Date& maturity,
                       Rate rate,
                       const Handle<DayCounter>& dayCounter)
    : RateHelper(maturity, rate, dayCounter) {}

    
}


#endif
