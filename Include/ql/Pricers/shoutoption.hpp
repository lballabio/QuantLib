
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file shoutoption.hpp
    \brief shout option

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.7  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_pricers_shout_option_h
#define quantlib_pricers_shout_option_h

#include "ql/Pricers/stepconditionoption.hpp"
#include "ql/Pricers/shoutcondition.hpp"

namespace QuantLib {

    namespace Pricers {

        class ShoutOption : public StepConditionOption {
          public:
            // constructor
            ShoutOption(Type type, double underlying, double strike,
                           Rate dividendYield, Rate riskFreeRate,
                           Time residualTime, double volatility,
                           int timeSteps, int gridPoints);
                void initializeStepCondition() const;

            // This method must be implemented to imply volatilities
            Handle<SingleAssetOption> clone() const{
                return Handle<SingleAssetOption>(new ShoutOption(*this));
            }
        };


        // inline definitions
        
        inline ShoutOption::ShoutOption(Type type, double underlying,
            double strike, Rate dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility, int timeSteps,
            int gridPoints)
        : StepConditionOption(type, underlying, strike, dividendYield,
                             riskFreeRate, residualTime, volatility,
                             timeSteps, gridPoints){}

        inline void ShoutOption::initializeStepCondition() const {
            stepCondition_ = Handle<FiniteDifferences::StandardStepCondition>(
                new ShoutCondition(initialPrices_, residualTime_,
                    riskFreeRate_));
        }

    }

}


#endif

