/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

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

#include <ql/math/sampledcurve.hpp>

namespace QuantLib {

    Real SampledCurve::valueAtCenter() const {
        QL_REQUIRE(!empty(), "empty sampled curve");
        Size jmid = size()/2;
        if (size() % 2 == 1)
            return values_[jmid];
        else
            return (values_[jmid]+values_[jmid-1])/2.0;
    }

    Real SampledCurve::firstDerivativeAtCenter() const {
        QL_REQUIRE(size()>=3,
                   "the size of the curve must be at least 3");
        Size jmid = size()/2;
        if (size() % 2 == 1) {
            return (values_[jmid+1]-values_[jmid-1])/
                (grid_[jmid+1]-grid_[jmid-1]);
        } else {
            return (values_[jmid]-values_[jmid-1])/
                (grid_[jmid]-grid_[jmid-1]);
        }
    }

    Real SampledCurve::secondDerivativeAtCenter() const {
        QL_REQUIRE(size()>=4,
                   "the size of the curve must be at least 4");
        Size jmid = size()/2;
        if (size() % 2 == 1) {
            Real deltaPlus = (values_[jmid+1]-values_[jmid])/
                (grid_[jmid+1]-grid_[jmid]);
            Real deltaMinus = (values_[jmid]-values_[jmid-1])/
                (grid_[jmid]-grid_[jmid-1]);
            Real dS = (grid_[jmid+1]-grid_[jmid-1])/2.0;
            return (deltaPlus-deltaMinus)/dS;
        } else {
            Real deltaPlus = (values_[jmid+1]-values_[jmid-1])/
                (grid_[jmid+1]-grid_[jmid-1]);
            Real deltaMinus = (values_[jmid]-values_[jmid-2])/
                (grid_[jmid]-grid_[jmid-2]);
            return (deltaPlus-deltaMinus)/
                (grid_[jmid]-grid_[jmid-1]);
        }
    }

    void SampledCurve::regrid(const Array &new_grid) {
        CubicInterpolation priceSpline(grid_.begin(), grid_.end(),
                                       values_.begin(),
                                       CubicInterpolation::Spline, false,
                                       CubicInterpolation::SecondDerivative, 0.0,
                                       CubicInterpolation::SecondDerivative, 0.0);
        priceSpline.update();
        Array newValues(new_grid.size());
        Array::iterator val;
        Array::const_iterator grid;
        for (val = newValues.begin(), grid = new_grid.begin() ;
             grid != new_grid.end();
             val++, grid++) {
            *val = priceSpline(*grid, true);
        }
        values_.swap(newValues);
        grid_ = new_grid;
    }

}

