     
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
 * QuantLib license is also available at 
 *      http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file multiperiodoption.h
    \brief base class for option with events happening at different periods
    
    $Source$
    $Log$
    Revision 1.3  2001/03/21 11:33:02  marmar
    Main loop transfered from method value to method calculate.
    Methods vega and rho moved from BSMNumericalOption to BSMOption

    Revision 1.2  2001/03/21 09:56:47  marmar
    virtual method added

    Revision 1.1  2001/03/20 15:13:09  marmar
    MultiPeriodOption is a generalization of DividendAmericanOption

*/

#ifndef shaft_multi_period_option_pricer_h
#define shaft_multi_period_option_pricer_h

#include "qldefines.h"
#include "bsmnumericaloption.h"
#include "handle.h"
#include "standardstepcondition.h"
#include "standardfdmodel.h"
#include <vector>

namespace QuantLib {

    namespace Pricers {

        class MultiPeriodOption : public BSMNumericalOption {
          public:
            // constructor
            MultiPeriodOption(Type type, double underlying, 
                double strike, Rate dividendYield, Rate riskFreeRate, 
                Time residualTime, double volatility, 
                const std::vector<Time>& dates, 
                int timeSteps, int gridPoints);
            protected:
            int timeStepPerPeriod_, dateNumber_;
            std::vector<Time> dates_;
            Handle<BSMOption> analitic_;
            mutable Array prices_, controlPrices_;
            mutable Handle<FiniteDifferences::StandardStepCondition> 
                                                            stepCondition_;
            mutable Handle<FiniteDifferences::StandardFiniteDifferenceModel> 
                                                            model_;
            void calculate() const;
            virtual void initializeModel() const;
            virtual void initializeStepCondition() const;
            virtual void executeIntermediateStep(int step) const = 0;
        };

    }
}


#endif
