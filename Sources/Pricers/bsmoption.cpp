
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

/*! \file bsmoption.cpp
    \brief common code for option evaluation

    $Source$
    $Log$
    Revision 1.32  2001/05/09 11:06:19  nando
    A few comments modified/removed

    Revision 1.31  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.30  2001/04/06 18:46:22  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.29  2001/04/06 16:12:18  marmar
    Bug fixed in multi-period option

    Revision 1.28  2001/04/05 07:57:46  marmar
    One bug fixed in bermudan option, theta, rho, and vega  still not working

    Revision 1.27  2001/04/04 13:32:54  enri
    tons of typos fixed

    Revision 1.26  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.25  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

*/

#include "ql/Pricers/bsmoption.hpp"
#include "ql/Solvers1D/brent.hpp"

namespace QuantLib {

    namespace Pricers {

        const double BSMOption::dVolMultiplier_ = 0.0001;
        const double BSMOption::dRMultiplier_ = 0.0001;

        BSMOption::BSMOption(Type type, double underlying, double strike,
            Rate dividendYield, Rate riskFreeRate, Time residualTime,
            double volatility): type_(type), underlying_(underlying),
            strike_(strike), dividendYield_(dividendYield),
            rhoComputed_(false), vegaComputed_(false),
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
            hasBeenCalculated_ = false;
        }

        void BSMOption::setRiskFreeRate(Rate newRiskFreeRate) {
            riskFreeRate_ = newRiskFreeRate;
            hasBeenCalculated_ = false;
        }

        double BSMOption::vega() const {

            if(!vegaComputed_){

                double valuePlus = value();

                Handle<BSMOption> brandNewFD = clone();
                double volMinus = volatility_ * (1.0 - dVolMultiplier_);
                brandNewFD -> setVolatility(volMinus);
                double valueMinus = brandNewFD -> value();

                vega_ = (valuePlus - valueMinus )/
                        (volatility_ * dVolMultiplier_);
                vegaComputed_ = true;
            }
            return vega_;
        }

        double BSMOption::rho() const {

            if(!rhoComputed_){
                double valuePlus = value();

                Handle<BSMOption> brandNewFD = clone();
                Rate rMinus=riskFreeRate_ * (1.0 - dRMultiplier_);
                brandNewFD -> setRiskFreeRate(rMinus);
                double valueMinus = brandNewFD -> value();

                rho_=(valuePlus - valueMinus) /
                    (riskFreeRate_ * dRMultiplier_);
                rhoComputed_  = true;
            }
            return rho_;
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
