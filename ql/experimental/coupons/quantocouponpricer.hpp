/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Toyin Akin

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file quantocouponpricer.hpp
    \brief quanto-adjusted coupon
*/

#ifndef quantlib_coupon_quanto_pricer_hpp
#define quantlib_coupon_quanto_pricer_hpp

#include <ql/cashflows/couponpricer.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <utility>

namespace QuantLib {

    class BlackIborQuantoCouponPricer : public BlackIborCouponPricer {
      public:
        BlackIborQuantoCouponPricer(Handle<BlackVolTermStructure> fxRateBlackVolatility,
                                    Handle<Quote> underlyingFxCorrelation,
                                    const Handle<OptionletVolatilityStructure>& capletVolatility)
        : BlackIborCouponPricer(capletVolatility),
          fxRateBlackVolatility_(std::move(fxRateBlackVolatility)),
          underlyingFxCorrelation_(std::move(underlyingFxCorrelation)) {
            registerWith(fxRateBlackVolatility_);
            registerWith(underlyingFxCorrelation_);
        }

      protected:
        Rate adjustedFixing(Rate fixing = Null<Rate>()) const override;

      private:
        Handle<BlackVolTermStructure> fxRateBlackVolatility_;
        Handle<Quote> underlyingFxCorrelation_;
    };

}


#endif
