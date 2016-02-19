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

/*! \file lgmfxparametrization.hpp
    \brief fx process parametrization in the context of
           the cross currency lgm
*/

#ifndef quantlib_lgm_fx_parametrization_hpp
#define quantlib_lgm_fx_parametrization_hpp

#include <ql/patterns/curiouslyrecurring.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/math/array.hpp>

namespace QuantLib {

namespace detail {

template <class Impl>
class LgmFxParametrization : public CuriouslyRecurringTemplate<Impl> {
  public:
    void update() const;
    //! inspectors
    const Real sigma(const Time t) const;
    const Real variance(const Time t) const;
    const Real stdDeviation(const Time t) const;
    const Array &times() const;

    //! constructor with step size for numerical differentiation
    LgmFxParametrization() : h_(1E-6) {}

    //! interface
    const void updateImpl() {}                   // optional to implement (.)
    const Real varianceImpl(const Time t) const; // must be implemented (*)
    const Real sigmaImpl(const Time t) const;    // (.)
    const Real stdDeviationImpl(const Time t) const; // (.)
    const Array &timesImpl() const;                  // (.)

  private:
    const Real h_;
};

// inline

template <class Impl> inline void LgmFxParametrization<Impl>::update() const {
    return this->impl().updateImpl();
}

template <class Impl>
inline const Real LgmFxParametrization<Impl>::sigma(const Time t) const {
    return this->impl().sigmaImpl(t);
}

template <class Impl>
inline const Real LgmFxParametrization<Impl>::variance(const Time t) const {
    return this->impl().varianceImpl(t);
}

template <class Impl>
inline const Real LgmFxParametrization<Impl>::stdDeviation(const Time t) const {
    return this->impl().stdDeviationImpl(t);
}

template <class Impl>
inline const Array &LgmFxParametrization<Impl>::times() const {
    return this->impl().timesImpl();
}

// default implementations

template <class Impl>
inline const Real LgmFxParametrization<Impl>::sigmaImpl(const Time t) const {
    return std::sqrt((variance(t + 0.5 * h_) - variance(t - 0.5 * h_)) / h_);
}

template <class Impl>
inline const Real LgmFxParametrization<Impl>::varianceImpl(const Time t) const {
    QL_FAIL("variance implementation not provided");
}

template <class Impl>
inline const Real
LgmFxParametrization<Impl>::stdDeviationImpl(const Time t) const {
    return std::sqrt(variance(t));
}

template <class Impl>
inline const Array &LgmFxParametrization<Impl>::timesImpl() const {
    QL_FAIL("times not provided");
}

} // namespace detail

} // namespace QuantLib

#endif
