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

/*! \file garch.hpp
    \brief GARCH volatility model
*/

#ifndef quantlib_garch_volatility_model_hpp
#define quantlib_garch_volatility_model_hpp

#include <ql/volatilitymodel.hpp>
#include <vector>

namespace QuantLib {

    //! GARCH volatility model
    /*! Volatilities are assumed to be expressed on an annual basis.
    */
    class Garch11 : public VolatilityCompositor {
      private:
        Real alpha_, beta_, gamma_, vl_;
      public:
        Garch11(Real a, Real b, Real vl) :
            alpha_(a), beta_(b), vl_(vl) {gamma_ = 1 - a - b;};
        Garch11(const TimeSeries<Volatility>& qs) {
            calibrate(qs);
        };
        TimeSeries<Volatility>
        calculate(const TimeSeries<Volatility>& quoteSeries);

        TimeSeries<Volatility>
        calculate(const TimeSeries<Volatility>& quoteSeries,
                  Real, Real, Real);

        void calibrate(const TimeSeries<Volatility>& quoteSeries);
    private:
        Real costFunction(const TimeSeries<Volatility>& qs,
                          Real alpha, Real beta, Real omega);
    };

}


#endif
