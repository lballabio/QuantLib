/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

/*! \file lmfixedvolmodel.hpp
    \brief model of constant volatilities for libor market models
*/

#ifndef quantlib_libor_market_fixed_volatility_model_hpp
#define quantlib_libor_market_fixed_volatility_model_hpp

#include <ql/legacy/libormarketmodels/lmvolmodel.hpp>

namespace QuantLib {

    class LmFixedVolatilityModel : public LmVolatilityModel {
      public:
        LmFixedVolatilityModel(Array volatilities, const std::vector<Time>& startTimes);

        Array volatility(Time t, const Array& x = {}) const override;
        Volatility volatility(Size i, Time t, const Array& x) const override;

      private:
        void generateArguments() override;

        const Array volatilities_;
        const std::vector<Time> startTimes_;
    };

}


#endif

