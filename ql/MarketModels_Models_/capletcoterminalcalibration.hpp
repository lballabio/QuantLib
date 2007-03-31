/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Mark Joshi

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


#ifndef quantlib_caplet_coterminal_calibration_hpp
#define quantlib_caplet_coterminal_calibration_hpp

#include <ql/types.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class TimeDependantCorrelationStructure;
    class PiecewiseConstantVariance;
    class CurveState;
    class MarketModel;
    class Matrix;

    bool capletCoterminalCalibration(
            const TimeDependantCorrelationStructure& corr,
            const std::vector<boost::shared_ptr<PiecewiseConstantVariance> >& displacedSwapVariances,
            const std::vector<Volatility>& displacedCapletVols,
            const CurveState& cs,
            const Spread displacement,
            const std::vector<Real>& alpha,
            std::vector<Matrix>& pseudoRoots);

}

#endif
