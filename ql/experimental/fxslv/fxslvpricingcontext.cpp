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

#include <ql/experimental/fxslv/fxslvpricingcontext.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdhestondoublebarrierengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <algorithm>
#include <utility>
#include <cmath>

namespace QuantLib {

    FxSLVPricingContext::FxSLVPricingContext(FxPairMarket market, Volatility localVolFallback)
    : market_(std::move(market)) {
        // the trading-time curves, with the settlement forward, and local vol
        const auto process = market_.localVolProcess(localVolFallback);
        domesticTS_ = process->riskFreeRate();
        foreignTS_ = process->dividendYield();
        localVol_ = process->localVolatility();
    }

    void FxSLVPricingContext::calibrate(
        StochVolCalibrator& hestonCal,
        SLVLeverageCalibrator& leverageCal,
        const std::vector<StochVolCalibrator::Pillar>& pillars,
        const Date& leverageEndDate,
        std::vector<Date> mandatoryDates) {
        QL_REQUIRE(!pillars.empty(),
                   "FxSLVPricingContext::calibrate: pillar list must not be empty");

        hestonCal.calibrate(market_.spotQuote(), domesticTS_, foreignTS_,
                            Handle<BlackVolTermStructure>(market_.volSurface()),
                            market_.conventions().settlement(), pillars);
        hestonModel_ = ext::dynamic_pointer_cast<HestonModel>(hestonCal.calibratedModel());
        QL_REQUIRE(hestonModel_,
                   "FxSLVPricingContext: StochVolCalibrator did not return a HestonModel; "
                   "ensure a HestonCalibrator (or subclass) is used with the "
                   "HestonSLVLeverageCalibrator");
        hestonErrors_ = hestonCal.calibrationErrors();
        hestonRmse_ = hestonCal.rmse();

        // local vol jumps at the pillars: time steps on them
        for (const Date& d : market_.pillars())
            if (d > market_.asOf() && d <= leverageEndDate)
                mandatoryDates.push_back(d);
        std::sort(mandatoryDates.begin(), mandatoryDates.end());
        mandatoryDates.erase(std::unique(mandatoryDates.begin(), mandatoryDates.end()),
                             mandatoryDates.end());

        leverageCal.setStochVolModel(hestonModel_);
        leverageCal.calibrate(localVol_, leverageEndDate, mandatoryDates);
        leverageFunction_ = leverageCal.leverageFunction();
        mixingFactor_ = leverageCal.mixingFactor();
    }

    FxSLVPricingContext FxSLVPricingContext::withSpot(Real spot) const {
        QL_REQUIRE(hestonModel_, "FxSLVPricingContext: calibrate() has not been called");
        FxSLVPricingContext result = *this;
        result.market_ = market_.withSpot(spot);
        // the same parameters on the moved spot
        result.hestonModel_ = ext::make_shared<HestonModel>(ext::make_shared<HestonProcess>(
            domesticTS_, foreignTS_, result.market_.spotQuote(), hestonModel_->v0(),
            hestonModel_->kappa(), hestonModel_->theta(), hestonModel_->sigma(),
            hestonModel_->rho()));
        return result;
    }

    ext::shared_ptr<PricingEngine>
    FxSLVPricingContext::vanillaEngine(const FdmGridConfig& grid,
                                       const FdmSchemeDesc& scheme) const {
        QL_REQUIRE(hestonModel_,
                   "FxSLVPricingContext: calibrate() must be called before creating engines");
        return ext::make_shared<FdHestonVanillaEngine>(
            hestonModel_,
            grid.tGrid, grid.xGrid, grid.vGrid, grid.dampingSteps,
            scheme,
            leverageFunction_,
            mixingFactor_);
    }

    ext::shared_ptr<PricingEngine>
    FxSLVPricingContext::barrierEngine(const FdmGridConfig& grid,
                                        const FdmSchemeDesc& scheme) const {
        QL_REQUIRE(hestonModel_,
                   "FxSLVPricingContext: calibrate() must be called before creating engines");
        return ext::make_shared<FdHestonBarrierEngine>(
            hestonModel_,
            grid.tGrid, grid.xGrid, grid.vGrid, grid.dampingSteps,
            scheme,
            leverageFunction_,
            mixingFactor_);
    }

    ext::shared_ptr<PricingEngine>
    FxSLVPricingContext::doubleBarrierEngine(const FdmGridConfig& grid,
                                              const FdmSchemeDesc& scheme) const {
        QL_REQUIRE(hestonModel_,
                   "FxSLVPricingContext: calibrate() must be called before creating engines");
        return ext::make_shared<FdHestonDoubleBarrierEngine>(
            hestonModel_,
            grid.tGrid, grid.xGrid, grid.vGrid, grid.dampingSteps,
            scheme,
            leverageFunction_,
            mixingFactor_);
    }

    ext::shared_ptr<HestonModel> FxSLVPricingContext::hestonModel() const {
        QL_REQUIRE(hestonModel_,
                   "FxSLVPricingContext: calibrate() has not been called");
        return hestonModel_;
    }

    ext::shared_ptr<LocalVolTermStructure>
    FxSLVPricingContext::leverageFunction() const {
        QL_REQUIRE(leverageFunction_,
                   "FxSLVPricingContext: calibrate() has not been called");
        return leverageFunction_;
    }

    std::vector<Real> FxSLVPricingContext::hestonCalibrationErrors() const {
        return hestonErrors_;
    }

    Real FxSLVPricingContext::hestonRmse() const {
        return hestonRmse_;
    }

}
