
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file ratehelper.hpp
    \brief rate helpers
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
