/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

#include <ql/experimental/barrieroption/analyticdoublebarrierbinaryengine.hpp>
#include <ql/exercise.hpp>

using std::fabs;

namespace QuantLib {

   // number of iterations ...
   static Real PI= 3.14159265358979323846264338327950;

    // calc helper object 
    class AnalyticDoubleBarrierBinaryEngine_helper
    {
    
    public:
        AnalyticDoubleBarrierBinaryEngine_helper(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const boost::shared_ptr<CashOrNothingPayoff> &payoff,
             const boost::shared_ptr<Exercise> &exercise,
             const DoubleBarrierOption::arguments &arguments):
        process_(process),
        payoff_(payoff),
        exercise_(exercise),
        arguments_(arguments)
        {
        }

        Real payoffAtExpiry(Real spot, Real variance,
                            DoubleBarrier::Type barrierType,
                            Size maxIteration = 100,
                            Real requiredConvergence = 1e-8);
        Real payoffKIKO(Real spot, Real variance,
                        DoubleBarrier::Type barrierType,
                        Size maxIteration = 1000,
                        Real requiredConvergence = 1e-8);

    private:

        const boost::shared_ptr<GeneralizedBlackScholesProcess>& process_;
        const boost::shared_ptr<CashOrNothingPayoff> &payoff_;
        const boost::shared_ptr<Exercise> &exercise_;
        const DoubleBarrierOption::arguments &arguments_;
    };


    // helper object methods
    Real AnalyticDoubleBarrierBinaryEngine_helper::payoffAtExpiry(
         Real spot, Real variance, DoubleBarrier::Type barrierType,
         Size maxIteration, Real requiredConvergence)
    {
        QL_REQUIRE(spot>0.0,
                   "positive spot value required");

        QL_REQUIRE(variance>=0.0,
                   "negative variance not allowed");

        Time residualTime = process_->time(arguments_.exercise->lastDate());
        QL_REQUIRE(residualTime>0.0,
                   "expiration time must be > 0");

        Option::Type type   = payoff_->optionType();
        Real strike = payoff_->strike();
        Real cash = payoff_->cashPayoff();
        Real barrier_lo = arguments_.barrier_lo;
        Real barrier_hi = arguments_.barrier_hi;

        Real sigmaq = variance/residualTime;
        Real r = process_->riskFreeRate()->zeroRate(residualTime, Continuous,
                                             NoFrequency);
        Real q = process_->dividendYield()->zeroRate(residualTime,
                                                   Continuous, NoFrequency);
        Real b = r - q;

        Real alpha = -0.5 * ( 2*b/sigmaq - 1);
        Real beta = -0.25 * std::pow(( 2*b/sigmaq - 1), 2) - 2 * r/sigmaq;
        Real Z = std::log(barrier_hi / barrier_lo);
        Real factor = ((2*PI*cash)/std::pow(Z,2)); // common factor
        Real lo_alpha = std::pow(spot/barrier_lo, alpha); 
        Real hi_alpha = std::pow(spot/barrier_hi, alpha); 

        Real tot = 0, term = 0;
        for (Size i = 1 ; i < maxIteration ; ++i)
        {
           Real term1 = (lo_alpha-std::pow(-1.0, (int)i)*hi_alpha) /
                              (std::pow(alpha,2)+std::pow(i*PI/Z, 2));
           Real term2 = std::sin(i*PI/Z * std::log(spot/barrier_lo));
           Real term3 = std::exp(-0.5*(std::pow(i*PI/Z,2)-beta)*variance);
           term = factor * i * term1 * term2 * term3;
           tot += term;
        }

        // Check if convergence is sufficiently fast (for extreme parameters with big alpha the convergence can be very
        // poor, see for example Hui "One-touch double barrier binary option value")
        QL_REQUIRE(std::fabs(term) < requiredConvergence, "serie did not converge sufficiently fast");

        if (barrierType == DoubleBarrier::KnockOut)
           return std::max(tot, 0.0); // KO
        else {
           Rate discount = process_->riskFreeRate()->discount(
                                             arguments_.exercise->lastDate());
           QL_REQUIRE(discount>0.0,
                        "positive discount required");
           return std::max(cash * discount - tot, 0.0); // KI
        }
    }

    // helper object methods
    Real AnalyticDoubleBarrierBinaryEngine_helper::payoffKIKO(
         Real spot, Real variance, DoubleBarrier::Type barrierType,
         Size maxIteration, Real requiredConvergence)
    {
        QL_REQUIRE(spot>0.0,
                   "positive spot value required");

        QL_REQUIRE(variance>=0.0,
                   "negative variance not allowed");

        Time residualTime = process_->time(arguments_.exercise->lastDate());
        QL_REQUIRE(residualTime>0.0,
                   "expiration time must be > 0");

        Real cash = payoff_->cashPayoff();
        Real barrier_lo = arguments_.barrier_lo;
        Real barrier_hi = arguments_.barrier_hi;
        if (barrierType == DoubleBarrier::KOKI)
           std::swap(barrier_lo, barrier_hi);

        Real sigmaq = variance/residualTime;
        Real r = process_->riskFreeRate()->zeroRate(residualTime, Continuous,
                                             NoFrequency);
        Real q = process_->dividendYield()->zeroRate(residualTime,
                                                   Continuous, NoFrequency);
        Real b = r - q;

        Real alpha = -0.5 * ( 2*b/sigmaq - 1);
        Real beta = -0.25 * std::pow(( 2*b/sigmaq - 1), 2) - 2 * r/sigmaq;
        Real Z = std::log(barrier_hi / barrier_lo);
        Real log_S_L = std::log(spot / barrier_lo);

        Real tot = 0, term = 0;
        for (Size i = 1 ; i < maxIteration ; ++i)
        {
            Real factor = std::pow(i*PI/Z,2)-beta;
            Real term1 = (beta - std::pow(i*PI/Z,2) * std::exp(-0.5*factor*variance)) / factor;
            Real term2 = std::sin(i * PI/Z * log_S_L);
            term = (2.0/(i*PI)) * term1 * term2;
            tot += term;
        }
        tot += 1 - log_S_L / Z;
        tot *= cash*std::pow(spot/barrier_lo, alpha);

        // Check if convergence is sufficiently fast
        QL_REQUIRE(fabs(term) < requiredConvergence, "serie did not converge sufficiently fast");

        return std::max(tot, 0.0);
    }

