
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

    $Id$
*/

// $Source$
// $Log$
// Revision 1.35  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

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
            // clone used for root finding
            Handle<BSMOption> tempBSM = clone();
            // objective function
            BSMFunction bsmf(tempBSM, targetValue);
            // solver
            Solvers1D::Brent s1d = Solvers1D::Brent();
            s1d.setMaxEvaluations(maxEvaluations);
            s1d.setLowBound(minVol);
            s1d.setHiBound(maxVol);

            return s1d.solve(bsmf, accuracy, volatility_, minVol, maxVol);
        }

    }

}
