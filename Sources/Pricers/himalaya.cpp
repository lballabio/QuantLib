/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file himalaya.cpp
    
    $Source$
    $Log$
    Revision 1.2  2001/02/13 10:02:57  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.1  2001/02/07 10:15:09  marmar
    Himalaya-type option pricer

*/

#include "handle.h"
#include "himalayapathpricer.h"
#include "himalaya.h"

namespace QuantLib {

    namespace Pricers {
        
        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::StandardMultiPathGenerator;
        using MonteCarlo::HimalayaPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        Himalaya::Himalaya(const Array &underlying, const Array 
            &dividendYield, const Math::Matrix &covariance, 
            Rate riskFreeRate, double strike,
            const std::vector<Time> &timeDelays, long samples, long seed)
        : MultiFactorPricer(samples, seed){
            int  n = covariance.rows(); 
            QL_REQUIRE(covariance.columns() == n,
                "Himalaya: covariance matrix not square");
            QL_REQUIRE(underlying.size() == n,
                "Himalaya: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(dividendYield.size() == n,
                "Himalaya: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(timeDelays.size() == n,
                "Himalaya: size of time-stpes diffrent form that of assets");
                
            //! Initialize the path generator
            Array mu(riskFreeRate - dividendYield
                                    - 0.5 * covariance.diagonal());

            Handle<StandardMultiPathGenerator> pathGenerator(
                new StandardMultiPathGenerator(timeDelays, mu, 
                                        covariance, seed));
            double residualTime = std::accumulate(
                            timeDelays.begin(), timeDelays.end(), 0.0);
            
            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(new HimalayaPathPricer(
                underlying, strike, QL_EXP(-riskFreeRate*residualTime)));
 
             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer);
        }

    }

}
