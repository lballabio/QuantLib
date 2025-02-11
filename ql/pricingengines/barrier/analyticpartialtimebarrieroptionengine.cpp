/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/exercise.hpp>
#include <ql/pricingengines/barrier/analyticpartialtimebarrieroptionengine.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <utility>

namespace QuantLib {

    AnalyticPartialTimeBarrierOptionEngine::AnalyticPartialTimeBarrierOptionEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::calculate(PartialTimeBarrierOption::arguments& arguments,
                                                          ext::shared_ptr<PlainVanillaPayoff> payoff,
                                                          ext::shared_ptr<GeneralizedBlackScholesProcess> process) const {
        Barrier::Type barrierType = arguments.barrierType;
        PartialBarrier::Range barrierRange = arguments.barrierRange;
        Rate r = process->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
        Rate q = process->dividendYield()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
        Real barrier = arguments.barrier;
        Real strike = payoff->strike();
        
        switch (barrierType) {
          case Barrier::DownOut:
            switch (barrierRange) {
              case PartialBarrier::Start:
                return CA(1, barrier, strike, r, q);
                break;
              case PartialBarrier::EndB1:
                return CoB1(barrier, strike, r, q);
                break;
              case PartialBarrier::EndB2:
                return CoB2(Barrier::DownOut, barrier, strike, r, q);
                break;
              default:
                QL_FAIL("invalid barrier range");
            }
            break;

          case Barrier::DownIn:
            switch (barrierRange) {
              case PartialBarrier::Start:
                return CIA(1, barrier, strike, r, q);
                break;
              case PartialBarrier::EndB1:
              case PartialBarrier::EndB2:
                QL_FAIL("Down-and-in partial-time end barrier is not implemented");
              default:
                QL_FAIL("invalid barrier range");
            }
            break;

          case Barrier::UpOut:
            switch (barrierRange) {
              case PartialBarrier::Start:
                return CA(-1, barrier, strike, r, q);
                break;
              case PartialBarrier::EndB1:
                return CoB1(barrier, strike, r, q);
                break;
              case PartialBarrier::EndB2:
                return CoB2(Barrier::UpOut, barrier, strike, r, q);
                break;
              default:
                QL_FAIL("invalid barrier range");
            }
            break;

            case Barrier::UpIn:
              switch (barrierRange) {
                case PartialBarrier::Start:
                  return CIA(-1, barrier, strike, r, q);
                  break;
                case PartialBarrier::EndB1:
                case PartialBarrier::EndB2:
                  QL_FAIL("Up-and-in partial-time end barrier is not implemented");
                default:
                  QL_FAIL("invalid barrier range");
              }
              break;
            default:
              QL_FAIL("unknown barrier type");
          }

        return 0.0;
    }

    void AnalyticPartialTimeBarrierOptionEngine::calculate() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        QL_REQUIRE(payoff->strike()>0.0,
                   "strike must be positive");

        Real spot = process_->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        auto getSymmetricBarrierType = [](Barrier::Type barrierType) -> Barrier::Type {
          if (barrierType == Barrier::UpIn) return Barrier::DownIn;
          if (barrierType == Barrier::DownIn) return Barrier::UpIn;
          if (barrierType == Barrier::UpOut) return Barrier::DownOut;
          return Barrier::UpOut;
        };

