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

/*! \file distribution.cpp
    \brief Discretized probability density and cumulative probability
 */

#include <ql/experimental/credit/distribution.hpp>
#include <ql/math/comparison.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    //-------------------------------------------------------------------------
    Distribution::Distribution (int nBuckets, Real xmin, Real xmax)
    //-------------------------------------------------------------------------
        : size_(nBuckets),
          xmin_(xmin), xmax_(xmax), count_(nBuckets),
          x_(nBuckets,0), dx_(nBuckets,0),
          density_(nBuckets,0),
          cumulativeDensity_(nBuckets,0),
          excessProbability_(nBuckets,0),
          cumulativeExcessProbability_(nBuckets,0),
          average_(nBuckets,0),
          overFlow_(0), underFlow_(0),
          isNormalized_(false) {
        for (int i = 0; i < nBuckets; i++) {
            dx_[i] = (xmax - xmin) / nBuckets;
            x_[i] = (i == 0 ? xmin : x_[i-1] + dx_[i-1]);
        }
        // ensure we match exactly the domain, otherwise we might fail the
        //   locate test because of precission mismatches
        dx_.back() = xmax - x_.back();
    }

    //-------------------------------------------------------------------------
    int Distribution::locate (Real x) {
    //-------------------------------------------------------------------------
        QL_REQUIRE ((x >= x_.front() || close(x, x_.front())) &&
                    (x <= x_.back() + dx_.back()
                     || close(x, x_.back() + dx_.back())),
                    "coordinate " << x
                    << " out of range [" << x_.front() << "; "
                    << x_.back() + dx_.back() << "]");
        for (Size i = 0; i < x_.size(); i++) {
            if (x_[i] > x)
                return i - 1;
        }
        return x_.size() - 1;
    }

    //-------------------------------------------------------------------------
    Real Distribution::dx (Real x) {
    //-------------------------------------------------------------------------
        int i = locate (x);
        return dx_[i];
    }

    //-------------------------------------------------------------------------
    void Distribution::add (Real value) {
    //-------------------------------------------------------------------------
        isNormalized_ = false;
        if (value < x_.front()) underFlow_++;
        else {
            for (Size i = 0; i < count_.size(); i++) {
                if (x_[i] + dx_[i] > value) {
                    count_[i]++;
                    average_[i] += value;
                    return;
                }
            }
            overFlow_++;
        }
    }

    //-------------------------------------------------------------------------
    void Distribution::addDensity (int bucket, Real value) {
    //-------------------------------------------------------------------------
        QL_REQUIRE (bucket >= 0 && bucket < size_, "bucket out of range");
        isNormalized_ = false;
        density_[bucket] += value;
    }

    //-------------------------------------------------------------------------
    void Distribution::addAverage (int bucket, Real value) {
    //-------------------------------------------------------------------------
        QL_REQUIRE (bucket >= 0 && bucket < size_, "bucket out of range");
        isNormalized_ = false;
        average_[bucket] += value;
    }

    //-------------------------------------------------------------------------
    void Distribution::normalize () {
    //-------------------------------------------------------------------------
        if (isNormalized_)
            return;

        int count = underFlow_ + overFlow_;
        for (int i = 0; i < size_; i++)
            count += count_[i];

        excessProbability_[0] = 1.0;
        cumulativeExcessProbability_[0] = 0.0;
        for (int i = 0; i < size_; i++) {
            if (count > 0) {
                density_[i] = 1.0 / dx_[i] * count_[i] / count;
                if (count_[i] > 0)
                    average_[i] /= count_[i];
            }
            if (density_[i] == 0.0)
                average_[i] = x_[i] + dx_[i]/2;

            cumulativeDensity_[i] = density_[i] * dx_[i];
            if (i > 0) {
                cumulativeDensity_[i] += cumulativeDensity_[i-1];
                excessProbability_[i] = 1.0 - cumulativeDensity_[i-1];
//                     excessProbability_[i] = excessProbability_[i-1]
//                         - density_[i-1] * dx_[i-1];
//                     cumulativeExcessProbability_[i]
//                         = (excessProbability_[i-1] +
//                            excessProbability_[i]) / 2 * dx_[i-1]
//                         + cumulativeExcessProbability_[i-1];
                cumulativeExcessProbability_[i]
                    = excessProbability_[i-1] * dx_[i-1]
                    + cumulativeExcessProbability_[i-1];
            }
        }

        isNormalized_ = true;
    }

    //-------------------------------------------------------------------------
    Real Distribution::confidenceLevel (Real quantil) {
    //-------------------------------------------------------------------------
        normalize();
        for (int i = 0; i < size_; i++) {
            if (cumulativeDensity_[i] > quantil)
                return x_[i] + dx_[i];
        }
        return x_.back() + dx_.back();
    }

    //-------------------------------------------------------------------------
    Real Distribution::expectedValue () {
    //-------------------------------------------------------------------------
        normalize();
        Real expected = 0;
        for (int i = 0; i < size_; i++) {
            Real x = x_[i] + dx_[i]/2;
            expected += x * dx_[i] * density_[i];
        }
        return expected;
    }

    //-------------------------------------------------------------------------
    Real Distribution::trancheExpectedValue (Real a, Real d) {
    //-------------------------------------------------------------------------
        normalize();
        Real expected = 0;
        for (int i = 0; i < size_; i++) {
            Real x = x_[i] + dx_[i]/2;
            if (x < a)
                continue;
            if (x > d)
                break;
            expected += (x - a) * dx_[i] * density_[i];
        }

        expected += (d - a) * (1.0 - cumulativeDensity (d));

        return expected;
    }

