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

/*! \file plainbasketoption.cpp
    
    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/03/28 13:40:52  marmar
    MultiPathGenerator now has a default for mean

    Revision 1.2  2001/02/13 10:02:57  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.1  2001/02/02 11:00:16  marmar
    Example of Monte Carlo pricer on mutiple assets

*/

#include "handle.h"
#include "rate.h"
#include "basketpathpricer.h"
#include "plainbasketoption.h"

namespace QuantLib {

    namespace Pricers {
        
        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::StandardMultiPathGenerator;
        using MonteCarlo::BasketPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        PlainBasketOption::PlainBasketOption(const Array &underlying, 
            const Array &dividendYield, const Math::Matrix &covariance, 
            Rate riskFreeRate,  double residualTime, int timesteps, 
            long samples, long seed)
        : MultiFactorPricer(samples, seed){
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "PlainBasketOption: covariance matrix not square");
            QL_REQUIRE(covariance.rows() == underlying.size(),
                "PlainBasketOption: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(covariance.rows() == dividendYield.size(),
                "PlainBasketOption: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(residualTime > 0,
                "PlainBasketOption: residual time must be positive");
            QL_REQUIRE(timesteps > 0,
                "PlainBasketOption: time steps must be positive");
                
            //! Initialize the path generator
            double deltaT = residualTime/timesteps;
            Array mu(deltaT * (riskFreeRate - dividendYield
                                    - 0.5 * covariance.diagonal()));

            Handle<StandardMultiPathGenerator> pathGenerator(
                new StandardMultiPathGenerator(timesteps, 
                                               covariance*deltaT, 
                                               mu, 
                                               seed));
            
            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(new BasketPathPricer(
                underlying, QL_EXP(-riskFreeRate*residualTime)));
 
             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer);
        }

    }

}
