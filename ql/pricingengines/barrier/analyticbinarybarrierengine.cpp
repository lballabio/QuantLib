/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Thema Consulting SA

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
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/barrier/analyticbinarybarrierengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <utility>

namespace QuantLib {

    // calc helper object 
    class AnalyticBinaryBarrierEngine_helper
    {
    
    public:
        AnalyticBinaryBarrierEngine_helper(
             const std::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const std::shared_ptr<StrikedTypePayoff> &payoff,
             const std::shared_ptr<AmericanExercise> &exercise,
             const BarrierOption::arguments &arguments):
        process_(process),
        payoff_(payoff),
        exercise_(exercise),
        arguments_(arguments)
        {
        }

        Real payoffAtExpiry(Real spot, Real variance, Real discount);
    private:
        const std::shared_ptr<GeneralizedBlackScholesProcess>& process_;
        const std::shared_ptr<StrikedTypePayoff> &payoff_;
        const std::shared_ptr<AmericanExercise> &exercise_;
        const BarrierOption::arguments &arguments_;
    };


    AnalyticBinaryBarrierEngine::AnalyticBinaryBarrierEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticBinaryBarrierEngine::calculate() const {

        std::shared_ptr<AmericanExercise> ex =
            std::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(ex->payoffAtExpiry(), "payoff must be at expiry");
        QL_REQUIRE(ex->dates()[0] <=
                   process_->blackVolatility()->referenceDate(),
                   "American option with window exercise not handled yet");

        std::shared_ptr<StrikedTypePayoff> payoff =
            std::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        Real variance =
            process_->blackVolatility()->blackVariance(ex->lastDate(),
                                                       payoff->strike());
        Real barrier = arguments_.barrier;
        QL_REQUIRE(barrier>0.0,
                   "positive barrier value required");
        Barrier::Type barrierType = arguments_.barrierType;

        // KO degenerate cases
        if ( (barrierType == Barrier::DownOut && spot <= barrier) ||
             (barrierType == Barrier::UpOut && spot >= barrier))
        {
            // knocked out, no value
            results_.value = 0;
            results_.delta = 0;
            results_.gamma = 0;
            results_.vega = 0;
            results_.theta = 0;
            results_.rho = 0;
            results_.dividendRho = 0;
            return;
        }

        // KI degenerate cases
        if ((barrierType == Barrier::DownIn && spot <= barrier) ||
           (barrierType == Barrier::UpIn && spot >= barrier)) {
            // knocked in - is a digital european
            std::shared_ptr<Exercise> exercise(new EuropeanExercise(
                                             arguments_.exercise->lastDate()));

            std::shared_ptr<PricingEngine> engine(
                                       new AnalyticEuropeanEngine(process_));

            VanillaOption opt(payoff, exercise);
            opt.setPricingEngine(engine);
            results_.value = opt.NPV();
            results_.delta = opt.delta();
            results_.gamma = opt.gamma();
            results_.vega = opt.vega();
            results_.theta = opt.theta();
            results_.rho = opt.rho();
            results_.dividendRho = opt.dividendRho();
            return;
        }

        Rate riskFreeDiscount =
            process_->riskFreeRate()->discount(ex->lastDate());

        AnalyticBinaryBarrierEngine_helper helper(process_,
           payoff, ex, arguments_);
        results_.value = helper.payoffAtExpiry(spot, variance, riskFreeDiscount);
    }