//     Real Distribution::cumulativeExcessProbability (Real a, Real b) {
//         //normalize();
//         Real integral = 0.0;
//         for (int i = 0; i < size_; i++) {
//             if (x_[i] >= b) break;
//             if (x_[i] >= a)
//                 integral += dx_[i] * excessProbability_[i];
//         }
//         return integral;
//     }

    //-------------------------------------------------------------------------
    Real Distribution::cumulativeExcessProbability (Real a, Real b) {
    //-------------------------------------------------------------------------
        normalize();
        QL_REQUIRE (b <= xmax_,
                 "end of interval " << b << " out of range ["
                 << xmin_ << ", " << xmax_ << "]");
        QL_REQUIRE (a >= xmin_,
                 "start of interval " << a << " out of range ["
                 << xmin_ << ", " << xmax_ << "]");

        int i = locate (a);
        int j = locate (b);
        return cumulativeExcessProbability_[j]-cumulativeExcessProbability_[i];
    }

    //-------------------------------------------------------------------------
    Real Distribution::cumulativeDensity (Real x) {
    //-------------------------------------------------------------------------
        Real tiny = dx_.back() * 1e-3;
        QL_REQUIRE (x > 0, "x must be positive");
        normalize();
        for (int i = 0; i < size_; i++) {
            if (x_[i] + dx_[i] + tiny >= x)
                return ((x - x_[i]) * cumulativeDensity_[i]
                     + (x_[i] + dx_[i] - x) * cumulativeDensity_[i-1]) / dx_[i];
        }
        QL_FAIL ("x = " << x << " beyond distribution cutoff "
                 << x_.back() + dx_.back());
    }

    //-------------------------------------------------------------------------
    void Distribution::tranche (Real attachmentPoint, Real detachmentPoint) {
    //-------------------------------------------------------------------------
        QL_REQUIRE (attachmentPoint < detachmentPoint,
                 "attachment >= detachment point");
        QL_REQUIRE (x_.back() > attachmentPoint && x_.back() > detachmentPoint,
                 "attachment or detachment too large");

        // shift
        while (x_[1] < attachmentPoint) {
            x_.erase(x_.begin());
            dx_.erase(dx_.begin());
            count_.erase(count_.begin());
            density_.erase(density_.begin());
            cumulativeDensity_.erase(cumulativeDensity_.begin());
            excessProbability_.erase(excessProbability_.begin());
        }

        // truncate
        for (Size i = 0; i < x_.size(); i++) {
            x_[i] -= attachmentPoint; // = x_[i-1] + dx_[i-1];
            if (x_[i] > detachmentPoint - attachmentPoint)
                excessProbability_[i] = 0.0;
        }

        // force spike at zero
        excessProbability_[0] = 1.0;

        // update density and cumlated
        for (Size i = 0; i < x_.size(); i++) {
            density_[i] = (excessProbability_[i] - excessProbability_[i+1])
                / dx_[i];
            cumulativeDensity_[i] = density_[i] * dx_[i];
            if (i > 0) cumulativeDensity_[i] += cumulativeDensity_[i-1];
        }
    }

    //-------------------------------------------------------------------------
    Distribution ManipulateDistribution::convolve (const Distribution& d1,
                                                   const Distribution& d2) {
    //-------------------------------------------------------------------------
        // force equal constant bucket sizes
        QL_REQUIRE (d1.dx_[0] == d2.dx_[0], "bucket sizes differ in d1 and d2");
        for (Size i = 1; i < d1.size(); i++)
            QL_REQUIRE (d1.dx_[i] == d1.dx_[i-1], "bucket size varies in d1");
        for (Size i = 1; i < d2.size(); i++)
            QL_REQUIRE (d2.dx_[i] == d2.dx_[i-1], "bucket size varies in d2");

        // force offset 0
        QL_REQUIRE (d1.xmin_ == 0.0 && d2.xmin_ == 0.0,
                 "distributions offset larger than 0");

        Distribution dist(d1.size() + d2.size() - 1,
                          0.0, // assuming both distributions have xmin = 0
                          d1.xmax_ + d2.xmax_);

        for (Size i1 = 0; i1 < d1.size(); i1++) {
            Real dx = d1.dx_[i1];
            for (Size i2 = 0; i2 < d2.size(); i2++)
                dist.density_[i1+i2] = d1.density_[i1] * d2.density_[i2] * dx;
        }

        // update cumulated and excess
        dist.excessProbability_[0] = 1.0;
        for (Size i = 0; i < dist.size(); i++) {
            dist.cumulativeDensity_[i] = dist.density_[i] * dist.dx_[i];
            if (i > 0) {
                dist.cumulativeDensity_[i] += dist.cumulativeDensity_[i-1];
                dist.excessProbability_[i] = dist.excessProbability_[i-1]
                    - dist.density_[i-1] * dist.dx_[i-1];
            }
        }

        return dist;
    }

}
