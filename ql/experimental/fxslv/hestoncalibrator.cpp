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

#include <ql/experimental/fxslv/hestoncalibrator.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>
#include <limits>
#include <utility>

namespace QuantLib {

    HestonCalibrator::HestonCalibrator(
        HestonParams initialParams,
        EndCriteria endCriteria,
        BlackCalibrationHelper::CalibrationErrorType errorType,
        std::vector<bool> fixParameters,
        bool fellerConstraint)
    : initialParams_(initialParams), endCriteria_(endCriteria),
      errorType_(errorType), fixParameters_(std::move(fixParameters)),
      fellerConstraint_(fellerConstraint) {}

    void HestonCalibrator::calibrate(const Handle<Quote>& spot,
                                     const Handle<YieldTermStructure>& domesticTS,
                                     const Handle<YieldTermStructure>& foreignTS,
                                     const Handle<BlackVolTermStructure>& volSurface,
                                     const Calendar& calendar,
                                     const std::vector<Pillar>& pillars) {
        QL_REQUIRE(!pillars.empty(), "HestonCalibrator: at least one calibration pillar required");

        // Build the Heston process and model with initial parameters.
        const Date refDate = domesticTS->referenceDate();

        auto process = ext::make_shared<HestonProcess>(
            domesticTS, foreignTS, spot,
            initialParams_.v0, initialParams_.kappa,
            initialParams_.theta, initialParams_.sigma, initialParams_.rho);

        hestonModel_ = ext::make_shared<HestonModel>(process);

        // Levenberg-Marquardt penalises every trial violating the constraints
        // alike, so it can never leave a start violating them: reject it.
        const Array start = hestonModel_->params();
        QL_REQUIRE(hestonModel_->constraint()->test(start),
                   "HestonCalibrator: initial parameters violate the model's constraints: "
                       << "v0=" << initialParams_.v0 << " kappa=" << initialParams_.kappa
                       << " theta=" << initialParams_.theta << " sigma=" << initialParams_.sigma
                       << " rho=" << initialParams_.rho);
        const Constraint constraint =
            fellerConstraint_ ? Constraint(HestonModel::FellerConstraint()) : Constraint();
        QL_REQUIRE(!fellerConstraint_ || constraint.test(start),
                   "HestonCalibrator: initial parameters violate the Feller condition: "
                       << "2*kappa*theta=" << 2.0 * initialParams_.kappa * initialParams_.theta
                       << " sigma^2=" << initialParams_.sigma * initialParams_.sigma);

        // Attach an analytic engine to the model for fast calibration.
        auto engine = ext::make_shared<AnalyticHestonEngine>(hestonModel_);

        // Build one HestonModelHelper per pillar.
        helpers_.clear();
        helpers_.reserve(pillars.size());

        for (const auto& p : pillars) {
            // Query the market vol surface for the calibration vol at this pillar.
            const Date expiryDate = calendar.advance(refDate, p.expiry);
            const Volatility marketVol =
                volSurface->blackVol(expiryDate, p.strike, true);

            auto volQuote = ext::make_shared<SimpleQuote>(marketVol);

            auto helper = ext::make_shared<HestonModelHelper>(
                p.expiry, calendar,
                spot, p.strike,
                Handle<Quote>(volQuote),
                domesticTS, foreignTS,
                errorType_);

            helper->setPricingEngine(engine);
            helpers_.push_back(helper);
        }

        // Calibrate using Levenberg-Marquardt, with the Feller condition if requested.
        LevenbergMarquardt optimiser;
        hestonModel_->calibrate(
            std::vector<ext::shared_ptr<CalibrationHelper>>(helpers_.begin(),
                                                            helpers_.end()),
            optimiser,
            endCriteria_,
            constraint,
            {},
            fixParameters_);

        const EndCriteria::Type ending = hestonModel_->endCriteria();
        QL_REQUIRE(ending != EndCriteria::MaxIterations && ending != EndCriteria::None,
                   "HestonCalibrator: calibration did not converge (" << ending << ")");
        // a fit stuck at its start, e.g. with every trial around it giving
        // non-finite prices, ends like a converged one
        const Array end = hestonModel_->params();
        bool moved = false;
        for (Size i = 0; i < start.size(); ++i)
            moved = moved || end[i] != start[i];
        QL_REQUIRE(moved || fixParameters_ == std::vector<bool>(start.size(), true),
                   "HestonCalibrator: calibration never left its initial parameters");

        // Collect per-pillar implied-vol errors.
        calibrationErrors_.clear();
        calibrationErrors_.reserve(helpers_.size());
        for (const auto& h : helpers_) {
            // Signed error: model implied vol minus market implied vol, or
            // NaN for a degenerate model value (e.g. a near-zero price) whose
            // implied vol root search fails.
            const Real mktVol = h->volatility()->value();
            Real modelVol = std::numeric_limits<Real>::quiet_NaN();
            try {
                modelVol = h->impliedVolatility(h->modelValue(),
                                                1.0e-4, 1000, 0.001, 10.0);
            } catch (const Error&) {
                // no implied vol: the error stays NaN
            }
            calibrationErrors_.push_back(modelVol - mktVol);
        }

        // Build the implied vol surface backed by the calibrated model.
        impliedVolSurface_ =
            ext::make_shared<HestonBlackVolSurface>(Handle<HestonModel>(hestonModel_));
    }

    ext::shared_ptr<CalibratedModel> HestonCalibrator::calibratedModel() const {
        QL_REQUIRE(hestonModel_, "HestonCalibrator: calibrate() has not been called");
        return hestonModel_;
    }

    std::vector<Real> HestonCalibrator::calibrationErrors() const {
        return calibrationErrors_;
    }

    ext::shared_ptr<BlackVolTermStructure> HestonCalibrator::impliedVolSurface() const {
        QL_REQUIRE(impliedVolSurface_,
                   "HestonCalibrator: calibrate() has not been called");
        return impliedVolSurface_;
    }

    ext::shared_ptr<HestonModel> HestonCalibrator::calibratedHestonModel() const {
        QL_REQUIRE(hestonModel_, "HestonCalibrator: calibrate() has not been called");
        return hestonModel_;
    }

    const std::vector<ext::shared_ptr<BlackCalibrationHelper>>&
    HestonCalibrator::helpers() const {
        return helpers_;
    }

    Real StochVolCalibrator::rmse() const {
        const std::vector<Real> errs = calibrationErrors();
        QL_REQUIRE(!errs.empty(),
                   "StochVolCalibrator: no calibration errors available");
        Real sumSq = 0.0;
        for (Real e : errs)
            sumSq += e * e;
        return std::sqrt(sumSq / static_cast<Real>(errs.size()));
    }

}
