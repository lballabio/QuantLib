/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

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

#include <ql/processes/gsrprocess.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    GsrProcess::GsrProcess(const Array& times,
                           const Array& vols,
                           const Array& reversions,
                           const Real T,
                           const Date& referenceDate,
                           DayCounter dc)
    : ForwardMeasureProcess1D(T), core_(times, vols, reversions, T), referenceDate_(referenceDate),
      dc_(std::move(dc)) {
        flushCache();
    }

    void GsrProcess::checkT(const Time t) const {
        QL_REQUIRE(t <= getForwardMeasureTime() && t >= 0.0,
                   "t (" << t
                         << ") must not be greater than forward measure time ("
                         << getForwardMeasureTime() << ") and non-negative");
    }

    Real GsrProcess::time(const Date &d) const {
        QL_REQUIRE(
            referenceDate_ != Date() && dc_ != DayCounter(),
            "time can not be computed without reference date and day counter");
        return dc_.yearFraction(referenceDate_, d);
    }

    Real GsrProcess::x0() const { return 0.0; }

    Real GsrProcess::drift(Time t, Real x) const {
        return core_.y(t) -
               core_.G(t, getForwardMeasureTime()) * sigma(t) * sigma(t) -
               reversion(t) * x;
    }

    Real GsrProcess::diffusion(Time t, Real) const {
        checkT(t);
        return sigma(t);
    }

    Real GsrProcess::expectation(Time w, Real xw, Time dt) const {
        checkT(w + dt);
        return core_.expectation_x0dep_part(w, xw, dt) +
               core_.expectation_rn_part(w, dt) +
               core_.expectation_tf_part(w, dt);
    }



    Real GsrProcess::stdDeviation(Time t0, Real x0, Time dt) const {
        return std::sqrt(variance(t0, x0, dt));
    }

    Real GsrProcess::variance(Time w, Real, Time dt) const {
        checkT(w + dt);
        return core_.variance(w,dt);
    }

    Real GsrProcess::sigma(Time t) const { return core_.sigma(t); }

    Real GsrProcess::reversion(Time t) const { return core_.reversion(t); }

    Real GsrProcess::y(Time t) const {
        checkT(t);
        return core_.y(t);
    }

    Real GsrProcess::G(Time t, Time w, Real) const {
        QL_REQUIRE(w >= t, "G(t,w) should be called with w ("
                               << w << ") not lesser than t (" << t << ")");
        QL_REQUIRE(t >= 0.0 && w <= getForwardMeasureTime(),
                   "G(t,w) should be called with (t,w)=("
                       << t << "," << w << ") in Range [0,"
                       << getForwardMeasureTime() << "].");

        return core_.G(t,w);
    }


}
