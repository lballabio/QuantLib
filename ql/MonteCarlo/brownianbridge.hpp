
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file brownianbridge.hpp
    \brief Browian bridge
*/

// ===========================================================================
// NOTE: The following copyright notice applies to the original code of
// the brownian bridge construction, which was taken from
// "Monte Carlo Methods in Finance", by Peter Jäckel.
//
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#ifndef   quantlib_brownian_bridge_h
#define   quantlib_brownian_bridge_h

#include <ql/MonteCarlo/path.hpp>
#include <ql/MonteCarlo/sample.hpp>

namespace QuantLib {

    //! Builds Wiener process paths using Gaussian variates
    /*! For more details: "Monte Carlo Methods in Finance" by P. Jäckel,
        section 10.8.3
    */
    template <class GSG> // Gaussian Sequence Generator
    class BrownianBridge {
      public:
        typedef Sample<std::vector<double> > sample_type;
        //! normalised (unit time, unit variance) Wiener process paths
        BrownianBridge(GSG generator);
        //! unit variance Wiener process paths
        BrownianBridge(Time length,
                       Size timeSteps,
                       GSG generator);
        //! unit variance Wiener process paths
        BrownianBridge(const TimeGrid& timeGrid,
                       GSG generator);
        //! general Wiener process paths
        BrownianBridge(const std::vector<double>& sigma,
                       const TimeGrid& timeGrid,
                       GSG generator);
        BrownianBridge(const Handle<BlackVolTermStructure>& blackVol,
                       const TimeGrid& timeGrid,
                       GSG generator);
        //! \name inspectors
        //@{
        const sample_type& next() const;
        const sample_type& antithetic() const;
        Size size() const { return dimension_; }
        const TimeGrid& timeGrid() const { return timeGrid_; }
        //@}
      private:
        void initialize(const std::vector<double>& v);
        GSG generator_;
        Size dimension_;
        TimeGrid timeGrid_;
        mutable Sample<std::vector<double> > next_;
        std::vector<Size> bridgeIndex_, leftIndex_, rightIndex_;
        std::vector<double> leftWeight_, rightWeight_, stdDev_;
    };


    template <class GSG>
    BrownianBridge<GSG>::BrownianBridge(GSG generator)
    : generator_(generator), dimension_(generator_.dimension()),
      timeGrid_(Time(dimension_), dimension_),
      next_(std::vector<double>(dimension_), 1.0),
      bridgeIndex_(dimension_),
      leftIndex_(dimension_),  rightIndex_(dimension_),
      leftWeight_(dimension_), rightWeight_(dimension_),
      stdDev_(dimension_) {
        initialize(std::vector<double>(dimension_, 1.0));
    }

    template <class GSG>
    BrownianBridge<GSG>::BrownianBridge(Time length, Size timeSteps,
                                        GSG generator)
    : generator_(generator), dimension_(generator_.dimension()),
      timeGrid_(length, timeSteps),
      next_(std::vector<double>(dimension_), 1.0),
      bridgeIndex_(dimension_),
      leftIndex_(dimension_),  rightIndex_(dimension_),
      leftWeight_(dimension_), rightWeight_(dimension_),
      stdDev_(dimension_) {

        QL_REQUIRE(dimension_>0,
                   "BrownianBridge::BrownianBridge : "
                   "there must be at least one step");

        QL_REQUIRE(dimension_==timeGrid_.size(),
                   "BrownianBridge::BrownianBridge : "
                   "GSG/timeGrid mismatch");

        initialize(std::vector<double>(dimension_, 1.0));
    }

    template <class GSG>
    BrownianBridge<GSG>::BrownianBridge(const TimeGrid& timeGrid,
                                        GSG generator)
    : generator_(generator), dimension_(generator_.dimension()),
      timeGrid_(timeGrid),
      next_(std::vector<double>(dimension_), 1.0),
      bridgeIndex_(dimension_),
      leftIndex_(dimension_),  rightIndex_(dimension_),
      leftWeight_(dimension_), rightWeight_(dimension_),
      stdDev_(dimension_) {

        QL_REQUIRE(dimension_>0,
                   "BrownianBridge::BrownianBridge : "
                   "there must be at least one step");

        QL_REQUIRE(dimension_==timeGrid_.size(),
                   "BrownianBridge::BrownianBridge : "
                   "GSG/timeGrid mismatch");

        initialize(std::vector<double>(dimension_, 1.0));
    }

