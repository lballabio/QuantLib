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

using std::exp;
using std::pow;
using std::sqrt;

namespace QuantLib {

GsrProcess::GsrProcess(const Array &times, const Array &vols,
                       const Array &reversions, const Real T)
    : ForwardMeasureProcess1D(T), times_(times), vols_(vols),
      reversions_(reversions), revZero_(reversions.size(), false) {
        QL_REQUIRE(times.size() == vols.size() - 1,
                   "number of volatilities ("
                       << vols.size() << ") compared to number of times ("
                       << times_.size() << " must be bigger by one");
        QL_REQUIRE(times.size() == reversions.size() - 1 ||
                       reversions.size() == 1,
                   "number of reversions ("
                       << vols.size() << ") compared to number of times ("
                       << times_.size() << " must be bigger by one, or exactly "
                                           "1 reversion must be given");
        for (int i = 0; i < ((int)times.size()) - 1; i++)
            QL_REQUIRE(times[i] < times[i + 1], "times must be increasing ("
                                                    << times[i] << "@" << i
                                                    << " , " << times[i + 1]
                                                    << "@" << i + 1 << ")");
        flushCache();
    }

    Real GsrProcess::x0() const { return 0.0; }

    Real GsrProcess::drift(Time t, Real x) const {
        QL_REQUIRE(t <= getForwardMeasureTime(),
                   "t (" << t
                         << ") must not be greater than forward measure time ("
                         << getForwardMeasureTime() << ")");
        return y(t) - G(t, this->getForwardMeasureTime(), x) *
                          vol(lowerIndex(t)) * vol(lowerIndex(t)) -
               rev(lowerIndex(t)) * x;
    }

    Real GsrProcess::diffusion(Time t, Real) const {
        QL_REQUIRE(t <= getForwardMeasureTime(),
                   "t (" << t
                         << ") must not be greater than forward measure time ("
                         << getForwardMeasureTime() << ")");
        return vol(lowerIndex(t));
    }

    Real GsrProcess::expectation(Time w, Real xw, Time dt) const {

        Real t = w + dt;
        QL_REQUIRE(t <= getForwardMeasureTime(),
                   "t (" << t
                         << ") must not be greater than forward measure time ("
                         << getForwardMeasureTime() << ")");

        return expectationp1(w, xw, dt) + expectationp2(w, dt);
    }

    void GsrProcess::flushCache() const {
        // this method must be called if parameters change (see the note
        // in the header), so we can ensure here that the zero reversion
        // flag is kept consistent, too
        for (int i = 0; i < (int)reversions_.size(); i++)
            // if (close(reversions_[i], 0.0))
            if (std::fabs(reversions_[i]) < 1E-4)
                revZero_[i] = true;
            else
                revZero_[i] = false;
        cache1_.clear();
        cache2_.clear();
        cache3_.clear();
        cache4_.clear();
        cache5_.clear();
    }

    Real GsrProcess::expectationp1(Time w, Real xw, Time dt) const {
        Real t = w + dt;
        std::pair<Real, Real> key;
        key = std::make_pair(w, t);
        std::map<std::pair<Real, Real>, Real>::const_iterator k =
            cache1_.find(key);
        if (k != cache1_.end())
            return xw * (k->second);
        // A(w,t)x(w)
        Real res2 = 1.0;
        for (int i = lowerIndex(w); i <= upperIndex(t) - 1; i++) {
            res2 *= exp(-rev(i) * (cappedTime(i + 1, t) - flooredTime(i, w)));
        }
        cache1_.insert(std::make_pair(key, res2));
        return res2 * xw;
    }

