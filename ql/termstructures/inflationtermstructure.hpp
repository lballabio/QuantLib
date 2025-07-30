/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon

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

    class InflationIndex;

    //! Interface for inflation term structures.
    /*! \ingroup inflationtermstructures */
    class InflationTermStructure : public TermStructure {
      public:
        //! \name Constructors
        //@{
        InflationTermStructure(Date baseDate,
                               Frequency frequency,
                               const DayCounter& dayCounter = DayCounter(),
                               ext::shared_ptr<Seasonality> seasonality = {},
                               Rate baseRate = Null<Rate>());

        InflationTermStructure(const Date& referenceDate,
                               Date baseDate,
                               Frequency frequency,
                               const DayCounter& dayCounter = DayCounter(),
                               ext::shared_ptr<Seasonality> seasonality = {},
                               Rate baseRate = Null<Rate>());

        InflationTermStructure(Natural settlementDays,
                               const Calendar& calendar,
                               Date baseDate,
                               Frequency frequency,
                               const DayCounter& dayCounter = DayCounter(),
                               ext::shared_ptr<Seasonality> seasonality = {},
                               Rate baseRate = Null<Rate>());
        //@}

        QL_DEPRECATED_DISABLE_WARNING
        ~InflationTermStructure() override = default;
        QL_DEPRECATED_ENABLE_WARNING

        //! \name Inflation interface
        //@{
        /*! \deprecated Do not use; inflation curves always have an explicit
                        base date now.
                        Deprecated in version 1.39.
        */
        [[deprecated("Do not use; inflation curves always have an explicit base date now.")]]
        virtual Period observationLag() const;

        virtual Frequency frequency() const;
        virtual Rate baseRate() const;

        //! minimum (base) date
        /*! The last date for which we have information. */
        virtual Date baseDate() const;

        /*! \deprecated Do not use; inflation curves always have an explicit
                        base date now.
                        Deprecated in version 1.39.
        */
        [[deprecated("Do not use; inflation curves always have an explicit base date now.")]]
        bool hasExplicitBaseDate() const {
            return true;
        }
        //@}

        //! \name Seasonality
        //@{
        void setSeasonality(const ext::shared_ptr<Seasonality>& seasonality);
        ext::shared_ptr<Seasonality> seasonality() const;
        bool hasSeasonality() const;
        //@}

      protected:
        void checkRange(const Date&,
                        bool extrapolate) const;
        void checkRange(Time t,
                        bool extrapolate) const;

        ext::shared_ptr<Seasonality> seasonality_;

        /*! \deprecated Do not use; inflation curves always have an explicit
                        base date now.
                        Deprecated in version 1.39.
        */
        [[deprecated("Do not use; inflation curves always have an explicit base date now.")]]
        Period observationLag_;

        Frequency frequency_;
        mutable Rate baseRate_;
        // Can be set by subclasses that don't have baseDate available in constructors.
        Date baseDate_;
    };

    //! Interface for zero inflation term structures.
    class ZeroInflationTermStructure : public InflationTermStructure {
      public:
        //! \name Constructors
        //@{
        ZeroInflationTermStructure(Date baseDate,
                                   Frequency frequency,
                                   const DayCounter& dayCounter,
                                   const ext::shared_ptr<Seasonality>& seasonality = {});

        ZeroInflationTermStructure(const Date& referenceDate,
                                   Date baseDate,
                                   Frequency frequency,
                                   const DayCounter& dayCounter,
                                   const ext::shared_ptr<Seasonality>& seasonality = {});

        ZeroInflationTermStructure(Natural settlementDays,
                                   const Calendar& calendar,
                                   Date baseDate,
                                   Frequency frequency,
                                   const DayCounter& dayCounter,
                                   const ext::shared_ptr<Seasonality>& seasonality = {});
        //@}

        //! \name Inspectors
        //@{
        //! zero-coupon inflation rate.
        /*! Essentially the fair rate for a zero-coupon inflation swap
            (by definition), i.e. the zero term structure uses yearly
            compounding, which is assumed for ZCIIS instrument quotes.

            \note by default you get the same as lag and interpolation
            as the term structure.
            If you want to get predictions of RPI/CPI/etc then use an
            index.
        */
        Rate zeroRate(const Date& d, const Period& instObsLag = Period(-1,Days),
                      bool forceLinearInterpolation = false,
                      bool extrapolate = false) const;
        //! zero-coupon inflation rate.
        /*! \warning Since inflation is highly linked to dates (lags,
                     interpolation, months for seasonality, etc) this
                     method cannot account for all effects.  If you
                     call it, You'll have to manage lag, seasonality
                     etc. yourself.
        */
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
        YoYInflationTermStructure(Date baseDate,
                                  Rate baseYoYRate,
                                  Frequency frequency,
                                  const DayCounter& dayCounter,
                                  const ext::shared_ptr<Seasonality>& seasonality = {});

        YoYInflationTermStructure(const Date& referenceDate,
                                  Date baseDate,
                                  Rate baseYoYRate,
                                  Frequency frequency,
                                  const DayCounter& dayCounter,
                                  const ext::shared_ptr<Seasonality>& seasonality = {});

        YoYInflationTermStructure(Natural settlementDays,
                                  const Calendar& calendar,
                                  Date baseDate,
                                  Rate baseYoYRate,
                                  Frequency frequency,
                                  const DayCounter& dayCounter,
                                  const ext::shared_ptr<Seasonality>& seasonality = {});

        /*! \deprecated Use an overload with an explicit base date and without indexIsInterpolated.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use an overload with an explicit base date and without indexIsInterpolated")]]
        YoYInflationTermStructure(Date baseDate,
                                  Rate baseYoYRate,
                                  Frequency frequency,
                                  bool indexIsInterpolated,
                                  const DayCounter& dayCounter,
                                  const ext::shared_ptr<Seasonality>& seasonality = {});

        /*! \deprecated Use an overload with an explicit base date and without indexIsInterpolated.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use an overload with an explicit base date and without indexIsInterpolated")]]
        YoYInflationTermStructure(const Date& referenceDate,
                                  Date baseDate,
                                  Rate baseYoYRate,
                                  Frequency frequency,
                                  bool indexIsInterpolated,
                                  const DayCounter& dayCounter,
                                  const ext::shared_ptr<Seasonality>& seasonality = {});

        /*! \deprecated Use an overload with an explicit base date and without indexIsInterpolated.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use an overload with an explicit base date and without indexIsInterpolated")]]
        YoYInflationTermStructure(Natural settlementDays,
                                  const Calendar& calendar,
                                  Date baseDate,
                                  Rate baseYoYRate,
                                  Frequency frequency,
                                  bool indexIsInterpolated,
                                  const DayCounter& dayCounter,
                                  const ext::shared_ptr<Seasonality>& seasonality = {});
        //@}

        QL_DEPRECATED_DISABLE_WARNING
        ~YoYInflationTermStructure() override = default;
        QL_DEPRECATED_ENABLE_WARNING

        //! \name Inspectors
        //@{
        //! year-on-year inflation rate.
        /*! The forceLinearInterpolation parameter is relative to the
            frequency of the TS.

            \note this is not the year-on-year swap (YYIIS) rate.
        */
        Rate yoyRate(const Date& d, const Period& instObsLag = Period(-1,Days),
                     bool forceLinearInterpolation = false,
                     bool extrapolate = false) const;
        //! year-on-year inflation rate.
        /*! \warning Since inflation is highly linked to dates (lags,
                     interpolation, months for seasonality, etc) this
                     method cannot account for all effects.  If you
                     call it, You'll have to manage lag, seasonality
                     etc. yourself.
        */
        Rate yoyRate(Time t,
                     bool extrapolate = false) const;
        //@}

        /*! \deprecated This method will disappear. When it does, the curve will behave as if it returned false.
                        Deprecated in version 1.37.
        */
        [[deprecated("This method will disappear. When it does, the curve will behave as if it returned false")]]
        virtual bool indexIsInterpolated() const;
      protected:
        //! to be defined in derived classes
        virtual Rate yoyRateImpl(Time time) const = 0;

        /*! \deprecated This data member will disappear. When it does, the curve will behave as if it was false.
                        Deprecated in version 1.37.
        */
        [[deprecated("This data member will disappear. When it does, the curve will behave as if it was false")]]
        bool indexIsInterpolated_ = false;
    };


    //! utility function giving the inflation period for a given date
    std::pair<Date,Date> inflationPeriod(const Date&,
                                         Frequency);

    //! utility function giving the time between two dates depending on
    //! index frequency and interpolation, and a day counter
    Time inflationYearFraction(Frequency ,
                               bool indexIsInterpolated,
                               const DayCounter&,
                               const Date&, const Date&);


    // inline

    inline Period InflationTermStructure::observationLag() const {
        QL_DEPRECATED_DISABLE_WARNING
        return observationLag_;
        QL_DEPRECATED_ENABLE_WARNING
    }

    inline Frequency InflationTermStructure::frequency() const {
        return frequency_;
    }

    inline Rate InflationTermStructure::baseRate() const {
        QL_REQUIRE(baseRate_ != Null<Real>(), "base rate not available");
        return baseRate_;
    }

    inline ext::shared_ptr<Seasonality> InflationTermStructure::seasonality() const {
        return seasonality_;
    }

    inline bool InflationTermStructure::hasSeasonality() const {
        return static_cast<bool>(seasonality_);
    }

    inline bool YoYInflationTermStructure::indexIsInterpolated() const {
        QL_DEPRECATED_DISABLE_WARNING
        return indexIsInterpolated_;
        QL_DEPRECATED_ENABLE_WARNING
    }

}

#endif
