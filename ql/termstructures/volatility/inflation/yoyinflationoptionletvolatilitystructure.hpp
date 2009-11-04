/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file yoyinflationoptionletvolatilitystructure.hpp
    \brief yoy inflation volatility structures
 */

#ifndef quantlib_yoy_optionlet_volatility_structures_hpp
#define quantlib_yoy_optionlet_volatility_structures_hpp

#include <ql/termstructures/voltermstructure.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/time/calendars/target.hpp>

namespace QuantLib {

    /*! Abstract interface ... no data, only results.

        Basically used to change the BlackVariance() methods to
        totalVariance.  Also deal with lagged observations of an index
        with a (usually different) availability lag.
    */
    class YoYOptionletVolatilitySurface : public VolatilityTermStructure {
    public:
        //! \name Constructor
        //! calculate the reference date based on the global evaluation date
        YoYOptionletVolatilitySurface(Natural settlementDays,
                                      const Calendar&,
                                      BusinessDayConvention bdc,
                                      const DayCounter& dc,
                                      const Period& observationLag,
                                      Frequency frequency,
                                      bool indexIsInterpolated);

        virtual ~YoYOptionletVolatilitySurface() {}

        //! \name Volatility (only)
        //@{
        //! Returns the volatility for a given maturity date and strike rate
        //! that observes inflation, by default, with the observation lag
        //! of the term structure.
        //! Because inflation is highly linked to dates (for interpolation, periods, etc)
        //! we do NOT provide a time version.
        Volatility volatility(const Date& maturityDate,
                              Rate strike,
                              const Period &obsLag = Period(-1,Days),
                              bool extrapolate = false) const;
        //! returns the volatility for a given option tenor and strike rate
        Volatility volatility(const Period& optionTenor,
                              Rate strike,
                              const Period &obsLag = Period(-1,Days),
                              bool extrapolate = false) const;

        //! Returns the total integrated variance for a given exercise date and strike rate.
        /*! Total integrated variance is useful because it scales out
         t for the optionlet pricing formulae.  Note that it is
         called "total" because the surface does not know whether
         it represents Black, Bachelier or Displaced Diffusion
         variance.  These are virtual so alternate connections
         between const vol and total var are possible.

         Because inflation is highly linked to dates (for interpolation, periods, etc)
         we do NOT provide a time version
         */
        virtual Volatility totalVariance(const Date& exerciseDate,
                                         Rate strike,
                                         const Period &obsLag = Period(-1,Days),
                                         bool extrapolate = false) const;
        //! returns the total integrated variance for a given option tenor and strike rate
        virtual Volatility totalVariance(const Period& optionTenor,
                                         Rate strike,
                                         const Period &obsLag = Period(-1,Days),
                                         bool extrapolate = false) const;

        //! The TS observes with a lag that is usually different from the
        //! availability lag of the index.  An inflation rate is given,
        //! by default, for the maturity requested assuming this lag.
        virtual Period observationLag() const { return observationLag_; }
        virtual Frequency frequency() const { return frequency_; }
        virtual bool indexIsInterpolated() const { return indexIsInterpolated_; }
        virtual Date baseDate() const;
        //! base date will be in the past because of observation lag
        virtual Time timeFromBase(const Date &date,
                                  const Period& obsLag = Period(-1,Days)) const;
        //@}

        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const = 0;
        //@}

        // acts as zero time value for boostrapping
        virtual Volatility baseLevel() const {
            QL_REQUIRE(baseLevel_ != Null<Volatility>(),
                       "Base volatility, for baseDate(), not set.");
            return baseLevel_;
        }

    protected:
        virtual void checkRange(const Date &, Rate strike, bool extrapolate) const;
        virtual void checkRange(Time, Rate strike, bool extrapolate) const;

        //! Implements the actual volatility surface calculation in
        //! derived classes e.g. bilinear interpolation.  N.B. does
        //! not derive the surface.
        virtual Volatility volatilityImpl(Time length,
                                          Rate strike) const = 0;

        // acts as zero time value for boostrapping
        virtual void setBaseLevel(Volatility v) { baseLevel_ = v; }
        mutable Volatility baseLevel_;

        // so you do not need an index
        Period observationLag_;
        Frequency frequency_;
        bool indexIsInterpolated_;
    };


    //! Constant surface, no K or T dependence.
    class ConstantYoYOptionletVolatility
    : public YoYOptionletVolatilitySurface {
    public:
        //! \name Constructor
        //@{
        //! calculate the reference date based on the global evaluation date
        ConstantYoYOptionletVolatility(const Volatility v,
                                       Natural settlementDays,
                                       const Calendar&,
                                       BusinessDayConvention bdc,
                                       const DayCounter& dc,
                                       const Period &observationLag,
                                       Frequency frequency,
                                       bool indexIsInterpolated,
                                       Rate minStrike = -1.0,  // -100%
                                       Rate maxStrike = 100.0);  // +10,000%
        //@}
        virtual ~ConstantYoYOptionletVolatility() {}

        //! \name Limits
        //@{
        virtual Date maxDate() const { return Date::maxDate(); }
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const { return minStrike_; }
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const { return maxStrike_; }
        //@}

    protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const;

        Volatility volatility_;
        Rate minStrike_, maxStrike_;
    };



} // namespace QuantLib

#endif

