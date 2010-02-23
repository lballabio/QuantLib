/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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


#ifndef quantlib_generic_longstaff_schwartz_hpp
#define quantlib_generic_longstaff_schwartz_hpp

#include <ql/methods/montecarlo/nodedata.hpp>

namespace QuantLib {

    //! returns the biased estimate obtained while regressing
    /* TODO document:
       n exercises, n+1 elements in simulationData
       simulationData[0][j] -> cashflows up to first exercise, j-th path
       simulationData[i+1][j] -> i-th exercise, j-th path

       simulationData[0][j].foo unused (unusable?) if foo != cumulatedCashFlows

       basisCoefficients.size() = n
    */
    Real genericLongstaffSchwartzRegression(
        std::vector<std::vector<NodeData> >& simulationData,
        std::vector<std::vector<Real> >& basisCoefficients);

}


#endif

