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

/*! \file everestoption.cpp
    
    $Source$
    $Log$
    Revision 1.1  2001/03/06 17:02:04  marmar
    First, simplified version, of everest option introduced

*/

#include "handle.h"
#include "everestpathpricer.h"
#include "everestoption.h"

namespace QuantLib {

    namespace Pricers {
        
        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::StandardMultiPathGenerator;
        using MonteCarlo::EverestPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        EverestOption::EverestOption(const Array &underlying, 
                const Array &dividendYield, 
                const Math::Matrix &covariance, 
                Rate riskFreeRate, Time residualTime,
                long samples, long seed)
        : MultiFactorPricer(samples, seed){
            int  n = covariance.rows(); 
            QL_REQUIRE(covariance.columns() == n,
                "EverestOption: covariance matrix not square");
            QL_REQUIRE(underlying.size() == n,
                "EverestOption: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(dividendYield.size() == n,
                "EverestOption: dividendYield size does not match"
                " that of covariance matrix");
                
            //! Initialize the path generator
            Array mu(riskFreeRate - dividendYield
                                    - 0.5 * covariance.diagonal());

            Handle<StandardMultiPathGenerator> pathGenerator(
                new StandardMultiPathGenerator(1, mu, covariance, seed));
            
            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(new EverestPathPricer(
                underlying, QL_EXP(-riskFreeRate*residualTime)));
 
             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer);
        }

    }

}
