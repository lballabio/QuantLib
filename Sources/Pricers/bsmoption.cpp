
/*
 * Copyright (C) 2000, 2001
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
 *   http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file bsmoption.cpp
    \brief common code for option evaluation

    $Source$
    $Name$
    $Log$
    Revision 1.22  2001/02/19 12:19:29  marmar
    Added trailing _ to protected and private members

    Revision 1.21  2001/02/15 15:57:41  marmar
    Defined QL_MIN_VOLATILITY 0.0005 and
    QL_MAX_VOLATILITY 3.0

    Revision 1.20  2001/02/15 15:27:52  marmar
    Now impliedVol is called with default values for maxVolatility
    and minVolatility

    Revision 1.19  2001/02/15 11:56:54  nando
    impliedVol require targetValue>0.0

    Revision 1.18  2001/02/14 10:57:41  marmar
    BSMOption has  a cleaner constructor

    Revision 1.17  2001/01/17 14:37:57  nando
    tabs removed

    Revision 1.16  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.15  2000/12/20 17:00:59  enri
    modified to use new macros

    Revision 1.14  2000/12/14 12:40:14  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "bsmoption.h"
#include "brent.h"

namespace QuantLib {

    namespace Pricers {

        BSMOption::BSMOption(Type type, double underlying, double strike,
            Rate dividendYield, Rate riskFreeRate, Time residualTime,
            double volatility): type_(type), underlying_(underlying), 
            strike_(strike), dividendYield_(dividendYield), 
            residualTime_(residualTime), hasBeenCalculated_(false) {
            QL_REQUIRE(strike > 0.0,
                "BSMOption::BSMOption : strike must be positive");
            QL_REQUIRE(underlying > 0.0,
                "BSMOption::BSMOption : underlying must be positive");
            QL_REQUIRE(residualTime > 0.0,
                "BSMOption::BSMOption : residual time must be positive");
            //! Checks on volatility values are in setVolatility
            setVolatility(volatility); 
            //! Checks on the risk-free rate are in setRiskFreeRate
            setRiskFreeRate(riskFreeRate);
        }

        void BSMOption::setVolatility(double volatility) {
            QL_REQUIRE(volatility >= QL_MIN_VOLATILITY,
                 "BSMOption::setVolatility : Volatility to small");

            QL_REQUIRE(volatility <= QL_MAX_VOLATILITY, 
                "BSMOption::setVolatility : Volatility to high "
                "for a meaningful result");
 
            volatility_ = volatility;
            hasBeenCalculated_=false;
        }

        void BSMOption::setRiskFreeRate(Rate newRiskFreeRate) {
            riskFreeRate_ = newRiskFreeRate;
            hasBeenCalculated_ = false;
        }

        double BSMOption::impliedVolatility(double targetValue, double accuracy, 
                    int maxEvaluations, double minVol, double maxVol) const {
            // check option targetValue boundary condition
            QL_REQUIRE(targetValue > 0.0,
             "BSMOption::impliedVol : targetValue must be positive");
            // the following checks may be improved
             switch (type_) {
              case Call:
                QL_REQUIRE(targetValue <= underlying_,
                  "BSMOption::impliedVol : call option targetValue (" +
                  DoubleFormatter::toString(targetValue) +
                  ") > underlying value (" +
                  DoubleFormatter::toString(underlying_) + ") not allowed");
                break;
              case Put:
                QL_REQUIRE(targetValue <= strike_,
                  "BSMOption::impliedVol : put option targetValue (" +
                  DoubleFormatter::toString(targetValue) +
                  ") > strike value (" + DoubleFormatter::toString(strike_) +
                  ") not allowed");
                break;
              case Straddle:
                // to be verified
                QL_REQUIRE(targetValue < underlying_+strike_,
                  "BSMOption::impliedFlatVol : straddle option targetValue (" +
                  DoubleFormatter::toString(targetValue) +
                  ") >= (underlying+strike) value (" +
                  DoubleFormatter::toString(underlying_+strike_) +
                  ") not allowed");
                break;
              default:
                throw IllegalArgumentError("BSMOption: invalid option type");
            }
            // clone used for root finding
            Handle<BSMOption> tempBSM = clone();
            // objective function
            BSMFunction bsmf(tempBSM, targetValue);
            // solver
            Solvers1D::Brent s1d = Solvers1D::Brent();
            s1d.setMaxEvaluations(maxEvaluations);
            s1d.setLowBound(minVol);
            s1d.setHiBound(maxVol);

            return s1d.solve(bsmf, accuracy, volatility_, 0.05);
        }

    }

}
