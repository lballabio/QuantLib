
/*
 Copyright (C) 2003 Neil Firth

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

/*! \file analyticamericanengine.cpp
    \brief Digital payoff American option engine using analytic formulas
*/

#include <ql/PricingEngines/Vanilla/vanillaengines.hpp>

namespace QuantLib {

    void AnalyticAmericanEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "AnalyticAmericanEngine::calculate() : "
                   "not an American Option");

        #if defined(HAVE_BOOST)
        Handle<AmericanExercise> ex = 
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex,
                   "AnalyticAmericanEngine: non-American exercise given");
        #else
        Handle<AmericanExercise> ex = arguments_.exercise;
        #endif
        QL_REQUIRE(!ex->payoffAtExpiry(),
                   "AnalyticAmericanEngine::calculate() : "
                   "payoff at expiry not handled yet");

        #if defined(HAVE_BOOST)
        Handle<CashOrNothingPayoff> payoff = 
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticAmericanEngine: the payoff given is not Cash-Or-Nothing");
        #else
        Handle<CashOrNothingPayoff> payoff = arguments_.payoff;
        #endif

        double cashPayoff = payoff->cashPayoff();

        double underlying = arguments_.underlying;

        double strike = payoff->strike();
        double variance = arguments_.volTS->blackVariance(
            ex->lastDate(), strike);

        Rate dividendDiscount = arguments_.dividendTS->discount(
            ex->lastDate());

        Rate riskFreeDiscount = arguments_.riskFreeTS->discount(
            ex->lastDate());

        double b_temp = QL_LOG(dividendDiscount/riskFreeDiscount) - 0.5*variance;
        double mu = b_temp/variance;
        // numerically is this the best way to calculate the square root.
        // check in Numerical Recipes.            
        double lambda = QL_SQRT(mu*mu+2.0*QL_LOG(dividendDiscount/riskFreeDiscount)/variance);
        double l_plus = mu + lambda;
        double l_minus = mu - lambda;
        double root_two_pi = M_SQRT2 * M_SQRTPI;
        double log_H_S = QL_LOG (payoff->strike()/underlying);
        double z_temp = lambda*QL_SQRT(variance);
        double z = (log_H_S/QL_SQRT(variance)) + z_temp;
        double zbar = z - 2*z_temp; 

        double pow_plus = QL_POW (payoff->strike()/underlying, l_plus);
        double pow_minus = QL_POW (payoff->strike()/underlying, l_minus);

        CumulativeNormalDistribution f;

        // up option, or call
        if (arguments_.underlying < payoff->strike()) {
            double f_minus_z = f(-z);
            double f_minus_zbar = f(-zbar);
            double mod_exp_z2 = QL_EXP(-z*z/2);
            double mod_exp_zbar2 = QL_EXP(-zbar*zbar/2); 
            double denom_delta = underlying * QL_SQRT(variance) * root_two_pi;

            results_.value = cashPayoff*(pow_plus *f_minus_z 
                                         + pow_minus *f_minus_zbar);

            results_.delta = 
                cashPayoff*(pow_minus*((QL_EXP(-zbar*zbar/2)) / denom_delta -
                                       l_minus * f_minus_zbar / underlying)
                            + pow_plus * ((QL_EXP(-z*z/2)) / denom_delta -
                                          l_plus * f_minus_z / underlying));

            Time tRho = arguments_.riskFreeTS->dayCounter().yearFraction(
                arguments_.riskFreeTS->referenceDate(),
                arguments_.exercise->lastDate());
            double denom_rho = lambda*QL_SQRT(variance/tRho)*root_two_pi;
            results_.rho = cashPayoff*
                (pow_plus * (-QL_SQRT(tRho)*(mu+1)*mod_exp_z2/denom_rho +
                ((1+((mu+1)/lambda))*log_H_S * f_minus_z / (variance/tRho)))
                +pow_minus*(QL_SQRT(tRho)*(mu+1)*mod_exp_zbar2/denom_rho
                + ((1-((mu+1)/lambda))*log_H_S 
                * f_minus_zbar / (variance/tRho))));


            // down option, or put
        } else {
            double f_z = f(z);
            double f_zbar = f(zbar);
            double mod_exp_z2 = QL_EXP(-z*z/2);
            double mod_exp_zbar2 = QL_EXP(-zbar*zbar/2);
            double denom_delta = underlying*QL_SQRT(variance)*root_two_pi;

            results_.value = cashPayoff*(pow_plus * f_z + pow_minus * f_zbar);

            results_.delta = 
                cashPayoff*(-pow_minus*(mod_exp_zbar2/denom_delta
                                        + l_minus * f_zbar / underlying)
                            -pow_plus * (mod_exp_z2/denom_delta
                                         + l_plus * f_z / underlying));

            Time tRho = arguments_.riskFreeTS->dayCounter().yearFraction(
                arguments_.riskFreeTS->referenceDate(),
                arguments_.exercise->lastDate());
            double denom_rho = lambda*QL_SQRT(variance/tRho)*root_two_pi;
            results_.rho = 
                cashPayoff*(pow_plus*(-QL_SQRT(tRho)*(mu+1)*mod_exp_z2/denom_rho
                                      + ((1+((mu+1)/lambda))*log_H_S 
                                         * f_z / (variance/tRho)))
                            +pow_minus*(QL_SQRT(tRho)*(mu+1)*mod_exp_zbar2/denom_rho
                                        + ((1-((mu+1)/lambda))*log_H_S 
                                           * f_zbar / (variance/tRho)))); 
        }
    }

}