    Real GsrProcess::expectationp2(Time w, Time dt) const {

        Real t = w + dt;

        std::pair<Real, Real> key;
        key = std::make_pair(w, t);
        std::map<std::pair<Real, Real>, Real>::const_iterator k =
            cache2_.find(key);
        if (k != cache2_.end())
            return k->second;

        Real T = getForwardMeasureTime();

        Real res = 0.0;

        // \int A(s,t)y(s)
        for (int k = lowerIndex(w); k <= upperIndex(t) - 1; k++) {
            // l<k
            for (int l = 0; l <= k - 1; l++) {
                Real res2 = 1.0;
                // alpha_l
                res2 *= revZero(l) ? vol(l) * vol(l) * (time2(l + 1) - time2(l))
                                   : vol(l) * vol(l) / (2.0 * rev(l)) *
                                         (1.0 - exp(-2.0 * rev(l) *
                                                    (time2(l + 1) - time2(l))));
                // zeta_i (i>k)
                for (int i = k + 1; i <= upperIndex(t) - 1; i++)
                    res2 *= exp(-rev(i) * (cappedTime(i + 1, t) - time2(i)));
                // beta_j (j<k)
                for (int j = l + 1; j <= k - 1; j++)
                    res2 *= exp(-2.0 * rev(j) * (time2(j + 1) - time2(j)));
                // zeta_k beta_k
                res2 *=
                    revZero(k)
                        ? 2.0 * time2(k) - flooredTime(k, w) -
                              cappedTime(k + 1, t) -
                              2.0 * (time2(k) - cappedTime(k + 1, t))
                        : (exp(rev(k) * (2.0 * time2(k) - flooredTime(k, w) -
                                         cappedTime(k + 1, t))) -
                           exp(2.0 * rev(k) *
                               (time2(k) - cappedTime(k + 1, t)))) /
                              rev(k);
                // add to sum
                res += res2;
            }
            // l=k
            Real res2 = 1.0;
            // alpha_k zeta_k
            res2 *=
                revZero(k)
                    ? vol(k) * vol(k) / 4.0 *
                          (4.0 * pow(cappedTime(k + 1, t) - time2(k), 2.0) -
                           (pow(flooredTime(k, w) - 2.0 * time2(k) +
                                    cappedTime(k + 1, t),
                                2.0) +
                            pow(cappedTime(k + 1, t) - flooredTime(k, w), 2.0)))
                    : vol(k) * vol(k) / (2.0 * rev(k) * rev(k)) *
                          (exp(-2.0 * rev(k) *
                               (cappedTime(k + 1, t) - time2(k))) +
                           1.0 -
                           (exp(-rev(k) * (flooredTime(k, w) - 2.0 * time2(k) +
                                           cappedTime(k + 1, t))) +
                            exp(-rev(k) *
                                (cappedTime(k + 1, t) - flooredTime(k, w)))));
            // zeta_i (i>k)
            for (int i = k + 1; i <= upperIndex(t) - 1; i++)
                res2 *= exp(-rev(i) * (cappedTime(i + 1, t) - time2(i)));
            // no beta_j in this case ...
            res += res2;
        }

        // int -A(s,t) \sigma^2 G(s,T)
        for (int k = lowerIndex(w); k <= upperIndex(t) - 1; k++) {
            Real res2 = 0.0;
            // l>k
            for (int l = k + 1; l <= upperIndex(T) - 1; l++) {
                Real res3 = 1.0;
                // eta_l
                res3 *= revZero(l)
                            ? cappedTime(l + 1, T) - time2(l)
                            : (1.0 - exp(-rev(l) *
                                         (cappedTime(l + 1, T) - time2(l)))) /
                                  rev(l);
                // zeta_i (i>k)
                for (int i = k + 1; i <= upperIndex(t) - 1; i++)
                    res3 *= exp(-rev(i) * (cappedTime(i + 1, t) - time2(i)));
                // gamma_j (j>k)
                for (int j = k + 1; j <= l - 1; j++)
                    res3 *= exp(-rev(j) * (time2(j + 1) - time2(j)));
                // zeta_k gamma_k
                res3 *=
                    revZero(k)
                        ? (cappedTime(k + 1, t) - time2(k + 1) -
                           (2.0 * flooredTime(k, w) - cappedTime(k + 1, t) -
                            time2(k + 1))) /
                              2.0
                        : (exp(rev(k) * (cappedTime(k + 1, t) - time2(k + 1))) -
                           exp(rev(k) *
                               (2.0 * flooredTime(k, w) - cappedTime(k + 1, t) -
                                time2(k + 1)))) /
                              (2.0 * rev(k));
                // add to sum
                res2 += res3;
            }
            // l=k
            Real res3 = 1.0;
            // eta_k zeta_k
            res3 *=
                revZero(k)
                    ? (-pow(cappedTime(k + 1, t) - cappedTime(k + 1, T), 2.0) -
                       2.0 *
                           pow(cappedTime(k + 1, t) - flooredTime(k, w), 2.0) +
                       pow(2.0 * flooredTime(k, w) - cappedTime(k + 1, T) -
                               cappedTime(k + 1, t),
                           2.0)) /
                          4.0
                    : (2.0 - exp(rev(k) * (cappedTime(k + 1, t) -
                                           cappedTime(k + 1, T))) -
                       (2.0 * exp(-rev(k) *
                                  (cappedTime(k + 1, t) - flooredTime(k, w))) -
                        exp(rev(k) *
                            (2.0 * flooredTime(k, w) - cappedTime(k + 1, T) -
                             cappedTime(k + 1, t))))) /
                          (2.0 * rev(k) * rev(k));
            // zeta_i (i>k)
            for (int i = k + 1; i <= upperIndex(t) - 1; i++)
                res3 *= exp(-rev(i) * (cappedTime(i + 1, t) - time2(i)));
            // no gamma_j in this case ...
            res2 += res3;
            // add to main accumulator
            res += -vol(k) * vol(k) * res2;
        }

        cache2_.insert(std::make_pair(key, res));

        return res;
    }

    Real GsrProcess::stdDeviation(Time t0, Real x0, Time dt) const {
        return sqrt(variance(t0, x0, dt));
    }

