/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file yearonyearinflationswap.hpp
    \brief Year-on-year inflation-indexed swap
*/

#ifndef quantlib_yyiis_hpp
#define quantlib_yyiis_hpp

#include <ql/instruments/inflationswap.hpp>

namespace QuantLib {

    //! Year-on-year inflation-indexed swap
    /*! Quoted as a fixed rate \f$ K \f$.  At start:
        \f[
        \sum_{i=1}^{M} P_n(0,t_i) N K =
        \sum_{i=1}^{M} P_n(0,t_i) N \left[ \frac{I(t_i)}{I(t_i-1)} - 1 \right]
        \f]
        where \f$ t_M \f$ is the maturity time, \f$ P_n(0,t) \f$ is the
        nominal discount factor at time \f$ t \f$, \f$ N \f$ is the
        notional, and \f$ I(t) \f$ is the inflation index value at
        time \f$ t \f$.

        \note The allowAmbiguousPayments parameter is to allow for
              payment arithmetic being ambiguous.  If the maturity is
              in, say, 30.01 years according to the day-counter and
              roll rules, does this mean that there is a payment in
              0.01 years? If allowAmbiguousPayments is false, the
              ambiguousPaymentPeriod parameter sets the period within
              which the answer is no.
    */
    class YearOnYearInflationSwap : public InflationSwap {
      public:
        class arguments;
        class engine;
        YearOnYearInflationSwap(const Date& start,
                                const Date& maturity,
                                const Period& lag,
                                Rate fixedRate,
                                const Calendar& calendar,
                                BusinessDayConvention convention,
                                const DayCounter& dayCounter,
                                bool allowAmbiguousPayments = false,
                                const Period& ambiguousPaymentPeriod =
                                                           Period(1, Months));
        //! \name Inspectors
        //@{
        //! \f$ K \f$ in the above formula.
        Rate fixedRate() const;
        std::vector<Date> paymentDates() const;
        //@}

        //! \name Instrument interface
        //@{
        void setupArguments(PricingEngine::arguments*) const;
        //@}

      protected:
        Rate fixedRate_;
        std::vector<Date> paymentDates_;
    };


    class YearOnYearInflationSwap::arguments : public InflationSwap::arguments {
      public:
        Rate fixedRate;
        std::vector<Date> paymentDates;
        void validate() const;
    };

    class YearOnYearInflationSwap::engine
        : public GenericEngine<YearOnYearInflationSwap::arguments,
                               YearOnYearInflationSwap::results> {};

}


#endif

