/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/processes/gsrprocesscore.hpp>
#include <cmath>

using std::exp;
using std::pow;

namespace QuantLib::detail {

GsrProcessCore::GsrProcessCore(const Array &times, const Array &vols,
                               const Array &reversions, const Real T)
    : times_(times), vols_(vols), reversions_(reversions),
      T_(T), revZero_(reversions.size(), false) {

    QL_REQUIRE(times.size() == vols.size() - 1,
               "number of volatilities ("
                   << vols.size() << ") compared to number of times ("
                   << times_.size() << " must be bigger by one");
    QL_REQUIRE(times.size() == reversions.size() - 1 || reversions.size() == 1,
               "number of reversions ("
                   << vols.size() << ") compared to number of times ("
                   << times_.size() << " must be bigger by one, or exactly "
                                       "1 reversion must be given");
    for (int i = 0; i < ((int)times.size()) - 1; i++)
        QL_REQUIRE(times[i] < times[i + 1], "times must be increasing ("
                                                << times[i] << "@" << i << " , "
                                                << times[i + 1] << "@" << i + 1
                                                << ")");
    flushCache();
}

void GsrProcessCore::flushCache() const {
    for (int i = 0; i < (int)reversions_.size(); i++)
        // small reversions cause numerical problems, so we keep them
        // away from zero
        if (std::fabs(reversions_[i]) < 1E-4)
            revZero_[i] = true;
        else
            revZero_[i] = false;
    cache1_.clear();
    cache2a_.clear();
    cache2b_.clear();
    cache3_.clear();
    cache4_.clear();
    cache5_.clear();
}

Real GsrProcessCore::expectation_x0dep_part(const Time w, const Real xw,
                                            const Time dt) const {
    Real t = w + dt;
    std::pair<Real, Real> key;
    key = std::make_pair(w, t);
    auto k = cache1_.find(key);
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

Real GsrProcessCore::expectation_rn_part(const Time w,
                                         const Time dt) const {

    Real t = w + dt;

    std::pair<Real, Real> key;
    key = std::make_pair(w, t);
    auto k =
        cache2a_.find(key);
    if (k != cache2a_.end())
        return k->second;

    Real res = 0.0;

    // \int A(s,t)y(s)
    for (int k = lowerIndex(w); k <= upperIndex(t) - 1; k++) {
        // l<k
        for (int l = 0; l <= k - 1; l++) {
            Real res2 = 1.0;
            // alpha_l
            res2 *= revZero(l) ? Real(vol(l) * vol(l) * (time2(l + 1) - time2(l)))
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
                    ? Real(2.0 * time2(k) - flooredTime(k, w) -
                          cappedTime(k + 1, t) -
                          2.0 * (time2(k) - cappedTime(k + 1, t)))
                    : Real((exp(rev(k) * (2.0 * time2(k) - flooredTime(k, w) -
                                     cappedTime(k + 1, t))) -
                       exp(2.0 * rev(k) * (time2(k) - cappedTime(k + 1, t)))) /
                          rev(k));
            // add to sum
            res += res2;
        }
        // l=k
        Real res2 = 1.0;
        // alpha_k zeta_k
        res2 *=
            revZero(k)
                ? Real(vol(k) * vol(k) / 4.0 *
                      (4.0 * pow(cappedTime(k + 1, t) - time2(k), 2.0) -
                       (pow(flooredTime(k, w) - 2.0 * time2(k) +
                                cappedTime(k + 1, t),
                            2.0) +
                        pow(cappedTime(k + 1, t) - flooredTime(k, w), 2.0))))
                : Real(vol(k) * vol(k) / (2.0 * rev(k) * rev(k)) *
                      (exp(-2.0 * rev(k) * (cappedTime(k + 1, t) - time2(k))) +
                       1.0 -
                       (exp(-rev(k) * (flooredTime(k, w) - 2.0 * time2(k) +
                                       cappedTime(k + 1, t))) +
                        exp(-rev(k) *
                            (cappedTime(k + 1, t) - flooredTime(k, w))))));
        // zeta_i (i>k)
        for (int i = k + 1; i <= upperIndex(t) - 1; i++)
            res2 *= exp(-rev(i) * (cappedTime(i + 1, t) - time2(i)));
        // no beta_j in this case ...
        res += res2;
    }

    cache2a_.insert(std::make_pair(key, res));

    return res;
} // expectation_rn_part

Real GsrProcessCore::expectation_tf_part(const Time w,
                                         const Time dt) const {

    Real t = w + dt;

    std::pair<Real, Real> key;
    key = std::make_pair(w, t);
    auto k =
        cache2b_.find(key);
    if (k != cache2b_.end())
        return k->second;

    Real res = 0.0;
    // int -A(s,t) \sigma^2 G(s,T)
    for (int k = lowerIndex(w); k <= upperIndex(t) - 1; k++) {
        Real res2 = 0.0;
        // l>k
        for (int l = k + 1; l <= upperIndex(T_) - 1; l++) {
            Real res3 = 1.0;
            // eta_l
            res3 *= revZero(l)
                        ? Real(cappedTime(l + 1, T_) - time2(l))
                        : (1.0 -
                           exp(-rev(l) * (cappedTime(l + 1, T_) - time2(l)))) /
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
                    ? Real((cappedTime(k + 1, t) - time2(k + 1) -
                       (2.0 * flooredTime(k, w) - cappedTime(k + 1, t) -
                        time2(k + 1))) /
                          2.0)
                    : Real((exp(rev(k) * (cappedTime(k + 1, t) - time2(k + 1))) -
                       exp(rev(k) * (2.0 * flooredTime(k, w) -
                                     cappedTime(k + 1, t) - time2(k + 1)))) /
                          (2.0 * rev(k)));
            // add to sum
            res2 += res3;
        }
        // l=k
        Real res3 = 1.0;
        // eta_k zeta_k
        res3 *=
            revZero(k)
                ? Real((-pow(cappedTime(k + 1, t) - cappedTime(k + 1, T_), 2.0) -
                   2.0 * pow(cappedTime(k + 1, t) - flooredTime(k, w), 2.0) +
                   pow(2.0 * flooredTime(k, w) - cappedTime(k + 1, T_) -
                           cappedTime(k + 1, t),
                       2.0)) /
                      4.0)
                : Real((2.0 - exp(rev(k) *
                             (cappedTime(k + 1, t) - cappedTime(k + 1, T_))) -
                   (2.0 * exp(-rev(k) *
                              (cappedTime(k + 1, t) - flooredTime(k, w))) -
                    exp(rev(k) *
                        (2.0 * flooredTime(k, w) - cappedTime(k + 1, T_) -
                         cappedTime(k + 1, t))))) /
                      (2.0 * rev(k) * rev(k)));
        // zeta_i (i>k)
        for (int i = k + 1; i <= upperIndex(t) - 1; i++)
            res3 *= exp(-rev(i) * (cappedTime(i + 1, t) - time2(i)));
        // no gamma_j in this case ...
        res2 += res3;
        // add to main accumulator
        res += -vol(k) * vol(k) * res2;
    }

    cache2b_.insert(std::make_pair(key, res));

    return res;
} // expectation_tf_part

Real GsrProcessCore::variance(const Time w, const Time dt) const {

    Real t = w + dt;

    std::pair<Real, Real> key;
    key = std::make_pair(w, t);
    auto k = cache3_.find(key);
    if (k != cache3_.end())
        return k->second;

    Real res = 0.0;
    for (int k = lowerIndex(w); k <= upperIndex(t) - 1; k++) {
        Real res2 = vol(k) * vol(k);
        // zeta_k^2
        res2 *= revZero(k)
                    ? Real(-(flooredTime(k, w) - cappedTime(k + 1, t)))
                    : (1.0 - exp(2.0 * rev(k) *
                                 (flooredTime(k, w) - cappedTime(k + 1, t)))) /
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

Real GsrProcessCore::y(const Time t) const {
    Real key;
    key = t;
    auto k = cache4_.find(key);
    if (k != cache4_.end())
        return k->second;

    Real res = 0.0;
    for (int i = 0; i <= upperIndex(t) - 1; i++) {
        Real res2 = 1.0;
        for (int j = i + 1; j <= upperIndex(t) - 1; j++) {
            res2 *= exp(-2.0 * rev(j) * (cappedTime(j + 1, t) - time2(j)));
        }
        res2 *= revZero(i) ? Real(vol(i) * vol(i) * (cappedTime(i + 1, t) - time2(i)))
                           : (vol(i) * vol(i) / (2.0 * rev(i)) *
                              (1.0 - exp(-2.0 * rev(i) *
                                         (cappedTime(i + 1, t) - time2(i)))));
        res += res2;
    }

    cache4_.insert(std::make_pair(key, res));
    return res;
}

Real GsrProcessCore::G(const Time t, const Time w) const {
    std::pair<Real, Real> key;
    key = std::make_pair(w, t);
    auto k = cache5_.find(key);
    if (k != cache5_.end())
        return k->second;

    Real res = 0.0;
    for (int i = lowerIndex(t); i <= upperIndex(w) - 1; i++) {
        Real res2 = 1.0;
        for (int j = lowerIndex(t); j <= i - 1; j++) {
            res2 *= exp(-rev(j) * (time2(j + 1) - flooredTime(j, t)));
        }
        res2 *= revZero(i) ? Real(cappedTime(i + 1, w) - flooredTime(i, t))
                           : (1.0 - exp(-rev(i) * (cappedTime(i + 1, w) -
                                                   flooredTime(i, t)))) /
                                 rev(i);
        res += res2;
    }

    cache5_.insert(std::make_pair(key, res));
    return res;
}

int GsrProcessCore::lowerIndex(const Time t) const {
    return static_cast<int>(std::upper_bound(times_.begin(), times_.end(), t) -
                            times_.begin());
}

int GsrProcessCore::upperIndex(const Time t) const {
    if (t < QL_MIN_POSITIVE_REAL)
        return 0;
    return static_cast<int>(
               std::upper_bound(times_.begin(), times_.end(), t - QL_EPSILON) -
               times_.begin()) +
           1;
}

Real GsrProcessCore::cappedTime(const Size index, const Real cap) const {
    return cap != Null<Real>() ? std::min(cap, time2(index)) : time2(index);
}

Real GsrProcessCore::flooredTime(const Size index,
                                 const Real floor) const {
    return floor != Null<Real>() ? std::max(floor, time2(index)) : time2(index);
}

Real GsrProcessCore::time2(const Size index) const {
    if (index == 0)
        return 0.0;
    if (index > times_.size())
        return T_; // FIXME how to ensure that forward
                   // measure time is geq all times
                   // given
    return times_[index - 1];
}

Real GsrProcessCore::vol(const Size index) const {
    if (index >= vols_.size())
        return vols_.back();
    return vols_[index];
}

Real GsrProcessCore::rev(const Size index) const {
    if (index >= reversions_.size())
        return reversions_.back();
    return reversions_[index];
}

bool GsrProcessCore::revZero(const Size index) const {
    if (index >= revZero_.size())
        return revZero_.back();
    return revZero_[index];
}

} // namesapce QuantLib
