/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026

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

#include <ql/models/shortrate/twofactormodels/gsr2.hpp>
#include <ql/quotes/simplequote.hpp>
#include <algorithm>
#include <set>
#include <utility>

using std::exp;

namespace QuantLib {

    // --- Constant correlation constructor ---

    Gsr2::Gsr2(const Handle<YieldTermStructure>& rateTermStructure,
               const Handle<YieldTermStructure>& spreadTermStructure,
               std::vector<Date> rateVolstepdates,
               const std::vector<Real>& rateVolatilities,
               const Real rateReversion,
               std::vector<Date> spreadVolstepdates,
               const std::vector<Real>& spreadVolatilities,
               const Real spreadReversion,
               const Real correlation,
               const Real T)
        : Gaussian2dModel(rateTermStructure, spreadTermStructure, correlation),
          CalibratedModel(2),
          rateSigma_(arguments_[0]),
          rateReversion_(rateReversion),
          rateVolstepdates_(std::move(rateVolstepdates)),
          spreadSigma_(arguments_[1]),
          spreadReversion_(spreadReversion),
          spreadVolstepdates_(std::move(spreadVolstepdates)),
          rhos_(1, correlation),
          T_(T) {

        QL_REQUIRE(!rateTermStructure.empty(), "rate term structure handle is empty");
        QL_REQUIRE(!spreadTermStructure.empty(), "spread term structure handle is empty");
        QL_REQUIRE(correlation >= -1.0 && correlation <= 1.0,
                   "correlation must be in [-1, 1], got " << correlation);

        rateVolsteptimesArray_ = Array(rateVolstepdates_.size());
        spreadVolsteptimesArray_ = Array(spreadVolstepdates_.size());

        updateTimes();

        QL_REQUIRE(rateVolatilities.size() == rateVolsteptimes_.size() + 1,
                   "need n+1 rate volatilities (" << rateVolatilities.size()
                       << ") for n step dates (" << rateVolsteptimes_.size() << ")");
        QL_REQUIRE(spreadVolatilities.size() == spreadVolsteptimes_.size() + 1,
                   "need n+1 spread volatilities (" << spreadVolatilities.size()
                       << ") for n step dates (" << spreadVolsteptimes_.size() << ")");

        rateSigma_ = PiecewiseConstantParameter(rateVolsteptimes_, NoConstraint());
        for (Size i = 0; i < rateSigma_.size(); i++)
            rateSigma_.setParam(i, rateVolatilities[i]);

        spreadSigma_ = PiecewiseConstantParameter(spreadVolsteptimes_, NoConstraint());
        for (Size i = 0; i < spreadSigma_.size(); i++)
            spreadSigma_.setParam(i, spreadVolatilities[i]);

        initialize(T);
    }

    // --- Piecewise-constant correlation constructor ---

    Gsr2::Gsr2(const Handle<YieldTermStructure>& rateTermStructure,
               const Handle<YieldTermStructure>& spreadTermStructure,
               std::vector<Date> rateVolstepdates,
               const std::vector<Real>& rateVolatilities,
               const Real rateReversion,
               std::vector<Date> spreadVolstepdates,
               const std::vector<Real>& spreadVolatilities,
               const Real spreadReversion,
               std::vector<Date> correlationStepdates,
               const std::vector<Real>& correlations,
               const Real T)
        : Gaussian2dModel(rateTermStructure, spreadTermStructure,
                          correlations.empty() ? 0.0 : correlations[0]),
          CalibratedModel(2),
          rateSigma_(arguments_[0]),
          rateReversion_(rateReversion),
          rateVolstepdates_(std::move(rateVolstepdates)),
          spreadSigma_(arguments_[1]),
          spreadReversion_(spreadReversion),
          spreadVolstepdates_(std::move(spreadVolstepdates)),
          rhoStepdates_(std::move(correlationStepdates)),
          rhos_(correlations),
          T_(T) {

        QL_REQUIRE(!rateTermStructure.empty(), "rate term structure handle is empty");
        QL_REQUIRE(!spreadTermStructure.empty(), "spread term structure handle is empty");
        QL_REQUIRE(correlations.size() == rhoStepdates_.size() + 1,
                   "need n+1 correlations (" << correlations.size()
                       << ") for n step dates (" << rhoStepdates_.size() << ")");
        for (Size i = 0; i < correlations.size(); ++i)
            QL_REQUIRE(correlations[i] >= -1.0 && correlations[i] <= 1.0,
                       "correlation[" << i << "] must be in [-1, 1], got " << correlations[i]);

        rateVolsteptimesArray_ = Array(rateVolstepdates_.size());
        spreadVolsteptimesArray_ = Array(spreadVolstepdates_.size());

        updateTimes();

        QL_REQUIRE(rateVolatilities.size() == rateVolsteptimes_.size() + 1,
                   "need n+1 rate volatilities (" << rateVolatilities.size()
                       << ") for n step dates (" << rateVolsteptimes_.size() << ")");
        QL_REQUIRE(spreadVolatilities.size() == spreadVolsteptimes_.size() + 1,
                   "need n+1 spread volatilities (" << spreadVolatilities.size()
                       << ") for n step dates (" << spreadVolsteptimes_.size() << ")");

        rateSigma_ = PiecewiseConstantParameter(rateVolsteptimes_, NoConstraint());
        for (Size i = 0; i < rateSigma_.size(); i++)
            rateSigma_.setParam(i, rateVolatilities[i]);

        spreadSigma_ = PiecewiseConstantParameter(spreadVolsteptimes_, NoConstraint());
        for (Size i = 0; i < spreadSigma_.size(); i++)
            spreadSigma_.setParam(i, spreadVolatilities[i]);

        initialize(T);
    }