    template <class GSG>
    BrownianBridge<GSG>::BrownianBridge(const std::vector<double>& variances,
                                        const TimeGrid& timeGrid,
                                        GSG generator)
    : generator_(generator), dimension_(generator_.dimension()),
      timeGrid_(timeGrid),
      next_(std::vector<double>(dimension_), 1.0),
      bridgeIndex_(dimension_),
      leftIndex_(dimension_),  rightIndex_(dimension_),
      leftWeight_(dimension_), rightWeight_(dimension_),
      stdDev_(dimension_) {

        QL_REQUIRE(dimension_>0,
                   "BrownianBridge::BrownianBridge : "
                   "there must be at least one step");

        QL_REQUIRE(dimension_==timeGrid_.size(),
                   "BrownianBridge::BrownianBridge : "
                   "GSG/timeGrid mismatch");

        QL_REQUIRE(dimension_==variances.size(),
                   "BrownianBridge::BrownianBridge : "
                   "GSG/timeGrid mismatch");

        initialize(variances);
    }

    template <class GSG>
    BrownianBridge<GSG>::BrownianBridge(
                                const Handle<BlackVolTermStructure>& blackVol,
                                const TimeGrid& timeGrid,
                                GSG generator)
    : generator_(generator), dimension_(generator_.dimension()),
      timeGrid_(timeGrid),
      next_(std::vector<double>(dimension_), 1.0),
      bridgeIndex_(dimension_),
      leftIndex_(dimension_),  rightIndex_(dimension_),
      leftWeight_(dimension_), rightWeight_(dimension_),
      stdDev_(dimension_) {

        QL_REQUIRE(dimension_>0,
                   "BrownianBridge::BrownianBridge : "
                   "there must be at least one step");

        QL_REQUIRE(dimension_==timeGrid_.size(),
                   "BrownianBridge::BrownianBridge : "
                   "GSG/timeGrid mismatch");

        std::vector<double> v(dimension_);
        for (Size i=0; i<dimension_; i++) {
            // problems here if the blackVol is asset dependant
            v[i]=blackVol->blackVariance(timeGrid_[i], 1.0);
        }
        initialize(v);
    }


    template <class GSG>
    void BrownianBridge<GSG>::initialize(const std::vector<double>& v) {

        std::vector<Size> map(dimension_);
        // map is used to indicate which points are already constructed.
        // If map[i] is zero, path point i is yet unconstructed.
        // map[i]-1 is the index of the variate that constructs
        // the path point # i.

        Size i,j,k,l;
        //  The first point in the construction is the global step.
        map[dimension_-1] = 1;
        //  The global step is constructed from the first variate.
        bridgeIndex_[0] = dimension_-1;
        //  The variance of the global step
        stdDev_[0] = QL_SQRT(v[dimension_-1]);
        //  The global step to the last point in time is special.
        leftWeight_[0] = rightWeight_[0] = 0.;
        for (j=0, i=1; i<dimension_; ++i) {
            // Find the next unpopulated entry in the map.
            while (map[j]) ++j;
            k=j;
            // Find the next populated entry in the map from there.
            while ((!map[k])) ++k;
            // l-1 is now the index of the point to be constructed next.
            l=j+((k-1-j)>>1);
            map[l]=i;
            // The i-th Gaussian variate will be used to set point l-1.
            bridgeIndex_[i] = l;
            leftIndex_[i]   = j;
            rightIndex_[i]  = k;
            leftWeight_[i]  = (v[k]-v[l])  /(v[k]-v[j-1]);
            rightWeight_[i] = (v[l]-v[j-1])/(v[k]-v[j]-1);
            stdDev_[i] = sqrt(((v[l]-v[j-1])*(v[k]-v[l]))/(v[k]-v[j-1]));
            j=k+1;
            if (j>=dimension_) j=0;	//	Wrap around.
        }
    }

    template <class GSG>
    inline const typename BrownianBridge<GSG>::sample_type&
    BrownianBridge<GSG>::next() const {

        typedef typename GSG::sample_type sequence_type;
        const sequence_type& sequence_ = generator_.nextSequence();

        next_.weight = sequence_.weight;

        Size i, j, k, l;
        // The global step.
        next_.value[dimension_-1] = stdDev_[0] * sequence_.value[0];
        for (i=1; i<dimension_; i++) {
            j = leftIndex_[i];
            k = rightIndex_[i];
            l = bridgeIndex_[i];
            // using only the drift component here for the time being ....
            if (j) next_.value[l] =
                       leftWeight_[i] * next_.value[j-1] +
                       rightWeight_[i] * next_.value[k]   +
                       stdDev_[i] * sequence_.value[i];
            else   next_.value[l] =
                       rightWeight_[i] * next_.value[k]   +
                       stdDev_[i] * sequence_.value[i];
        }

        return next_;
    }

    template <class GSG>
    inline const typename BrownianBridge<GSG>::sample_type&
    BrownianBridge<GSG>::antithetic() const {
        return next_;
    }

}


#endif
