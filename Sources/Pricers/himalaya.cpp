
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file himalaya.cpp

    \fullpath
    Sources/Pricers/%himalaya.cpp
    \brief Himalayan-type option pricer

*/

// $Id$
// $Log$
// Revision 1.20  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.19  2001/08/28 13:37:36  nando
// unsigned int instead of int
//
// Revision 1.18  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.17  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.16  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.15  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.14  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.13  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.12  2001/07/13 15:25:13  marmar
// MonteCarlo interface changed
//
// Revision 1.11  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/handle.hpp"
#include "ql/MonteCarlo/himalayapathpricer.hpp"
#include "ql/Pricers/himalaya.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::GaussianMultiPathGenerator;
        using MonteCarlo::HimalayaPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        Himalaya::Himalaya(const Array &underlying, const Array
            &dividendYield, const Math::Matrix &covariance,
            Rate riskFreeRate, double strike,
            const std::vector<Time> &timeDelays, long samples, long seed)
        : MultiFactorPricer(samples, seed){
            unsigned int  n = covariance.rows();
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

            Handle<GaussianMultiPathGenerator> pathGenerator(
                new GaussianMultiPathGenerator(timeDelays,
                                               covariance,
                                               mu,
                                               seed));
            double residualTime = timeDelays[timeDelays.size()-1];

            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(new HimalayaPathPricer(
                underlying, strike, QL_EXP(-riskFreeRate*residualTime)));

             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer,
                                        Math::Statistics());
        }

    }

}
