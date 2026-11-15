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

#include <ql/experimental/fxslv/fxvanillagreeks.hpp>
#include <ql/exercise.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/settings.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace QuantLib {

    namespace {
        // Reference tenor for rega/sega time-scaling: 1 month.
        const Real T_1M = 1.0 / 12.0;

        // Finite-difference Black-Scholes engine on a given spot mesher, as
        // FdBlackScholesVanillaEngine prices but for the grid: that engine
        // sizes its grid from the market it prices, so that each bump of the
        // market would reprice on a different grid, and second differences
        // such as volga take the change of grid for a sensitivity.
        class FixedMeshFdBlackScholesEngine : public VanillaOption::engine {
          public:
            FixedMeshFdBlackScholesEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                                          ext::shared_ptr<FdmMesher> mesher,
                                          Size tGrid,
                                          bool localVol)
            : process_(std::move(process)), mesher_(std::move(mesher)), tGrid_(tGrid),
              localVol_(localVol) {
                registerWith(process_);
            }

            void calculate() const override {
                const auto payoff = ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
                QL_REQUIRE(payoff, "non-striked payoff given");
                const Time maturity = process_->time(arguments_.exercise->lastDate());
                const auto calculator = ext::make_shared<FdmLogInnerValue>(payoff, mesher_, 0);
                const auto conditions = FdmStepConditionComposite::vanillaComposite(
                    DividendSchedule(), arguments_.exercise, mesher_, calculator,
                    process_->riskFreeRate()->referenceDate(),
                    process_->riskFreeRate()->dayCounter());
                const FdmSolverDesc solverDesc = {mesher_,    FdmBoundaryConditionSet(),
                                                  conditions, calculator,
                                                  maturity,   tGrid_,
                                                  0};
                // no overwrite of failed local vols: a failure is an error,
                // not a number to substitute
                const FdmBlackScholesSolver solver(
                    Handle<GeneralizedBlackScholesProcess>(process_), payoff->strike(), solverDesc,
                    FdmSchemeDesc::Douglas(), localVol_, -Null<Real>());
                results_.value = solver.valueAt(process_->x0());
            }

          private:
            ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
            ext::shared_ptr<FdmMesher> mesher_;
            Size tGrid_;
            bool localVol_;
        };
    }

    FxVanillaBumpRisk::FxVanillaBumpRisk(ext::shared_ptr<VanillaOption> option,
                                         FxPairMarket market,
                                         Real notional,
                                         Real spotBump,
                                         Real volBump,
                                         Real rrBump,
                                         Real bfBump,
                                         Size tGrid,
                                         Size xGrid,
                                         Size lvTimePts,
                                         Size lvStrikePts,
                                         Real lvStrikeSpread)
    : option_(std::move(option)), market_(std::move(market)), notional_(notional),
      spotBump_(spotBump), volBump_(volBump), rrBump_(rrBump), bfBump_(bfBump), tGrid_(tGrid),
      xGrid_(xGrid), lvTimePts_(lvTimePts), lvStrikePts_(lvStrikePts),
      lvStrikeSpread_(lvStrikeSpread) {
        QL_REQUIRE(option_ != nullptr, "null option");
        QL_REQUIRE(spotBump_ > 0.0 && volBump_ > 0.0, "spot and vol bumps must be positive");
        QL_REQUIRE(lvTimePts_ >= 2, "lvTimePts must be at least 2");
        QL_REQUIRE(lvStrikePts_ >= 2, "lvStrikePts must be at least 2");
        QL_REQUIRE(lvStrikeSpread_ > 0.0, "lvStrikeSpread must be positive");
        setLocalVolFallback(Null<Volatility>());
    }

    void FxVanillaBumpRisk::setLocalVolFallback(Volatility fallbackVol) {
        localVolFallback_ = fallbackVol;
        localVol_ = ext::dynamic_pointer_cast<FxLocalVolSurface>(
            market_.localVolProcess(fallbackVol)->localVolatility().currentLink());
        QL_ENSURE(localVol_, "the market's local vol is not an FxLocalVolSurface");
    }

    ext::shared_ptr<FdmMesher> FxVanillaBumpRisk::makeMesher(
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& proc) const {
        // the grid FdBlackScholesVanillaEngine would use for this market
        const auto payoff = ext::dynamic_pointer_cast<StrikedTypePayoff>(option_->payoff());
        QL_REQUIRE(payoff, "non-striked payoff given");
        const Time maturity = proc->time(option_->exercise()->lastDate());
        return ext::make_shared<FdmMesherComposite>(ext::make_shared<FdmBlackScholesMesher>(
            xGrid_, proc, maturity, payoff->strike(), Null<Real>(), Null<Real>(), 0.0001, 1.5,
            std::pair<Real, Real>(payoff->strike(), 0.1)));
    }

    Real FxVanillaBumpRisk::value(const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
                                  bool localVol,
                                  const ext::shared_ptr<FdmMesher>& mesher) const {
        if (localVol)
            option_->setPricingEngine(
                ext::make_shared<FixedMeshFdBlackScholesEngine>(process, mesher, tGrid_, true));
        else
            option_->setPricingEngine(ext::make_shared<AnalyticEuropeanEngine>(process));
        return option_->NPV() * notional_;
    }

    Real FxVanillaBumpRisk::value(const FxPairMarket& market,
                                  bool localVol,
                                  const ext::shared_ptr<FdmMesher>& mesher) const {
        return value(localVol ? market.localVolProcess(localVolFallback_) : market.blackProcess(),
                     localVol, mesher);
    }

    Real FxVanillaBumpRisk::rollTheta(bool localVol, Real todaysValue) const {
        const Date next = market_.conventions().settlement().calendar().advance(
            market_.asOf(), 1, Days);
        // the rolled market is built, and priced, on its own date
        struct EvaluationDateRestorer {
            Date date = Settings::instance().evaluationDate();
            ~EvaluationDateRestorer() { Settings::instance().evaluationDate() = date; }
        } restorer;
        Settings::instance().evaluationDate() = next;
        const FxPairMarket rolled = market_.rolled(next);
        // a market of its own, on a grid of its own
        const auto mesher = localVol ? makeMesher(rolled.blackProcess()) : nullptr;
        return value(rolled, localVol, mesher) - todaysValue;
    }

    std::pair<std::vector<Time>, std::vector<Real>> FxVanillaBumpRisk::buildLvGrid() const {
        const Date expiry = option_->exercise()->lastDate();
        const Time T = market_.blackProcess()->time(expiry);
        QL_REQUIRE(T > 0.0, "option has expired; cannot build local vol grid");

        const std::vector<Volatility>& atms = market_.quotes().atms;
        Real avgAtm = 0.0;
        for (Volatility v : atms)
            avgAtm += v;
        avgAtm /= static_cast<Real>(atms.size());

        std::vector<Time> times(lvTimePts_);
        for (Size i = 0; i < lvTimePts_; ++i)
            times[i] = T * (i + 1.0) / static_cast<Real>(lvTimePts_);

        const Real S = market_.spot();
        const Real halfWidth = lvStrikeSpread_ * avgAtm * std::sqrt(T);
        std::vector<Real> strikes(lvStrikePts_);
        for (Size j = 0; j < lvStrikePts_; ++j) {
            const Real u = 2.0 * j / static_cast<Real>(lvStrikePts_ - 1) - 1.0;
            strikes[j] = S * std::exp(halfWidth * u);
        }

        return {times, strikes};
    }

    ext::shared_ptr<FixedLocalVolSurface>
    FxVanillaBumpRisk::frozenLocalVol(const FxPairMarket& market,
                                      const std::vector<Time>& times,
                                      const std::vector<Real>& strikes) const {
        const auto lv = market.localVolProcess(localVolFallback_)->localVolatility();
        auto lvMatrix = ext::make_shared<Matrix>(strikes.size(), times.size());
        for (Size ti = 0; ti < times.size(); ++ti)
            for (Size si = 0; si < strikes.size(); ++si)
                (*lvMatrix)[si][ti] = lv->localVol(times[ti], strikes[si], true);
        auto result = ext::make_shared<FixedLocalVolSurface>(
            market.tradingTimeVolSurface()->referenceDate(), times, strikes, lvMatrix,
            market.tradingTimeDayCounter());
        result->enableExtrapolation();
        return result;
    }

    FxVanillaGreeks FxVanillaBumpRisk::calculate(bool localVol, StickyType sticky) const {
        QL_REQUIRE(Settings::instance().evaluationDate() == market_.asOf(),
                   "the evaluation date (" << Settings::instance().evaluationDate()
                                           << ") must be the market's date (" << market_.asOf()
                                           << ")");
        const Real S  = market_.spot();
        const Real ds = S * spotBump_;
        const Real dv = volBump_;

        // ── Settlement foreign discount factor, for the forward delta ─────────
        const Date expiry = option_->exercise()->lastDate();
        const Real Bf     = market_.blackProcess()->dividendYield()->discount(expiry);

        // one grid, today's, for every bump of the market
        const auto mesher = makeMesher(market_.blackProcess());

        // ── Bumped markets ────────────────────────────────────────────────────
        auto shiftAtms = [](const FxPairMarket& m, Real shift) {
            FxVolQuotes q = m.quotes();
            for (auto& atm : q.atms)
                atm += shift;
            return m.withQuotes(q);
        };
        // Each RR (BF) pillar is bumped by rrBump_ (bfBump_) * sqrt(T_1M / T_pillar):
        // the 1M pillar moves by the bump and longer tenors by less (∝ 1/sqrt(T)).
        auto shiftSmile = [&](bool riskReversals, Real sign) {
            FxVolQuotes q = market_.quotes();
            const Actual365Fixed dc;
            for (Size i = 0; i < market_.pillars().size(); ++i) {
                const Time t = dc.yearFraction(market_.asOf(), market_.pillars()[i]);
                const Real bump =
                    sign * (riskReversals ? rrBump_ : bfBump_) * std::sqrt(T_1M / t);
                for (auto& v : riskReversals ? q.riskReversals[i] : q.butterflies[i])
                    v += bump;
            }
            return market_.withQuotes(q);
        };
        // rega and sega: the surface as it stands, whatever the sticky rule
        auto regaSega = [&](bool lv) -> std::pair<Real, Real> {
            const Real rega = (value(shiftSmile(true, +1.0), lv, mesher) -
                               value(shiftSmile(true, -1.0), lv, mesher)) /
                              2.0;
            const Real sega = (value(shiftSmile(false, +1.0), lv, mesher) -
                               value(shiftSmile(false, -1.0), lv, mesher)) /
                              2.0;
            return {rega, sega};
        };

        // ── Scaling helpers ───────────────────────────────────────────────────
        // V is in domestic currency for the notional, so dV/dS is an amount
        // of foreign currency: the spot hedge.
        // spotDelta = dV/dS                          (foreign ccy)
        // fwdDelta  = d(V/Bd)/dF = spotDelta / Bf    (foreign ccy, in forwards)
        // spotGamma = d²V/dS² * 0.01*S               (spotDelta change per 1% spot)
        // fwdGamma  = spotGamma / Bf                 (fwdDelta change per 1% fwd)
        // vega      = dV/dσ * 0.01                   (domestic per 1% vol)
        // vanna     = d²V/(dS dσ) * 0.01             (spotDelta change per 1% vol)
        // navva     = d²V/(dS dσ) * 0.01 * 0.01*S    (vega change per 1% spot)
        // volga     = d²V/dσ² * 0.01²                (vega change per 1% vol)
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
            r.spotDelta = spotDelta_raw;
            r.fwdDelta  = r.spotDelta / Bf;
            r.spotGamma = spotGamma_raw * pct1S;
            r.fwdGamma  = r.spotGamma / Bf;
            r.theta     = theta_raw;
            r.vega      = vega_raw * 0.01;
            r.vanna     = vanna_raw * 0.01;
            r.navva     = r.vanna * pct1S;   // the same mixed partial
            r.volga     = volga_raw * 0.01 * 0.01;
            r.rega      = rega;
            r.sega      = sega;

            // the hedge of the value in foreign currency, V/S: delta - V/S,
            // and its derivatives in spot and vol
            FxPremiumAdjustedGreeks& pa = r.premiumAdjusted;
            pa.spotDelta = r.spotDelta - V0 / S;
            pa.fwdDelta  = pa.spotDelta / Bf;
            pa.spotGamma = r.spotGamma - 0.01 * r.spotDelta + 0.01 * V0 / S;
            pa.fwdGamma  = pa.spotGamma / Bf;
            pa.vanna     = r.vanna - r.vega / S;
            return r;
        };

        if (sticky == StickyType::Strike) {
            // ── Sticky-strike path ────────────────────────────────────────────
            // Black at the option's implied vol, read from a market's surface
            // and held as spot moves.
            const auto payoff = ext::dynamic_pointer_cast<StrikedTypePayoff>(option_->payoff());
            QL_REQUIRE(payoff, "non-striked payoff given");
            auto atTheVolOf = [&](const FxPairMarket& m, Real spot) {
                const Volatility vol =
                    m.tradingTimeVolSurface()->blackVol(expiry, payoff->strike(), true);
                const auto black = m.blackProcess();
                return ext::make_shared<GeneralizedBlackScholesProcess>(
                    makeQuoteHandle(spot), black->dividendYield(), black->riskFreeRate(),
                    Handle<BlackVolTermStructure>(ext::make_shared<BlackConstantVol>(
                        m.asOf(), NullCalendar(), vol, m.tradingTimeDayCounter())));
            };
            const FxPairMarket up = shiftAtms(market_, +dv), down = shiftAtms(market_, -dv);
            const Real V0 = value(atTheVolOf(market_, S), false, mesher);
            const Real Vup = value(atTheVolOf(market_, S + ds), false, mesher);
            const Real Vdn = value(atTheVolOf(market_, S - ds), false, mesher);
            const Real Vvp = value(atTheVolOf(up, S), false, mesher);
            const Real Vvm = value(atTheVolOf(down, S), false, mesher);
            const Real Vpp = value(atTheVolOf(up, S + ds), false, mesher);
            const Real Vmp = value(atTheVolOf(up, S - ds), false, mesher);
            const Real Vpm = value(atTheVolOf(down, S + ds), false, mesher);
            const Real Vmm = value(atTheVolOf(down, S - ds), false, mesher);

            // at today's spot, the surface as it stands: the sticky rule does
            // not matter for theta and for the risk reversal and butterfly bumps
            const Real theta_raw = rollTheta(false, value(market_, false, mesher));
            auto [rega, sega] = regaSega(false);
            return scaleResults(V0, Vup, Vdn, Vvp, Vvm, Vpp, Vpm, Vmp, Vmm,
                                theta_raw, rega, sega);
        }

        if (!localVol || sticky == StickyType::Delta) {
            // ── Black, or sticky-delta local vol ─────────────────────────────
            // Each bump is a market rebuilt from the bumped quotes: the smile
            // re-anchors in delta at the bumped spot, and the local vol is
            // that of the bumped surface.
            auto V = [&](const FxPairMarket& m) { return value(m, localVol, mesher); };
            const FxPairMarket spotUp = market_.withSpot(S + ds);
            const FxPairMarket spotDown = market_.withSpot(S - ds);
            const Real V0  = V(market_);
            const Real Vup = V(spotUp), Vdn = V(spotDown);
            const Real Vvp = V(shiftAtms(market_, +dv)), Vvm = V(shiftAtms(market_, -dv));
            const Real Vpp = V(shiftAtms(spotUp, +dv)), Vpm = V(shiftAtms(spotUp, -dv));
            const Real Vmp = V(shiftAtms(spotDown, +dv)), Vmm = V(shiftAtms(spotDown, -dv));
            const Real theta_raw = rollTheta(localVol, V0);
            auto [rega, sega] = regaSega(localVol);
            return scaleResults(V0, Vup, Vdn, Vvp, Vvm, Vpp, Vpm, Vmp, Vmm,
                                theta_raw, rega, sega);
        }

        // ── Frozen local-vol path (localVol=true only) ────────────────────────
        // Local vols frozen in (t, K) at today's spot, for the base, vol+dv
        // and vol-dv markets; spot moves under them.
        auto [times, strikes] = buildLvGrid();
        const auto lvBase = frozenLocalVol(market_, times, strikes);
        const auto lvUp = frozenLocalVol(shiftAtms(market_, +dv), times, strikes);
        const auto lvDown = frozenLocalVol(shiftAtms(market_, -dv), times, strikes);
        const auto black = market_.blackProcess();
        auto frozen = [&](const ext::shared_ptr<FixedLocalVolSurface>& lv, Real spot) {
            return ext::make_shared<GeneralizedBlackScholesProcess>(
                makeQuoteHandle(spot), black->dividendYield(), black->riskFreeRate(),
                black->blackVolatility(), Handle<LocalVolTermStructure>(lv));
        };
        const Real V0  = value(frozen(lvBase, S), true, mesher);
        const Real Vup = value(frozen(lvBase, S + ds), true, mesher);
        const Real Vdn = value(frozen(lvBase, S - ds), true, mesher);
        const Real Vvp = value(frozen(lvUp, S), true, mesher);
        const Real Vvm = value(frozen(lvDown, S), true, mesher);
        const Real Vpp = value(frozen(lvUp, S + ds), true, mesher);
        const Real Vpm = value(frozen(lvDown, S + ds), true, mesher);
        const Real Vmp = value(frozen(lvUp, S - ds), true, mesher);
        const Real Vmm = value(frozen(lvDown, S - ds), true, mesher);
        // theta and rega/sega: local vol not frozen, as the market rolls or
        // its smile moves
        const Real theta_raw = rollTheta(true, value(market_, true, mesher));
        auto [rega, sega] = regaSega(true);
        return scaleResults(V0, Vup, Vdn, Vvp, Vvm, Vpp, Vpm, Vmp, Vmm,
                            theta_raw, rega, sega);
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

        return ext::make_shared<FixedLocalVolSurface>(
            market_.tradingTimeVolSurface()->referenceDate(), times, strikes, lvMatrix,
            market_.tradingTimeDayCounter());
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
