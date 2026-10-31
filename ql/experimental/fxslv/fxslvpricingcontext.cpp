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
#include <utility>
#include <cmath>

namespace QuantLib {

    FxSLVPricingContext::FxSLVPricingContext(Handle<Quote>                 spot,
                                             Handle<YieldTermStructure>    domesticTS,
                                             Handle<YieldTermStructure>    foreignTS,
                                             Handle<BlackVolTermStructure> volSurface,
                                             Handle<LocalVolTermStructure> localVol,
                                             Calendar                      calendar)
    : spot_(std::move(spot)), domesticTS_(std::move(domesticTS)),
      foreignTS_(std::move(foreignTS)), volSurface_(std::move(volSurface)),
      localVol_(std::move(localVol)), calendar_(std::move(calendar)) {

        QL_REQUIRE(!spot_.empty(),       "FxSLVPricingContext: spot handle is empty");
        QL_REQUIRE(!domesticTS_.empty(), "FxSLVPricingContext: domesticTS handle is empty");
        QL_REQUIRE(!foreignTS_.empty(),  "FxSLVPricingContext: foreignTS handle is empty");
        QL_REQUIRE(!volSurface_.empty(), "FxSLVPricingContext: volSurface handle is empty");
        QL_REQUIRE(!localVol_.empty(),   "FxSLVPricingContext: localVol handle is empty");
    }

    void FxSLVPricingContext::calibrate(
        StochVolCalibrator& hestonCal,
        SLVLeverageCalibrator& leverageCal,
        const std::vector<StochVolCalibrator::Pillar>& pillars,
        const Date& leverageEndDate,
        const std::vector<Date>& mandatoryDates) {

        QL_REQUIRE(!pillars.empty(),
                   "FxSLVPricingContext::calibrate: pillar list must not be empty");

        // Step 1 — Calibrate the stochastic volatility model (e.g. Heston).
        hestonCal.calibrate(spot_, domesticTS_, foreignTS_,
                            volSurface_, calendar_, pillars);

        hestonModel_   = ext::dynamic_pointer_cast<HestonModel>(
                             hestonCal.calibratedModel());
        QL_REQUIRE(hestonModel_,
                   "FxSLVPricingContext: StochVolCalibrator did not return a HestonModel; "
                   "ensure a HestonCalibrator (or subclass) is used with the "
                   "HestonSLVLeverageCalibrator");

        hestonErrors_  = hestonCal.calibrationErrors();
        hestonRmse_    = hestonCal.rmse();

        // Step 2 — Inject the calibrated model into the leverage calibrator, then
        // solve the Fokker-Planck PDE for L(t,S).
        leverageCal.setStochVolModel(hestonModel_);
        leverageCal.calibrate(localVol_, leverageEndDate, mandatoryDates);

        leverageFunction_ = leverageCal.leverageFunction();
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
            leverageFunction_);
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
            leverageFunction_);
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
            leverageFunction_);
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
