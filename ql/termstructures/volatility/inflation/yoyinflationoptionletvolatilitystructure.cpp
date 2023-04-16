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

/*! \file yoyoptionletvolatilitystructures.cpp
 \brief yoy inflation volatility structures
 */

#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/volatility/inflation/yoyinflationoptionletvolatilitystructure.hpp>
#include <utility>

namespace QuantLib {

    YoYOptionletVolatilitySurface::
    YoYOptionletVolatilitySurface(Natural settlementDays,
                                  const Calendar &cal,
                                  BusinessDayConvention bdc,
                                  const DayCounter& dc,
                                  const Period& observationLag,
                                  Frequency frequency,
                                  bool indexIsInterpolated,
                                  VolatilityType volType,
                                  Real displacement)
    : VolatilityTermStructure(settlementDays, cal, bdc, dc),
      baseLevel_(Null<Volatility>()), observationLag_(observationLag),
      frequency_(frequency), indexIsInterpolated_(indexIsInterpolated),
      volType_(volType), displacement_(displacement) {
        QL_REQUIRE(close_enough(displacement, 0.0) || close_enough(displacement, 1.0),
                   "YoYOptionletVolatilitySurface: displacement (" << displacement
                                                                   << ") must be 0 or 1");
    }


    Date
    YoYOptionletVolatilitySurface::baseDate() const {

        // Depends on interpolation, or not, of observed index
        // and observation lag with which it was built.
        // We want this to work even if the index does not
        // have a yoy term structure.
        if (indexIsInterpolated()) {
            return referenceDate() - observationLag();
        } else {
            return inflationPeriod(referenceDate() - observationLag(),
                                   frequency()).first;
        }
    }


    void YoYOptionletVolatilitySurface::checkRange(const Date& d, Rate strike,
                                            bool extrapolate) const {
        QL_REQUIRE(d >= baseDate(),
                   "date (" << d << ") is before base date");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || d <= maxDate(),
                   "date (" << d << ") is past max curve date ("
                   << maxDate() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (strike >= minStrike() && strike <= maxStrike()),
                   "strike (" << strike << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]] at date = " << d);
    }


    void YoYOptionletVolatilitySurface::checkRange(Time t, Rate strike,
                                            bool extrapolate) const {
        QL_REQUIRE(t >= timeFromReference(baseDate()),
                   "time (" << t << ") is before base date");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                   << maxTime() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (strike >= minStrike() && strike <= maxStrike()),
                   "strike (" << strike << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "] at time = " << t);
    }


    Volatility
    YoYOptionletVolatilitySurface::volatility(const Date& maturityDate,
                                              Rate strike,
                                              const Period &obsLag,
                                              bool extrapolate) const {

        Period useLag = obsLag;
        if (obsLag==Period(-1,Days)) {
            useLag = observationLag();
        }

        if (indexIsInterpolated()) {
            YoYOptionletVolatilitySurface::checkRange(maturityDate-useLag, strike, extrapolate);
            Time t = timeFromReference(maturityDate-useLag);
            return volatilityImpl(t,strike);
        } else {
            std::pair<Date,Date> dd = inflationPeriod(maturityDate-useLag, frequency());
            YoYOptionletVolatilitySurface::checkRange(dd.first, strike, extrapolate);
            Time t = timeFromReference(dd.first);
            return volatilityImpl(t,strike);
        }
    }


    Volatility
    YoYOptionletVolatilitySurface::volatility(const Period& optionTenor,
                                              Rate strike,
                                              const Period &obsLag,
                                              bool extrapolate) const {
        Date maturityDate = optionDateFromTenor(optionTenor);
        return volatility(maturityDate, strike, obsLag, extrapolate);
    }

    Volatility YoYOptionletVolatilitySurface::volatility(Time time, Rate strike) const {
        return volatilityImpl(time, strike);
    }

    //! needed for total variance calculations
    Time
    YoYOptionletVolatilitySurface::timeFromBase(const Date &maturityDate,
                                                const Period& obsLag) const {

        Period useLag = obsLag;
        if (obsLag==Period(-1,Days)) {
            useLag = observationLag();
        }

        Date useDate;
        if (indexIsInterpolated()) {
            useDate = maturityDate - useLag;
        } else {
            useDate = inflationPeriod(maturityDate - useLag,
                                      frequency()).first;
        }

        // This assumes that the inflation term structure starts
        // as late as possible given the inflation index definition,
        // which is the usual case.
        return dayCounter().yearFraction(baseDate(), useDate);
    }


    Volatility
    YoYOptionletVolatilitySurface::totalVariance(const Date& maturityDate,
                                                 Rate strike,
                                                 const Period &obsLag,
                                                 bool extrapolate) const {

        Volatility vol = volatility(maturityDate, strike, obsLag, extrapolate);
        Time t = timeFromBase(maturityDate, obsLag);
        return vol*vol*t;
    }


    Volatility
    YoYOptionletVolatilitySurface::totalVariance(const Period& tenor,
                                                 Rate strike,
                                                 const Period &obsLag,
                                                 bool extrap) const {
        Date maturityDate = optionDateFromTenor(tenor);
        return totalVariance(maturityDate, strike, obsLag, extrap);
    }



    //========================================================================
    // constant yoy vol surface
    //========================================================================

    ConstantYoYOptionletVolatility::
    ConstantYoYOptionletVolatility(const Volatility v,
                                   Natural settlementDays,
                                   const Calendar& cal,
                                   BusinessDayConvention bdc,
                                   const DayCounter& dc,
                                   const Period &observationLag,
                                   Frequency frequency,
                                   bool indexIsInterpolated,
                                   Rate minStrike,
                                   Rate maxStrike,
                                   VolatilityType volType,
                                   Real displacement)
    : YoYOptionletVolatilitySurface(settlementDays, cal, bdc, dc,
                                    observationLag, frequency, indexIsInterpolated,
                                    volType, displacement),
      volatility_(std::make_shared<SimpleQuote>(v)), minStrike_(minStrike), maxStrike_(maxStrike) {}

    ConstantYoYOptionletVolatility::ConstantYoYOptionletVolatility(Handle<Quote> v,
                                                                   Natural settlementDays,
                                                                   const Calendar& cal,
                                                                   BusinessDayConvention bdc,
                                                                   const DayCounter& dc,
                                                                   const Period& observationLag,
                                                                   Frequency frequency,
                                                                   bool indexIsInterpolated,
                                                                   Rate minStrike,
                                                                   Rate maxStrike,
                                                                   VolatilityType volType,
                                                                   Real displacement)
    : YoYOptionletVolatilitySurface(settlementDays,
                                    cal,
                                    bdc,
                                    dc,
                                    observationLag,
                                    frequency,
                                    indexIsInterpolated,
                                    volType,
                                    displacement),
      volatility_(std::move(v)), minStrike_(minStrike), maxStrike_(maxStrike) {}

    Volatility ConstantYoYOptionletVolatility::volatilityImpl(Time, Rate) const {
        return volatility_->value();
    }

} // namespace QuantLib
