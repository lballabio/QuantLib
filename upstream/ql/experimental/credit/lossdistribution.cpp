/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#include <ql/experimental/credit/lossdistribution.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

using namespace std;

namespace QuantLib {

    //--------------------------------------------------------------------------
    Real LossDist::binomialProbabilityOfNEvents(int n, vector<Real>& p) {
    //--------------------------------------------------------------------------
        BinomialDistribution binomial (p[0], p.size());
        return binomial(n);
    }

    //--------------------------------------------------------------------------
    Real LossDist::binomialProbabilityOfAtLeastNEvents(int n, vector<Real>& p) {
    //--------------------------------------------------------------------------
        CumulativeBinomialDistribution binomial(p[0], p.size());
        return 1.0 - binomial(n-1);
        /*
        Real defp = 0;
        for (Size i = n; i <= p.size(); i++)
            defp += binomialProbabilityOfNEvents (i, p);

        return defp;
        */
    }

    //--------------------------------------------------------------------------
    vector<Real> LossDist::probabilityOfNEvents(vector<Real>& p) {
    //--------------------------------------------------------------------------
        Size n = p.size();
        vector<Real> probability(n+1, 0.0);
        vector<Real> prev;
        probability[0] = 1.0;
        for (Size j = 0; j < n; j++) {
            prev = probability;
            probability[0] = prev[0] * (1.0 - p[j]);
            for (Size i = 1; i <= j; i++)
                probability[i] = prev[i-1] * p[j] + prev[i] * (1.0 - p[j]);
            probability[j+1] = prev[j] * p[j];
        }

        return probability;
    }

    //--------------------------------------------------------------------------
    Real LossDist::probabilityOfNEvents(int k, vector<Real>& p) {
    //--------------------------------------------------------------------------
        return probabilityOfNEvents(p)[k];

//      vector<Real> w (p.size(), 0);
//      vector<Real> u (k+1, 0);
//      vector<Real> v (k+1, 0);

//      Real pZero = 1.0;
//      for (Size i = 0; i < w.size(); i++) {
//          pZero *= (1.0 - p[i]);
//          w[i] = p[i] / (1.0 - p[i]);
//      }

//      if (k == 0) return pZero;

//      int kk = k;
//      Real prodw = 1.0;

//      Cumulated probability of up to n events:
//      Cut off when the cumulated probability reaches 1,
//      i.e. set all following probabilities of exactly n events to zero.
//      Real sum = 1.0;

//      u[0] = 1.0;
//      for (int i = 1; i <= kk; i++) {
//          v[i] = 0;
//          for (Size j = 0; j < w.size(); j++)
//              v[i] += pow (w[j], i);
//          u[i] = 0;
//          for (int j = 1; j <= i; j++)
//              u[i] +=  pow (-1.0, j+1) * v[j] * u[i-j];
//          u[i] /= i;

//          cut off
//          if (sum * pZero >= 1.0 || u[i] < 0 || u[i] * pZero >= 1.0)
//              u[i] = 0;

//          sum += u[i];
//      }

//      return pZero * prodw * u[kk];
    }

    //--------------------------------------------------------------------------
    Real LossDist::probabilityOfAtLeastNEvents (int k, vector<Real>& p) {
    //--------------------------------------------------------------------------
        vector<Real> probability = probabilityOfNEvents(p);
        Real sum = 1.0;
        for (int j = 0; j < k; j++)
            sum -= probability[j];
        return sum;
        /*
        Real sum = 0;
        for (Size i = k; i <= p.size(); i++)
            sum += probabilityOfNEvents (i, p);
        return sum;
        */
    }

    //--------------------------------------------------------------------------
    Real ProbabilityOfNEvents::operator()(vector<Real> p) const {
    //--------------------------------------------------------------------------
        return LossDist::probabilityOfNEvents (n_, p);
    }

    //--------------------------------------------------------------------------
    Real ProbabilityOfAtLeastNEvents::operator()(vector<Real> p) const {
    //--------------------------------------------------------------------------
        return LossDist::probabilityOfAtLeastNEvents (n_, p);
    }

    //--------------------------------------------------------------------------
    Real BinomialProbabilityOfAtLeastNEvents::operator()(vector<Real> p) const {
        //--------------------------------------------------------------------------
        return LossDist::binomialProbabilityOfAtLeastNEvents(n_, p);
    }

    //--------------------------------------------------------------------------
    Distribution LossDistBinomial::operator()(Size n, Real volume,
                                              Real probability) const {
    //--------------------------------------------------------------------------
        n_ = n;
        probability_.clear();
        probability_.resize(n_+1, 0.0);
        Distribution dist (nBuckets_, 0.0, maximum_);
        BinomialDistribution binomial (probability, n);
        for (Size i = 0; i <= n; i++) {
            if (volume_ * i <= maximum_) {
                probability_[i] = binomial(i);
                Size bucket = dist.locate(volume * i);
                dist.addDensity (bucket, probability_[i] / dist.dx(bucket));
                dist.addAverage (bucket, volume * i);
            }
        }

        excessProbability_.clear();
        excessProbability_.resize(n_+1, 0.0);
        excessProbability_[n_] = probability_[n_];
        for (int k = n_-1; k >= 0; k--)
            excessProbability_[k] = excessProbability_[k+1] + probability_[k];

        dist.normalize();

        return dist;
    }

    //--------------------------------------------------------------------------
    Distribution LossDistBinomial::operator()(const vector<Real>& nominals,
                                    const vector<Real>& probabilities) const {
    //--------------------------------------------------------------------------
        return operator()(nominals.size(), nominals[0], probabilities[0]);
    }

