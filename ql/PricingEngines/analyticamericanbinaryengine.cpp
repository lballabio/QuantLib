
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

/*! \file analyticamericanbinaryengine.cpp
    \brief American binary option engine using analytic formulas
*/

#include <ql/PricingEngines/binaryengines.hpp>

namespace QuantLib {

    void AnalyticAmericanBinaryEngine::calculate() const {

        QL_REQUIRE(arguments_.exerciseType == Exercise::American,
                   "AnalyticAmericanBinaryEngine::calculate() : "
                   "not an American Option");

        QL_REQUIRE(arguments_.binaryType == Binary::CashAtHit,
                   "AnalyticAmericanBinaryEngine::calculate() : "
                   "not a CashAtHit Option");

        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> payoff = 
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticAmericanBinaryEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> payoff = arguments_.payoff;
        #endif

        // Binary::Type binaryType = arguments_.binaryType;
        double barrier = arguments_.barrier;
        double cashPayoff = arguments_.cashPayoff;

        double underlying = arguments_.underlying;
        Time maturity = arguments_.maturity;

        double strike = payoff->strike();
        double vol = arguments_.volTS->blackVol(arguments_.maturity, strike);

        Rate dividendRate =
            arguments_.dividendTS->zeroYield(arguments_.maturity);

        Rate riskFreeRate =
            arguments_.riskFreeTS->zeroYield(arguments_.maturity);

        double vol2 = vol*vol;
        double b_temp = riskFreeRate - dividendRate - 0.5*vol2;
        double mu = b_temp/vol2;
        // numerically is this the best way to calculate the square root.
        // check in Numerical Recipes.            
        double lambda = QL_SQRT(mu*mu+2*(riskFreeRate-dividendRate)/vol2);
        double l_plus = mu + lambda;
        double l_minus = mu - lambda;
        double root_tau = QL_SQRT (maturity);
        double root_two_pi = M_SQRT2 * M_SQRTPI;
        double log_H_S = QL_LOG (barrier/underlying);
        double z_temp = lambda*vol*root_tau;
        double z = (log_H_S/(vol*root_tau)) + z_temp;
        double zbar = z - 2*z_temp; 

        double pow_plus = QL_POW (barrier/underlying, l_plus);
        double pow_minus = QL_POW (barrier/underlying, l_minus);

        CumulativeNormalDistribution f;

        // up option, or call
        if (arguments_.underlying < arguments_.barrier) {
            double f_minus_z = f(-z);
            double f_minus_zbar = f(-zbar);
            double mod_exp_z2 = QL_EXP(-z*z/2);
            double mod_exp_zbar2 = QL_EXP(-zbar*zbar/2); 
            double denom_delta = underlying * root_tau * vol * root_two_pi;
            double denom_rho = lambda*vol*root_two_pi;

            results_.value = cashPayoff*(pow_plus *f_minus_z 
                                         + pow_minus *f_minus_zbar);

            results_.delta = 
                cashPayoff*(pow_minus*((QL_EXP(-zbar*zbar/2)) / denom_delta -
                                       l_minus * f_minus_zbar / underlying)
                            + pow_plus * ((QL_EXP(-z*z/2)) / denom_delta -
                                          l_plus * f_minus_z / underlying));

            results_.rho = 
                cashPayoff*(pow_plus*(-root_tau*(mu+1)*mod_exp_z2/denom_rho
                                      + ((1+((mu+1)/lambda))*log_H_S 
                                         * f_minus_z / vol2))
                            +pow_minus*(root_tau*(mu+1)*mod_exp_zbar2/denom_rho
                                        + ((1-((mu+1)/lambda))*log_H_S 
                                           * f_minus_zbar / vol2)));


            // down option, or put
        } else {
            double f_z = f(z);
            double f_zbar = f(zbar);
            double mod_exp_z2 = QL_EXP(-z*z/2);
            double mod_exp_zbar2 = QL_EXP(-zbar*zbar/2);
            double denom_delta = underlying*root_tau*vol*root_two_pi;
            double denom_rho = lambda*vol*root_two_pi;

            results_.value = cashPayoff*(pow_plus * f_z + pow_minus * f_zbar);

            results_.delta = 
                cashPayoff*(-pow_minus*(mod_exp_zbar2/denom_delta
                                        + l_minus * f_zbar / underlying)
                            -pow_plus * (mod_exp_z2/denom_delta
                                         + l_plus * f_z / underlying));

            results_.rho = 
                cashPayoff*(pow_plus*(-root_tau*(mu+1)*mod_exp_z2/denom_rho
                                      + ((1+((mu+1)/lambda))*log_H_S 
                                         * f_z / vol2))
                            +pow_minus*(root_tau*(mu+1)*mod_exp_zbar2/denom_rho
                                        + ((1-((mu+1)/lambda))*log_H_S 
                                           * f_zbar / vol2))); 
        }
    }

}

