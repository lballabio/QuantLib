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

/*! \file inversecumulativersg_multithreaded.hpp
    \brief Inverse cumulative random sequence generator (multithreaded)
*/

#ifndef quantlib_inversecumulative_rsg_multithreaded_hpp
#define quantlib_inversecumulative_rsg_multithreaded_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <vector>

namespace QuantLib {

//! Inverse cumulative random sequence generator (multithreaded)
/*! It uses a sequence of uniform deviate in (0, 1) as the
    source of cumulative distribution values.
    Then an inverse cumulative distribution is used to calculate
    the distribution deviate.

    The uniform deviate sequence is supplied by USG_MT.

    Class USG_MT must implement the following interface:
    \code
        USG_MT::sample_type USG::nextSequence(int threadId) const;
        Size USG_MT::dimension() const;
    \endcode

    The inverse cumulative distribution is supplied by IC.

    Class IC must implement the following interface:
    \code
        IC::IC();
        Real IC::operator() const;
    \endcode
*/

template <class USG_MT, class IC> class InverseCumulativeRsgMultiThreaded {
  public:
    typedef Sample<std::vector<Real> > sample_type;
    static const Size maxNumberOfThreads = USG_MT::maxNumberOfThreads;
    explicit InverseCumulativeRsgMultiThreaded(
        const USG_MT &uniformSequenceGeneratorMultiThreaded);
    InverseCumulativeRsgMultiThreaded(
        const USG_MT &uniformSequenceGeneratorMultiThreaded,
        const IC &inverseCumulative);
    //! returns next sample from the inverse cumulative distribution
    const sample_type &nextSequence(unsigned int threadId) const;
    const sample_type &lastSequence(unsigned int threadId) const {
        QL_REQUIRE(threadId < USG_MT::maxNumberOfThreads,
                   "thread id (" << threadId << ") out of bounds [0..."
                                 << USG_MT::maxNumberOfThreads - 1 << "]");
        return x_[threadId];
    }
    Size dimension() const { return dimension_; }

  private:
    USG_MT uniformSequenceGeneratorMultiThreaded_;
    Size dimension_;
    mutable std::vector<sample_type> x_;
    IC ICD_;
};

template <class USG_MT, class IC>
InverseCumulativeRsgMultiThreaded<
    USG_MT, IC>::InverseCumulativeRsgMultiThreaded(const USG_MT &usg_mt)
    : uniformSequenceGeneratorMultiThreaded_(usg_mt),
      dimension_(uniformSequenceGeneratorMultiThreaded_.dimension()),
      x_(std::vector<sample_type>(
          USG_MT::maxNumberOfThreads,
          sample_type(std::vector<Real>(dimension_), 1.0))) {}

template <class USG_MT, class IC>
InverseCumulativeRsgMultiThreaded<
    USG_MT, IC>::InverseCumulativeRsgMultiThreaded(const USG_MT &usg_mt,
                                                   const IC &inverseCum)
    : uniformSequenceGeneratorMultiThreaded_(usg_mt),
      dimension_(uniformSequenceGeneratorMultiThreaded_.dimension()),
      x_(std::vector<sample_type>(
          USG_MT::maxNumberOfThreads,
          sample_type(std::vector<Real>(dimension_), 1.0))),
      ICD_(inverseCum) {}

template <class USG_MT, class IC>
inline const typename InverseCumulativeRsgMultiThreaded<USG_MT,
                                                        IC>::sample_type &
InverseCumulativeRsgMultiThreaded<USG_MT, IC>::nextSequence(
    unsigned int threadId) const {

    QL_REQUIRE(threadId < USG_MT::maxNumberOfThreads,
               "thread id (" << threadId << ") out of bounds [0..."
               << USG_MT::maxNumberOfThreads - 1 << "]");
    typename USG_MT::sample_type sample =
        uniformSequenceGeneratorMultiThreaded_.nextSequence(threadId);
    x_[threadId].weight = sample.weight;
    for (Size i = 0; i < dimension_; i++) {
        x_[threadId].value[i] = ICD_(sample.value[i]);
    }
    return x_[threadId];
}

} // namespace QuantLib

#endif
