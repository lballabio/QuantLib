
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

/*!  \file cliquetoption.cpp

    \fullpath
    Sources/Pricers/%cliquetoption.cpp
   \brief Textbook example of european-style multi-period option.

*/

// $Id$
// $Log$
// Revision 1.13  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.12  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.11  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.10  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.9  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.8  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Pricers/cliquetoption.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib
{
    namespace Pricers
    {
        CliquetOption::CliquetOption(Type type,
                                     double underlying,
                                     Rate dividendYield,
                                     Rate riskFreeRate,
                                     const std::vector<Time> &dates,
                                     double volatility)
        : SingleAssetOption(type, underlying, underlying, dividendYield,
                    riskFreeRate, dates[dates.size()-1], volatility),
        numPeriods_(dates.size()-1),
        optionlet_(numPeriods_),
        weight_(numPeriods_){

            QL_REQUIRE(numPeriods_ >= 1,
                       "At least two dates are required for cliquet options");

            for(int i = 0; i < numPeriods_; i++){
                weight_[i] = QL_EXP(dividendYield * dates[i]);
                optionlet_[i] = Handle<EuropeanOption>(
                    new EuropeanOption(type,
                                          underlying,
                                          underlying,
                                          dividendYield,
                                          riskFreeRate,
                                          dates[i+1] - dates[i],
                                          volatility));
            }

        }

        Handle<SingleAssetOption> CliquetOption::clone() const {
            return Handle<SingleAssetOption>(new CliquetOption(*this));
        }

        double CliquetOption::value() const {
            double optValue = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optValue += weight_[i] * optionlet_[i] -> value();
            return optValue;
        }

        double CliquetOption::delta() const {
            double optDelta = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optDelta += weight_[i] * optionlet_[i] -> delta();
            return optDelta;
        }

        double CliquetOption::gamma() const {
            double optGamma = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optGamma += weight_[i] * optionlet_[i] -> gamma();
            return optGamma;
        }

        double CliquetOption::theta() const {
            double optTheta = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optTheta += weight_[i] * optionlet_[i] -> theta();
            return optTheta;
        }

        double CliquetOption::rho() const {
            double optRho = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optRho += weight_[i] * optionlet_[i] -> rho();
            return optRho;
        }

        double CliquetOption::vega() const {
            double optVega = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optVega += weight_[i] * optionlet_[i] -> vega();
            return optVega;
        }

    }

}







