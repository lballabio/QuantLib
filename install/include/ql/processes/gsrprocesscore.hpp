/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file gsrprocesscore.hpp
    \brief Core computations for the gsr process in risk neutral
           and T-forward measure.
    \warning Results are cached for performance reasons, so if
             parameters change, you need to call flushCache() to
             avoid inconsistent results.
*/

#ifndef quantlib_gsr_process_core_hpp
#define quantlib_gsr_process_core_hpp

#include <ql/math/array.hpp>
#include <ql/math/comparison.hpp>
#include <map>

namespace QuantLib::detail {

class GsrProcessCore {
  public:
    GsrProcessCore(const Array& times, const Array& vols, const Array& reversions, Real T = 60.0);

    // conditional expectation, x0 dependent part
    Real expectation_x0dep_part(Time w, Real xw, Time dt) const;

    // conditional expectation, x0 independent part
    // in the risk neutral measure
    Real expectation_rn_part(Time w, Time dt) const;

    // conditional expectation, drift adjustment for
    // the T-forward measure
    Real expectation_tf_part(Time w, Time dt) const;

    // conditional variance
    Real variance(Time w, Time dt) const;

    // y(t)
    Real y(Time t) const;

    // G(t,w)
    Real G(Time t, Time w) const;

    // sigma
    Real sigma(Time t) const;

    // reversion
    Real reversion(Time t) const;

    // reset cache
    void flushCache() const;

  protected:
    const Array &times_, &vols_, &reversions_;

  private:
    int lowerIndex(Time t) const;
    int upperIndex(Time t) const;
    Real time2(Size index) const;
    Real cappedTime(Size index, Real cap = Null<Real>()) const;
    Real flooredTime(Size index, Real floor = Null<Real>()) const;
    Real vol(Size index) const;
    Real rev(Size index) const;
    bool revZero(Size index) const;

    mutable std::map<std::pair<Real, Real>, Real> cache1_, cache2a_, cache2b_,
        cache3_, cache5_;
    mutable std::map<Real, Real> cache4_;
    Time T_;
    mutable std::vector<bool> revZero_;
}; // GsrProcessCore

// inline definitions

inline Real GsrProcessCore::sigma(const Time t) const {
    return vol(lowerIndex(t));
}

inline Real GsrProcessCore::reversion(const Time t) const {
    return rev(lowerIndex(t));
}

} // namespace QuantLib

#endif
