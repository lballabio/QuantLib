
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

/*! \file stepconditionoption.hpp
  \brief Base class for options requiring additional code to be executed at each timestep

  $Source$
  $Log$
  Revision 1.3  2001/04/06 18:46:20  nando
  changed Authors, Contributors, Licence and copyright header

  Revision 1.2  2001/04/04 12:13:23  nando
  Headers policy part 2:
  The Include directory is added to the compiler's include search path.
  Then both your code and user code specifies the sub-directory in
  #include directives, as in
  #include <Solvers1d/newton.hpp>

  Revision 1.1  2001/04/04 11:07:23  nando
  Headers policy part 1:
  Headers should have a .hpp (lowercase) filename extension
  All *.h renamed to *.hpp

  Revision 1.3  2001/03/21 11:33:02  marmar
  Main loop transfered from method value to method calculate.
  Methods vega and rho moved from BSMNumericalOption to BSMOption

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

#include "qldefines.hpp"
#include "handle.hpp"
#include "bsmnumericaloption.hpp"
#include "FiniteDifferences/standardstepcondition.hpp"
#include <algorithm>
#include <functional>

namespace QuantLib {
    namespace Pricers {
        class StepConditionOption : public BSMNumericalOption {
        protected:
            // constructor
            StepConditionOption(Type type, double underlying, double strike,
                                Rate dividendYield, Rate riskFreeRate,
                                Time residualTime, double volatility,
                                int timeSteps, int gridPoints);
            void calculate() const;
            virtual void initializeStepCondition() const = 0;
            mutable Handle<FiniteDifferences::StandardStepCondition >
                                                            stepCondition_;
            int timeSteps_;
        };
    }
}

#endif
