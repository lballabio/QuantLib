/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Raghav Subbarao

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

#include <ql/exercise.hpp>
#include <ql/experimental/fxslv/fxvanillagreeks.hpp>
#include <ql/exercise.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/settings.hpp>
#include <ql/quotes/simplequote.hpp>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace QuantLib {

    namespace {
        // Reference tenor for rega/sega time-scaling: 1 month.
        const Real T_1M = 1.0 / 12.0;
    }

    FxVanillaBumpRisk::FxVanillaBumpRisk(ext::shared_ptr<VanillaOption> option,
                                         ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                                         ext::shared_ptr<SimpleQuote> spotQuote,
                                         std::vector<ext::shared_ptr<SimpleQuote>> atmVolQuotes,
                                         std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> rrPillarQuotes,
                                         std::vector<Time> rrPillarTimes,
                                         std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> bfPillarQuotes,
                                         std::vector<Time> bfPillarTimes,
                                         Real notional, Real spotBump, Real volBump,
                                         Real rrBump, Real bfBump, Size tGrid,
                                         Size xGrid, Size lvTimePts, Size lvStrikePts, Real lvStrikeSpread)
    : option_(std::move(option)), process_(std::move(process)),
      spotQuote_(std::move(spotQuote)), atmVolQuotes_(std::move(atmVolQuotes)),
      rrPillarQuotes_(std::move(rrPillarQuotes)), rrPillarTimes_(std::move(rrPillarTimes)),
      bfPillarQuotes_(std::move(bfPillarQuotes)), bfPillarTimes_(std::move(bfPillarTimes)),
      notional_(notional), spotBump_(spotBump), volBump_(volBump),
      rrBump_(rrBump), bfBump_(bfBump), tGrid_(tGrid), xGrid_(xGrid),
      lvTimePts_(lvTimePts), lvStrikePts_(lvStrikePts), lvStrikeSpread_(lvStrikeSpread) 
    {
        QL_REQUIRE(option_ != nullptr, "null option");
        QL_REQUIRE(process_ != nullptr, "null process");
        QL_REQUIRE(spotQuote_ != nullptr, "null spot quote");
        QL_REQUIRE(!atmVolQuotes_.empty(), "at least one ATM vol quote required");
        QL_REQUIRE(rrPillarQuotes_.size() == rrPillarTimes_.size(),
                   "rrPillarQuotes and rrPillarTimes must have the same size");
        QL_REQUIRE(bfPillarQuotes_.size() == bfPillarTimes_.size(),
                   "bfPillarQuotes and bfPillarTimes must have the same size");
        for (Time t : rrPillarTimes_)
            QL_REQUIRE(t > 0.0, "rrPillarTimes must be positive");
        for (Time t : bfPillarTimes_)
            QL_REQUIRE(t > 0.0, "bfPillarTimes must be positive");
        QL_REQUIRE(lvTimePts_ >= 2, "lvTimePts must be at least 2");
        QL_REQUIRE(lvStrikePts_ >= 2, "lvStrikePts must be at least 2");
        QL_REQUIRE(lvStrikeSpread_ > 0.0, "lvStrikeSpread must be positive");

        buildLocalVol(Null<Volatility>());
    }

    void FxVanillaBumpRisk::setLocalVolFallback(Volatility fallbackVol) {
        buildLocalVol(fallbackVol);
    }

    void FxVanillaBumpRisk::buildLocalVol(Volatility fallbackVol) {
        localVol_ = ext::make_shared<FxLocalVolSurface>(
            process_->blackVolatility(), process_->riskFreeRate(), process_->dividendYield(),
            process_->stateVariable(), 1.0e-7, 0.0, fallbackVol);
        localVol_->enableExtrapolation();
        // the engines take the local vol from the process; give it ours
        localVolProcess_ = ext::make_shared<GeneralizedBlackScholesProcess>(
            process_->stateVariable(), process_->dividendYield(), process_->riskFreeRate(),
            process_->blackVolatility(), Handle<LocalVolTermStructure>(localVol_));
    }

    ext::shared_ptr<PricingEngine> FxVanillaBumpRisk::makeEngineFor(const ext::shared_ptr<GeneralizedBlackScholesProcess>& proc,
                                                                    bool localVol) const 
    {
        // no overwrite of failed local vols: a failure is an error, not a
        // number to substitute
        return ext::make_shared<FdBlackScholesVanillaEngine>(proc, tGrid_, xGrid_,
                                                             0, /*dampingSteps=*/
                                                             FdmSchemeDesc::Douglas(),
                                                             localVol,
                                                             -Null<Real>() /*illegalLocalVolOverwrite*/
                                                             );
    }

    ext::shared_ptr<PricingEngine> FxVanillaBumpRisk::makeEngine(bool localVol) const {
        return makeEngineFor(localVol ? localVolProcess_ : process_, localVol);
    }


    ext::shared_ptr<PricingEngine> FxVanillaBumpRisk::makeAnalyticEngine() const {
        return ext::make_shared<AnalyticEuropeanEngine>(process_);
    }

    std::pair<std::vector<Time>, std::vector<Real>> FxVanillaBumpRisk::buildLvGrid() const {
        const Date expiry = option_->exercise()->lastDate();
        const Time T = process_->time(expiry);
        QL_REQUIRE(T > 0.0, "option has expired; cannot build local vol grid");

        Real avgAtm = 0.0;
        for (const auto& q : atmVolQuotes_)
            avgAtm += q->value();
        avgAtm /= static_cast<Real>(atmVolQuotes_.size());

        std::vector<Time> times(lvTimePts_);
        for (Size i = 0; i < lvTimePts_; ++i)
            times[i] = T * (i + 1.0) / static_cast<Real>(lvTimePts_);

        const Real S = spotQuote_->value();
        const Real halfWidth = lvStrikeSpread_ * avgAtm * std::sqrt(T);
        std::vector<Real> strikes(lvStrikePts_);
        for (Size j = 0; j < lvStrikePts_; ++j) {
            const Real u = 2.0 * j / static_cast<Real>(lvStrikePts_ - 1) - 1.0;
            strikes[j] = S * std::exp(halfWidth * u);
        }

        return {times, strikes};
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess> FxVanillaBumpRisk::makeStickyProcess(const std::vector<Time>& times,
                                                                                         const std::vector<Real>& strikes) const 
    {
        auto fixedLV = buildFixedLocalVolSurface(times, strikes);
        fixedLV->enableExtrapolation();
        return ext::make_shared<GeneralizedBlackScholesProcess>(Handle<Quote>(spotQuote_),
                                                                process_->dividendYield(),
                                                                process_->riskFreeRate(),
                                                                process_->blackVolatility(),
                                                                Handle<LocalVolTermStructure>(fixedLV));
    }

    FxVanillaGreeks FxVanillaBumpRisk::calculate(bool localVol, StickyType sticky) const {
        const Real S  = spotQuote_->value();
        const Real ds = S * spotBump_;
        const Real dv = volBump_;

        // ── Discount factors and forward for delta/gamma scaling ──────────────
        const Date expiry = option_->exercise()->lastDate();
        const Time T_exp  = process_->time(expiry);
        const Real Bd     = process_->riskFreeRate()->discount(T_exp);
        const Real Bf     = process_->dividendYield()->discount(T_exp);

        // ── Capture base values for all bumped quotes ─────────────────────────
        std::vector<Real> sigmas(atmVolQuotes_.size());
        for (Size i = 0; i < atmVolQuotes_.size(); ++i)
            sigmas[i] = atmVolQuotes_[i]->value();

        std::vector<std::vector<Real>> rrBase(rrPillarQuotes_.size());
        for (Size i = 0; i < rrPillarQuotes_.size(); ++i) {
            rrBase[i].resize(rrPillarQuotes_[i].size());
            for (Size j = 0; j < rrPillarQuotes_[i].size(); ++j)
                rrBase[i][j] = rrPillarQuotes_[i][j]->value();
        }

        std::vector<std::vector<Real>> bfBase(bfPillarQuotes_.size());
        for (Size i = 0; i < bfPillarQuotes_.size(); ++i) {
            bfBase[i].resize(bfPillarQuotes_[i].size());
            for (Size j = 0; j < bfPillarQuotes_[i].size(); ++j)
                bfBase[i][j] = bfPillarQuotes_[i][j]->value();
        }

        // ── Helper lambdas ────────────────────────────────────────────────────
        auto shiftAtms = [&](Real delta) {
            for (Size i = 0; i < atmVolQuotes_.size(); ++i)
                atmVolQuotes_[i]->setValue(sigmas[i] + delta);
        };
        auto restoreAtms = [&]() {
            for (Size i = 0; i < atmVolQuotes_.size(); ++i)
                atmVolQuotes_[i]->setValue(sigmas[i]);
        };

        // Each RR pillar is bumped by rrBump_ * sqrt(T_1M / T_pillar).
        // This normalises so the 1M pillar moves by rrBump_ and longer
        // tenors move by progressively smaller amounts (∝ 1/sqrt(T)).
        auto shiftRRs = [&](Real sign) {
            for (Size i = 0; i < rrPillarQuotes_.size(); ++i) {
                const Real scaledBump = sign * rrBump_ * std::sqrt(T_1M / rrPillarTimes_[i]);
                for (Size j = 0; j < rrPillarQuotes_[i].size(); ++j)
                    rrPillarQuotes_[i][j]->setValue(rrBase[i][j] + scaledBump);
            }
        };
        auto restoreRRs = [&]() {
            for (Size i = 0; i < rrPillarQuotes_.size(); ++i)
                for (Size j = 0; j < rrPillarQuotes_[i].size(); ++j)
                    rrPillarQuotes_[i][j]->setValue(rrBase[i][j]);
        };

        auto shiftBFs = [&](Real sign) {
            for (Size i = 0; i < bfPillarQuotes_.size(); ++i) {
                const Real scaledBump = sign * bfBump_ * std::sqrt(T_1M / bfPillarTimes_[i]);
                for (Size j = 0; j < bfPillarQuotes_[i].size(); ++j)
                    bfPillarQuotes_[i][j]->setValue(bfBase[i][j] + scaledBump);
            }
        };
        auto restoreBFs = [&]() {
            for (Size i = 0; i < bfPillarQuotes_.size(); ++i)
                for (Size j = 0; j < bfPillarQuotes_[i].size(); ++j)
                    bfPillarQuotes_[i][j]->setValue(bfBase[i][j]);
        };

        // Compute rega and sega using the current engine (assumed non-frozen).
        // Returns {rega, sega}.  If no RR/BF quotes are provided, returns {0,0}.
        auto computeRegaSega = [&]() -> std::pair<Real, Real> {
            Real rega = 0.0, sega = 0.0;
            if (!rrPillarQuotes_.empty()) {
                shiftRRs(+1.0);
                const Real V_rrp = option_->NPV() * notional_;
                shiftRRs(-1.0);
                const Real V_rrm = option_->NPV() * notional_;
                restoreRRs();
                rega = (V_rrp - V_rrm) / 2.0;
            }
            if (!bfPillarQuotes_.empty()) {
                shiftBFs(+1.0);
                const Real V_bfp = option_->NPV() * notional_;
                shiftBFs(-1.0);
                const Real V_bfm = option_->NPV() * notional_;
                restoreBFs();
                sega = (V_bfp - V_bfm) / 2.0;
            }
            return {rega, sega};
        };

        // ── Scaling helpers ───────────────────────────────────────────────────
        // spotDelta = dV/dS * 0.01*S      (PV per 1% spot)
        // fwdDelta  = d(V/Bd)/dF * 0.01*F = spotDelta / Bd   (fwd-PV per 1% fwd)
        // spotGamma = d²V/dS² * (0.01*S)² (spot-delta change per 1% spot)
        // fwdGamma  = spotGamma / Bd
        // vega      = dV/dσ * 0.01        (PV per 1% vol)
        // vanna     = d²V/(dS dσ) * 0.01*S * 0.01  (spot-delta change per 1% vol)
        // volga     = d²V/dσ² * 0.01²    (vega change per 1% vol)
        const Real pct1S = 0.01 * S;

        auto scaleResults = [&](Real V0, Real Vup, Real Vdn,
                                Real Vvp, Real Vvm,
                                Real Vpp, Real Vpm, Real Vmp, Real Vmm,
                                Real theta_raw, Real rega, Real sega) {
            const Real spotDelta_raw = (Vup - Vdn) / (2.0 * ds);
            const Real spotGamma_raw = (Vup - 2.0 * V0 + Vdn) / (ds * ds);
            const Real vega_raw      = (Vvp - Vvm) / (2.0 * dv);
            const Real volga_raw     = (Vvp - 2.0 * V0 + Vvm) / (dv * dv);
            const Real vanna_raw     = (Vpp - Vpm - Vmp + Vmm) / (4.0 * ds * dv);

            FxVanillaGreeks r;
            r.npv       = V0;
            r.spotDelta = spotDelta_raw * pct1S;
            r.fwdDelta  = r.spotDelta / Bd;
            r.spotGamma = spotGamma_raw * pct1S * pct1S;
            r.fwdGamma  = r.spotGamma / Bd;
            r.theta     = theta_raw;
            r.vega      = vega_raw * 0.01;
            r.vanna     = vanna_raw * pct1S * 0.01;
            r.navva     = r.vanna;   // same by symmetry of mixed partials
            r.volga     = volga_raw * 0.01 * 0.01;
            r.rega      = rega;
            r.sega      = sega;
            return r;
        };

        if (!localVol || sticky == StickyType::Delta) {
            // ── BS analytic path (localVol=false) or sticky-delta local vol ──
            // localVol=false: analytical Black-Scholes, using the implied vol
            // interpolated from the FX variance surface at the option's (T, K).
            // localVol=true + sticky-delta: FD engine with Dupire local vol.
            if (!localVol)
                option_->setPricingEngine(makeAnalyticEngine());
            else
                option_->setPricingEngine(makeEngine(true));

            const Real V0  = option_->NPV() * notional_;

            spotQuote_->setValue(S + ds);
            const Real Vup = option_->NPV() * notional_;
            spotQuote_->setValue(S - ds);
            const Real Vdn = option_->NPV() * notional_;
            spotQuote_->setValue(S);

            shiftAtms(+dv);
            const Real Vvp = option_->NPV() * notional_;
            shiftAtms(-2.0 * dv);
            const Real Vvm = option_->NPV() * notional_;
            restoreAtms();

            const Date today = Settings::instance().evaluationDate();
            Settings::instance().evaluationDate() = today + 1;
            const Real Vnext = option_->NPV() * notional_;
            Settings::instance().evaluationDate() = today;
            const Real theta_raw = Vnext - V0;

            spotQuote_->setValue(S + ds);
            shiftAtms(+dv);
            const Real Vpp = option_->NPV() * notional_;
            shiftAtms(-2.0 * dv);
            const Real Vpm = option_->NPV() * notional_;
            restoreAtms();

            spotQuote_->setValue(S - ds);
            shiftAtms(+dv);
            const Real Vmp = option_->NPV() * notional_;
            shiftAtms(-2.0 * dv);
            const Real Vmm = option_->NPV() * notional_;
            restoreAtms();

            spotQuote_->setValue(S);

            auto [rega, sega] = computeRegaSega();

            return scaleResults(V0, Vup, Vdn, Vvp, Vvm, Vpp, Vpm, Vmp, Vmm,
                                theta_raw, rega, sega);

        } 
        else 
        {
            // ── Sticky-strike path (localVol=true only) ───────────────────────
            //
            // Build three frozen local vol surfaces at base, vol+dv, vol-dv.
            auto [times, strikes] = buildLvGrid();

            auto stickyBase = makeStickyProcess(times, strikes);

            shiftAtms(+dv);
            auto stickyVp = makeStickyProcess(times, strikes);

            shiftAtms(-2.0 * dv);
            auto stickyVm = makeStickyProcess(times, strikes);

            restoreAtms();

            auto engBase = makeEngineFor(stickyBase, true);
            auto engVp   = makeEngineFor(stickyVp,   true);
            auto engVm   = makeEngineFor(stickyVm,   true);

            // Base NPV and spot delta/gamma (frozen local vol, spot bumped).
            option_->setPricingEngine(engBase);
            const Real V0  = option_->NPV() * notional_;

            spotQuote_->setValue(S + ds);
            const Real Vup = option_->NPV() * notional_;
            spotQuote_->setValue(S - ds);
            const Real Vdn = option_->NPV() * notional_;
            spotQuote_->setValue(S);

            // Theta (frozen local vol, evaluation date shifted).
            const Date today = Settings::instance().evaluationDate();
            Settings::instance().evaluationDate() = today + 1;
            const Real Vnext = option_->NPV() * notional_;
            Settings::instance().evaluationDate() = today;
            const Real theta_raw = Vnext - V0;

            // Vega and volga using vol-bumped frozen surfaces.
            option_->setPricingEngine(engVp);
            const Real Vvp = option_->NPV() * notional_;

            option_->setPricingEngine(engVm);
            const Real Vvm = option_->NPV() * notional_;

            // Vanna: 4-point cross with frozen vol-bumped surfaces.
            spotQuote_->setValue(S + ds);
            option_->setPricingEngine(engVp);
            const Real Vpp = option_->NPV() * notional_;
            option_->setPricingEngine(engVm);
            const Real Vpm = option_->NPV() * notional_;

            spotQuote_->setValue(S - ds);
            option_->setPricingEngine(engVp);
            const Real Vmp = option_->NPV() * notional_;
            option_->setPricingEngine(engVm);
            const Real Vmm = option_->NPV() * notional_;

            spotQuote_->setValue(S);

            // Rega/sega: use non-frozen Dupire engine (sticky-delta dynamics
            // for vol-surface input sensitivities, per user requirement).
            option_->setPricingEngine(makeEngine(true));
            auto [rega, sega] = computeRegaSega();

            // Leave option with the base frozen engine.
            option_->setPricingEngine(engBase);

            return scaleResults(V0, Vup, Vdn, Vvp, Vvm, Vpp, Vpm, Vmp, Vmm,
                                theta_raw, rega, sega);
        }
    }

    ext::shared_ptr<FixedLocalVolSurface>
    FxVanillaBumpRisk::buildFixedLocalVolSurface(const std::vector<Time>& times,
                                                  const std::vector<Real>& strikes) const 
    {
        QL_REQUIRE(!times.empty(), "time grid is empty");
        QL_REQUIRE(!strikes.empty(), "strike grid is empty");

        auto lvMatrix = ext::make_shared<Matrix>(strikes.size(), times.size());
        for (Size ti = 0; ti < times.size(); ++ti)
            for (Size si = 0; si < strikes.size(); ++si)
                (*lvMatrix)[si][ti] = localVol_->localVol(times[ti], strikes[si], true);

        return ext::make_shared<FixedLocalVolSurface>(process_->blackVolatility()->referenceDate(),
                                                      times, strikes, lvMatrix,
                                                      process_->blackVolatility()->dayCounter());
    }

    void FxVanillaBumpRisk::printLocalVolComparison(const std::vector<Time>& times,
                                                     const std::vector<Real>& strikes,
                                                     std::ostream& out) const {
        const Size nT = times.size();
        const Size nK = strikes.size();

        // Dupire local vols, or the cells where they fail or fall back
        std::vector<std::vector<Real>> dupire(nK, std::vector<Real>(nT, Null<Real>()));
        std::vector<std::vector<bool>> fellBack(nK, std::vector<bool>(nT, false));
        std::vector<std::string> errors;
        Size nFallbacks = 0;
        for (Size si = 0; si < nK; ++si) {
            for (Size ti = 0; ti < nT; ++ti) {
                try {
                    const Size before = localVol_->fallbacks();
                    dupire[si][ti] = localVol_->localVol(times[ti], strikes[si], true);
                    if (localVol_->fallbacks() > before) {
                        fellBack[si][ti] = true;
                        ++nFallbacks;
                    }
                } catch (Error& e) {
                    if (errors.empty())
                        errors.emplace_back(e.what());
                    else
                        errors.emplace_back();
                }
            }
        }
        ext::shared_ptr<FixedLocalVolSurface> fixedLV;
        if (errors.empty()) {
            fixedLV = buildFixedLocalVolSurface(times, strikes);
            fixedLV->enableExtrapolation();
        }

        const int wt = 10;
        const int wv = 9;
        const int wk = 8;

        out << "\n--- Local vol surface: Dupire (D) vs FixedLocalVolSurface (F) ---\n";
        out << "(err on a D row = Dupire failed there; * = the fallback vol was used)\n";
        out << std::setw(wk) << "K\\t";
        for (Time t : times)
            out << std::setw(wt) << std::fixed << std::setprecision(2) << t;
        out << "\n" << std::string(wk + wt * nT, '-') << "\n";

        for (Size si = 0; si < nK; ++si) {
            const Real K = strikes[si];
            out << std::setw(wk - 1) << std::fixed << std::setprecision(4) << K << "D";
            for (Size ti = 0; ti < nT; ++ti) {
                std::ostringstream cell;
                if (dupire[si][ti] == Null<Real>())
                    cell << "err";
                else
                    cell << std::fixed << std::setprecision(2) << dupire[si][ti] * 100.0
                         << (fellBack[si][ti] ? "*" : "");
                out << std::setw(wv) << cell.str();
            }
            out << "\n";

            if (fixedLV) {
                out << std::setw(wk - 1) << std::fixed << std::setprecision(4) << K << "F";
                for (Size ti = 0; ti < nT; ++ti)
                    out << std::setw(wv) << std::fixed << std::setprecision(2)
                        << fixedLV->localVol(times[ti], K, true) * 100.0;
                out << "\n";
            }
            out << "\n";
        }

        if (errors.empty() && nFallbacks == 0) {
            out << "\nDupire local vol defined on the whole grid.\n";
        } else if (errors.empty()) {
            out << "\nDupire local vol not defined at " << nFallbacks << " / " << nT * nK
                << " cells, where the fallback vol was used.\n";
        } else {
            out << "\nDupire failed at " << errors.size() << " / " << nT * nK
                << " cells; the first error was:\n  " << errors.front() << "\n";
        }
    }

}
