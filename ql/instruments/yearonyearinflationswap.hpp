/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file yyiis.hpp
    \brief Year-on-year inflation-indexed swap
*/

#ifndef quantlib_yyiis_hpp
#define quantlib_yyiis_hpp

#include <ql/instruments/inflationswap.hpp>

namespace QuantLib {

    //! Year-on-year inflation-indexed swap
    /*! \note The allowAmbiguousPayments parameter is to allow for
              payment arithmetic being ambiguous.  If the maturity is
              in, say, 30.01 years according to the daycounter and
              roll rules does this mean that there is a payment in
              0.01 years?.
    */
    class YearOnYearInflationSwap : public InflationSwap {
      public:
        YearOnYearInflationSwap(
                   const Date& start,
                   const Date& maturity,
                   const Period& lag,
                   Rate fixedRate,
                   const Calendar& calendar,
                   BusinessDayConvention convention,
                   const DayCounter& dayCounter,
                   const Handle<YieldTermStructure>& yieldTS,
                   const Handle<YoYInflationTermStructure>& inflationTS,
                   bool allowAmbiguousPayments = false,
                   const Period& ambiguousPaymentPeriod = Period(1, Months));

        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}

        //! \name InflationSwap interface
        //@{
        Rate fairRate() const;
        //@}

        //! \name Inspectors
        //@{
        Rate fixedRate() const;
        std::vector<Date> paymentDates() const;
        //@}

      protected:
        //! \name Instrument interface
        //@{
        void setupExpired() const;
        void performCalculations() const;
        //@}

        Rate fixedRate_;
        Handle<YoYInflationTermStructure> inflationTS_;

        bool allowAmbiguousPayments_;
        Period ambiguousPaymentPeriod_;
        std::vector<Date> paymentDates_;

        mutable Rate fairRate_;
    };

}


#endif

