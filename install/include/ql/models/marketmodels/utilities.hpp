/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_market_model_utilities_hpp
#define quantlib_market_model_utilities_hpp

#include <ql/types.hpp>
#include <vector>
#include <valarray>

namespace QuantLib {

    void mergeTimes(const std::vector<std::vector<Time> >& times,
                    std::vector<Time>& mergedTimes,
                    std::vector<std::valarray<bool> >& isPresent);

    /*! Look for elements of a set in a subset.
        Returns a vector of booleans such that:
        element set[i] present/not present in subset.

       \pre both vectors must be strictly increasing.
    */
    std::valarray<bool> isInSubset(const std::vector<Time>& set,
                                 const std::vector<Time>& subset);

    //! check for strictly increasing times, first time greater than zero
    void checkIncreasingTimes(const std::vector<Time>& times);
    void checkIncreasingTimesAndCalculateTaus(const std::vector<Time>& times,
                                              std::vector<Time>& taus);
}

#endif
