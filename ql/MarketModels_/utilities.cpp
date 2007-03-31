/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/utilities.hpp>
#include <ql/errors.hpp>
#include <algorithm>

namespace QuantLib {

    void mergeTimes(const std::vector<std::vector<Time> >& times,
                    std::vector<Time>& mergedTimes,
                    std::vector<std::vector<bool> >& isPresent) {

        std::vector<Time> allTimes;
        Size i;
        for (i=0; i<times.size(); i++) {
            allTimes.insert(allTimes.end(),
                            times[i].begin(),
                            times[i].end());
        }

        // ...sort and compact the vector mergedTimes...
        std::sort(allTimes.begin(), allTimes.end());
        std::vector<Time>::iterator end = std::unique(allTimes.begin(),
                                                      allTimes.end());
        //mergedTimes.clear(); // shouldn't be cleared?
        mergedTimes.insert(mergedTimes.end(),
                           allTimes.begin(), end);

        isPresent.resize(times.size());
        for (i=0; i<times.size(); i++) {
            isPresent[i].resize(allTimes.size());
            for (Size j=0; j<allTimes.size(); j++) {
                isPresent[i][j] = std::binary_search(times[i].begin(),
                                                     times[i].end(),
                                                     allTimes[j]);
            }
        }
    }

    std::vector<bool> isInSubset(const std::vector<Time>& set,
                                 const std::vector<Time>& subset) {

        std::vector<bool> result(set.size(), false);
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
            while (true) {              // loop in subset
                subsetElement = subset[j];
                result[i] = false;
                // if smaller no hope, leave false and go to next i
                if (setElement < subsetElement) break;
                // if match, set result[i] to true and go to next i
                if (setElement == subsetElement) {
                    result[i] = true;
                    break;
                } 
                // if larger, leave false if at the end or go to next j
                if (j > dimsubSet-1) break;
                ++j;
            }
        }
        return result;
    }

}
