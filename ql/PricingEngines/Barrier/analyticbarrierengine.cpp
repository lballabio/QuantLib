
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file barrierengines.cpp
    \brief Barrier option engines
*/

#include <ql/PricingEngines/Barrier/analyticbarrierengine.hpp>

namespace QuantLib {

    void AnalyticBarrierEngine::calculate() const {

        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> payoff = 
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticAmericanBinaryEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> payoff = arguments_.payoff;
        #endif

        QL_REQUIRE(payoff->strike()>0.0,
            "AnalyticBarrierEngine::calculate() : "
            "strike must be positive");

        Barrier::Type barrierType = arguments_.barrierType;

        switch (payoff->optionType()) {
          case Option::Call:
            switch (barrierType) {
              case Barrier::DownIn:
                if (strike() >= barrier())
                    results_.value = C(1,1) + E(1);
                else
                    results_.value = A(1) - B(1) + D(1,1) + E(1);
                break;
              case Barrier::UpIn:
                if (strike() >= barrier())
                    results_.value = A(1) + E(-1);
                else
                    results_.value = B(1) - C(-1,1) + D(-1,1) + E(-1);
                break;
              case Barrier::DownOut:
                if (strike() >= barrier())
                    results_.value = A(1) - C(1,1) + F(1);
                else
                    results_.value = B(1) - D(1,1) + F(1);
                break;
              case Barrier::UpOut:
                if (strike() >= barrier())
                    results_.value = F(-1);
                else
                    results_.value = A(1) - B(1) + C(-1,1) - D(-1,1) + F(-1);
                break;
            }
            break;
          case Option::Put:
            switch (barrierType) {
              case Barrier::DownIn:
                if (strike() >= barrier())
                    results_.value = B(-1) - C(1,-1) + D(1,-1) + E(1);
                else
                    results_.value = A(-1) + E(1);
                break;
              case Barrier::UpIn:
                if (strike() >= barrier())
                    results_.value = A(-1) - B(-1) + D(-1,-1) + E(-1);
                else
                    results_.value = C(-1,-1) + E(-1);
                break;
              case Barrier::DownOut:
                if (strike() >= barrier())
                    results_.value = A(-1) - B(-1) + C(1,-1) - D(1,-1) + F(1);
                else
                    results_.value = F(1);
                break;
              case Barrier::UpOut:
                if (strike() >= barrier())
                    results_.value = B(-1) - D(-1,-1) + F(-1);
                else
                    results_.value = A(-1) - C(-1,-1) + F(-1);
                break;
            }
            break;
          case Option::Straddle:
            switch (barrierType) {
              case Barrier::DownIn:
                if (strike() >= barrier())
                    results_.value = C(1,1) + E(1) + B(-1) - 
                        C(1,-1) + D(1,-1) + E(1);
                else
                    results_.value = A(1) - B(1) + D(1,1) + 
                        E(1) + A(-1) + E(1);
                break;
              case Barrier::UpIn:
                if (strike() >= barrier())
                    results_.value = A(1) + E(-1) + A(-1) - 
                        B(-1) + D(-1,-1) + E(-1);
                else
                    results_.value = B(1) - C(-1,1) + D(-1,1) + 
                        E(-1) + C(-1,-1) + E(-1);
                break;
              case Barrier::DownOut:
                if (strike() >= barrier())
                    results_.value = A(1) - C(1,1) + F(1) + A(-1) - 
                        B(-1) + C(1,-1) - D(1,-1) + F(1);
                else
                    results_.value = B(1) - D(1,1) + F(1) + F(1);
                break;
              case Barrier::UpOut:
                if (strike() >= barrier())
                    results_.value = F(-1) + B(-1) - D(-1,-1) + F(-1);
                else
                    results_.value = A(1) - B(1) + C(-1,1) - D(-1,1) + 
                        F(-1) + A(-1) - C(-1,-1) + F(-1);
                break;
            }
            break;
          default:
            QL_FAIL("Option: unknown type");
        }
    }


    double AnalyticBarrierEngine::underlying() const {
        return arguments_.blackScholesProcess->stateVariable->value();
    }

    double AnalyticBarrierEngine::strike() const {
        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> payoff = 
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticAmericanBinaryEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> payoff = arguments_.payoff;
        #endif
        return payoff->strike();
    }

    Time AnalyticBarrierEngine::residualTime() const {
        return arguments_.blackScholesProcess->riskFreeTS->dayCounter().yearFraction(
            arguments_.blackScholesProcess->riskFreeTS->referenceDate(),
            arguments_.exercise->lastDate());
    }

    double AnalyticBarrierEngine::volatility() const {
        return arguments_.blackScholesProcess->volTS->blackVol(residualTime(), strike());
    }

