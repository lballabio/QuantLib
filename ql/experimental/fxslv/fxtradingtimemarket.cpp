/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

#include <ql/experimental/fxslv/fxlocalvolsurface.hpp>
#include <ql/experimental/fxslv/fxtradingtimemarket.hpp>
#include <ql/experimental/fxslv/fxtradingtimevariancesurface.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimeyieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    FxTradingTimeMarket::FxTradingTimeMarket(
        Handle<Quote> spot,
        const Handle<YieldTermStructure>& foreignCurve,
        const Handle<YieldTermStructure>& domesticCurve,
        const FxSettleConvention& settlement,
        const ext::shared_ptr<BlackVolTermStructure>& volSurface,
        Volatility localVolFallback)
    : spot_(std::move(spot)) {
        auto surface = ext::make_shared<FxTradingTimeVarianceSurface>(volSurface);
        tradingTime_ = ext::dynamic_pointer_cast<FxTradingTimeVariance>(volSurface)
                           ->tradingTimeStructure();
        dayCounter_ = TradingTimeDayCounter(tradingTime_);
        volSurface_ = Handle<BlackVolTermStructure>(surface);
        domesticCurve_ = Handle<YieldTermStructure>(
            ext::make_shared<TradingTimeYieldTermStructure>(domesticCurve, tradingTime_));
        foreignCurve_ = Handle<YieldTermStructure>(
            ext::make_shared<TradingTimeYieldTermStructure>(foreignCurve, tradingTime_));
        localVolProcess_ = makeFxLocalVolProcess(spot_, foreignCurve_, domesticCurve_, settlement,
                                                 volSurface_, localVolFallback);
        settlementForeignCurve_ = localVolProcess_->dividendYield();
        localVol_ = localVolProcess_->localVolatility();
    }

}