    Real AnalyticBinaryBarrierEngine_helper::payoffAtExpiry(
         Real spot, Real variance, Real discount)
    {
        Rate dividendDiscount =
            process_->dividendYield()->discount(exercise_->lastDate());

        QL_REQUIRE(spot>0.0,
                   "positive spot value required");

        QL_REQUIRE(discount>0.0,
                   "positive discount required");

        QL_REQUIRE(dividendDiscount>0.0,
                   "positive dividend discount required");

        QL_REQUIRE(variance>=0.0,
                   "negative variance not allowed");

        Option::Type type   = payoff_->optionType();
        Real strike = payoff_->strike();
        Real barrier = arguments_.barrier;
        QL_REQUIRE(barrier>0.0,
                   "positive barrier value required");
        Barrier::Type barrierType = arguments_.barrierType;

        Real stdDev = std::sqrt(variance);
        Real mu = std::log(dividendDiscount/discount)/variance - 0.5;
        Real K = 0;

        // binary cash-or-nothing payoff?
        std::shared_ptr<CashOrNothingPayoff> coo =
            std::dynamic_pointer_cast<CashOrNothingPayoff>(payoff_);
        if (coo != nullptr) {
            K = coo->cashPayoff();
        }

        // binary asset-or-nothing payoff?
        std::shared_ptr<AssetOrNothingPayoff> aoo =
            std::dynamic_pointer_cast<AssetOrNothingPayoff>(payoff_);
        if (aoo != nullptr) {
            mu += 1.0; 
            K = spot * dividendDiscount / discount; // forward
        }

        Real log_S_X = std::log(spot/strike);
        Real log_S_H = std::log(spot/barrier);
        Real log_H_S = std::log(barrier/spot);
        Real log_H2_SX = std::log(barrier*barrier/(spot*strike));
        Real H_S_2mu = std::pow(barrier/spot, 2*mu);

        Real eta = (barrierType == Barrier::DownIn ||
                    barrierType == Barrier::DownOut ? 1.0 : -1.0);
        Real phi = (type == Option::Call ? 1.0 : -1.0);

        Real x1, x2, y1, y2;
        Real cum_x1, cum_x2, cum_y1, cum_y2;
        if (variance>=QL_EPSILON) {

            // we calculate using mu*stddev instead of (mu+1)*stddev
            // because cash-or-nothing don't need it. asset-or-nothing
            // mu is really mu+1
            x1 = phi*(log_S_X/stdDev + mu*stdDev);
            x2 = phi*(log_S_H/stdDev + mu*stdDev);
            y1 = eta*(log_H2_SX/stdDev + mu*stdDev);
            y2 = eta*(log_H_S/stdDev + mu*stdDev);

            CumulativeNormalDistribution f;
            cum_x1 = f(x1);
            cum_x2 = f(x2);
            cum_y1 = f(y1);
            cum_y2 = f(y2);
        } else {
            if (log_S_X>0)
                cum_x1= 1.0;
            else
                cum_x1= 0.0;
            if (log_S_H>0)
                cum_x2= 1.0;
            else
                cum_x2= 0.0;
            if (log_H2_SX>0)
                cum_y1= 1.0;
            else
                cum_y1= 0.0;
            if (log_H_S>0)
                cum_y2= 1.0;
            else
                cum_y2= 0.0;
        }

        Real alpha = 0;

        switch (barrierType) {
            case Barrier::DownIn:
               if (type == Option::Call) {
                  // down-in and call
                  if (strike >= barrier) {
                     // B3 (eta=1, phi=1)
                     alpha = H_S_2mu * cum_y1;  
                  } else {
                     // B1-B2+B4 (eta=1, phi=1)
                     alpha = cum_x1 - cum_x2 + H_S_2mu * cum_y2; 
                  }
               }
               else {
                  // down-in and put 
                  if (strike >= barrier) {
                     // B2-B3+B4 (eta=1, phi=-1)
                     alpha = cum_x2 + H_S_2mu*(-cum_y1 + cum_y2);
                  } else {
                     // B1 (eta=1, phi=-1)
                     alpha = cum_x1;
                  }
               }
               break;

            case Barrier::UpIn:
               if (type == Option::Call) {
                  // up-in and call
                  if (strike >= barrier) {
                     // B1 (eta=-1, phi=1)
                     alpha = cum_x1;  
                  } else {
                     // B2-B3+B4 (eta=-1, phi=1)
                     alpha = cum_x2 + H_S_2mu * (-cum_y1 + cum_y2);
                  }
               }
               else {
                  // up-in and put 
                  if (strike >= barrier) {
                     // B1-B2+B4 (eta=-1, phi=-1)
                     alpha = cum_x1 - cum_x2 + H_S_2mu * cum_y2;
                  } else {
                     // B3 (eta=-1, phi=-1)
                     alpha = H_S_2mu * cum_y1;  
                  }
               }
               break;

            case Barrier::DownOut:
               if (type == Option::Call) {
                  // down-out and call
                  if (strike >= barrier) {
                     // B1-B3 (eta=1, phi=1)
                     alpha = cum_x1 - H_S_2mu * cum_y1; 
                  } else {
                     // B2-B4 (eta=1, phi=1)
                     alpha = cum_x2 - H_S_2mu * cum_y2; 
                  }
               }
               else {
                  // down-out and put 
                  if (strike >= barrier) {
                     // B1-B2+B3-B4 (eta=1, phi=-1)
                     alpha = cum_x1 - cum_x2 + H_S_2mu * (cum_y1-cum_y2);
                  } else {
                     // always 0
                     alpha = 0;  
                  }
               }
               break;
            case Barrier::UpOut:
               if (type == Option::Call) {
                  // up-out and call
                  if (strike >= barrier) {
                     // always 0
                     alpha = 0;  
                  } else {
                     // B1-B2+B3-B4 (eta=-1, phi=1)
                     alpha = cum_x1 - cum_x2 + H_S_2mu * (cum_y1-cum_y2);
                  }
               }
               else {
                  // up-out and put 
                  if (strike >= barrier) {
                     // B2-B4 (eta=-1, phi=-1)
                     alpha = cum_x2 - H_S_2mu * cum_y2;
                  } else {
                     // B1-B3 (eta=-1, phi=-1)
                     alpha = cum_x1 - H_S_2mu * cum_y1;
                  }
               }
               break;
            default:
                QL_FAIL("invalid barrier type");
        }

        return discount * K * alpha;
    }



}

