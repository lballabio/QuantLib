
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file shoutoption.h
    \brief shout option

    $Source$
    $Log$
    Revision 1.1  2001/03/02 08:36:44  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.


*/

#ifndef quantlib_pricers_shout_option_h
#define quantlib_pricers_shout_option_h

#include "qldefines.h"
#include "stepconditionoption.h"
#include "standardstepcondition.h"
#include "shoutcondition.h"

namespace QuantLib {
    namespace Pricers {
        class ShoutOption : public StepConditionOption {
        public:
            // constructor
            ShoutOption(Type type, double underlying, double strike, 
                           Rate dividendYield, Rate riskFreeRate, Time residualTime, 
                           double volatility, int timeSteps, int gridPoints)
                : StepConditionOption(type, underlying, strike, dividendYield, 
                                      riskFreeRate, residualTime,
                                      volatility, timeSteps, gridPoints)
                {
                    stepCondition_ = Handle<FiniteDifferences::StandardStepCondition>(
                        new ShoutCondition(initialPrices_,
                                           residualTime, riskFreeRate));   
                }
            // This method must be implemented to imply volatilities
            Handle<BSMOption> clone() const{    
                return Handle<BSMOption>(new ShoutOption(*this));
            }
        };
    }
}


#endif
