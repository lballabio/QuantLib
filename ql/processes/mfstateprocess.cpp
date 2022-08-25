/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include <ql/processes/mfstateprocess.hpp>

namespace QuantLib {

    MfStateProcess::MfStateProcess(Real reversion, const Array& times, const Array& vols)
    : reversion_(reversion), times_(times), vols_(vols) {
        if (reversion_ < QL_EPSILON && -reversion_ < QL_EPSILON)
            reversionZero_ = true;
        QL_REQUIRE(times.size() == vols.size() - 1,
                   "number of volatilities ("
                       << vols.size() << ") compared to number of times ("
                       << times_.size() << " must be bigger by one");
        for (int i = 0; i < ((int)times.size()) - 1; i++)
            QL_REQUIRE(times[i] < times[i + 1], "times must be increasing ("
                                                    << times[i] << "@" << i
                                                    << " , " << times[i + 1]
                                                    << "@" << i + 1 << ")");
        for (Size i = 0; i < vols.size(); i++)
            QL_REQUIRE(vols[i] >= 0.0, "volatilities must be non negative ("
                                           << vols[i] << "@" << i << ")");
    }

    Real MfStateProcess::x0() const { return 0.0; }

    Real MfStateProcess::drift(Time, Real) const { return 0.0; }

    Real MfStateProcess::diffusion(Time t, Real) const {
        Size i =
            std::upper_bound(times_.begin(), times_.end(), t) - times_.begin();
        return vols_[i];
    }

    Real MfStateProcess::expectation(Time, Real x0, Time dt) const {
        return x0;
    }

    Real MfStateProcess::stdDeviation(Time t, Real x0, Time dt) const {
        return std::sqrt(variance(t, x0, dt));
    }

    Real MfStateProcess::variance(Time t, Real, Time dt) const {

        if (dt < QL_EPSILON)
            return 0.0;
        if (times_.empty())
            return reversionZero_ ? dt
                                  : 1.0 / (2.0 * reversion_) *
                                        (std::exp(2.0 * reversion_ * (t + dt)) -
                                         std::exp(2.0 * reversion_ * t));

        Size i =
            std::upper_bound(times_.begin(), times_.end(), t) - times_.begin();
        Size j = std::upper_bound(times_.begin(), times_.end(), t + dt) -
                 times_.begin();

        Real v = 0.0;

        for (Size k = i; k < j; k++) {
            if (reversionZero_)
                v += vols_[k] * vols_[k] *
                     (times_[k] - std::max(k > 0 ? times_[k - 1] : 0.0, t));
            else
                v += 1.0 / (2.0 * reversion_) * vols_[k] * vols_[k] *
                     (std::exp(2.0 * reversion_ * times_[k]) -
                      std::exp(2.0 * reversion_ *
                               std::max(k > 0 ? times_[k - 1] : 0.0, t)));
        }

        if (reversionZero_)
            v += vols_[j] * vols_[j] *
                 (t + dt - std::max(j > 0 ? times_[j - 1] : 0.0, t));
        else
            v += 1.0 / (2.0 * reversion_) * vols_[j] * vols_[j] *
                 (std::exp(2.0 * reversion_ * (t + dt)) -
                  std::exp(2.0 * reversion_ *
                           (std::max(j > 0 ? times_[j - 1] : 0.0, t))));

        return v;
    }
}