    void Gsr2::initialize(Real T) {
        T_ = T;

        rateStateProcess_ = ext::make_shared<GsrProcess>(
            rateVolsteptimesArray_, rateSigma_.params(),
            Array(1, rateReversion_), T);

        spreadStateProcess_ = ext::make_shared<GsrProcess>(
            spreadVolsteptimesArray_, spreadSigma_.params(),
            Array(1, spreadReversion_), T);

        registerWith(termStructure());
        registerWith(spreadTermStructure_);
    }

    void Gsr2::updateTimes() const {
        rateVolsteptimes_.clear();
        for (Size i = 0; i < rateVolstepdates_.size(); ++i) {
            rateVolsteptimes_.push_back(termStructure()->timeFromReference(rateVolstepdates_[i]));
            rateVolsteptimesArray_[i] = rateVolsteptimes_[i];
            if (i == 0)
                QL_REQUIRE(rateVolsteptimes_[0] > 0.0,
                           "rate volsteptimes must be positive (" << rateVolsteptimes_[0] << ")");
            else
                QL_REQUIRE(rateVolsteptimes_[i] > rateVolsteptimes_[i - 1],
                           "rate volsteptimes must be strictly increasing");
        }

        spreadVolsteptimes_.clear();
        for (Size i = 0; i < spreadVolstepdates_.size(); ++i) {
            spreadVolsteptimes_.push_back(
                termStructure()->timeFromReference(spreadVolstepdates_[i]));
            spreadVolsteptimesArray_[i] = spreadVolsteptimes_[i];
            if (i == 0)
                QL_REQUIRE(spreadVolsteptimes_[0] > 0.0,
                           "spread volsteptimes must be positive (" << spreadVolsteptimes_[0] << ")");
            else
                QL_REQUIRE(spreadVolsteptimes_[i] > spreadVolsteptimes_[i - 1],
                           "spread volsteptimes must be strictly increasing");
        }

        rhoSteptimes_.clear();
        for (Size i = 0; i < rhoStepdates_.size(); ++i) {
            rhoSteptimes_.push_back(termStructure()->timeFromReference(rhoStepdates_[i]));
            if (i == 0)
                QL_REQUIRE(rhoSteptimes_[0] > 0.0,
                           "correlation steptimes must be positive (" << rhoSteptimes_[0] << ")");
            else
                QL_REQUIRE(rhoSteptimes_[i] > rhoSteptimes_[i - 1],
                           "correlation steptimes must be strictly increasing");
        }
    }

    void Gsr2::generateArguments() {
        auto rp = ext::static_pointer_cast<GsrProcess>(rateStateProcess_);
        rp->flushCache();
        rp->setVols(rateSigma_.params());

        auto sp = ext::static_pointer_cast<GsrProcess>(spreadStateProcess_);
        sp->flushCache();
        sp->setVols(spreadSigma_.params());

        notifyObservers();
    }

    void Gsr2::update() {
        if (rateStateProcess_ != nullptr) {
            auto rp = ext::static_pointer_cast<GsrProcess>(rateStateProcess_);
            rp->flushCache();
            rp->notifyObservers();
        }
        if (spreadStateProcess_ != nullptr) {
            auto sp = ext::static_pointer_cast<GsrProcess>(spreadStateProcess_);
            sp->flushCache();
            sp->notifyObservers();
        }
        LazyObject::update();
    }

    void Gsr2::performCalculations() const {
        Gaussian2dModel::performCalculations();
        updateTimes();
    }

    Real Gsr2::rhoAt(Time t) const {
        if (rhoSteptimes_.empty())
            return rhos_[0];
        // Piecewise constant: find the interval
        auto it = std::upper_bound(rhoSteptimes_.begin(), rhoSteptimes_.end(), t);
        Size idx = it - rhoSteptimes_.begin();
        return rhos_[idx];
    }

    Real Gsr2::correlation(Time t) const {
        return rhoAt(t);
    }

    Real Gsr2::singleFactorZerobond(Time T, Time t, Real y,
                                     const ext::shared_ptr<GsrProcess>& process,
                                     const Handle<YieldTermStructure>& curve) const {
        calculate();

        if (t == 0.0)
            return curve->discount(T, true);

        Real x = y * process->stdDeviation(0.0, 0.0, t) +
                 process->expectation(0.0, 0.0, t);
        Real gtT = process->G(t, T, x);

        Real d = curve->discount(T, true) / curve->discount(t, true);
        return d * exp(-x * gtT - 0.5 * process->y(t) * gtT * gtT);
    }

