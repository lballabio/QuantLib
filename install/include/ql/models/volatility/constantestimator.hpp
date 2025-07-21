/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

/*! \file constantestimator.hpp
    \brief Constant volatility estimator
*/

#ifndef quantlib_constant_estimator_hpp
#define quantlib_constant_estimator_hpp

#include <ql/volatilitymodel.hpp>
#include <vector>

namespace QuantLib {

    //! Constant-estimator volatility model
    /*! Volatilities are assumed to be expressed on an annual basis.
    */
    class ConstantEstimator : public VolatilityCompositor {
      private:
        Size size_;
      public:
        ConstantEstimator(Size size)
        : size_(size) {}
        TimeSeries<Volatility> calculate(const TimeSeries<Volatility>&) override;
        void calibrate(const TimeSeries<Volatility>&) override {}
    };

}


#endif
