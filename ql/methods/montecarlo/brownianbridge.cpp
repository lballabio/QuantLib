/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

// ===========================================================================
// NOTE: The following copyright notice applies to the original code,
//
// Copyright (C) 2002 Peter Jäckel "Monte Carlo Methods in Finance".
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#include <ql/methods/montecarlo/brownianbridge.hpp>

namespace QuantLib {

    BrownianBridge::BrownianBridge(Size steps)
    : size_(steps), t_(size_), sqrtdt_(size_),
      bridgeIndex_(size_), leftIndex_(size_), rightIndex_(size_),
      leftWeight_(size_), rightWeight_(size_), stdDev_(size_) {
        for (Size i=0; i<size_; ++i)
            t_[i] = static_cast<Time>(i+1);
        initialize();
    }

    BrownianBridge::BrownianBridge(const std::vector<Time>& times)
    : size_(times.size()), t_(times), sqrtdt_(size_),
      bridgeIndex_(size_), leftIndex_(size_), rightIndex_(size_),
      leftWeight_(size_), rightWeight_(size_), stdDev_(size_) {
        initialize();
    }

    BrownianBridge::BrownianBridge(const TimeGrid& timeGrid)
    : size_(timeGrid.size()-1), t_(size_), sqrtdt_(size_),
      bridgeIndex_(size_), leftIndex_(size_), rightIndex_(size_),
      leftWeight_(size_), rightWeight_(size_), stdDev_(size_) {
        for (Size i=0; i<size_; ++i)
            t_[i] = timeGrid[i+1];
        initialize();
    }


    void BrownianBridge::initialize() {

        sqrtdt_[0] = std::sqrt(t_[0]);
        for (Size i=1; i<size_; ++i)
            sqrtdt_[i] = std::sqrt(t_[i]-t_[i-1]);

        // map is used to indicate which points are already constructed.
        // If map[i] is zero, path point i is yet unconstructed.
        // map[i]-1 is the index of the variate that constructs
        // the path point # i.
        std::vector<Size> map(size_, 0);

        //  The first point in the construction is the global step.
        map[size_-1] = 1;
        //  The global step is constructed from the first variate.
        bridgeIndex_[0] = size_-1;
        //  The variance of the global step
        stdDev_[0] = std::sqrt(t_[size_-1]);
        //  The global step to the last point in time is special.
        leftWeight_[0] = rightWeight_[0] = 0.0;
        for (Size j=0, i=1; i<size_; ++i) {
            // Find the next unpopulated entry in the map.
            while (map[j] != 0U)
                ++j;
            Size k = j;
            // Find the next populated entry in the map from there.
            while (map[k] == 0U)
                ++k;
            // l-1 is now the index of the point to be constructed next.
            Size l = j + ((k-1-j)>>1);
            map[l] = i;
            // The i-th Gaussian variate will be used to set point l-1.
            bridgeIndex_[i] = l;
            leftIndex_[i]   = j;
            rightIndex_[i]  = k;
            if (j != 0) {
                leftWeight_[i]= (t_[k]-t_[l])/(t_[k]-t_[j-1]);
                rightWeight_[i] = (t_[l]-t_[j-1])/(t_[k]-t_[j-1]);
                stdDev_[i] =
                    std::sqrt(((t_[l]-t_[j-1])*(t_[k]-t_[l]))
                              /(t_[k]-t_[j-1]));
            } else {
                leftWeight_[i]  = (t_[k]-t_[l])/t_[k];
                rightWeight_[i] =  t_[l]/t_[k];
                stdDev_[i] = std::sqrt(t_[l]*(t_[k]-t_[l])/t_[k]);
            }
            j=k+1;
            if (j>=size_)
                j=0;    //  wrap around
        }
    }

}

