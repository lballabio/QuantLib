
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file grid.cpp
    \brief Grid constructor for finite differences pricers

    \fullpath
    ql/InterestRateModelling/%grid.cpp
*/

// $Id$

#include "ql/InterestRateModelling/grid.hpp"

//! This is a safety check to be sure we have enough grid points.
#define QL_NUM_OPT_MIN_GRID_POINTS            100
//! This is a safety check to be sure we have enough grid points.
#define QL_NUM_OPT_GRID_POINTS_PER_YEAR        50

namespace QuantLib {

    namespace InterestRateModelling {

        Grid::Grid(size_t gridPoints,
                          double initialCenter,
                          double strikeCenter,
                          Time residualTime,
                          Time timeDelay,
                          const OneFactorModel& model)
        : Array(safeGridPoints(gridPoints, residualTime)) {
            double maxCenter = QL_MAX(initialCenter, strikeCenter);
            double minCenter = QL_MIN(initialCenter, strikeCenter);
            double volatility = model.process()->diffusion(0.0, initialCenter);
            //double volSqrtTime = volatility*QL_SQRT(timeDelay);
            //double minMaxFactor = 4.0*volSqrtTime + 0.08;
            double volSqrtTime = volatility*QL_SQRT(residualTime);
            double minMaxFactor = volSqrtTime;
            double xMin = minCenter - minMaxFactor;
            double xMax = maxCenter + minMaxFactor;
            if (xMin<model.minStateVariable())
                xMin = model.minStateVariable();
            if (xMax>model.maxStateVariable())
                xMax = model.maxStateVariable();
            dx_ = (xMax - xMin)/(size()-1);
            for (Size j=0; j<size(); j++)
                (*this)[j] = xMin + j*dx_;
            index_ = size_t(((initialCenter - xMin)/dx_ + 0.5));
        }

        size_t Grid::safeGridPoints(size_t gridPoints,
                                           Time residualTime) const {
            return QL_MAX(gridPoints, residualTime>1.0 ?
                static_cast<size_t>(
                    (QL_NUM_OPT_MIN_GRID_POINTS +
                    (residualTime-1.0) *
                    QL_NUM_OPT_GRID_POINTS_PER_YEAR))
                : QL_NUM_OPT_MIN_GRID_POINTS);
        }
    }
}
