/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fxexpirycut.hpp
    \brief Time of day at which FX options expire
*/

#ifndef quantlib_fx_expiry_cut_hpp
#define quantlib_fx_expiry_cut_hpp

#include <ql/errors.hpp>
#include <ql/time/date.hpp>

namespace QuantLib {

    //! Time of day at which FX options expire, e.g. the New York 10:00 cut
    /*! Times of day in the FX analytics, the cut's and those of vol
        events alike, are on a single clock, New York time; converting
        cuts and events from other time zones is left to the caller.

        Dates are whole days, so an option expiring on a date is measured
        to the start of that date, and the cut decides only which events
        it sees: those at instants strictly before the cut on its expiry
        date.  An event at the cut itself, e.g. a 10:00 release against
        the 10:00 cut, comes after the expiry.

        The cut is not part of the settlement convention: it does not
        change any value date, and options on one pair trade with several
        cuts.
    */
    class FxExpiryCut {
      public:
        //! The cut at the given time on the New York clock.
        explicit FxExpiryCut(Natural hour, Natural minute = 0)
        : timeOfDay_((hour * 60.0 + minute) / (24.0 * 60.0)) {
            QL_REQUIRE(hour < 24 && minute < 60,
                       "invalid expiry cut " << hour << ":" << minute);
            QL_REQUIRE(timeOfDay_ > 0.0, "an expiry cut at midnight is not supported");
        }

        //! The cut as a fraction of the day, in (0, 1).
        Real timeOfDay() const { return timeOfDay_; }

        //! Whether the instant at the time of day on the date comes before the cut on the expiry.
        /*! Instants closer than \f$ 10^{-9} \f$ of a day (about 0.1 ms)
            are the same instant, so that times of day computed in
            different ways compare as equal.
        */
        bool precedes(const Date& date, Real timeOfDay, const Date& expiry) const {
            const auto days = static_cast<Real>(expiry.serialNumber()) -
                              static_cast<Real>(date.serialNumber());
            return timeOfDay < days + timeOfDay_ - sameInstant;
        }

        //! Instants closer than this fraction of a day are the same instant.
        static constexpr Real sameInstant = 1.0e-9;

      private:
        Real timeOfDay_;
    };

}

#endif
