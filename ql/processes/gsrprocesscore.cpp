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

namespace QuantLib::detail {

namespace {

    // exprel(x) = (exp(x) - 1) / x
    Real exprel(const Real x) {
        return x == 0.0 ? Real(1.0) : Real(std::expm1(x) / x);
    }

}

GsrProcessCore::GsrProcessCore(Array times, Array vols,
                               Array reversions, const Real T)
    : times_(std::move(times)), vols_(std::move(vols)), reversions_(std::move(reversions)),
      T_(T) {
    flushCache();
    checkTimesVolsReversions();
}

void GsrProcessCore::setTimes(Array times) {
    times_ = std::move(times);
    checkTimesVolsReversions();
}

void GsrProcessCore::setVols(Array vols) {
    vols_ = std::move(vols);
    checkTimesVolsReversions();
}

void GsrProcessCore::setReversions(Array reversions) {
    reversions_ = std::move(reversions);
    checkTimesVolsReversions();
}

void GsrProcessCore::checkTimesVolsReversions() const {
    QL_REQUIRE(times_.size() == vols_.size() - 1,
               "number of volatilities ("
                   << vols_.size() << ") compared to number of times ("
                   << times_.size() << " must be bigger by one");
    QL_REQUIRE(times_.size() == reversions_.size() - 1 || reversions_.size() == 1,
               "number of reversions ("
                   << vols_.size() << ") compared to number of times ("
                   << times_.size() << " must be bigger by one, or exactly "
                                       "1 reversion must be given");
    for (int i = 0; i < ((int)times_.size()) - 1; i++)
        QL_REQUIRE(times_[i] < times_[i + 1], "times must be increasing ("
                                                << times_[i] << "@" << i << " , "
                                                << times_[i + 1] << "@" << i + 1
                                                << ")");
}

void GsrProcessCore::flushCache() const {
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
            const Real dl = time2(l + 1) - time2(l);
            res2 *= vol(l) * vol(l) * dl * (revZero(l) ? Real(1.0) : exprel(-2.0 * rev(l) * dl));
            // zeta_i (i>k)
            for (int i = k + 1; i <= upperIndex(t) - 1; i++)
                res2 *= exp(-rev(i) * (cappedTime(i + 1, t) - time2(i)));
            // beta_j (j<k)
            for (int j = l + 1; j <= k - 1; j++)
                res2 *= exp(-2.0 * rev(j) * (time2(j + 1) - time2(j)));
            // zeta_k beta_k
            const Real b = cappedTime(k + 1, t) - flooredTime(k, w);
            res2 *= b * (revZero(k) ? Real(1.0) : Real(exp(2.0 * rev(k) * (time2(k) - cappedTime(k + 1, t))) * exprel(rev(k) * b)));
            // add to sum
            res += res2;
        }
        // l=k
        Real res2 = 1.0;
        // alpha_k zeta_k = vol^2 / (2 r^2) (1 - exp(-r a)) (1 - exp(-r b)),
        // a = f - 2 t_k + c, b = c - f, f = flooredTime(k, w), c = cappedTime(k + 1, t)
        const Real a = flooredTime(k, w) - 2.0 * time2(k) + cappedTime(k + 1, t);
        const Real b = cappedTime(k + 1, t) - flooredTime(k, w);
        res2 *= vol(k) * vol(k) / 2.0 * a * b * (revZero(k) ? Real(1.0) : Real(exprel(-rev(k) * a) * exprel(-rev(k) * b)));
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
            const Real el = cappedTime(l + 1, T_) - time2(l);
            res3 *= el * (revZero(l) ? Real(1.0) : exprel(-rev(l) * el));
            // zeta_i (i>k)
            for (int i = k + 1; i <= upperIndex(t) - 1; i++)
                res3 *= exp(-rev(i) * (cappedTime(i + 1, t) - time2(i)));
            // gamma_j (j>k)
            for (int j = k + 1; j <= l - 1; j++)
                res3 *= exp(-rev(j) * (time2(j + 1) - time2(j)));
            // zeta_k gamma_k
            const Real b = cappedTime(k + 1, t) - flooredTime(k, w);
            res3 *= b * (revZero(k) ? Real(1.0) : Real(exp(rev(k) * (2.0 * flooredTime(k, w) - cappedTime(k + 1, t) - time2(k + 1))) * exprel(2.0 * rev(k) * b)));
            // add to sum
            res2 += res3;
        }
        // l=k
        Real res3 = 1.0;
        // eta_k zeta_k = ((1 - exp(-r a)) (1 - exp(-2 r b)) + (1 - exp(-r b))^2) / (2 r^2),
        // a = C - c, b = c - f, C = cappedTime(k + 1, T), c = cappedTime(k + 1, t),
        // f = flooredTime(k, w)
        const Real a = cappedTime(k + 1, T_) - cappedTime(k + 1, t);
        const Real b = cappedTime(k + 1, t) - flooredTime(k, w);
        if (revZero(k)) {
            res3 *= a * b + b * b / 2.0;
        } else {
            const Real eb = exprel(-rev(k) * b);
            res3 *= a * b * exprel(-rev(k) * a) * exprel(-2.0 * rev(k) * b) + b * b / 2.0 * eb * eb;
        }
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
        const Real b = cappedTime(k + 1, t) - flooredTime(k, w);
        res2 *= b * (revZero(k) ? Real(1.0) : exprel(-2.0 * rev(k) * b));
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
        const Real d = cappedTime(i + 1, t) - time2(i);
        res2 *= vol(i) * vol(i) * d * (revZero(i) ? Real(1.0) : exprel(-2.0 * rev(i) * d));
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
        const Real d = cappedTime(i + 1, w) - flooredTime(i, t);
        res2 *= d * (revZero(i) ? Real(1.0) : exprel(-rev(i) * d));
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
    return std::fabs(rev(index)) < 1E-12;
}

} // namesapce QuantLib