        auto tmp_arguments_ = arguments_;
        if (payoff->optionType() == Option::Put)
        {
          Real spotSq = spot * spot;
          Real callStrike = spotSq / payoff->strike();
          ext::shared_ptr<PlainVanillaPayoff> callPayoff =
            ext::make_shared<PlainVanillaPayoff>(Option::Call, callStrike);
          tmp_arguments_.barrierType = getSymmetricBarrierType(arguments_.barrierType);
          tmp_arguments_.barrier = spotSq / arguments_.barrier;
          tmp_arguments_.payoff = callPayoff;
          auto callProcess = ext::make_shared<GeneralizedBlackScholesProcess>(
              process_->stateVariable(),
              process_->riskFreeRate(),
              process_->dividendYield(),
              process_->blackVolatility()
            );

          results_.value = payoff->strike() / spot * calculate(tmp_arguments_, callPayoff, callProcess);
        } else
          results_.value = calculate(tmp_arguments_, payoff, process_);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::CoB2(
                                      Barrier::Type barrierType, 
                                      Real barrier, Real strike, Rate r, Rate q) const {
        Real result = 0.0;
        Real b = r - q;
        Real T = residualTime();
        Real S = underlying();
        Real mu_ = mu(strike, b);
        Real g1_ = g1(barrier, strike, b);
        Real g2_ = g2(barrier, strike, b);
        Real g3_ = g3(barrier, strike, b);
        Real g4_ = g4(barrier, strike, b);
        Real e1_ = e1(barrier, strike, b);
        Real e2_ = e2(barrier, strike, b);
        Real e3_ = e3(barrier, strike, b);
        Real e4_ = e4(barrier, strike, b);
        Real rho_ = rho();
        Real HSMu = HS(S, barrier, 2 * mu_);
        Real HSMu1 = HS(S, barrier, 2 * (mu_ + 1));
        Real X1 = strike * std::exp(-r * T);

        if (strike < barrier){
            switch (barrierType) {
              case Barrier::DownOut:
                result = S * std::exp((b - r) * T);
                result *= (M(g1_, e1_, rho_) - HSMu1 * M(g3_, -e3_, -rho_));
                result -= X1 * (M(g2_, e2_, rho_)-HSMu*M(g4_, -e4_, -rho_));
                return result;

              case Barrier::UpOut:
                result = S * std::exp((b - r) * T);
                result *= (M(-g1_, -e1_, rho_) - HSMu1 * M(-g3_, e3_, -rho_));
                result -= X1 * (M(-g2_, -e2_, rho_) - HSMu * M(-g4_, e4_, -rho_));
                result -= S * std::exp((b - r) * T) * 
                          (M(-d1(strike, b), -e1_, rho_) - HSMu1 * 
                          M(e3_, -f1(barrier, strike, b),-rho_));
                result += X1 * (M(-d2(strike, b), -e2_, rho_) - HSMu * 
                          M(e4_, -f2(barrier, strike, b), -rho_));
                return result;

              default:
                QL_FAIL("invalid barrier type");
            }
        } else {
            QL_FAIL("case of strike>barrier is not implemented for OutEnd B2 type");
        }
    }

    Real AnalyticPartialTimeBarrierOptionEngine::CoB1(Real barrier, Real strike, Rate r, Rate q) const {
        Real result = 0.0;
        Rate b = r - q;
        Real T = residualTime();
        Real S = underlying();
        Real mu_ = mu(strike, b);
        Real g1_ = g1(barrier, strike, b);
        Real g2_ = g2(barrier, strike, b);
        Real g3_ = g3(barrier, strike, b);
        Real g4_ = g4(barrier, strike, b);
        Real e1_ = e1(barrier, strike, b);
        Real e2_ = e2(barrier, strike, b);
        Real e3_ = e3(barrier, strike, b);
        Real e4_ = e4(barrier, strike, b);
        Real rho_ = rho();
        Real HSMu = HS(S, barrier, 2 * mu_);
        Real HSMu1 = HS(S, barrier, 2 * (mu_ + 1));
        Real X1 = strike * std::exp(-r * T);

        if (strike > barrier) {
            result = S * std::exp((b - r) * T);
            result *= (M(d1(strike, b), e1_, rho_) - HSMu1 * M(f1(barrier, strike, b), -e3_, -rho_));
            result -= X1 * (M(d2(strike, b), e2_, rho_) - HSMu * M(f2(barrier, strike, b), -e4_, -rho_));
            return result;
        } else {
            Real S1 = S * std::exp((b - r) * T);
            result = S1;
            result *= (M(-g1_, -e1_, rho_) - HSMu1 * M(-g3_,e3_,-rho_));
            result -= X1 * (M(-g2_, -e2_, rho_) - HSMu * M(-g4_, e4_, -rho_));
            result -= S1 * (M(-d1(strike, b), -e1_, rho_) - HSMu1 * M(-f1(barrier, strike, b), e3_, -rho_));
            result += X1 * (M(-d2(strike, b), -e2_, rho_) - HSMu * M(-f2(barrier, strike, b), e4_, -rho_));
            result += S1 * (M(g1_, e1_, rho_) - HSMu1 * M(g3_, -e3_, -rho_));
            result -= X1 * (M(g2_, e2_, rho_) - HSMu * M(g4_, -e4_, -rho_));
            return result;
        }
    }

    // eta = -1: Up-and-In Call
    // eta =  1: Down-and-In Call
    Real AnalyticPartialTimeBarrierOptionEngine::CIA(Integer eta, Real barrier, Real strike, Rate r, Rate q) const {
        ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);

        VanillaOption europeanOption(payoff, exercise);

        europeanOption.setPricingEngine(
                        ext::make_shared<AnalyticEuropeanEngine>(process_));