    AnalyticDoubleBarrierBinaryEngine::AnalyticDoubleBarrierBinaryEngine(
              const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

    void AnalyticDoubleBarrierBinaryEngine::calculate() const {

        if (arguments_.barrierType == DoubleBarrier::KIKO ||
            arguments_.barrierType == DoubleBarrier::KOKI) {
            boost::shared_ptr<AmericanExercise> ex =
                boost::dynamic_pointer_cast<AmericanExercise>(
                                                   arguments_.exercise);
            QL_REQUIRE(ex, "KIKO/KOKI options must have American exercise");
            QL_REQUIRE(ex->dates()[0] <=
                       process_->blackVolatility()->referenceDate(),
                       "American option with window exercise not handled yet");
        } else {
            boost::shared_ptr<EuropeanExercise> ex =
                boost::dynamic_pointer_cast<EuropeanExercise>(
                                                   arguments_.exercise);
            QL_REQUIRE(ex, "non-European exercise given");
        }
        boost::shared_ptr<CashOrNothingPayoff> payoff =
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "a cash-or-nothing payoff must be given");

        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        Real variance =
            process_->blackVolatility()->blackVariance(
                                             arguments_.exercise->lastDate(),
                                             payoff->strike());
        Real barrier_lo = arguments_.barrier_lo;
        Real barrier_hi = arguments_.barrier_hi;
        DoubleBarrier::Type barrierType = arguments_.barrierType;
        QL_REQUIRE(barrier_lo>0.0,
                   "positive low barrier value required");
        QL_REQUIRE(barrier_hi>0.0,
                   "positive high barrier value required");
        QL_REQUIRE(barrier_lo < barrier_hi,
                   "barrier_lo must be < barrier_hi");
        QL_REQUIRE(barrierType == DoubleBarrier::KnockIn ||
                   barrierType == DoubleBarrier::KnockOut ||
                   barrierType == DoubleBarrier::KIKO ||
                   barrierType == DoubleBarrier::KOKI,
                   "Unsupported barrier type");

        // degenerate cases
        switch (barrierType) {
          case DoubleBarrier::KnockOut:
            if (spot <= barrier_lo || spot >= barrier_hi) {
                // knocked out, no value
                results_.value = 0;
                results_.delta = 0;
                results_.gamma = 0;
                results_.vega = 0;
                results_.rho = 0;
                return;
            }
            break;

          case DoubleBarrier::KnockIn:
            if (spot <= barrier_lo || spot >= barrier_hi) {
                // knocked in - pays
                results_.value = payoff->cashPayoff();
                results_.delta = 0;
                results_.gamma = 0;
                results_.vega = 0;
                results_.rho = 0;
                return;
            }
            break;

          case DoubleBarrier::KIKO:
            if (spot >= barrier_hi) {
                // knocked out, no value
                results_.value = 0;
                results_.delta = 0;
                results_.gamma = 0;
                results_.vega = 0;
                results_.rho = 0;
                return;
            } else if (spot <= barrier_lo) {
                // knocked in, pays
                results_.value = payoff->cashPayoff();
                results_.delta = 0;
                results_.gamma = 0;
                results_.vega = 0;
                results_.rho = 0;
                return;
            }
            break;

          case DoubleBarrier::KOKI:
            if (spot <= barrier_lo) {
                // knocked out, no value
                results_.value = 0;
                results_.delta = 0;
                results_.gamma = 0;
                results_.vega = 0;
                results_.rho = 0;
                return;
            } else if (spot >= barrier_hi) {
                // knocked in, pays
                results_.value = payoff->cashPayoff();
                results_.delta = 0;
                results_.gamma = 0;
                results_.vega = 0;
                results_.rho = 0;
                return;
            }
            break;
        }

        AnalyticDoubleBarrierBinaryEngine_helper helper(process_,
           payoff, arguments_.exercise, arguments_);
        switch (barrierType)
        {
          case DoubleBarrier::KnockOut:
          case DoubleBarrier::KnockIn:
            results_.value = helper.payoffAtExpiry(spot, variance, barrierType);
            break;

          case DoubleBarrier::KIKO:
          case DoubleBarrier::KOKI:
            results_.value = helper.payoffKIKO(spot, variance, barrierType);
            break;
        }
    }

}

