
/*
 * Copyright (C) 2000
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

/*! \file stepconditionoption.h
  \brief Base class for options requiring additional code to be executed at each timestep
  
  $Source$
  $Log$
  Revision 1.2  2001/03/02 13:49:42  marmar
  Purely virtual method initializeStepCondition()
  introduced in the design of StepConditionOption

  Revision 1.1  2001/03/02 08:36:44  enri
  Shout options added:
  	* BSMAmericanOption is now AmericanOption, same interface
  	* ShoutOption added
  	* both ShoutOption and AmericanOption inherit from
  	  StepConditionOption
  offline.doxy.linux added.

*/

#ifndef quantlib_pricers_stepconditionoption_h
#define quantlib_pricers_stepconditionoption_h

#include "qldefines.h"
#include "handle.h"
#include "bsmnumericaloption.h"
#include "standardstepcondition.h"
#include <algorithm>
#include <functional>

namespace QuantLib {
    namespace Pricers {
        class StepConditionOption : public BSMNumericalOption {
        public:
            double value() const;
        protected:
            // constructor
            StepConditionOption(Type type, double underlying, double strike, 
                                Rate dividendYield, Rate riskFreeRate, 
                                Time residualTime, double volatility, 
                                int timeSteps, int gridPoints);
            virtual void initializeStepCondition() const = 0;                                
            mutable Handle<FiniteDifferences::StandardStepCondition > 
                                                            stepCondition_;
            int timeSteps_;
        };
    }
}

#endif