        return europeanOption.NPV() - CA(eta, barrier, strike, r, q);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::CA(Integer eta, Real barrier, Real strike, Rate r, Rate q) const {
        //Partial-Time-Start- OUT  Call Option calculation
        Real b = r - q;
        Real rho_ = rho();
        Real T = residualTime();
        Real S = underlying();
        Real mu_ = mu(strike, b);
        Real e1_ = e1(barrier, strike, b);
        Real e2_ = e2(barrier, strike, b);
        Real e3_ = e3(barrier, strike, b);
        Real e4_ = e4(barrier, strike, b);
        Real HSMu = HS(S, barrier,2 * mu_);
        Real HSMu1 = HS(S, barrier, 2 * (mu_ + 1));

        Real result;
        result = S * std::exp((b - r) * T);
        result *= (M(d1(strike, b), eta * e1_, eta * rho_)-HSMu1 * 
                  M(f1(barrier, strike, b), eta * e3_, eta * rho_));
        result -= (strike * std::exp(-r * T) * 
                  (M(d2(strike, b),eta * e2_, eta * rho_) - HSMu *
                  M(f2(barrier, strike, b), eta * e4_, eta * rho_)));
        return result;
    }

    Real AnalyticPartialTimeBarrierOptionEngine::underlying() const {
        return process_->x0();
    }

    Time AnalyticPartialTimeBarrierOptionEngine::residualTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    Time AnalyticPartialTimeBarrierOptionEngine::coverEventTime() const {
        return process_->time(arguments_.coverEventDate);
    }

    Volatility AnalyticPartialTimeBarrierOptionEngine::volatility(Time t, Real strike) const {
        return process_->blackVolatility()->blackVol(t, strike);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::f1(Real barrier, Real strike, Rate b) const {
        Real S = underlying();
        Real T = residualTime();
        Real sigma = volatility(T, strike);
        return (std::log(S / strike) + 2 * std::log(barrier / S) + 
              (b + (std::pow(sigma, 2) / 2))*T) / (sigma*std::sqrt(T));
    }

    Real AnalyticPartialTimeBarrierOptionEngine::f2(Real barrier, Real strike, Rate b) const {
        Time T = residualTime();
        return f1(barrier, strike, b) - volatility(T, strike) * std::sqrt(T);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::M(Real a, Real b, Real rho) const {
        BivariateCumulativeNormalDistributionDr78 CmlNormDist(rho);
        return CmlNormDist(a,b);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::rho() const {
        return std::sqrt(coverEventTime() / residualTime());
    }

    Rate AnalyticPartialTimeBarrierOptionEngine::mu(Real strike, Rate b) const {
        Volatility vol = volatility(coverEventTime(), strike);
        return (b - (vol * vol) / 2) / (vol * vol);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::d1(Real strike, Rate b) const {
        Time T2 = residualTime();
        Volatility vol = volatility(T2, strike);
        return (std::log(underlying() / strike) + (b + vol * vol / 2) * T2) / (std::sqrt(T2) * vol);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::d2(Real strike, Rate b) const {
        Time T2 = residualTime();
        Volatility vol = volatility(T2, strike);
        return d1(strike, b) - vol * std::sqrt(T2);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::e1(Real barrier, Real strike, Rate b) const {
        Time T1 = coverEventTime();
        Volatility vol = volatility(T1, strike);
        return (std::log(underlying() / barrier) + (b + vol * vol / 2) * T1) / (std::sqrt(T1) * vol);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::e2(Real barrier, Real strike, Rate b) const {
        Time T1 = coverEventTime();
        Volatility vol = volatility(T1, strike);
        return e1(barrier, strike, b) - vol * std::sqrt(T1);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::e3(Real barrier, Real strike, Rate b) const {
        Time T1 = coverEventTime();
        Real vol = volatility(T1, strike);
        return e1(barrier, strike, b) + (2 * std::log(barrier / underlying()) / (vol * std::sqrt(T1)));
    }

    Real AnalyticPartialTimeBarrierOptionEngine::e4(Real barrier, Real strike, Rate b) const {
        Time t = coverEventTime();
        return e3(barrier, strike, b) - volatility(t, strike) * std::sqrt(t);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::g1(Real barrier, Real strike, Rate b) const {
        Time T2 = residualTime();
        Volatility vol = volatility(T2, strike);
        return (std::log(underlying() / barrier) + (b + vol * vol / 2) * T2) / (std::sqrt(T2) * vol);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::g2(Real barrier, Real strike, Rate b) const {
        Time T2 = residualTime();
        Volatility vol = volatility(T2, strike);
        return g1(barrier, strike, b) - vol * std::sqrt(T2);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::g3(Real barrier, Real strike, Rate b) const {
        Time T2 = residualTime();
        Real vol = volatility(T2, strike);
        return g1(barrier, strike, b) + (2 * std::log(barrier / underlying()) /(vol * std::sqrt(T2)));
    }

    Real AnalyticPartialTimeBarrierOptionEngine::g4(Real barrier, Real strike, Rate b) const {
        Time T2 = residualTime();
        Real vol = volatility(T2, strike);
        return g3(barrier, strike, b) - vol * std::sqrt(T2);
    }

    Real AnalyticPartialTimeBarrierOptionEngine::HS(Real S, Real H, Real power) const {
        return std::pow((H / S), power);
    }

}

