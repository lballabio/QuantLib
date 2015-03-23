/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Thema Consulting SA (Programmer: Riccardo Ghetta)

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

#include <ql/pricingengines/barrier/analyticdoublebarrierbinaryengine.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

   // number of iterations ...
   static const int BINARY_DOUBLE_MAXITER = 100;
   static Real PI= 3.14159265358979324;

    // calc helper object 
    class AnalyticDoubleBarrierBinaryEngine_helper
    {
    
    public:
        AnalyticDoubleBarrierBinaryEngine_helper(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const boost::shared_ptr<CashOrNothingPayoff> &payoff,
             const boost::shared_ptr<AmericanExercise> &exercise,
             const DoubleBarrierOption::arguments &arguments):
        process_(process),
        payoff_(payoff),
        exercise_(exercise),
        arguments_(arguments)
        {
        }

        Real payoffAtExpiry(Real spot, Real variance);
    private:

        const boost::shared_ptr<GeneralizedBlackScholesProcess>& process_;
        const boost::shared_ptr<CashOrNothingPayoff> &payoff_;
        const boost::shared_ptr<AmericanExercise> &exercise_;
        const DoubleBarrierOption::arguments &arguments_;
    };


    // helper object methods
    Real AnalyticDoubleBarrierBinaryEngine_helper::payoffAtExpiry(
         Real spot, Real variance)
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
        DoubleBarrier::Type barrierType = arguments_.barrierType;

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

        Real tot = 0;
        for (int i = 1 ; i < BINARY_DOUBLE_MAXITER ; ++i)
        {
           Real term1 = (lo_alpha-std::pow(-1.0, i)*hi_alpha) /
                              (std::pow(alpha,2)+std::pow(i*PI/Z, 2));
           Real term2 = std::sin(i*PI/Z * std::log(spot/barrier_lo));
           Real term3 = std::exp(-0.5*(std::pow(i*PI/Z,2)-beta)*variance);
           tot += factor * i * term1 * term2 * term3;
        }

        if (barrierType == DoubleBarrier::KnockOut)
           return tot; // KO
        else {
           Rate discount = process_->riskFreeRate()->discount(
                                             arguments_.exercise->lastDate());
           QL_REQUIRE(discount>0.0,
                        "positive discount required");
           return cash * discount - tot; // KI
        }
    }


    AnalyticDoubleBarrierBinaryEngine::AnalyticDoubleBarrierBinaryEngine(
              const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

    void AnalyticDoubleBarrierBinaryEngine::calculate() const {

        boost::shared_ptr<AmericanExercise> ex =
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(ex->payoffAtExpiry(), "payoff must be at expiry");
        QL_REQUIRE(ex->dates()[0] <=
                   process_->blackVolatility()->referenceDate(),
                   "American option with window exercise not handled yet");

        boost::shared_ptr<CashOrNothingPayoff> payoff =
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "a cash-or-nothing payoff must be given");

        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        Real variance =
            process_->blackVolatility()->blackVariance(ex->lastDate(),
                                                       payoff->strike());
        Real barrier_lo = arguments_.barrier_lo;
        QL_REQUIRE(barrier_lo>0.0,
                   "positive low barrier value required");
        Real barrier_hi = arguments_.barrier_hi;
        QL_REQUIRE(barrier_hi>0.0,
                   "positive high barrier value required");
        QL_REQUIRE(barrier_lo <= barrier_hi,
                   "barrier_lo must be <= barrier_hi");
        DoubleBarrier::Type barrierType = arguments_.barrierType;

        // KO degenerate cases
        if ( barrierType == DoubleBarrier::KnockOut &&
             (spot <= barrier_lo || spot >= barrier_hi))
        {
            // knocked out, no value
            results_.value = 0;
            results_.delta = 0;
            results_.gamma = 0;
            results_.vega = 0;
            results_.rho = 0;
            return;
        }

        // KI degenerate cases
        if ( barrierType == DoubleBarrier::KnockIn &&
             (spot <= barrier_lo || spot >= barrier_hi)) {
            // knocked in - pays
            results_.value = payoff->cashPayoff();
            results_.delta = 0;
            results_.gamma = 0;
            results_.vega = 0;
            results_.rho = 0;
            return;
        }

        AnalyticDoubleBarrierBinaryEngine_helper helper(process_,
           payoff, ex, arguments_);
        results_.value = helper.payoffAtExpiry(spot, variance);
    }

}

