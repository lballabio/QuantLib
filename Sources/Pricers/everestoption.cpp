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
    Revision 1.5  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.4  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.3  2001/03/28 13:40:52  marmar
    MultiPathGenerator now has a default for mean

    Revision 1.2  2001/03/07 09:32:54  marmar
    Spot prices not necessary for evaluation of everest option

    Revision 1.1  2001/03/06 17:02:04  marmar
    First, simplified version, of everest option introduced

*/

#include "handle.hpp"
#include "MonteCarlo/everestpathpricer.hpp"
#include "Pricers/everestoption.hpp"

namespace QuantLib {

    namespace Pricers {
        
        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::StandardMultiPathGenerator;
        using MonteCarlo::EverestPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        EverestOption::EverestOption(const Array &dividendYield, 
                                     const Math::Matrix &covariance, 
                                     Rate riskFreeRate, 
                                     Time residualTime,
                                     long samples, long seed)
        : MultiFactorPricer(samples, seed){
            int  n = covariance.rows(); 
            QL_REQUIRE(covariance.columns() == n,
                "EverestOption: covariance matrix not square");
            QL_REQUIRE(dividendYield.size() == n,
                "EverestOption: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(residualTime > 0,
                "EverestOption: residualTime must be positive");
                
            //! Initialize the path generator
            Array mu(riskFreeRate - dividendYield
                                    - 0.5 * covariance.diagonal());

            std::vector<Time> timeDisp(1);
            timeDisp[0] = residualTime;
            Handle<StandardMultiPathGenerator> pathGenerator(
                    new StandardMultiPathGenerator(timeDisp, 
                                                   covariance, 
                                                   mu,
                                                   seed));
            
            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(
                new EverestPathPricer(QL_EXP(-riskFreeRate*residualTime)));
 
             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer);
        }

    }

}
