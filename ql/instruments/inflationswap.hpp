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

/*! \file inflationswap.hpp
    \brief Abstract base class for inflation swaps
*/

#ifndef quantlib_inflation_swap_hpp
#define quantlib_inflation_swap_hpp

#include <ql/instrument.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! Abstract base class for inflation swaps.
    /*! Inflation swaps need two term structures: a yield curve, and
        an inflation term structure (either zero-based, i.e., the rate
        \f$ r(t) \f$ equals \f$ I(t)/I(t_0) - 1 \f$ where \f$ I \f$ if
        the index and \f$ t_0 \f$ is the base time, or year-on-year,
        i.e., \f$ r(t) = I(t)/I(t_p) - 1 \f$ where the previous time
        \f$ t_p \f$ is defined as \f$ t \f$ minus one year.)

        \ingroup instruments
    */
    class InflationSwap : public Instrument {
      public:
        class arguments;
        class results;
        //! the constructor sets common data members
        InflationSwap(const Date& start, const Date& maturity,
                      const Period& lag, const Calendar& calendar,
                      BusinessDayConvention convention,
                      const DayCounter& dayCounter);
        //! \name Inspectors
        /*! The inflation rate is taken relative to the base date,
            which is a lag period before the start date of the swap.
        */
        //@{
        Date baseDate() const;
        Period lag() const;
        Date startDate() const;
        Date maturityDate() const;
        Calendar calendar() const;
        BusinessDayConvention businessDayConvention() const;
        DayCounter dayCounter() const;
        //@}
        //! \name Results
        //@{
        Rate fairRate() const;
        //@}
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        void setupArguments(PricingEngine::arguments*) const;
      protected:
        void fetchResults(const PricingEngine::results*) const;
        void setupExpired() const;
        //@}
        Date start_;
        Date maturity_;
        Period lag_;
        Calendar calendar_;
        BusinessDayConvention bdc_;
        DayCounter dayCounter_;
        Date baseDate_;
        // results
        mutable Rate fairRate_;
    };

    class InflationSwap::arguments : public virtual PricingEngine::arguments {
      public:
        Date start;
        Date maturity;
        Period lag;
        Calendar calendar;
        BusinessDayConvention bdc;
        DayCounter dayCounter;
        Date baseDate;
        void validate() const;
    };

    class InflationSwap::results : public Instrument::results {
      public:
        Rate fairRate;
        void reset();
    };

}


#endif
