/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Jack Gillett
 
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

#include <ql/experimental/forward/analytichestonforwardeuropeanengine.hpp>
#include <complex>
#include <utility>

namespace QuantLib {


    class P12Integrand {
      private:
        ext::shared_ptr<AnalyticHestonEngine>& engine_;
        Real logK_, phiRightLimit_;
        Time tenor_;
        std::complex<Real> i_, adj_;
      public:
        P12Integrand(ext::shared_ptr<AnalyticHestonEngine>& engine,
                     Real logK,
                     Time tenor,
                     bool P1, // true for P1, false for P2
                     Real phiRightLimit = 100) : engine_(engine), logK_(logK),
            phiRightLimit_(phiRightLimit), tenor_(tenor), i_(std::complex<Real>(0.0, 1.0)) {

            // Only difference between P1 and P2 integral is the additional term in the chF evaluation
            if (P1) {
                adj_ = std::complex<Real>(0.0, -1.0);
            } else {
                adj_ = std::complex<Real>(0.0, 0.0);
            }
        }

        // QL Gaussian Quadrature - map phi from [-1 to 1] to {0, phiRightLimit] 
        Real operator()(Real phi) const {
            Real phiDash = (0.5+1e-8+0.5*phi) * phiRightLimit_; // Map phi to full range
            return 0.5*phiRightLimit_*std::real((std::exp(-phiDash*logK_*i_) / (phiDash*i_)) * engine_->chF(phiDash+adj_, tenor_));
        }
    };


    class P12HatIntegrand {
      private:
        Time tenor_, resetTime_;
        Handle<Quote>& s0_;
        bool P1_;
        Real logK_, phiRightLimit_, nuRightLimit_;
        const AnalyticHestonForwardEuropeanEngine* const parent_;
        GaussLegendreIntegration innerIntegrator_;
      public:
        P12HatIntegrand(Time tenor,
                        Time resetTime,
                        Handle<Quote>& s0,
                        Real logK,
                        bool P1, // true for P1, false for P2
                        const AnalyticHestonForwardEuropeanEngine* const parent,
                        Real phiRightLimit,
                        Real nuRightLimit) : tenor_(tenor), resetTime_(resetTime),
            s0_(s0), P1_(P1), logK_(logK), phiRightLimit_(phiRightLimit),
            nuRightLimit_(nuRightLimit), parent_(parent), innerIntegrator_(128) {}
        Real operator()(Real nu) const {

            // Rescale nu to [-1, 1]
            Real nuDash = nuRightLimit_ * (0.5 * nu + 0.5 + 1e-8);

            // Calculate the chF from var(t) to expiry
            ext::shared_ptr<AnalyticHestonEngine> engine = parent_->forwardChF(s0_, nuDash);
            P12Integrand pIntegrand(engine, logK_, tenor_, P1_, phiRightLimit_);
            Real p1Integral = innerIntegrator_(pIntegrand);

            // Calculate the value of the propagator to nu
            Real propagator = parent_->propagator(resetTime_, nuDash);

            // Take the product, and scale integral's value back up to [0, right_lim]
            return propagator * (0.5 + p1Integral/M_PI);
        }
    };


    AnalyticHestonForwardEuropeanEngine::AnalyticHestonForwardEuropeanEngine(
        ext::shared_ptr<HestonProcess> process, Size integrationOrder)
    : process_(std::move(process)), integrationOrder_(integrationOrder), outerIntegrator_(128) {

        v0_ = process_->v0();
        rho_ = process_->rho();
        kappa_ = process_->kappa();
        theta_ = process_->theta();
        sigma_ = process_->sigma();
        s0_ = process_->s0();

        QL_REQUIRE(sigma_ > 0.1,
                   "Very low values (<~10%) for Heston Vol-of-Vol cause numerical issues" \
                   "in this implementation of the propagator function, try using" \
                   "MCForwardEuropeanHestonEngine Monte-Carlo engine instead");

        riskFreeRate_ = process_->riskFreeRate();
        dividendYield_ = process_->dividendYield();

        // Some of the required constant intermediate variables can be calculated now
        kappaHat_ = kappa_ - rho_ * sigma_;
        thetaHat_ = kappa_ * theta_ / kappaHat_;
        R_ = 4 * kappaHat_ * thetaHat_ / (sigma_ * sigma_);
    }


