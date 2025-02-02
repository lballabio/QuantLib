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
#include <ql/experimental/exoticoptions/analyticpartialtimebarrieroptionengine.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <utility>
#include <iostream>

namespace QuantLib {

    template<Option::Type OptionType>
    AnalyticPartialTimeBarrierOptionEngine<OptionType>::AnalyticPartialTimeBarrierOptionEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::calculate(PartialTimeBarrierOption::arguments& arguments) const {
        PartialBarrier::Type barrierType = arguments.barrierType;
        PartialBarrier::Range barrierRange = arguments.barrierRange;
        
        switch (barrierType) {
          case PartialBarrier::DownOut:
            switch (barrierRange) {
              case PartialBarrier::Start:
                return CA(1);
                break;
              case PartialBarrier::EndB1:
                return CoB1();
                break;
              case PartialBarrier::EndB2:
                return CoB2(PartialBarrier::DownOut);
                break;
              default:
                QL_FAIL("invalid barrier range");
            }
            break;

          case PartialBarrier::DownIn:
            switch (barrierRange) {
              case PartialBarrier::Start:
                return CIA(1);
                break;
              case PartialBarrier::End:
                QL_FAIL("Down-and-in partial-time end barrier is not implemented");
              default:
                QL_FAIL("invalid barrier range");
            }
            break;

          case PartialBarrier::UpOut:
            switch (barrierRange) {
              case PartialBarrier::Start:
                return CA(-1);
                break;
              case PartialBarrier::EndB1:
                return CoB1();
                break;
              case PartialBarrier::EndB2:
                return CoB2(PartialBarrier::UpOut);
                break;
              default:
                QL_FAIL("invalid barrier range");
            }
            break;

            case PartialBarrier::UpIn:
              switch (barrierRange) {
                case PartialBarrier::Start:
                  return CIA(-1);
                  break;
                case PartialBarrier::End:
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

    template<Option::Type OptionType>
    void AnalyticPartialTimeBarrierOptionEngine<OptionType>::calculate() const {
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
        if constexpr (OptionType == Option::Put)
        {
          Real spotSq = spot * spot;
          Real callStrike = spotSq / payoff->strike();
          ext::shared_ptr<StrikedTypePayoff> callPayoff =
            ext::make_shared<PlainVanillaPayoff>(Option::Call, callStrike);
          tmp_arguments_.barrierType = getSymmetricBarrierType(arguments_.barrierType);
          tmp_arguments_.barrier = spotSq / arguments_.barrier;
          tmp_arguments_.payoff = callPayoff;

          results_.value = payoff->strike() / spot * calculate(tmp_arguments_);
        } else 
          results_.value = calculate(tmp_arguments_ );
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::CoB2(
                                      PartialBarrier::Type barrierType) const {
        Real result = 0.0;
        Real b = riskFreeRate()-dividendYield();
        if (strike()<barrier()){
            switch (barrierType) {
              case PartialBarrier::DownOut:
                result = underlying()*std::exp((b-riskFreeRate())*residualTime());
                result *= (M(g1(),e1(),rho())-HS(underlying(),barrier(),2*(mu()+1))*M(g3(),-e3(),-rho()));
                result -= strike()*std::exp(-riskFreeRate()*residualTime())*(M(g2(),e2(),rho())-HS(underlying(),barrier(),2*mu())*M(g4(),-e4(),-rho()));
                return result;

              case PartialBarrier::UpOut:
                result = underlying()*std::exp((b-riskFreeRate())*residualTime());
                result *= (M(-g1(),-e1(),rho())-HS(underlying(),barrier(),2*(mu()+1))*M(-g3(),e3(),-rho()));
                result -= strike()*std::exp(-riskFreeRate()*residualTime())*(M(-g2(),-e2(),rho())-HS(underlying(),barrier(),2*mu())*M(-g4(),e4(),-rho()));
                result -= underlying()*std::exp((b-riskFreeRate())*residualTime())*(M(-d1(),-e1(),rho())-HS(underlying(),barrier(),2*(mu()+1))*M(e3(),-f1(),-rho()));
                result += strike()*std::exp(-riskFreeRate()*residualTime())*(M(-d2(),-e2(),rho())-HS(underlying(),barrier(),2*mu())*M(e4(),-f2(),-rho()));
                return result;

              default:
                QL_FAIL("invalid barrier type");
            }
        } else {
            QL_FAIL("case of strike>barrier is not implemented for OutEnd B2 type");
        }
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::CoB1() const {
        Real result = 0.0;
        Real b = riskFreeRate()-dividendYield();
        if (strike()>barrier()) {
            result = underlying()*std::exp((b-riskFreeRate())*residualTime());
            result *= (M(d1(),e1(),rho())-HS(underlying(),barrier(),2*(mu()+1))*M(f1(),-e3(),-rho()));
            result -= (strike()*std::exp(-riskFreeRate()*residualTime()))*(M(d2(),e2(),rho())-HS(underlying(),barrier(),2*mu())*M(f2(),-e4(),-rho()));
            return result;
        } else {
            Real S1 = underlying()*std::exp((b-riskFreeRate())*residualTime());
            Real X1 = (strike()*std::exp(-riskFreeRate()*residualTime()));
            Real HS1 = HS(underlying(),barrier(),2*(mu()+1));
            Real HS2 = HS(underlying(), barrier(), 2 * mu());
            result = S1;
            result *= (M(-g1(),-e1(),rho())-HS1*M(-g3(),e3(),-rho()));
            result -= X1*(M(-g2(), -e2(), rho()) - HS2*M(-g4(), e4(), -rho()));
            result -= S1*(M(-d1(), -e1(), rho()) - HS1*M(-f1(), e3(), -rho()));
            result += X1*(M(-d2(), -e2(), rho()) - HS2*M(-f2(), e4(), -rho()));
            result += S1*(M(g1(), e1(), rho()) - HS1*M(g3(), -e3(), -rho()));
            result -= X1*(M(g2(), e2(), rho()) - HS2*M(g4(), -e4(), -rho()));
            return result;
        }
    }

    // eta = -1: Up-and-In Call
    // eta =  1: Down-and-In Call
    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::CIA(Integer eta) const {
        ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);

        VanillaOption europeanOption(payoff, exercise);

        europeanOption.setPricingEngine(
                        ext::make_shared<AnalyticEuropeanEngine>(process_));

        return europeanOption.NPV() - CA(eta);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::CA(Integer eta) const {
        //Partial-Time-Start- OUT  Call Option calculation
        Real b = riskFreeRate()-dividendYield();
        Real result;
        result = underlying()*std::exp((b-riskFreeRate())*residualTime());
        result *= (M(d1(),eta*e1(),eta*rho())-HS(underlying(),barrier(),2*(mu()+1))*M(f1(),eta*e3(),eta*rho()));
        result -= (strike()*std::exp(-riskFreeRate()*residualTime())*(M(d2(),eta*e2(),eta*rho())-HS(underlying(),barrier(),2*mu())*M(f2(),eta*e4(),eta*rho())));
        return result;
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::underlying() const {
        return process_->x0();
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::strike() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        if constexpr (OptionType == Option::Put)
          return underlying() * underlying() / payoff->strike();
        else
          return payoff->strike();
    }

    template<Option::Type OptionType>
    Time AnalyticPartialTimeBarrierOptionEngine<OptionType>::residualTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    template<Option::Type OptionType>
    Time AnalyticPartialTimeBarrierOptionEngine<OptionType>::coverEventTime() const {
        return process_->time(arguments_.coverEventDate);
    }

    template<Option::Type OptionType>
    Volatility AnalyticPartialTimeBarrierOptionEngine<OptionType>::volatility(Time t) const {
        return process_->blackVolatility()->blackVol(t, strike());
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::stdDeviation() const {
        Time T = residualTime();
        return volatility(T) * std::sqrt(T);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::barrier() const {
        if constexpr (OptionType == Option::Put)
          return underlying() * underlying() / arguments_.barrier;
        else
          return arguments_.barrier;
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::rebate() const {
        return arguments_.rebate;
    }

    template<Option::Type OptionType>
    Rate AnalyticPartialTimeBarrierOptionEngine<OptionType>::riskFreeRate() const {
        if constexpr (OptionType == Option::Put)
          return process_->dividendYield()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
        else
          return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
    }

    template<Option::Type OptionType>
    DiscountFactor AnalyticPartialTimeBarrierOptionEngine<OptionType>::riskFreeDiscount() const {
        if constexpr (OptionType == Option::Put)
          return process_->dividendYield()->discount(residualTime());
        else
          return process_->riskFreeRate()->discount(residualTime());
    }

    template<Option::Type OptionType>
    Rate AnalyticPartialTimeBarrierOptionEngine<OptionType>::dividendYield() const {
        if constexpr (OptionType == Option::Put)
          return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
        else
          return process_->dividendYield()->zeroRate(residualTime(), Continuous,
                                                   NoFrequency);
    }

    template<Option::Type OptionType>
    DiscountFactor AnalyticPartialTimeBarrierOptionEngine<OptionType>::dividendDiscount() const {
        if constexpr (OptionType == Option::Put)
          return process_->riskFreeRate()->discount(residualTime());
        else
          return process_->dividendYield()->discount(residualTime());
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::f1() const {
        Real S = underlying();
        Real T = residualTime();
        Real sigma = volatility(T);
        return (std::log(S / strike()) + 2 * std::log(barrier() / S) + ((riskFreeRate()-dividendYield()) + (std::pow(sigma, 2) / 2))*T) / (sigma*std::sqrt(T));
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::f2() const {
        Time T = residualTime();
        return f1() - volatility(T)*std::sqrt(T);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::M(Real a,Real b,Real rho) const {
        BivariateCumulativeNormalDistributionDr78 CmlNormDist(rho);
        return CmlNormDist(a,b);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::rho() const {
        return std::sqrt(coverEventTime()/residualTime());
    }

    template<Option::Type OptionType>
    Rate AnalyticPartialTimeBarrierOptionEngine<OptionType>::mu() const {
        Volatility vol = volatility(coverEventTime());
        return ((riskFreeRate() - dividendYield()) - (vol * vol) / 2) / (vol * vol);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::d1() const {
        Real b = riskFreeRate()-dividendYield();
        Time T2 = residualTime();
        Volatility vol = volatility(T2);
        return (std::log(underlying()/strike())+(b+vol*vol/2)*T2)/(std::sqrt(T2)*vol);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::d2() const {
        Time T2 = residualTime();
        Volatility vol = volatility(T2);
        return d1() - vol*std::sqrt(T2);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::e1() const {
        Real b = riskFreeRate()-dividendYield();
        Time T1 = coverEventTime();
        Volatility vol = volatility(T1);
        return (std::log(underlying()/barrier())+(b+vol*vol/2)*T1)/(std::sqrt(T1)*vol);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::e2() const {
        Time T1 = coverEventTime();
        Volatility vol = volatility(T1);
        return e1() - vol*std::sqrt(T1);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::e3() const {
        Time T1 = coverEventTime();
        Real vol = volatility(T1);
        return e1()+(2*std::log(barrier()/underlying()) /(vol*std::sqrt(T1)));
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::e4() const {
        Time t = coverEventTime();
        return e3()-volatility(t)*std::sqrt(t);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::g1() const {
        Real b = riskFreeRate()-dividendYield();
        Time T2 = residualTime();
        Volatility vol = volatility(T2);
        return (std::log(underlying()/barrier())+(b+vol*vol/2)*T2)/(std::sqrt(T2)*vol);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::g2() const {
        Time T2 = residualTime();
        Volatility vol = volatility(T2);
        return g1() - vol*std::sqrt(T2);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::g3() const {
        Time T2 = residualTime();
        Real vol = volatility(T2);
        return g1()+(2*std::log(barrier()/underlying()) /(vol*std::sqrt(T2)));
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::g4() const {
        Time T2 = residualTime();
        Real vol = volatility(T2);
        return g3()-vol*std::sqrt(T2);
    }

    template<Option::Type OptionType>
    Real AnalyticPartialTimeBarrierOptionEngine<OptionType>::HS(Real S, Real H, Real power) const {
        return std::pow((H/S),power);
    }

    template class AnalyticPartialTimeBarrierOptionEngine<Option::Call>;
    template class AnalyticPartialTimeBarrierOptionEngine<Option::Put>;
}

