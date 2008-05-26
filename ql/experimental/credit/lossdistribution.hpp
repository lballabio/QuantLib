/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file lossdistribution.hpp
    \brief Loss distributions and probability of n defaults
*/

#ifndef quantlib_loss_distribution_hpp
#define quantlib_loss_distribution_hpp

#include <ql/experimental/credit/distribution.hpp>
#include <ql/experimental/credit/onefactorcopula.hpp>

namespace QuantLib {

    namespace detail {

        /* Probability of exactly n defaults (Hull-White)

          \todo Stabilize the algorithm:
          The recursive implementation is capable of
          generating probabilities < 0 and > 1 when the number of events is
          large (> ~30) and close to the number of underlyings in the basket,
          i.e. overall probability for N events becomes (should become) minimal.
          This numerical effect is currently cut off by setting the probability
          of N events to zero when either the calculated probability
          becomes invalid (outside [0;1]) or the cumulative probability of up
          to N events exceeds 1.
        */
        Real probabilityOfNEvents(Size n, const std::vector<Real>& prob);

        /* Probability of at least n defaults (Hull-White) */
        Real probabilityOfAtLeastNEvents(Size n, const std::vector<Real>& prob);

    }

    //! Loss distribution with Hull-White bucketing
    /*! See John Hull and Alan White, "Valuation of a CDO and nth to
        default CDS without Monte Carlo simulation", Journal of
        Derivatives 12, 2, 2004

        Independence of default events is assumed. Correlation is
        built in separately via a copula approach that reuses the
        results here (conditional independence).

        For the case that probabilities vary and volumes are identical
        (or do not matter as in a nth to default basket), loss
        distribution and probability of n defaults can be calculated
        using HullWhite algorithm, see class
        ProbabilityOfAtLeastNEvents.

        If both volumes and probabilities vary, the loss distribution
        is not accessible via the probability of n defaults, but needs
        a more general algorithm (Probability Bucketing, Monte Carlo
        simulation).
    */
    class LossDistBucketing {
      public:
        LossDistBucketing(Size nBuckets, Real maximum)
        : nBuckets_(nBuckets), maximum_(maximum) {}
        Distribution operator()(const std::vector<Real>& volumes,
                                const std::vector<Real>& probabilities) const;
        Size buckets() const { return nBuckets_; }
        Real maximum() const { return maximum_; }
      private:
        Size locateTargetBucket(Real loss) const;

        Size nBuckets_;
        Real maximum_;
    };

    //! Loss distribution with Monte Carlo simulation
    class LossDistMonteCarlo {
      public:
        LossDistMonteCarlo(Size nBuckets, Real maximum, Size simulations)
        : nBuckets_(nBuckets), maximum_(maximum), simulations_(simulations) {}
        Distribution operator()(const std::vector<Real>& volumes,
                                const std::vector<Real>& probabilities) const;
        Size buckets() const { return nBuckets_; }
        Real maximum() const { return maximum_; }
      private:
        Size nBuckets_;
        Real maximum_;
        Size simulations_;
    };

    //! Probability of N events
    /*! Implementation of the probability of N events when individual
        probabilities vary, following:

        John Hull and Alan White, "Valuation of a CDO and nth to
        default CDS without Monte Carlo simulation", Journal of
        Derivatives 12, 2, 2004
    */
    class ProbabilityOfNEvents {
      public:
        ProbabilityOfNEvents(Size n) : n_(n) {}
        Real operator()(const std::vector<Real>& p) const;
      private:
        Size n_;
    };

    //! Probability of at least N events
    /*! Implementation of the probability of at least N events when
        individual probabilities vary, following:

        John Hull and Alan White, "Valuation of a CDO and nth to
        default CDS without Monte Carlo simulation", Journal of
        Derivatives 12, 2, 2004
    */
    class ProbabilityOfAtLeastNEvents {
      public:
        ProbabilityOfAtLeastNEvents(Size n) : n_(n) {}
        Real operator()(const std::vector<Real>& p) const;
      private:
        Size n_;
    };

}

#endif