    Real Gsr2::crossVarianceInterval(Real a_r, Real a_s, Time T,
                                      Time s0, Time s1) {
        // Closed-form integral:
        //   ∫_{s0}^{s1} B(a_r, T-s) · B(a_s, T-s) ds
        //
        // where B(a, τ) = (1 - e^{-aτ}) / a.
        //
        // Let τ_0 = T - s1, τ_1 = T - s0, Δ = s1 - s0 = τ_1 - τ_0.
        //
        // Result = 1/(a_r·a_s) · [Δ
        //          + (e^{-a_r·τ_0} - e^{-a_r·τ_1}) / a_r
        //          + (e^{-a_s·τ_0} - e^{-a_s·τ_1}) / a_s
        //          - (e^{-(a_r+a_s)·τ_0} - e^{-(a_r+a_s)·τ_1}) / (a_r+a_s)]

        Real tau0 = T - s1;
        Real tau1 = T - s0;
        Real delta = s1 - s0;

        if (delta < QL_EPSILON)
            return 0.0;

        auto expDiff = [](Real a, Real tau0, Real tau1) -> Real {
            if (std::fabs(a) < QL_EPSILON)
                return tau1 - tau0;
            return (exp(-a * tau0) - exp(-a * tau1)) / a;
        };

        return (1.0 / (a_r * a_s)) *
               (delta + expDiff(a_r, tau0, tau1) + expDiff(a_s, tau0, tau1)
                      - expDiff(a_r + a_s, tau0, tau1));
    }

    Real Gsr2::crossVariance(Time t, Time T) const {
        // Cross-covariance:
        //   C(t,T) = ∫_t^T ρ(s) · σ_r(s) · σ_s(s) · B(a_r, T-s) · B(a_s, T-s) ds
        //
        // We break [t, T] into sub-intervals where ρ, σ_r, σ_s are all
        // constant, then use the closed-form integral for each piece.

        Real tau = T - t;
        if (tau < QL_EPSILON)
            return 0.0;

        auto rp = ext::static_pointer_cast<GsrProcess>(rateStateProcess_);
        auto sp = ext::static_pointer_cast<GsrProcess>(spreadStateProcess_);

        Real a_r = rp->reversion(t);
        Real a_s = sp->reversion(t);

        // Build sorted set of breakpoints in [t, T] from all step dates
        std::set<Time> breaks;
        breaks.insert(t);
        breaks.insert(T);
        for (auto s : rateVolsteptimes_)
            if (s > t && s < T) breaks.insert(s);
        for (auto s : spreadVolsteptimes_)
            if (s > t && s < T) breaks.insert(s);
        for (auto s : rhoSteptimes_)
            if (s > t && s < T) breaks.insert(s);

        std::vector<Time> bv(breaks.begin(), breaks.end());

        Real result = 0.0;
        for (Size i = 0; i + 1 < bv.size(); ++i) {
            Time s0 = bv[i];
            Time s1 = bv[i + 1];
            Real rho_i = rhoAt(s0);
            Real sigma_r_i = rp->sigma(s0);
            Real sigma_s_i = sp->sigma(s0);

            if (std::fabs(rho_i) < QL_EPSILON)
                continue;

            result += rho_i * sigma_r_i * sigma_s_i *
                      crossVarianceInterval(a_r, a_s, T, s0, s1);
        }

        return result;
    }

    Real Gsr2::discountZerobondImpl(Time T, Time t, Real yRate, Real ySpread,
                                     const Handle<YieldTermStructure>& /*yts*/) const {
        auto rp = ext::static_pointer_cast<GsrProcess>(rateStateProcess_);
        auto sp = ext::static_pointer_cast<GsrProcess>(spreadStateProcess_);

        Real zbRate = singleFactorZerobond(T, t, yRate, rp, termStructure());
        Real zbSpread = singleFactorZerobond(T, t, ySpread, sp, spreadTermStructure_);
        Real cross = crossVariance(t, T);

        return zbRate * zbSpread * exp(cross);
    }

    Real Gsr2::forecastZerobondImpl(Time T, Time t, Real yRate,
                                     const Handle<YieldTermStructure>& yts) const {
        auto rp = ext::static_pointer_cast<GsrProcess>(rateStateProcess_);
        const Handle<YieldTermStructure>& curve = yts.empty() ? termStructure() : yts;
        return singleFactorZerobond(T, t, yRate, rp, curve);
    }

    Real Gsr2::numeraireImpl(Time t, Real yRate, Real ySpread,
                              const Handle<YieldTermStructure>& /*yts*/) const {
        calculate();

        if (t == 0.0) {
            auto rp = ext::static_pointer_cast<GsrProcess>(rateStateProcess_);
            Real T_fwd = rp->getForwardMeasureTime();
            return termStructure()->discount(T_fwd, true) *
                   spreadTermStructure_->discount(T_fwd, true) *
                   exp(crossVariance(0.0, T_fwd));
        }

        auto rp = ext::static_pointer_cast<GsrProcess>(rateStateProcess_);
        Real T_fwd = rp->getForwardMeasureTime();
        return discountZerobond(T_fwd, t, yRate, ySpread);
    }
}
