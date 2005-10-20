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
        SampledCurve(Size gridSize=0) {
            gridSize_ = gridSize;
            grid_ = Array(gridSize);
            values_ = Array(gridSize);
        }

        void setLogSpacing(Real min, Real max) {
            Real gridLogSpacing =
                (std::log(max) - std::log(min)) / (gridSize_ -1);
            Real edx = std::exp(gridLogSpacing);
            grid_[0] = min;
            for (Size j=1; j < gridSize_; j++) {
                grid_[j] = grid_[j-1]*edx;
            }
        }

        void setLinearSpacing(Real min, Real max) {
            Real gridSpacing =
                (max - min) / (gridSize_ -1);
            grid_[0] = min;
            for (Size j=1; j < gridSize_; j++) {
                grid_[j] = grid_[j-1] + gridSpacing;
            }
        }

        template <class F>
        void sample(F &f) {
            for(Size j=0; j < size(); j++)
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

        Real & gridValue(Size i) {
            return grid_[i];
        }

        Real & value(Size i) {
            return values_[i];
        }

        Size size() {
            return gridSize_;
        }

        void swap(SampledCurve &from) {
            using std::swap;
            swap(gridSize_, from.gridSize_);
            grid_.swap(from.grid_);
            values_.swap(from.values_);
        }

        SampledCurve& operator=(const SampledCurve& from) {
            SampledCurve temp(from);
            swap(temp);
            return *this;
        }

        inline bool empty() const {
            return gridSize_ == 0;
        }

        Real valueAtCenter() {
            QL_REQUIRE(!empty(),
                       "size must not be empty");
            Size jmid = size()/2;
            if (size() % 2 == 1)
                return values_[jmid];
            else
                return (values_[jmid]+values_[jmid-1])/2.0;
        }

        Real firstDerivativeAtCenter() {
            QL_REQUIRE(size()>=3,
                       "the size of the two vectors must be at least 3");
            Size jmid = size()/2;
            if (size() % 2 == 1)
                return (values_[jmid+1]-values_[jmid-1])/
                    (grid_[jmid+1]-grid_[jmid-1]);
            else
                return (values_[jmid]-values_[jmid-1])/
                    (grid_[jmid]-grid_[jmid-1]);
        }

        Real secondDerivativeAtCenter() {
            QL_REQUIRE(size()>=4,
                       "the size of the two vectors must be at least 4");
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
      private:
        Size gridSize_;
        Array grid_;
        Array values_;
    };

    typedef SampledCurve SampledCurveSet;

}


#endif

