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

/*! \file inflationtermstructure.hpp
    \brief Base classes for inflation term structures.
*/

#ifndef quantlib_inflation_termstructure_hpp
#define quantlib_inflation_termstructure_hpp

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/inflation/seasonality.hpp>

namespace QuantLib {

    //! Interface for inflation term structures.
    /*! \ingroup inflationtermstructures */
    class InflationTermStructure : public TermStructure {
      public:
        //! \name Constructors
        //@{
        InflationTermStructure(const Period& lag,
                               Frequency frequency,
                               Rate baseRate,
                               const Handle<YieldTermStructure>& yTS,
                               const DayCounter& dayCounter = DayCounter(),
                               const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());
        InflationTermStructure(const Date& referenceDate,
                               const Period& lag,
                               Frequency frequency,
                               Rate baseRate,
                               const Handle<YieldTermStructure>& yTS,
                               const Calendar& calendar = Calendar(),
                               const DayCounter& dayCounter = DayCounter(),
                               const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());
        InflationTermStructure(Natural settlementDays,
                               const Calendar& calendar,
                               const Period& lag,
                               Frequency frequency,
                               Rate baseRate,
                               const Handle<YieldTermStructure>& yTS,
                               const DayCounter& dayCounter = DayCounter(),
                               const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());
        //@}

        //! \name Inflation interface
        //@{
        virtual Period lag() const;
        virtual Frequency frequency() const;
        virtual Rate baseRate() const;
        virtual Handle<YieldTermStructure> nominalTermStructure() const;

        //! minimum (base) date
        /*! Important in inflation since it starts before nominal
            reference date.
        */
        virtual Date baseDate() const = 0;
        //@}

        //! Functions to set and get seasonality.
        /*! Calling setSeasonality with no arguments means unsetting
            as the default is used to choose unsetting.
        */
        void setSeasonality(const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());
        boost::shared_ptr<Seasonality> seasonality() const;
        bool hasSeasonality() const;

      protected:
        Handle<YieldTermStructure> nominalTermStructure_;

        // connection with base index:
        //  lag to base date
        //  index
        //  whether or not to connect with the index at the short end
        //  (don't if you have no index set up)
        Period lag_;
        Frequency frequency_;

        // This next part is required for piecewise- constructors
        // because, for inflation, they need more than just the
        // instruments to build the term structure, since the rate at
        // time 0-lag is non-zero, since we deal (effectively) with
        // "forwards".
        virtual void setBaseRate(const Rate &r){baseRate_=r;}
        mutable Rate baseRate_;

        // range-checking
        void checkRange(const Date&,
                        bool extrapolate) const;
        void checkRange(Time t,
                        bool extrapolate) const;

        boost::shared_ptr<Seasonality> seasonality_;
        bool hasSeasonalityCorrection_; //true if seasonality is set
    };


    //! Interface for zero inflation term structures.
    // Child classes use templates but do not want that exposed to
    // general users.
    class ZeroInflationTermStructure : public InflationTermStructure {
      public:
        //! \name Constructors
        //@{
        ZeroInflationTermStructure(const DayCounter& dayCounter,
                                   const Period& lag,
                                   Frequency frequency,
                                   Rate baseZeroRate,
                                   const Handle<YieldTermStructure>& yTS,
                                   const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());

        ZeroInflationTermStructure(const Date& referenceDate,
                                   const Calendar& calendar,
                                   const DayCounter& dayCounter,
                                   const Period& lag,
                                   Frequency frequency,
                                   Rate baseZeroRate,
                                   const Handle<YieldTermStructure>& yTS,
                                   const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());

        ZeroInflationTermStructure(Natural settlementDays,
                                   const Calendar& calendar,
                                   const DayCounter& dayCounter,
                                   const Period& lag,
                                   Frequency frequency,
                                   Rate baseZeroRate,
                                   const Handle<YieldTermStructure>& yTS,
                                   const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());
        //@}

        //! \name Inspectors
        //@{
        //! zero-coupon inflation rate
        /*! Essentially the fair rate for a zero-coupon inflation swap
            (by definition), i.e. the zero term structure uses yearly
            compounding, which is assumed for ZCIIS instrument quotes.
        */
        Rate zeroRate(const Date &d,
                      bool extrapolate = false) const;
        Rate zeroRate(Time t,
                      bool extrapolate = false) const;
        //@}
      protected:
        //! to be defined in derived classes
        virtual Rate zeroRateImpl(Time t) const = 0;
    };


    //! Base class for year-on-year inflation term structures.
    class YoYInflationTermStructure : public InflationTermStructure {
      public:
        //! \name Constructors
        //@{
        YoYInflationTermStructure(const DayCounter& dayCounter,
                                  const Period& lag,
                                  Frequency frequency,
                                  Rate baseYoYRate,
                                  const Handle<YieldTermStructure>& yieldTS,
                                  const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());

        YoYInflationTermStructure(const Date& referenceDate,
                                  const Calendar& calendar,
                                  const DayCounter& dayCounter,
                                  const Period& lag,
                                  Frequency frequency,
                                  Rate baseYoYRate,
                                  const Handle<YieldTermStructure>& yieldTS,
                                  const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());

        YoYInflationTermStructure(Natural settlementDays,
                                  const Calendar& calendar,
                                  const DayCounter& dayCounter,
                                  const Period& lag,
                                  Frequency frequency,
                                  Rate baseYoYRate,
                                  const Handle<YieldTermStructure>& yieldTS,
                                  const boost::shared_ptr<Seasonality> &seasonality = boost::shared_ptr<Seasonality>());
        //@}

        //! \name Inspectors
        //@{
        //! year-on-year inflation rate
        /*! \note this is not the year-on-year swap (YYIIS) rate. */
        Rate yoyRate(const Date &d,
                     bool extrapolate = false) const;
        Rate yoyRate(Time time,
                     bool extrapolate = false) const;
        //@}
      protected:
        //! to be defined in derived classes
        virtual Rate yoyRateImpl(Time time) const = 0;
    };


    //! utility function giving the inflation period for a given date
    std::pair<Date,Date> inflationPeriod(const Date &,
                                         Frequency);

}


#endif
