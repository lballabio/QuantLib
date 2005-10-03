/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, Joseph Wang

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

/*! \file sampledcurve.hpp
    \brief a class that contains a sampled curve
*/
#ifndef quantlib_sampledcurve_hpp
#define quantlib_sampledcurve_hpp
#include <ql/Math/array.hpp>
#include <ql/types.hpp>

namespace QuantLib {
    //! This class contains a sampled curve.  
    /*! Initially the class will contain one indexed curve */

    class SampledCurve {
    public:
        SampledCurve(Size gridSize) {
            gridSize_ = gridSize;
            grid_ = Array(gridSize);
            values_ = Array(gridSize);
        } 

        void setLogSpacing(Real min, Real max) {
            Real gridLogSpacing =
                (std::log(max) - std::log(min)) / (gridSize_ -1);
            Real edx = std::exp(gridLogSpacing);
            grid_[0] = min;
            values_[0] = 0.0;
            for (Size j=1; j < gridSize_; j++) {
                grid_[j] = grid_[j-1]*edx;
                values_[j] = 0.0;
            }
        }

        void setLinearSpacing(Real min, Real max) {
            Real gridSpacing =
                (max - min) / (gridSize_ -1);
            grid_[0] = min;
            values_[0] = 0.0;
            for (Size j=1; j < gridSize_; j++) {
                grid_[j] = grid_[j-1] + gridSpacing;
                values_[j] = 0.0;
            }
        }
        
        template <class F>
        void sample(F &f) {
            for(Size j=0; j < grid_.size(); j++) 
                values_[j] = f(grid_[j]);
        }

        void setGrid(const Array &g) {
            grid_ = g;
        }

        void setValues(const Array &g) {
            values_ = g;
        }

        Array & grid() {
            return grid_;
        }
        
        Array & values() {
            return values_;
        }

        Real & value(int i) {
            return values_[i];
        }
    private:
        Size gridSize_;
        Array grid_;
        Array values_;
  };
 typedef SampledCurve SampledCurveSet;
}
#endif

