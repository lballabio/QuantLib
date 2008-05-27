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

#include <ql/experimental/credit/lossdistribution.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

using namespace std;

namespace QuantLib {

    namespace detail {

        //--------------------------------------------------------------------
        Real probabilityOfNEvents (Size k, const vector<Real>& defProb) {
        //--------------------------------------------------------------------
            vector<Real> w (defProb.size(), 0);
            vector<Real> u (k+1, 0);
            vector<Real> v (k+1, 0);

            Real pZero = 1.0;
            for (Size i = 0; i < w.size(); i++) {
                pZero *= (1.0 - defProb[i]);
                w[i] = defProb[i] / (1.0 - defProb[i]);
            }

            if (k == 0) return pZero;

            Size kk = k;
            Real prodw = 1.0;

            /*
              if (k > 0.5 * defProb.size()) {
              kk = defProb.size() - k;
              for (Size i = 0; i < w.size(); i++) {
              prodw *= w[i];
              w[i] = 1.0 / w[i];
              }
              }
            */

            // Cumulated probability of up to n events: Cut off when
            // the cumulated probability reaches 1, i.e. set all
            // following probabilities of exactly n events to zero.
            Real sum = 1.0;

            u[0] = 1.0;
            for (Size i = 1; i <= kk; i++) {
                v[i] = 0;
                for (Size j = 0; j < w.size(); j++)
                    v[i] += pow (w[j], int(i));
                u[i] = 0;
                for (Size j = 1; j <= i; j++)
                    u[i] +=  pow (-1.0, int(j+1)) * v[j] * u[i-j];
                u[i] /= i;

                // cut off
                if (sum * pZero >= 1.0 || u[i] < 0 || u[i] * pZero >= 1.0)
                    u[i] = 0;

                sum += u[i];
            }

            return pZero * prodw * u[kk];
        }

        //--------------------------------------------------------------------
        Real probabilityOfAtLeastNEvents (Size n, const vector<Real>& defProb) {
        //--------------------------------------------------------------------
            Real defp = 0;
            for (Size i = n; i <= defProb.size(); i++)
                defp += probabilityOfNEvents (i, defProb);

            return defp;
        }

    }

    //--------------------------------------------------------------------------
    Real ProbabilityOfNEvents::operator()(const vector<Real>& prob) const {
    //--------------------------------------------------------------------------
        return detail::probabilityOfNEvents (n_, prob);
    }

    //--------------------------------------------------------------------------
    Real ProbabilityOfAtLeastNEvents::operator()(
                                             const vector<Real>& prob) const {
    //--------------------------------------------------------------------------
        Real defp = 0;
        for (Size i = n_; i <= prob.size(); i++)
            defp += detail::probabilityOfNEvents (i, prob);
        return defp;
    }

    //--------------------------------------------------------------------------
    Distribution LossDistBucketing::operator()(
                                    const vector<Real>& nominals,
                                    const vector<Real>& probabilities) const {
    //--------------------------------------------------------------------------
        QL_REQUIRE (nominals.size() == probabilities.size(),
                    "vector sizes differ");

        Distribution dist (nBuckets_, 0.0, maximum_);

        vector<Real> p (nBuckets_ + 1, 0.0), a (nBuckets_ + 1, 0.0);

        p[0] = 1.0;
        a[0] = 0.0;
        Real dx = maximum_ / nBuckets_;
        for (Size k = 1; k < nBuckets_; k++)
            a[k] = 0.5 * (dx * (k-1) + dx * k);
        a[nBuckets_] = maximum_ - dx/2;

        for (Size i = 0; i < nominals.size(); i++) {
            vector<Real> aa = a, pp = p;
            for (Size k = Size(a.size())-1; k >= 0; k--) {
                if (p[k] > 0) {
                    Size u = locateTargetBucket (a[k] + nominals[i]);

                    QL_REQUIRE (u >= 0 && u <= nBuckets_ + 1,
                                "u= " << u << " out of range [0, " << nBuckets_ + 1 << "]");

                    //      if (u < k)
                    //        MESSAGE ("contract=" << i << " u=" << u << "<k=" << k);

                    Real dp = p[k] * probabilities[i];
                    if (Size(u) == k)
                        aa[k] = a[k] + probabilities[i] * nominals[i];
                    else {
                        if (u <= nBuckets_) { // overflow in u = nBuckets + 1
                            if (p[u] + dp > 0)
                                aa[u] = (p[u] * a[u] + dp * (a[k] + nominals[i])) / (p[u] + dp);
                            pp[u] = p[u] + dp;
                        }
                        pp[k] = p[k] - dp;
                    }
                    a = aa;
                    p = pp;
                }
            }
        }

        dist.addDensity (0, (p[0] + p[1]) / dx);
        dist.addAverage (0, (a[0] + a[1]) / dx);
        for (Size i = 2; i <= nBuckets_; i++) {
            dist.addDensity (i-1, p[i] / dx);
            dist.addAverage (i-1, a[i]);
        }

        return dist;
    }

    //--------------------------------------------------------------------------
    Size LossDistBucketing::locateTargetBucket (Real loss) const {
    //--------------------------------------------------------------------------
        QL_REQUIRE (loss >= 0, "loss " << loss << " must be >= 0");

        for (Size i = 0; i <= nBuckets_; i++)
            if (maximum_ * i / nBuckets_ >= loss) return i;

        // overflow
        return nBuckets_ + 1;
    }

    //--------------------------------------------------------------------------
    Distribution LossDistMonteCarlo::operator()(
                                    const vector<Real>& nominals,
                                    const vector<Real>& probabilities) const {
    //--------------------------------------------------------------------------
        Distribution dist (nBuckets_, 0.0, maximum_);
        MersenneTwisterUniformRng mt;
        for (Size i = 0; i < simulations_; i++) {
            Real e = 0;
            for (Size j = 0; j < nominals.size(); j++) {
                Real r = mt.next().value;
                if (r <= probabilities[j])
                    e += nominals[j];
            }
            dist.add (e);
        }

        dist.normalize();

        return dist;
    }

}