    void AnalyticHestonForwardEuropeanEngine::calculate() const {
        // This is a european option pricer
        QL_REQUIRE(this->arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        // We only price plain vanillas
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(this->arguments_.payoff);
        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        Time resetTime = this->process_->time(this->arguments_.resetDate);
        Time expiryTime = this->process_->time(this->arguments_.exercise->lastDate());
        Time tenor = expiryTime - resetTime;
        Real moneyness = this->arguments_.moneyness;

        // K needs to be scaled to forward AT RESET TIME, not spot...
        Real expiryDcf = riskFreeRate_->discount(expiryTime);
        Real resetDcf = riskFreeRate_->discount(resetTime);
        Real expiryDividendDiscount = dividendYield_->discount(expiryTime);
        Real resetDividendDiscount = dividendYield_->discount(resetTime);
        Real expiryRatio = expiryDcf / expiryDividendDiscount;
        Real resetRatio = resetDcf / resetDividendDiscount;

        QL_REQUIRE(resetTime >= 0.0, "Reset Date cannot be in the past");
        QL_REQUIRE(expiryTime >= 0.0, "Expiry Date cannot be in the past");

        // Use some heuristics to decide upon phiRightLimit and nuRightLimit
        Real phiRightLimit = 100.0;
        Real nuRightLimit = std::max(2.0, 10.0 * (1+std::max(0.0, rho_)) * sigma_ * std::sqrt(resetTime * std::max(v0_, theta_)));

        // do the 2D integral calculation. For very short times, we just fall back on the standard
        // calculation, both for accuracy and because tStar==0 causes some numerical issues...
        std::pair<Real, Real> P1HatP2Hat;
        if (resetTime <= 1e-3) {
            Handle<Quote> tempQuote(ext::shared_ptr<Quote>(new SimpleQuote(s0_->value())));
            P1HatP2Hat = calculateP1P2(tenor, tempQuote, moneyness * s0_->value(), expiryRatio, phiRightLimit);
        } else {
            P1HatP2Hat = calculateP1P2Hat(tenor, resetTime, moneyness, expiryRatio/resetRatio, phiRightLimit, nuRightLimit);
        }

        // Apply the payoff functions
        Real value = 0.0;
        Real F = s0_->value() / expiryRatio;
        switch (payoff->optionType()){
            case Option::Call:
                value = expiryDcf * (F*P1HatP2Hat.first - moneyness*s0_->value()*P1HatP2Hat.second/resetRatio);
                break;
            case Option::Put:
                value = expiryDcf * (moneyness*s0_->value()*(1-P1HatP2Hat.second)/resetRatio - F*(1-P1HatP2Hat.first));
                break;
            default:
                QL_FAIL("unknown option type");
            }

        results_.value = value;

        results_.additionalResults["dcf"] = expiryDcf;
        results_.additionalResults["qf"] = expiryDividendDiscount;
        results_.additionalResults["expiryRatio"] = expiryRatio;
        results_.additionalResults["resetRatio"] = resetRatio;
        results_.additionalResults["moneyness"] = moneyness;
        results_.additionalResults["s0"] = s0_->value();
        results_.additionalResults["fwd"] = F;
        results_.additionalResults["resetTime"] = resetTime;
        results_.additionalResults["expiryTime"] = expiryTime;
        results_.additionalResults["P1Hat"] = P1HatP2Hat.first;
        results_.additionalResults["P2Hat"] = P1HatP2Hat.second;
        results_.additionalResults["phiRightLimit"] = phiRightLimit;
        results_.additionalResults["nuRightLimit"] = nuRightLimit;
    }


    std::pair<Real, Real> AnalyticHestonForwardEuropeanEngine::calculateP1P2Hat(Time tenor,
                                                                                Time resetTime,
                                                                                Real moneyness,
                                                                                Real ratio,
                                                                                Real phiRightLimit,
                                                                                Real nuRightLimit) const {

        Handle<Quote> unitQuote(ext::shared_ptr<Quote>(new SimpleQuote(1.0)));

        // Re-expressing moneyness in terms of the forward here (strike fixes to spot, but in
        // our pricing calculation we need to compare it to the future at expiry)
        Real logMoneyness = std::log(moneyness*ratio);

        P12HatIntegrand p1HatIntegrand(tenor, resetTime, unitQuote, logMoneyness, true, this, phiRightLimit, nuRightLimit);
        P12HatIntegrand p2HatIntegrand(tenor, resetTime, unitQuote, logMoneyness, false, this, phiRightLimit, nuRightLimit);

        Real p1HatIntegral = 0.5 * nuRightLimit * outerIntegrator_(p1HatIntegrand);
        Real p2HatIntegral = 0.5 * nuRightLimit * outerIntegrator_(p2HatIntegrand);

        std::pair<Real, Real> P1HatP2Hat(p1HatIntegral, p2HatIntegral);

        return P1HatP2Hat;
    }


    Real AnalyticHestonForwardEuropeanEngine::propagator(Time resetTime,
                                                         Real varReset) const {
        Real B, Lambda, term1, term2, term3;

        B = 4 * kappaHat_ / (sigma_ * sigma_ * (1 - std::exp(-kappaHat_ * resetTime)));
        Lambda = B * std::exp(-kappaHat_ * resetTime) * v0_;

        // Now construct equation (18) from the paper term-by-term
        term1 = std::exp(-0.5*(B * varReset + Lambda)) * B / 2;
        term2 = std::pow(B * varReset / Lambda, 0.5*(R_/2 - 1));
        term3 = modifiedBesselFunction_i(Real(R_/2 - 1),Real(std::sqrt(Lambda * B * varReset)));

        return term1 * term2 * term3;
    }

    ext::shared_ptr<AnalyticHestonEngine> AnalyticHestonForwardEuropeanEngine::forwardChF(
                                      Handle<Quote>& spotReset,
                                      Real varReset) const {

        // Probably a wasteful implementation here, could be improved by importing
        // only the CF-generating parts of the AnalyticHestonEngine (currently private)
        ext::shared_ptr<HestonProcess> hestonProcess(new
            HestonProcess(riskFreeRate_, dividendYield_, spotReset,
                varReset, kappa_, theta_, sigma_, rho_));

        ext::shared_ptr<HestonModel> hestonModel(new HestonModel(hestonProcess));

        ext::shared_ptr<AnalyticHestonEngine> analyticHestonEngine(
            new AnalyticHestonEngine(hestonModel, integrationOrder_));

        // Not sure how to pass only the chF, so just pass the whole thing for now!
        return analyticHestonEngine;
    }


    std::pair<Real, Real> AnalyticHestonForwardEuropeanEngine::calculateP1P2(Time tenor,
                                                                             Handle<Quote>& St,
                                                                             Real K,
                                                                             Real ratio,
                                                                             Real phiRightLimit) const {

        ext::shared_ptr<AnalyticHestonEngine> engine = forwardChF(St, v0_);
        Real logK = std::log(K*ratio/St->value());

        // Integrate the CF and the complex integrand over positive phi
        GaussLegendreIntegration integrator = GaussLegendreIntegration(128);
        P12Integrand p1Integrand(engine, logK, tenor, true, phiRightLimit);
        P12Integrand p2Integrand(engine, logK, tenor, false, phiRightLimit);

        Real p1Integral = integrator(p1Integrand);
        Real p2Integral = integrator(p2Integrand);

        std::pair<Real, Real> P1P2(0.5 + p1Integral/M_PI, 0.5 + p2Integral/M_PI);

        return P1P2;
    }
}
