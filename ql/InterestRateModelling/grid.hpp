
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file grid.hpp
    \brief Abstract interest rate model class

    \fullpath
    ql/InterestRateModelling/%grid.hpp
*/

// $Id$


#ifndef quantlib_grid_h
#define quantlib_grid_h

#include <cmath>
#include "ql/InterestRateModelling/onefactormodel.hpp"

//! This is a safety check to be sure we have enough grid points.
#define QL_NUM_OPT_MIN_GRID_POINTS            100
//! This is a safety check to be sure we have enough grid points.
#define QL_NUM_OPT_GRID_POINTS_PER_YEAR        50

namespace QuantLib {

    namespace InterestRateModelling {

        class Grid : public Array {
          public:
            Grid(Size gridPoints,
                 double initialCenter,
                 double strikeCenter,
                 Time residualTime,
                 Time timeDelay,
                 const Handle<OneFactorModel>& model)
            : Array(safeGridPoints(gridPoints, residualTime)) {
                initialize(gridPoints, initialCenter, strikeCenter,
                    residualTime, timeDelay, model);
            }
            Grid(Size gridPoints,
                 double initialCenter,
                 double strikeCenter,
                 Time residualTime,
                 Time timeDelay,
                 OneFactorModel* model)
            : Array(safeGridPoints(gridPoints, residualTime)) {
                initialize(gridPoints, initialCenter, strikeCenter,
                    residualTime, timeDelay,
                    Handle<OneFactorModel>(model,false));
            }
            double xMin() {return (*this)[0];}
            double xMax() {return (*this)[size()-1];}
            double dx() { return dx_;}
            Size index() const {return index_;}
            Size safeGridPoints(unsigned int gridPoints,
              Time residualTime) const;
          private:
            void initialize(Size gridPoints,
                 double initialCenter,
                 double strikeCenter,
                 Time residualTime,
                 Time timeDelay,
                 const Handle<OneFactorModel>& model);
            double dx_;
            Size index_;

        };


        inline void Grid::initialize(Size gridPoints,
          double initialCenter, double strikeCenter,
          Time residualTime, Time timeDelay,
          const Handle<OneFactorModel>& model) {
            double maxCenter = QL_MAX(initialCenter, strikeCenter);
            double minCenter = QL_MIN(initialCenter, strikeCenter);
            double yMax = model->process()->variable(0.5, 0.0);
            double volatility = QL_MAX(
              model->process()->diffusion(0.0, initialCenter),
              model->process()->diffusion(0.0, yMax));
            //double volSqrtTime = volatility*QL_SQRT(timeDelay);
            //double minMaxFactor = 4.0*volSqrtTime + 0.08;
            double volSqrtTime = volatility*QL_SQRT(residualTime);
            double minMaxFactor = volSqrtTime +
                model->process()->variable(0.08, 0.0);
            double xMin = minCenter - minMaxFactor;
            double xMax = maxCenter + minMaxFactor;
            if (xMin<model->minStateVariable())
                xMin = model->minStateVariable();
            if (xMax>model->maxStateVariable())
                xMax = model->maxStateVariable();
            dx_ = (xMax - xMin)/(size()-1);
            for (unsigned j=0; j<size(); j++)
                (*this)[j] = xMin + j*dx_;
            index_ = Size(((initialCenter - xMin)/dx_ + 0.5));
        }

        inline Size Grid::safeGridPoints(
            Size gridPoints, Time residualTime) const {
            return QL_MAX(gridPoints, residualTime>1.0 ?
                static_cast<Size>(
                    (QL_NUM_OPT_MIN_GRID_POINTS +
                    (residualTime-1.0) *
                    QL_NUM_OPT_GRID_POINTS_PER_YEAR))
                : QL_NUM_OPT_MIN_GRID_POINTS);
        }
    }
}

#endif
