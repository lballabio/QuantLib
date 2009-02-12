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

/*! \file yoyoptionletvolatilitystructures.hpp
    \brief yoy inflation volatility structures
*/

#ifndef quantlib_yoy_optionlet_volatility_structures_hpp
#define quantlib_yoy_optionlet_volatility_structures_hpp

#include <ql/termstructures/voltermstructure.hpp>
#include <ql/math/interpolation.hpp>

namespace QuantLib {

    /*! Abstract interface ... no data, only results.
        Basically used to change the BlackVariance() methods to totalVariance.
    */
    class YoYOptionletVolatilitySurface : public VolatilityTermStructure {
      public:
        //! \name Constructor
        //! calculate the reference date based on the global evaluation date
        YoYOptionletVolatilitySurface(Natural settlementDays,
                                      const Calendar&,
                                      BusinessDayConvention bdc,
                                      const DayCounter& dc,
                                      const Period &lag);

        virtual ~YoYOptionletVolatilitySurface() {}

        //! \name Volatility (only)
        //@{
        //! returns the volatility for a given exercise date and strike rate
        Volatility volatility(const Date& exerciseDate,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given option tenor and strike rate
        Volatility volatility(const Period& optionTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given exercise time and strike rate
        Volatility volatility(Time t,
                              Rate strike,
                              bool extrapolate = false) const;
        //! Returns the total integrated variance for a given exercise date and strike rate.
        /*! Total integrated variance is useful because it scales out
            t for the optionlet pricing formulae.  Note that it is
            called "total" because the surface does not know whether
            it represents Black, Bachelier or Displaced Diffusion
            variance.  These are virtual so alternate connections
            between const vol and total var are possible.
        */
        virtual Volatility totalVariance(const Date& exerciseDate,
                                         Rate strike,
                                         bool extrapolate = false) const;
        //! returns the total integrated variance for a given option tenor and strike rate
        virtual Volatility totalVariance(const Period& optionTenor,
                                         Rate strike,
                                         bool extrapolate = false) const;
        //! returns the total integrated variance for a given start time and strike rate
        virtual Volatility totalVariance(Time t,
                                         Rate strike,
                                         bool extrapolate = false) const;

        virtual Period lag() const {return lag_;}
        virtual Date baseDate() const;
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
        //! Implements the actual volatility surface calculation in
        //! derived classes e.g. bilinear interpolation.  N.B. does
        //! not derive the surface!
        virtual Volatility volatilityImpl(Time length,
                                          Rate strike) const = 0;
        void checkRange(Time, Rate strike, bool extrapolate) const;

        // acts as zero time value for boostrapping
        virtual void setBaseLevel(Volatility v) {baseLevel_ = v;}
        mutable Volatility baseLevel_;
        mutable Period lag_;
    };

    // FIXME? what about lags etc? <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    // inline definitions
    inline
    YoYOptionletVolatilitySurface::
    YoYOptionletVolatilitySurface(Natural settlementDays,
                                  const Calendar &cal,
                                  BusinessDayConvention bdc,
                                  const DayCounter& dc,
                                  const Period &lag)
    : VolatilityTermStructure(settlementDays, cal, bdc, dc),
      baseLevel_(Null<Volatility>()), lag_(lag) {}

    inline Volatility
    YoYOptionletVolatilitySurface::volatility(const Date& start,
                                              Rate strike,
                                              bool extrapolate) const {
        Time t = timeFromReference(start);
        checkRange(t, strike, extrapolate);
        return volatilityImpl(t,strike);
    }

    inline Volatility
    YoYOptionletVolatilitySurface::volatility(Time t,
                                              Rate strike,
                                              bool extrapolate) const {
        checkRange(t, strike, extrapolate);
        return volatilityImpl(t,strike);
    }

    inline Volatility
    YoYOptionletVolatilitySurface::volatility(const Period& optionTenor,
                                              Rate strike,
                                              bool extrapolate) const {
        Date exerciseDate = optionDateFromTenor(optionTenor);
        return volatility(exerciseDate, strike, extrapolate);
    }

    inline void // FIXME? what about lags etc?
    YoYOptionletVolatilitySurface::checkRange(Time t,
                                              Rate k,
                                              bool extrapolate) const {
        TermStructure::checkRange(t, extrapolate);
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

    inline Volatility
    YoYOptionletVolatilitySurface::totalVariance(const Date& start,
                                                 Rate strike,
                                                 bool extrap) const {
        Time t = timeFromReference(start);
        checkRange(t, strike, extrap);
        Volatility vol = volatilityImpl(t, strike);
        return vol*vol*t;
    }

    inline Volatility
    YoYOptionletVolatilitySurface::totalVariance(Time t,
                                                 Rate strike,
                                                 bool extrap) const {
        checkRange(t, strike, extrap);
        Volatility vol = volatilityImpl(t, strike);
        return vol*vol*t;
    }

    inline Volatility
    YoYOptionletVolatilitySurface::totalVariance(const Period& optionT,
                                                 Rate strike,
                                                 bool extrap) const {
        Date exerciseDate = optionDateFromTenor(optionT);
        return totalVariance(exerciseDate, strike, extrap);
    }

    inline Date
    YoYOptionletVolatilitySurface::baseDate() const {
        return calendar().advance(referenceDate(), -lag(),
                                  businessDayConvention());
    }



    //! Interpolated flat smile surface
    /*! (Of course, interpolated in T direction and constant in K direction.) */
    template <class Interpolator1D>
    class InterpolatedYoYOptionletVolatilityCurve
        : public YoYOptionletVolatilitySurface {
      public:
        //! \name Constructor
        //! calculate the reference date based on the global evaluation date
        InterpolatedYoYOptionletVolatilityCurve(
                       Natural settlementDays,
                       const Calendar&,
                       BusinessDayConvention bdc,
                       const DayCounter& dc,
                       const Period &lag,
                       const std::vector<Date> &d,
                       const std::vector<Volatility> &v,
                       Rate minStrike,
                       Rate maxStrike,
                       const Interpolator1D &interpolator = Interpolator1D());

        virtual ~InterpolatedYoYOptionletVolatilityCurve() {}

        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const {return minStrike_;}
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const {return maxStrike_;}
        virtual Date maxDate() const {
            //FIXME approx
            return optionDateFromTenor(
                              Period((int)ceil(interpolation_.xMax()),Years));
        }
        //@}

        //! \name Bootstrap interface
        //@{
        virtual const std::vector<Time>& times() const {return times_;}
        virtual const std::vector<Date>& dates() const {return dates_;}
        virtual const std::vector<Real>& data() const {return data_;}
        virtual std::vector<std::pair<Date, Real> > nodes() const {return nodes_;}
        //@}

      protected:
        // we need a second constructor for when we have no data
        // this is protected as we only expect to use it in the
        // piecewise versions
        InterpolatedYoYOptionletVolatilityCurve(
                       Natural settlementDays,
                       const Calendar&,
                       BusinessDayConvention bdc,
                       const DayCounter& dc,
                       const Period &lag,
                       Rate minStrike,
                       Rate maxStrike,
                       Volatility baseYoYVolatility,
                       const Interpolator1D &interpolator = Interpolator1D());

        // we do specify data representation here
        // because the bootstrapper needs this specifically
        mutable std::vector<Date> dates_;
        mutable std::vector<Time> times_;
        mutable std::vector<Real> data_;
        std::vector<std::pair<Date, Real> > nodes_;
        //@}

        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const;

         // must have this name to work with bootstrap
        Interpolator1D interpolator_;
        mutable Interpolation interpolation_;

        Rate minStrike_, maxStrike_;
    };



    /*! The dates are those of the volatility ... there is no lag on
        the dates BUT they are relative to a start date earlier than
        the reference date as always for inflation.
    */
    template<class Interpolator1D>
    InterpolatedYoYOptionletVolatilityCurve<Interpolator1D>::
    InterpolatedYoYOptionletVolatilityCurve(Natural settlementDays,
                                            const Calendar &cal,
                                            BusinessDayConvention bdc,
                                            const DayCounter& dc,
                                            const Period &lag,
                                            const std::vector<Date> &d,
                                            const std::vector<Volatility> &v,
                                            Rate minStrike,
                                            Rate maxStrike,
                                            const Interpolator1D &interpolator)
    : YoYOptionletVolatilitySurface(settlementDays, cal, bdc, dc, lag),
      dates_(d), data_(v),
      interpolator_(interpolator),
      minStrike_(minStrike), maxStrike_(maxStrike) {
        QL_REQUIRE(d.size() == v.size(),
                   "must have same number of dates and vols: "
                   << d.size() << " vs " << v.size());
        QL_REQUIRE(d.size() > 1,
                   "must have at least two dates: " << d.size());

        for (Size i = 0; i < d.size(); i++ ){
            times_.push_back( this->timeFromReference(dates_[i]) );
            nodes_.push_back( std::make_pair( dates_[i], data_[i]) );
        }

        interpolation_ =
            interpolator_.interpolate(times_.begin(),
                                      times_.end(),
                                      data_.begin() );
        // set the base vol level to that predicted by the interpolation
        // this is allowed by the extrapolation
        Time baseTime = this->timeFromReference(baseDate());
        setBaseLevel(interpolation_(baseTime,true));
    }


    template<class Interpolator1D>
    InterpolatedYoYOptionletVolatilityCurve<Interpolator1D>::
    InterpolatedYoYOptionletVolatilityCurve(Natural settlementDays,
                                const Calendar &cal,
                                            BusinessDayConvention bdc,
                                            const DayCounter& dc,
                                            const Period &lag,
                                            Rate minStrike,
                                            Rate maxStrike,
                                            Volatility baseYoYVolatility,
                                            const Interpolator1D &interpolator)
    : YoYOptionletVolatilitySurface(settlementDays, cal, bdc, dc, lag),
      interpolator_(interpolator), minStrike_(minStrike), maxStrike_(maxStrike) {
        // don't have the data yet except for the base volatility
        // must set to communicate with bootstrap
        setBaseLevel(baseYoYVolatility);
    }



    //! For the curve strike is ignored because the smile is (can only be) flat.
    template<class Interpolator1D>
    inline Volatility InterpolatedYoYOptionletVolatilityCurve<Interpolator1D>::
    volatilityImpl(const Time t, Rate strike) const {
        return interpolation_(t);
    }




    //! Constant surface, no K or T dependence.
    class ConstantYoYOptionletVolatility
        : public YoYOptionletVolatilitySurface {
      public:
        //! \name Constructor
        //@{
        //! calculate the reference date based on the global evaluation date
        ConstantYoYOptionletVolatility(const Volatility v, const Period &lag,
                                       Rate minStrike = -1.0,  // -100%
                                       Rate maxStrike = 10.0,  // +1,000%
                                       Calendar cal = TARGET() );
        //@}
        virtual ~ConstantYoYOptionletVolatility() {}

        //! \name Limits
        //@{
        virtual Date maxDate() const { return Date::maxDate();}
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const {return minStrike_;}
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const {return maxStrike_;}
        //@}

      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const;

        Volatility volatility_;
        Rate minStrike_, maxStrike_;
    };


    inline
    ConstantYoYOptionletVolatility::
    ConstantYoYOptionletVolatility(const Volatility v, const Period &lag,
                                   Rate minStrike,
                                   Rate maxStrike,
                                   Calendar cal)
    : YoYOptionletVolatilitySurface(0, cal, Unadjusted, Actual365Fixed(), lag),
      volatility_(v), minStrike_(minStrike), maxStrike_(maxStrike) {}


    inline Volatility ConstantYoYOptionletVolatility::
    volatilityImpl(const Time t, Rate strike) const {
        return volatility_;
    }

}

#endif

