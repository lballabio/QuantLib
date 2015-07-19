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

/*! \file inversecumulativerng_multithreaded.hpp
    \brief Inverse Cumulative Gaussian random-number generator
*/

#ifndef quantlib_inversecumulative_rng_multithreaded_hpp
#define quantlib_inversecumulative_rng_multithreaded_hpp

#include <ql/methods/montecarlo/sample.hpp>

namespace QuantLib {

//! Inverse cumulative random number generator (multithreaded)
/*! It uses a uniform deviate in (0, 1) as the source of cumulative
    distribution values.
    Then an inverse cumulative distribution is used to calculate
    the distribution deviate.

    The uniform deviate is supplied by RNG_MT.

    Class RNG_MT must implement the following interface:
    \code
        RNG_MT::sample_type RNG::next(int threadId) const;
    \endcode

    The inverse cumulative distribution is supplied by IC.

    Class IC must implement the following interface:
    \code
        IC::IC();
        Real IC::operator() const;
    \endcode
*/

template <class RNG_MT, class IC> class InverseCumulativeRngMultiThreaded {
  public:
    typedef Sample<Real> sample_type;
    typedef RNG_MT urng_mt_type;
    explicit InverseCumulativeRngMultiThreaded(
        const RNG_MT &uniformGeneratorMultiThreaded);
    //! returns a sample from a Gaussian distribution
    sample_type next(int threadId = 0) const;

  private:
    RNG_MT uniformGeneratorMultiThreaded_;
    IC ICND_;
};

template <class RNG_MT, class IC>
InverseCumulativeRng<RNG, IC>::InverseCumulativeRng(const RNG_MT &ug_mt)
    : uniformGeneratorMultiThreaded_(ug_mt) {}

template <class RNG_MT, class IC>
inline typename InverseCumulativeRng<RNG_MT, IC>::sample_type
InverseCumulativeRng<RNG, IC>::next(int threadId) const {
    typename RNG_MT::sample_type sample =
        uniformGeneratorMultiThreaded_.next(threadId);
    return sample_type(ICND_(sample.value), sample.weight);
}
} // namespace QuantLib

#endif
