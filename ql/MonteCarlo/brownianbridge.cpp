
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file brownianbridge.cpp
    \brief Browian bridge

    \fullpath
    ql/MonteCarlo/%brownianbridge.cpp

*/

// $Id$

// ===========================================================================
// NOTE: The following copyright notice applies to the original code,
// which was taken from "Monte Carlo Methods in Finance", by Peter Jäckel.
// Only minor modifications were applied to integrate it in QuantLib
//
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================



#include "brownianbridge.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        BrownianBridge::BrownianBridge(unsigned long numberOfSteps)
        : numberOfSteps_(numberOfSteps), leftIndex_(numberOfSteps),
          rightIndex_(numberOfSteps), bridgeIndex_(numberOfSteps),
          leftWeight_(numberOfSteps), rightWeight_(numberOfSteps),
          sigma_(numberOfSteps) {

            QL_REQUIRE(numberOfSteps_,
              "BrownianBridge::BrownianBridge : "
              "there must be at least one step");

            std::vector<unsigned long> map(numberOfSteps_);
            // map is used to indicate which points are already constructed.
            // If map[i] is zero, path point i is yet unconstructed.
            // map[i]-1 is the index of the variate that constructs the path point # i.

            unsigned long i,j,k,l;
            //  The first point in the construction is the global step.
            map[numberOfSteps_-1] = 1;
            //  The global step is constructed from the first variate.
            bridgeIndex_[0] = numberOfSteps_-1;
            //  The variance of the global step is numberOfSteps*1.0.
            sigma_[0] = sqrt(numberOfSteps_);
            //  The global step to the last point in time is special.
            leftWeight_[0] = rightWeight_[0] = 0.;
            for (j=0,i=1;i<numberOfSteps_;++i){
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
                leftWeight_[i]  = (k-l)/(k+1.-j);
                rightWeight_[i] = (l+1.-j)/(k+1.-j);
                sigma_[i]       = sqrt(((l+1.-j)*(k-l))/(k+1.-j));
                j=k+1;
                if (j>=numberOfSteps_) j=0;	//	Wrap around.
            }
        }

        void BrownianBridge::buildPath(std::vector<double>& path,
            const std::vector<double>& gaussianVariates ) {
            QL_REQUIRE(gaussianVariates.size() == numberOfSteps_ &&
                      path.size()             == numberOfSteps_,
                      "BrownianBridge::buildPath : "
                      "");
            unsigned long i,j,k,l;
            // The global step.
            path[numberOfSteps_-1] = sigma_[0]*gaussianVariates[0];
            for (i=1;i<numberOfSteps_;i++){
                j = leftIndex_[i];
                k = rightIndex_[i];
                l = bridgeIndex_[i];
                if (j) path[l] = leftWeight_[i]*path[j-1] + rightWeight_[i]*path[k] +
                                 sigma_[i]*gaussianVariates[i];
                else   path[l] = rightWeight_[i]*path[k] + sigma_[i]*gaussianVariates[i];
            }
        }

    }
}
