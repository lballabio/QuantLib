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

/*! \file yoyinflationoptionletvolatilitystructure2.hpp
    \brief experimental yoy inflation volatility structures
 */

#ifndef quantlib_yoy_optionlet_volatility_structures2_hpp
#define quantlib_yoy_optionlet_volatility_structures2_hpp

#include <ql/termstructures/voltermstructure.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/termstructures/volatility/inflation/yoyinflationoptionletvolatilitystructure.hpp>

namespace QuantLib {

    //! Interpolated flat smile surface
    /*! Interpolated in T direction and constant in K direction. */
    template <class Interpolator1D>
    class InterpolatedYoYOptionletVolatilityCurve
        : public YoYOptionletVolatilitySurface,
          protected InterpolatedCurve<Interpolator1D> {
    public:
        //! \name Constructor
        //@{
        //! calculate the reference date based on the global evaluation date
        /*! The dates are those of the volatility ... there is no lag
            on the dates <b>but</b> they are relative to a start date
            earlier than the reference date as always for inflation.
        */
        InterpolatedYoYOptionletVolatilityCurve(Natural settlementDays,
                                                const Calendar&,
                                                BusinessDayConvention bdc,
                                                const DayCounter& dc,
                                                const Period &lag,
                                                Frequency frequency,
                                                bool indexIsInterpolated,
                                                const std::vector<Date> &d,
                                                const std::vector<Volatility> &v,
                                                Rate minStrike,
                                                Rate maxStrike,
                                                const Interpolator1D &i =
                                                            Interpolator1D());
        //@}

        ~InterpolatedYoYOptionletVolatilityCurve() override = default;

        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        Real minStrike() const override { return minStrike_; }
        //! the maximum strike for which the term structure can return vols
        Real maxStrike() const override { return maxStrike_; }
        Date maxDate() const override {
            //FIXME approx
            return optionDateFromTenor(Period((int)ceil(this->interpolation_.xMax()),Years));
        }
        //@}

        //! \name Bootstrap interface
        //@{
        virtual const std::vector<Time>& times() const {return this->times_;}
        virtual const std::vector<Date>& dates() const {return dates_;}
        virtual const std::vector<Real>& data() const {return this->data_;}
        virtual std::vector<std::pair<Date, Real> > nodes() const {return nodes_;}
        //@}

    protected:
        // we need a second constructor for when we have no data
        // this is protected as we only expect to use it in the
        // piecewise versions
        InterpolatedYoYOptionletVolatilityCurve(Natural settlementDays,
                                                const Calendar&,
                                                BusinessDayConvention bdc,
                                                const DayCounter& dc,
                                                const Period &lag,
                                                Frequency frequency,
                                                bool indexIsInterpolated,
                                                Rate minStrike,
                                                Rate maxStrike,
                                                Volatility baseYoYVolatility,
                                                const Interpolator1D &i =
                                                            Interpolator1D());

        // we do specify data representation here
        // because the bootstrapper needs this specifically
        mutable std::vector<Date> dates_;
        std::vector<std::pair<Date, Real> > nodes_;
        //@}

        //! implements the actual volatility calculation in derived classes
        Volatility volatilityImpl(Time length, Rate strike) const override;

        Rate minStrike_, maxStrike_;
    };



    template<class Interpolator1D>
    InterpolatedYoYOptionletVolatilityCurve<Interpolator1D>::
    InterpolatedYoYOptionletVolatilityCurve(Natural settlementDays,
                                            const Calendar &cal,
                                            BusinessDayConvention bdc,
                                            const DayCounter& dc,
                                            const Period &lag,
                                            Frequency frequency,
                                            bool indexIsInterpolated,
                                            const std::vector<Date> &d,
                                            const std::vector<Volatility> &v,
                                            Rate minStrike,
                                            Rate maxStrike,
                                            const Interpolator1D &i)
    : YoYOptionletVolatilitySurface(settlementDays, cal, bdc, dc, lag,
                                    frequency, indexIsInterpolated),
      InterpolatedCurve<Interpolator1D>(i),
      dates_(d), minStrike_(minStrike), maxStrike_(maxStrike) {
        QL_REQUIRE(d.size() == v.size(),
                   "must have same number of dates and vols: "
                   << d.size() << " vs " << v.size());
        QL_REQUIRE(d.size() > 1,
                   "must have at least two dates: " << d.size());

        for (Size j = 0; j < d.size(); j++ ){
            this->times_.push_back( this->timeFromReference(dates_[j]) );
            this->data_.push_back(v[j]),
            nodes_.push_back( std::make_pair( dates_[j], this->data_[j]) );
        }

        this->setupInterpolation();
        // set the base vol level to that predicted by the interpolation
        // this is allowed by the extrapolation
        Time baseTime = this->timeFromReference(baseDate());
        setBaseLevel(this->interpolation_(baseTime,true));
    }


    template<class Interpolator1D>
    InterpolatedYoYOptionletVolatilityCurve<Interpolator1D>::
    InterpolatedYoYOptionletVolatilityCurve(Natural settlementDays,
                                            const Calendar &cal,
                                            BusinessDayConvention bdc,
                                            const DayCounter& dc,
                                            const Period &lag,
                                            Frequency frequency,
                                            bool indexIsInterpolated,
                                            Rate minStrike,
                                            Rate maxStrike,
                                            Volatility baseYoYVolatility,
                                            const Interpolator1D &i)
    : YoYOptionletVolatilitySurface(settlementDays, cal, bdc, dc, lag,
                                    frequency, indexIsInterpolated),
      InterpolatedCurve<Interpolator1D>(i),
      minStrike_(minStrike), maxStrike_(maxStrike) {
        // don't have the data yet except for the base volatility
        // must set to communicate with bootstrap
        setBaseLevel(baseYoYVolatility);
    }



    //! For the curve strike is ignored because the smile is (can only be) flat.
    template<class Interpolator1D>
    inline Volatility InterpolatedYoYOptionletVolatilityCurve<Interpolator1D>::
    volatilityImpl(const Time t,
                   Rate) const {
        return this->interpolation_(t);
    }

} // namespace QuantLib

#endif


#ifndef id_a03d961bf7e8c2a1ef13c95ae239e3a9
#define id_a03d961bf7e8c2a1ef13c95ae239e3a9
inline bool test_a03d961bf7e8c2a1ef13c95ae239e3a9(int* i) { return i != 0; }
#endif
