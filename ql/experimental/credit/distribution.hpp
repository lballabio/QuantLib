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

/*! \file distribution.hpp
    \brief Discretized probability density and cumulative probability
*/

#ifndef quantlib_probability_distribution_hpp
#define quantlib_probability_distribution_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Discretized probability density and cumulative probability
    class Distribution {
      public:
        Distribution(Size nBuckets, Real xmin, Real xmax);
        Distribution() {};

        void add(Real value);
        void addDensity(Size bucket, Real value);
        void addAverage(Size bucket, Real value);
        void normalize();

        Size size() const { return size_; }
        Real density(Size k) { return density_.at(k); }
        Real average(Size k) { return average_.at(k); }
        Real x(Size k) { return x_.at(k); }
        std::vector<Real>& x() { return x_; }
        Real dx(Size k) { return x_.at(k); }
        std::vector<Real>& dx() { return dx_; }
        Real dx(Real x);

        Real confidenceLevel(Real quantile);
        Real cumulativeDensity(Real x);
        Real cumulativeExcessProbability(Real a, Real b);
        Real expectedValue();
        Real trancheExpectedValue(Real a, Real d);

        template <class F>
        Real expectedValue(F& f) {
            normalize();
            Real expected = 0;
            for (Size i = 0; i < size_; i++) {
                Real x = x_[i] + dx_[i]/2;
                expected += f(x) * dx_[i] * density_[i];
            }
            return expected;
        }

        /*! Transform the loss distribution into the tranche loss
            distribution for losses L_T = min(L,D) - min(L,A).
            The effects are:
            #- shift the distribution to the left by A, then
            #- cut off at D-A, Pr(L_T > D-A) = 0
            #- ensure Pr(L_T >= 0) = 1, i.e. a density spike at L_T = 0
        */
        void tranche(Real attachmentPoint, Real detachmentPoint);

        //! index of the grid point to the left of x
        Size locate(Real x);

        static Distribution convolve(const Distribution& d1,
                                     const Distribution& d2,
                                     Size buckets = 0);

      private:
        Size size_;
        Real xmin_, xmax_;
        std::vector<Size> count_;
        // x: coordinate of left hand cell bundary
        // dx: cell width
        std::vector<Real> x_, dx_;
        // density: probability density, densitx*dx = prob. of loss in cell i
        // cumulatedDensity: cumulated (integrated) from x = 0
        // excessProbability: cumulated from x_i to infinity
        // cumulativeExcessProbability: integrated excessProbability from x = 0
        std::vector<Real> density_, cumulativeDensity_;
        std::vector<Real> excessProbability_, cumulativeExcessProbability_;
        // average loss in cell i
        std::vector<Real> average_;

        Size overFlow_, underFlow_;
        bool isNormalized_;
    };

}

#endif
