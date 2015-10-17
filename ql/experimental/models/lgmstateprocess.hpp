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

/*! \file lgmstateprocess.hpp
    \brief LGM state process
*/

#ifndef quantlib_lgm_stateprocess_hpp
#define quantlib_lgm_stateprocess_hpp

namespace QuantLib {

template <class Impl> class LgmStateProcess : public StochasticProcess1D {
  public:
    LgmStateProcess(const boost::shared_ptr<detail::LgmParametrization<Impl> >
                        &parametrization);
    //! \name StochasticProcess interface
    //@{
    Real x0() const;
    Real drift(Time t, Real x) const;
    Real diffusion(Time t, Real x) const;
    Real expectation(Time t0, Real x0, Time dt) const;
    Real stdDeviation(Time t0, Real x0, Time dt) const;
    Real variance(Time t0, Real x0, Time dt) const;
    //@}
  private:
    const boost::shared_ptr<detail::LgmParametrization<Impl> > parametrization_;
};

// inline

template <class Impl> Real LgmStateProcess<Impl>::x0() const { return 0.0; }

template <class Impl> Real LgmStateProcess<Impl>::drift(Time, Real) const {
    return 0.0;
}

template <class Impl>
Real LgmStateProcess<Impl>::diffusion(Time t, Real) const {
    return parametrization_->alpha(t);
}

template <class Impl>
Real LgmStateProcess<Impl>::expectation(Time, Real x0, Time) const {
    return x0;
}

template <class Impl>
Real LgmStateProcess<Impl>::variance(Time t0, Real, Time dt) const {
    return parametrization_->zeta(t0 + dt) - parametrization_->zeta(t0);
}

template <class Impl>
Real LgmStateProcess<Impl>::stdDeviation(Time t0, Real x0, Time dt) const {
    return std::sqrt(variance(t0, x0, dt));
}

// implementation

template <class Impl>
LgmStateProcess<Impl>::LgmStateProcess(
    const boost::shared_ptr<detail::LgmParametrization<Impl> > &parametrization)
    : parametrization_(parametrization) {}

} // namespace QuantLib

#endif
