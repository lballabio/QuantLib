
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

/*! \file ratehelper.h
    \brief rate helpers

    $Source$
    $Log$
    Revision 1.2  2001/03/14 14:03:44  lballabio
    Fixed Doxygen documentation and makefiles

    Revision 1.1  2001/03/07 10:34:25  nando
    added ratehelper.cpp and ratehelper.h.
    Borland, Linux and Visual C++ updated
    Also added some missing files to Visual C++


*/

#ifndef quantlib_ratehelper_h
#define quantlib_ratehelper_h

#include "qldefines.h"
#include "date.h"
#include "daycounter.h"
#include "calendar.h"
#include "rate.h"
#include "termstructure.h"

namespace QuantLib {

    //! %rate helper
    class RateHelper {
      public:
        RateHelper(const Date& maturity,
                   Rate rate,
                   const Handle<DayCounter>& dayCounter);
        virtual ~RateHelper() {}
        virtual double value() const = 0;
        virtual double guess() const = 0;
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
        Time timeToMatutity_;
        Rate rate_;
        Handle<DayCounter> dayCounter_;
        Handle<TermStructure> termStructure_;
    };

    
    //! %deposit rate
    class DepositRate : public RateHelper {
      public:
        DepositRate(const Date& maturity,
                    Rate rate,
                    const Handle<DayCounter>& dayCounter);
        double value() const;
        double guess() const;
    };

    //! %forward rate
    class ForwardRate : public RateHelper {
      public:
        ForwardRate(const Date& maturity,
                    Rate rate,
                    const Handle<DayCounter>& dayCounter);
        double value() const;
        double guess() const;
    };

    //! %swap rate
    class SwapRate : public RateHelper {
      public:
        SwapRate(const Date& maturity,
                 Rate rate,
                 const Handle<DayCounter>& dayCounter);
        double value() const;
        double guess() const;
    };



    // inline
    RateHelper::RateHelper(const Date& maturity,
                           Rate rate,
                           const Handle<DayCounter>& dayCounter) 
    : maturity_(maturity), rate_(rate), dayCounter_(dayCounter) {}



    void RateHelper::setTermStructure(const Handle<TermStructure>& termStructure) {
        termStructure_ = termStructure;
        timeToMatutity_ = dayCounter_->yearFraction(termStructure_->settlementDate(),
            maturity_);
    }

    
    Date RateHelper::maturity() const {
        return maturity_;
    }


    Rate RateHelper::rate() const {
        return rate_;
    }

    
    Handle<DayCounter> RateHelper::dayCounter() const {
        return dayCounter_;
    }



    DepositRate::DepositRate(const Date& maturity,
                             Rate rate,
                             const Handle<DayCounter>& dayCounter)
    : RateHelper(maturity, rate, dayCounter) {}


    ForwardRate::ForwardRate(const Date& maturity,
                             Rate rate,
                             const Handle<DayCounter>& dayCounter)
    : RateHelper(maturity, rate, dayCounter) {}


    SwapRate::SwapRate(const Date& maturity,
                       Rate rate,
                       const Handle<DayCounter>& dayCounter)
    : RateHelper(maturity, rate, dayCounter) {}

    
}


#endif