    double AnalyticBarrierEngine::stdDeviation() const {
        return volatility() * QL_SQRT(residualTime());
    }

    double AnalyticBarrierEngine::barrier() const {
        return arguments_.barrier;
    }

    double AnalyticBarrierEngine::rebate() const {
        return arguments_.rebate;
    }

    Rate AnalyticBarrierEngine::riskFreeRate() const {
        return arguments_.blackScholesProcess->riskFreeTS->zeroYield(residualTime());
    }

    DiscountFactor AnalyticBarrierEngine::riskFreeDiscount() const {
        return arguments_.blackScholesProcess->riskFreeTS->discount(residualTime());
    }

    Rate AnalyticBarrierEngine::dividendYield() const {
        return arguments_.blackScholesProcess->dividendTS->zeroYield(residualTime());
    }

    DiscountFactor AnalyticBarrierEngine::dividendDiscount() const {
        return arguments_.blackScholesProcess->dividendTS->discount(residualTime());
    }

    double AnalyticBarrierEngine::mu() const {
        double vol = volatility();
        return (riskFreeRate() - dividendYield())/(vol * vol) - 0.5;
    }

    double AnalyticBarrierEngine::muSigma() const {
        return (1 + mu()) * stdDeviation();
    }

    double AnalyticBarrierEngine::A(double phi) const {
        double x1 = 
            QL_LOG(underlying()/strike())/stdDeviation() + muSigma();
        double N1 = f_(phi*x1);
        double N2 = f_(phi*(x1-stdDeviation()));
        return phi*(underlying() * dividendDiscount() * N1
                    - strike() * riskFreeDiscount() * N2);
    }

    double AnalyticBarrierEngine::B(double phi) const {
        double x2 = 
            QL_LOG(underlying()/barrier())/stdDeviation() + muSigma();
        double N1 = f_(phi*x2);
        double N2 = f_(phi*(x2-stdDeviation()));
        return phi*(underlying() * dividendDiscount() * N1
                    - strike() * riskFreeDiscount() * N2);
    }

    double AnalyticBarrierEngine::C(double eta, double phi) const {
        double HS = barrier()/underlying();
        double powHS0 = QL_POW(HS, 2 * mu());
        double powHS1 = powHS0 * HS * HS;
        double y1 = 
            QL_LOG(barrier()*HS/strike())/stdDeviation() + muSigma();
        double N1 = f_(eta*y1);
        double N2 = f_(eta*(y1-stdDeviation()));
        return phi*(underlying() * dividendDiscount() * powHS1 * N1
                    - strike() * riskFreeDiscount() * powHS0 * N2);
    }

    double AnalyticBarrierEngine::D(double eta, double phi) const {
        double HS = barrier()/underlying();
        double powHS0 = QL_POW(HS, 2 * mu());
        double powHS1 = powHS0 * HS * HS;
        double y2 = 
            QL_LOG(barrier()/underlying())/stdDeviation() + muSigma();
        double N1 = f_(eta*y2);
        double N2 = f_(eta*(y2-stdDeviation()));
        return phi*(underlying() * dividendDiscount() * powHS1 * N1
                    - strike() * riskFreeDiscount() * powHS0 * N2);
    }

    double AnalyticBarrierEngine::E(double eta) const {
        if (rebate() > 0) {
            double powHS0 = QL_POW(barrier()/underlying(), 2 * mu());
            double x2 = 
                QL_LOG(underlying()/barrier())/stdDeviation() + muSigma();
            double y2 = 
                QL_LOG(barrier()/underlying())/stdDeviation() + muSigma();
            double N1 = f_(eta*(x2 - stdDeviation()));
            double N2 = f_(eta*(y2 - stdDeviation()));
            return rebate() * riskFreeDiscount() * (N1 - powHS0 * N2);
        } else {
            return 0.0;
        }
    }

    double AnalyticBarrierEngine::F(double eta) const {
        if (rebate() > 0) {
            double m = mu();
            double vol = volatility();
            double lambda = QL_SQRT(m*m + 2.0*riskFreeRate()/(vol * vol));
            double HS = barrier()/underlying();
            double powHSplus = QL_POW(HS, m + lambda);
            double powHSminus = QL_POW(HS, m - lambda);

            double sigmaSqrtT = stdDeviation();
            double z = QL_LOG(barrier()/underlying())/sigmaSqrtT
                + lambda * sigmaSqrtT;

            double N1 = f_(eta * z);
            double N2 = f_(eta * (z - 2.0 * lambda * sigmaSqrtT));
            return rebate() * (powHSplus * N1 + powHSminus * N2);
        } else {
            return 0.0;
        }
    }

}

