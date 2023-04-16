/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

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


#ifndef quantlib_market_model_differences_hpp
#define quantlib_market_model_differences_hpp

#include <ql/types.hpp>
#include <memory>
#include <vector>

namespace QuantLib {
    class MarketModel;
    class PiecewiseConstantCorrelation;
    class PiecewiseConstantVariance;

    std::vector<Volatility> rateVolDifferences(
                                           const MarketModel& marketModel1,
                                           const MarketModel& marketModel2);

    std::vector<Spread> rateInstVolDifferences(
                                           const MarketModel& marketModel1,
                                           const MarketModel& marketModel2,
                                           Size index);

    std::vector<Real> coterminalSwapPseudoRoots(
            const PiecewiseConstantCorrelation&,
            const std::vector<std::shared_ptr<PiecewiseConstantVariance> >&,
            const std::vector<Time>&);
}

#endif
