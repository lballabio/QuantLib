/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Chris Kenyon

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

/*! \file cpivolatilitystructure.hpp
    \brief zero inflation (i.e. CPI/RPI/HICP/etc.) volatility structures
 */

#ifndef quantlib_cpi_volatility_structure_hpp
#define quantlib_cpi_volatility_structure_hpp

#include <ql/termstructures/voltermstructure.hpp>

namespace QuantLib {

    //! zero inflation (i.e. CPI/RPI/HICP/etc.) volatility structures
    /*! Abstract interface. CPI volatility is always with respect to
        some base date.  Also deal with lagged observations of an index
        with a (usually different) availability lag.
    */
    class CPIVolatilitySurface : public VolatilityTermStructure {
      public:
        /*! calculates the reference date based on the global
            evaluation date.
        */
        CPIVolatilitySurface(Natural settlementDays,
                             const Calendar&,
                             BusinessDayConvention bdc,
                             const DayCounter& dc,
                             const Period& observationLag,
                             Frequency frequency,
                             bool indexIsInterpolated);

        //! \name Volatility
        /*! by default, inflation is observed with the lag
            of the term structure.

            Because inflation is highly linked to dates (for
            interpolation, periods, etc) time-based overload of the
            methods are not provided.
        */
        //@{
        //! Returns the volatility for a given maturity date and strike rate.
        Volatility volatility(const Date& maturityDate,
                              Rate strike,
                              const Period &obsLag = Period(-1,Days),
                              bool extrapolate = false) const;
        //! returns the volatility for a given option tenor and strike rate
        Volatility volatility(const Period& optionTenor,
                              Rate strike,
                              const Period &obsLag = Period(-1,Days),
                              bool extrapolate = false) const;
        /*! Returns the volatility for a given time and strike rate. No adjustments
          due to lags and interpolation are applied to the input time. */
        Volatility volatility(Time time, Rate strike) const;

        //! Returns the total integrated variance for a given exercise
        //! date and strike rate.
        /*! Total integrated variance is useful because it scales out
            t for the optionlet pricing formulae.  Note that it is
            called "total" because the surface does not know whether
            it represents Black, Bachelier or Displaced Diffusion
            variance.  These are virtual so alternate connections
            between const vol and total var are possible.
        */
        virtual Volatility totalVariance(const Date& exerciseDate,
                                         Rate strike,
                                         const Period &obsLag = Period(-1,Days),
                                         bool extrapolate = false) const;
        //! returns the total integrated variance for a given option
        //! tenor and strike rate.
        virtual Volatility totalVariance(const Period& optionTenor,
                                         Rate strike,
                                         const Period &obsLag = Period(-1,Days),
                                         bool extrapolate = false) const;
        //@}

        //! \name Inspectors
        //@{
        /*! The term structure observes with a lag that is usually
            different from the availability lag of the index.  An
            inflation rate is given, by default, for the maturity
            requested assuming this lag.
        */
        virtual Period observationLag() const { return observationLag_; }
        virtual Frequency frequency() const { return frequency_; }
        virtual bool indexIsInterpolated() const {
            return indexIsInterpolated_;
        }
        virtual Date baseDate() const;
        //! base date will be in the past because of observation lag
        virtual Time timeFromBase(const Date &date,
                                  const Period& obsLag = Period(-1,Days)) const;
        // acts as zero time value for boostrapping
        virtual Volatility baseLevel() const {
            QL_REQUIRE(baseLevel_ != Null<Volatility>(),
                       "Base volatility, for baseDate(), not set.");
            return baseLevel_;
        }
        //@}

        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        Real minStrike() const override = 0;
        //! the maximum strike for which the term structure can return vols
        Real maxStrike() const override = 0;
        //@}
      protected:
        virtual void checkRange(const Date&, Rate strike, bool extrapolate) const;
        virtual void checkRange(Time, Rate strike, bool extrapolate) const;

        /*! Implements the actual volatility surface calculation in
            derived classes e.g. bilinear interpolation.  N.B. does
            not derive the surface.
        */
        virtual Volatility volatilityImpl(Time length,
                                          Rate strike) const = 0;

        mutable Volatility baseLevel_;
        // so you do not need an index
        Period observationLag_;
        Frequency frequency_;
        bool indexIsInterpolated_;
    };

}

#endif

