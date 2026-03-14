/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Mark Joshi

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

#include <ql/models/marketmodels/utilities.hpp>
#include <ql/errors.hpp>
#include <algorithm>
#include <valarray>

namespace QuantLib {

    void mergeTimes(const std::vector<std::vector<Time> >& times,
                    std::vector<Time>& mergedTimes,
                    std::vector<std::valarray<bool> >& isPresent) {

        std::vector<Time> allTimes;
        for (const auto& time : times) {
            allTimes.insert(allTimes.end(), time.begin(), time.end());
        }

        // ...sort and compact the vector mergedTimes...
        std::sort(allTimes.begin(), allTimes.end());
        auto end = std::unique(allTimes.begin(), allTimes.end());
        //mergedTimes.clear(); // shouldn't be cleared?
        mergedTimes.insert(mergedTimes.end(),
                           allTimes.begin(), end);

        isPresent.resize(times.size());
        for (Size i=0; i<times.size(); i++) {
            isPresent[i].resize(allTimes.size());
            for (Size j=0; j<allTimes.size(); j++) {
                isPresent[i][j] = std::binary_search(times[i].begin(),
                                                     times[i].end(),
                                                     allTimes[j]);
            }
        }
    }

    std::valarray<bool> isInSubset(const std::vector<Time>& set,
                                   const std::vector<Time>& subset) {

        std::valarray<bool> result(false,set.size());
        Size dimsubSet = subset.size();
        if (dimsubSet==0)
            return result;
        Size dimSet = set.size();
        Time setElement, subsetElement;

        QL_REQUIRE(dimSet >= dimsubSet,
                   "set is required to be larger or equal than subset");

        for (Size i=0; i<dimSet; ++i) {  // loop in set
            Size j=0;
            setElement = set[i];
            for (;;) {              // loop in subset
                subsetElement = subset[j];
                result[i] = false;
                // if smaller no hope, leave false and go to next i
                if (setElement < subsetElement)
                    break;
                // if match, set result[i] to true and go to next i
                if (setElement == subsetElement) {
                    result[i] = true;
                    break;
                }
                // if larger, leave false if at the end or go to next j
                if (j == dimsubSet-1)
                    break;
                ++j;
            }
        }
        return result;
    }

    void checkIncreasingTimes(const std::vector<Time>& times) {
        Size nTimes = times.size();
        QL_REQUIRE(nTimes>0,
                   "at least one time is required");
        QL_REQUIRE(times[0]>0.0,
                   "first time (" << times[0] <<
                   ") must be greater than zero");
        for (Size i=0; i<nTimes-1; ++i)
            QL_REQUIRE(times[i+1]-times[i]>0,
                       "non increasing rate times: "
                       "times[" << i   << "]=" << times[i] << ", "
                       "times[" << i+1 << "]=" << times[i+1]);
    }

    void checkIncreasingTimesAndCalculateTaus(const std::vector<Time>& times,
                                              std::vector<Time>& taus) {
        Size nTimes = times.size();
        QL_REQUIRE(nTimes>1,
                   "at least two times are required, " << nTimes << " provided");
        QL_REQUIRE(times[0]>0.0,
                   "first time (" << times[0] <<
                   ") must be greater than zero");
        if (taus.size()!=nTimes-1)
            taus.resize(nTimes-1);
        for (Size i=0; i<nTimes-1; ++i) {
            taus[i]=times[i+1]-times[i];
            QL_REQUIRE(taus[i]>0,
                       "non increasing rate times: "
                       "times[" << i   << "]=" << times[i] << ", "
                       "times[" << i+1 << "]=" << times[i+1]);
        }
    }


}
