
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

/*! \file averagepriceasian.cpp

    \fullpath
    Sources/Pricers/%averagepriceasian.cpp
    \brief example of Monte Carlo pricer using a control variate

*/

// $Id$
// $Log$
// Revision 1.20  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.19  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.18  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.17  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.16  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.15  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.14  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.13  2001/07/13 15:25:13  marmar
// MonteCarlo interface changed
//
// Revision 1.12  2001/06/22 13:19:21  nando
// 80 colums limit enforced
//
// Revision 1.11  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/MonteCarlo/gaussianpathgenerator.hpp"
#include "ql/Pricers/averagepriceasian.hpp"
#include "ql/MonteCarlo/controlvariatedpathpricer.hpp"
#include "ql/MonteCarlo/avgpriceasianpathpricer.hpp"
#include "ql/MonteCarlo/geometricasianpathpricer.hpp"
#include "ql/Pricers/geometricasianoption.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::OneFactorMonteCarloOption;
        using MonteCarlo::PathPricer;
        using MonteCarlo::GaussianPathGenerator;
        using MonteCarlo::ControlVariatedPathPricer;
        using MonteCarlo::AveragePriceAsianPathPricer;
        using MonteCarlo::GeometricAsianPathPricer;

        AveragePriceAsian::AveragePriceAsian(Option::Type type,
                                             double       underlying,
                                             double       strike,
                                             Rate         dividendYield,
                                             Rate         riskFreeRate,
                                             double       residualTime,
                                             double       volatility,
                                             int          timesteps,
                                             long         samples,
                                             long         seed)
        : McPricer(samples, seed) {
            //! Initialize the path generator
            double deltaT = residualTime/timesteps;
            double mu = deltaT * (riskFreeRate - dividendYield
                                    - 0.5 * volatility * volatility);
            double variance = volatility*volatility*deltaT;

            Handle<GaussianPathGenerator> pathGenerator(
                    new GaussianPathGenerator(timesteps, mu, variance, seed));

            //! Initialize the pricer on the single Path
            Handle<PathPricer> spPricer(
                new AveragePriceAsianPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            Handle<PathPricer> controlVariateSpPricer(
                new GeometricAsianPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            double controlVariatePrice = GeometricAsianOption(type, underlying,
                strike, dividendYield, riskFreeRate, residualTime,
                volatility).value();

            Handle<PathPricer> controlVariatedPricer(
                new ControlVariatedPathPricer(spPricer,
                    controlVariateSpPricer, controlVariatePrice));

            //! Initialize the one-dimensional Monte Carlo
            montecarloPricer_ = OneFactorMonteCarloOption(
                pathGenerator, controlVariatedPricer,
                Math::Statistics());
        }

    }

}
