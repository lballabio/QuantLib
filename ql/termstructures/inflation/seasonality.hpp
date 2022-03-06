/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Piero Del Boca
 Copyright (C) 2009 Chris Kenyon
 Copyright (C) 2015 Bernd Lewerenz

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

#ifndef quantlib_seasonality_hpp
#define quantlib_seasonality_hpp

#include <ql/time/daycounter.hpp>
#include <ql/time/frequency.hpp>
#include <ql/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class InflationTermStructure;

    //! A transformation of an existing inflation swap rate.
    /*! This is an abstract class and contains the functions
        correctXXXRate which returns rates with the seasonality
        correction.  Currently only the price multiplicative version
        is implemented, but this covers stationary (1-year) and
        non-stationary (multi-year) seasonality depending on how many
        years of factors are given.  Seasonality is piecewise
        constant, hence it will work with un-interpolated inflation
        indices.

        A seasonality assumption can be used to fill in inflation swap
        curves between maturities that are usually given in integer
        numbers of years, e.g. 8,9,10,15,20, etc.  Historical
        seasonality may be observed in reported CPI values,
        alternatively it may be affected by known future events, e.g.
        announced changes in VAT rates.  Thus seasonality may be
        stationary or non-stationary.

        If seasonality is additive then both swap rates will show
        affects.  Additive seasonality is not implemented.
    */
    class Seasonality {

        public:

        //! \name Seasonality interface
        //@{
          virtual Rate
          correctZeroRate(const Date& d, Rate r, const InflationTermStructure& iTS) const = 0;
          virtual Rate
          correctYoYRate(const Date& d, Rate r, const InflationTermStructure& iTS) const = 0;
          /*! It is possible for multi-year seasonalities to be
              inconsistent with the inflation term structure they are
              given to.  This method enables testing - but programmers
              are not required to implement it.  E.g. for price
              seasonality the corrections at whole years after the
              inflation curve base date should be the same or else there
              can be an inconsistency with quoted instruments.
              Alternatively, the seasonality can be set _before_ the
              inflation curve is bootstrapped.
          */
          virtual bool isConsistent(const InflationTermStructure& iTS) const;
          //@}

          virtual ~Seasonality() = default;
    };

    //! Multiplicative seasonality in the price index (CPI/RPI/HICP/etc).

    /*! Stationary multiplicative seasonality in CPI/RPI/HICP (i.e. in
        price) implies that zero inflation swap rates are affected,
        but that year-on-year inflation swap rates show no effect.  Of
        course, if the seasonality in CPI/RPI/HICP is non-stationary
        then both swap rates will be affected.

        Factors must be in multiples of the minimum required for one
        year, e.g. 12 for monthly, and these factors are reused for as
        long as is required, i.e. they wrap around.  So, for example,
        if 24 factors are given this repeats every two years.  True
        stationary seasonality can be obtained by giving the same
        number of factors as the frequency dictates e.g. 12 for
        monthly seasonality.

        \warning Multi-year seasonality (i.e. non-stationary) is
                 fragile: the user <b>must</b> ensure that corrections
                 at whole years before and after the inflation term
                 structure base date are the same.  Otherwise there
                 can be an inconsistency with quoted rates.  This is
                 enforced if the frequency is lower than daily.  This
                 is not enforced for daily seasonality because this
                 will always be inconsistent due to weekends,
                 holidays, leap years, etc.  If you use multi-year
                 daily seasonality it is up to you to check.

        \note Factors are normalized relative to their appropriate
              reference dates.  For zero inflation this is the
              inflation curve true base date: since you have a fixing
              for that date the seasonality factor must be one.  For
              YoY inflation the reference is always one year earlier.

        Seasonality is treated as piecewise constant, hence it works
        correctly with uninterpolated indices if the seasonality
        correction factor frequency is the same as the index frequency
        (or less).
    */
    class MultiplicativePriceSeasonality : public Seasonality {

        private:
            Date seasonalityBaseDate_;
            Frequency frequency_;
            std::vector<Rate> seasonalityFactors_;

        public:

            //Constructors
            //
            MultiplicativePriceSeasonality() = default;

            MultiplicativePriceSeasonality(const Date& seasonalityBaseDate,
                                           Frequency frequency,
                                           const std::vector<Rate>& seasonalityFactors);

            virtual void set(const Date& seasonalityBaseDate,
                             Frequency frequency,
                             const std::vector<Rate>& seasonalityFactors);

            //! inspectors
            //@{
            virtual Date seasonalityBaseDate() const;
            virtual Frequency frequency() const;
            virtual std::vector<Rate> seasonalityFactors() const;
            //! The factor returned is NOT normalized relative to ANYTHING.
            virtual Rate seasonalityFactor(const Date &d) const;
            //@}

            //! \name Seasonality interface
            //@{
            Rate correctZeroRate(const Date& d,
                                 Rate r,
                                 const InflationTermStructure& iTS) const override;
            Rate
            correctYoYRate(const Date& d, Rate r, const InflationTermStructure& iTS) const override;
            bool isConsistent(const InflationTermStructure& iTS) const override;
            //@}

            //Destructor
            ~MultiplicativePriceSeasonality() override = default;
            ;

          protected:
            virtual void validate() const;
            virtual Rate seasonalityCorrection(Rate r, const Date &d, const DayCounter &dc,
                                               const Date &curveBaseDate, bool isZeroRate) const;
    };


    class KerkhofSeasonality : public MultiplicativePriceSeasonality {
      public:
        KerkhofSeasonality(const Date& seasonalityBaseDate,
                           const std::vector<Rate>& seasonalityFactors)
        : MultiplicativePriceSeasonality(seasonalityBaseDate,Monthly,
                                         seasonalityFactors) {}

        /*Rate correctZeroRate(const Date &d, const Rate r,
                               const InflationTermStructure& iTS) const;*/
        Real seasonalityFactor(const Date& to) const override;

      protected:
        Rate seasonalityCorrection(Rate rate,
                                   const Date& atDate,
                                   const DayCounter& dc,
                                   const Date& curveBaseDate,
                                   bool isZeroRate) const override;
    };

}  // end of namespace QuantLib

#endif



#ifndef id_b5c709f994b355e5cb3b2593b1d7fc51
#define id_b5c709f994b355e5cb3b2593b1d7fc51
inline bool test_b5c709f994b355e5cb3b2593b1d7fc51(int* i) { return i != 0; }
#endif
