
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file himalaya.cpp

    $Source$
    $Log$
    Revision 1.10  2001/05/24 13:57:52  nando
    smoothing #include xx.hpp and cutting old Log messages

*/

#include "ql/handle.hpp"
#include "ql/MonteCarlo/himalayapathpricer.hpp"
#include "ql/Pricers/himalaya.hpp"

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
            QL_REQUIRE(timeDelays.size() >= 1,
                "Himalaya: you must have at least one time-step");

            //! Initialize the path generator
            Array mu(riskFreeRate - dividendYield
                                    - 0.5 * covariance.diagonal());

            Handle<StandardMultiPathGenerator> pathGenerator(
                new StandardMultiPathGenerator(timeDelays,
                                               covariance,
                                               mu,
                                               seed));
            double residualTime = timeDelays[timeDelays.size()-1];

            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(new HimalayaPathPricer(
                underlying, strike, QL_EXP(-riskFreeRate*residualTime)));

             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer);
        }

    }

}