    //--------------------------------------------------------------------------
    Distribution LossDistHomogeneous::operator()(Real volume,
                                                 const vector<Real>& p) const {
    //--------------------------------------------------------------------------
        volume_ = volume;
        n_ = p.size();
        probability_.clear();
        probability_.resize(n_+1, 0.0);
        vector<Real> prev;
        probability_[0] = 1.0;
        for (Size k = 0; k < n_; k++) {
            prev = probability_;
            probability_[0] = prev[0] * (1.0 - p[k]);
            for (Size i = 1; i <= k; i++)
                probability_[i] = prev[i-1] * p[k] + prev[i] * (1.0 - p[k]);
            probability_[k+1] = prev[k] * p[k];
        }

        excessProbability_.clear();
        excessProbability_.resize(n_+1, 0.0);
        excessProbability_[n_] = probability_[n_];
        for (int k = n_ - 1; k >= 0; k--)
            excessProbability_[k] = excessProbability_[k+1] + probability_[k];

        Distribution dist (nBuckets_, 0.0, maximum_);
        for (Size i = 0; i <= n_; i++) {
            if (volume * i <= maximum_) {
                Size bucket = dist.locate(volume * i);
                dist.addDensity (bucket, probability_[i] / dist.dx(bucket));
                dist.addAverage (bucket, volume*i);
            }
        }

        dist.normalize();

        return dist;
    }

    //--------------------------------------------------------------------------
    Distribution LossDistHomogeneous::operator()(const vector<Real>& nominals,
                                    const vector<Real>& probabilities) const {
    //--------------------------------------------------------------------------
        return operator()(nominals[0], probabilities);
    }

    //--------------------------------------------------------------------------
    Distribution LossDistBucketing::operator()(const vector<Real>& nominals,
                                    const vector<Real>& probabilities) const {
    //--------------------------------------------------------------------------
        QL_REQUIRE (nominals.size() == probabilities.size(), "sizes differ: "
                    << nominals.size() << " vs " << probabilities.size());

        vector<Real> p (nBuckets_, 0.0);
        vector<Real> a (nBuckets_, 0.0);
        vector<Real> ap (nBuckets_, 0.0);

        p[0] = 1.0;
        a[0] = 0.0;
        Real dx = maximum_ / nBuckets_;
        for (Size k = 1; k < nBuckets_; k++)
            a[k] = dx * k + dx/2;

        for (Size i = 0; i < nominals.size(); i++) {
            Real L = nominals[i];
            Real P = probabilities[i];
            for (int k = a.size()-1; k >= 0; k--) {
                if (p[k] > 0) {
                    int u = locateTargetBucket (a[k] + L, k);
                    QL_REQUIRE (u >= 0, "u=" << u << " at i=" << i << " k=" << k);
                    QL_REQUIRE (u >= k, "u=" << u << "<k=" << k << " at i=" << i);

                    Real dp = p[k] * P;
                    if (u == k)
                        a[k] += P * L;
                    else {
                        // no update of a[u] and p[u] if u is beyond grid end
                        if (u < int(nBuckets_)) {
                            // a[u] remains unchanged, if dp = 0
                            if (dp > 0.0) {
                                // on Windows, p[u]/dp could cause a NaN for
                                // some very small values of p[k].
                                // Writing the above as (p[u]/p[k])/P prevents
                                // the NaN. What can I say?
                                Real f = 1.0 / (1.0 + (p[u]/p[k]) / P);
                                a[u] = (1.0 - f) * a[u] + f * (a[k] + L);
                            }
                            /* formulation of Hull-White:
                               if (p[u] + dp > 0)
                                  a[u] = (p[u] * a[u] + dp * (a[k] + L))
                                         / (p[u] + dp);
                            */
                            p[u] += dp;
                        }
                        p[k] -= dp;
                    }
                }
                QL_REQUIRE(a[k] + epsilon_ >= dx * k && a[k] < dx * (k+1),
                           "a out of range at k=" << k << ", contract " << i);
            }
        }

        Distribution dist (nBuckets_, 0.0, maximum_);
        for (Size i = 0; i < nBuckets_; i++) {
            dist.addDensity (i, p[i] / dx);
            dist.addAverage (i, a[i]);
        }

        return dist;
    }

    //--------------------------------------------------------------------------
    int LossDistBucketing::locateTargetBucket (Real loss, Size i0) const {
    //--------------------------------------------------------------------------
        QL_REQUIRE (loss >= 0, "loss " << loss << " must be >= 0");
        Real dx = maximum_ / nBuckets_;
        for (Size i = i0; i < nBuckets_; i++)
            if (dx * i > loss + epsilon_) return i - 1;
        return nBuckets_;
    }

    //--------------------------------------------------------------------------
    Distribution LossDistMonteCarlo::operator()(const vector<Real>& nominals,
                                   const vector<Real>& probabilities) const {
    //--------------------------------------------------------------------------
        Distribution dist (nBuckets_, 0.0, maximum_);
        // KnuthUniformRng rng(seed_);
        // LecuyerUniformRng rng(seed_);
        MersenneTwisterUniformRng rng(seed_);
        for (Size i = 0; i < simulations_; i++) {
            Real e = 0;
            for (Size j = 0; j < nominals.size(); j++) {
                Real r = rng.next().value;
                if (r <= probabilities[j])
                    e += nominals[j];
            }
            dist.add (e + epsilon_);
        }

        dist.normalize();

        return dist;
    }

}
