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

/*! \file lgmparametrization.hpp
    \brief basis class for zeta and H functions
*/

#ifndef quantlib_lgm_parametrization_hpp
#define quantlib_lgm_parametrization_hpp

#include <ql/patterns/curiouslyrecurring.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/math/array.hpp>

namespace QuantLib {

namespace detail {

template <class Impl>
class LgmParametrization : public CuriouslyRecurringTemplate<Impl> {
  public:
    void update() const;
    //! inspectors
    const Real zeta(const Time t) const;
    const Real alpha(const Time t) const;
    const Real H(const Time t) const;
    const Real Hprime(const Time t) const;
    const Real Hprime2(const Time t) const;
    const Array &times() const;

    //! corresponding Hull White parameters
    const Real hullWhiteSigma(Time t) const;
    const Real hullWhiteKappa(Time t) const;

    //! constructor with step for numerical differentiation
    LgmParametrization() : h_(1E-6) {}

    //! interface
    const void updateImpl() const {}          // optional to implement (.)
    const Real zetaImpl(const Time) const;    // must be implemented   (*)
    const Real alphaImpl(const Time) const;   // (.)
    const Real HImpl(const Time) const;       // (*)
    const Real HprimeImpl(const Time) const;  // (.)
    const Real Hprime2Impl(const Time) const; // (.)
    const Array &timesImpl() const;           // (.)

  private:
    const Real h_;
};

// inline

template <class Impl> inline void LgmParametrization<Impl>::update() const {
    return this->impl().updateImpl();
}

template <class Impl>
inline const Real LgmParametrization<Impl>::zeta(const Time t) const {
    return this->impl().zetaImpl(t);
}

template <class Impl>
inline const Real LgmParametrization<Impl>::alpha(const Time t) const {
    return this->impl().alphaImpl(t);
}

template <class Impl>
inline const Real LgmParametrization<Impl>::H(const Time t) const {
    return this->impl().HImpl(t);
}

template <class Impl>
inline const Real LgmParametrization<Impl>::Hprime(const Time t) const {
    return this->impl().HprimeImpl(t);
}

template <class Impl>
inline const Real LgmParametrization<Impl>::Hprime2(const Time t) const {
    return this->impl().Hprime2Impl(t);
}

template <class Impl>
inline const Array &LgmParametrization<Impl>::times() const {
    return this->impl().timesImpl();
}

template <class Impl>
const Real LgmParametrization<Impl>::hullWhiteSigma(Time t) const {
    return Hprime(t) * alpha(t);
}

template <class Impl>
const Real LgmParametrization<Impl>::hullWhiteKappa(Time t) const {
    return -Hprime2(t) / Hprime(t);
}

// default implementations

template <class Impl>
inline const Real LgmParametrization<Impl>::zetaImpl(const Time t) const {
    QL_FAIL("zeta implemnentation not provided");
}

template <class Impl>
inline const Real LgmParametrization<Impl>::alphaImpl(const Time t) const {
    return std::sqrt((zeta(t + 0.5 * h_) - zeta(t - 0.5 * h_)) / h_);
}

template <class Impl>
inline const Real LgmParametrization<Impl>::HImpl(const Time t) const {
    QL_FAIL("H implementation not provided");
}

template <class Impl>
inline const Real LgmParametrization<Impl>::HprimeImpl(const Time t) const {
    return (H(t + 0.5 * h_) - H(t - 0.5 * h_)) / h_;
}

template <class Impl>
inline const Real LgmParametrization<Impl>::Hprime2Impl(const Time t) const {
    return (H(t + 0.5 * h_) - 2.0 * H(t) + H(t - 0.5 * h_)) / (h_ * h_);
}

template <class Impl>
inline const Array &LgmParametrization<Impl>::timesImpl() const {
    QL_FAIL("times not provided");
}

} // namespace detail

} // namespace QuantLib

#endif