    Real GsrProcess::variance(Time w, Real, Time dt) const {

        Real t = w + dt;
        QL_REQUIRE(t <= getForwardMeasureTime(),
                   "t (" << t
                         << ") must not be greater than forward measure time ("
                         << getForwardMeasureTime() << ")");

        std::pair<Real, Real> key;
        key = std::make_pair(w, t);
        std::map<std::pair<Real, Real>, Real>::const_iterator k =
            cache3_.find(key);
        if (k != cache3_.end())
            return k->second;

        Real res = 0.0;
        for (int k = lowerIndex(w); k <= upperIndex(t) - 1; k++) {
            Real res2 = vol(k) * vol(k);
            // zeta_k^2
            res2 *= revZero(k)
                        ? -(flooredTime(k, w) - cappedTime(k + 1, t))
                        : (1.0 - exp(2.0 * rev(k) * (flooredTime(k, w) -
                                                     cappedTime(k + 1, t)))) /
                              (2.0 * rev(k));
            // zeta_i (i>k)
            for (int i = k + 1; i <= upperIndex(t) - 1; i++) {
                res2 *= exp(-2.0 * rev(i) * (cappedTime(i + 1, t) - time2(i)));
            }
            res += res2;
        }

        cache3_.insert(std::make_pair(key, res));
        return res;
    }

    Real GsrProcess::sigma(Time t) const { return vol(lowerIndex(t)); }

    Real GsrProcess::reversion(Time t) const { return rev(lowerIndex(t)); }

    Real GsrProcess::y(Time t) const {

        QL_REQUIRE(t >= 0.0 && t <= getForwardMeasureTime(),
                   "y(t) should be called with t (" << t << ") in Range [0,"
                                                    << getForwardMeasureTime()
                                                    << "].");

        Real key;
        key = t;
        std::map<Real, Real>::const_iterator k = cache4_.find(key);
        if (k != cache4_.end())
            return k->second;

        Real res = 0.0;
        for (int i = 0; i <= upperIndex(t) - 1; i++) {
            Real res2 = 1.0;
            for (int j = i + 1; j <= upperIndex(t) - 1; j++) {
                res2 *= exp(-2.0 * rev(j) * (cappedTime(j + 1, t) - time2(j)));
            }
            res2 *= revZero(i)
                        ? vol(i) * vol(i) * (cappedTime(i + 1, t) - time2(i))
                        : (vol(i) * vol(i) / (2.0 * rev(i)) *
                           (1.0 - exp(-2.0 * rev(i) *
                                      (cappedTime(i + 1, t) - time2(i)))));
            res += res2;
        }

        cache4_.insert(std::make_pair(key, res));
        return res;
    }

    Real GsrProcess::G(Time t, Time w, Real) const {

        QL_REQUIRE(w >= t, "G(t,w) should be called with w ("
                               << w << ") not lesser than t (" << t << ")");
        QL_REQUIRE(t >= 0.0 && w <= getForwardMeasureTime(),
                   "G(t,w) should be called with (t,w)=("
                       << t << "," << w << ") in Range [0,"
                       << getForwardMeasureTime() << "].");

        std::pair<Real, Real> key;
        key = std::make_pair(w, t);
        std::map<std::pair<Real, Real>, Real>::const_iterator k =
            cache5_.find(key);
        if (k != cache5_.end())
            return k->second;

        Real res = 0.0;
        for (int i = lowerIndex(t); i <= upperIndex(w) - 1; i++) {
            Real res2 = 1.0;
            for (int j = lowerIndex(t); j <= i - 1; j++) {
                res2 *= exp(-rev(j) * (time2(j + 1) - flooredTime(j, t)));
            }
            res2 *= revZero(i) ? cappedTime(i + 1, w) - flooredTime(i, t)
                               : (1.0 - exp(-rev(i) * (cappedTime(i + 1, w) -
                                                       flooredTime(i, t)))) /
                                     rev(i);
            res += res2;
        }

        cache5_.insert(std::make_pair(key, res));
        return res;
    }

    const int GsrProcess::lowerIndex(Time t) const {
        return static_cast<int>(std::upper_bound(times_.begin(), times_.end(), t) -
                           times_.begin());
    }

    const int GsrProcess::upperIndex(Time t) const {
        if (t < QL_EPSILON)
            return 0;
        return static_cast<int>(std::upper_bound(times_.begin(), times_.end(), t - QL_EPSILON) -
                                times_.begin()) + 1;
    }

    const Real GsrProcess::cappedTime(Size index, Real cap) const {
        return cap != Null<Real>() ? std::min(cap, time2(index)) : time2(index);
    }

    const Real GsrProcess::flooredTime(Size index, Real floor) const {
        return floor != Null<Real>() ? std::max(floor, time2(index))
                                     : time2(index);
    }

    const Real GsrProcess::time2(Size index) const {
        if (index == 0)
            return 0.0;
        if (index > times_.size())
            return getForwardMeasureTime(); // FIXME how to ensure that forward
                                            // measure time is geq all times
                                            // given
        return times_[index - 1];
    }

    const Real GsrProcess::vol(Size index) const {
        if (index >= vols_.size())
            return vols_.back();
        return vols_[index];
    }

    const Real GsrProcess::rev(Size index) const {
        if (index >= reversions_.size())
            return reversions_.back();
        return reversions_[index];
    }

    const bool GsrProcess::revZero(Size index) const {
        if (index >= revZero_.size())
            return revZero_.back();
        return revZero_[index];
    }

}
