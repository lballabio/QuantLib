
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
    \brief Option requiring additional code to be executed at each time step

    $Id$
*/

// $Source$
// $Log$
// Revision 1.7  2001/06/05 09:35:13  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.6  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_pricers_stepconditionoption_h
#define quantlib_pricers_stepconditionoption_h

#include "ql/Pricers/bsmnumericaloption.hpp"
#include "ql/FiniteDifferences/standardstepcondition.hpp"

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
