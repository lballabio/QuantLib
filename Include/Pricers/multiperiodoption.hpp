     
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

/*! \file multiperiodoption.hpp
    \brief base class for option with events happening at different periods
    
    $Source$
    $Log$
    Revision 1.3  2001/04/06 07:35:43  marmar
    Code simplified and cleand

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

    Revision 1.2  2001/03/21 09:56:47  marmar
    virtual method added

    Revision 1.1  2001/03/20 15:13:09  marmar
    MultiPeriodOption is a generalization of DividendAmericanOption

*/

#ifndef shaft_multi_period_option_pricer_h
#define shaft_multi_period_option_pricer_h

#include "qldefines.hpp"
#include "bsmnumericaloption.hpp"
#include "handle.hpp"
#include "FiniteDifferences/standardstepcondition.hpp"
#include "FiniteDifferences/standardfdmodel.hpp"
#include <vector>

namespace QuantLib {

    namespace Pricers {

        class MultiPeriodOption : public BSMNumericalOption {
          protected:
            // constructor
            MultiPeriodOption(Type type, double underlying, 
                double strike, Rate dividendYield, Rate riskFreeRate, 
                Time residualTime, double volatility, 
                const std::vector<Time>& dates, 
                int timeSteps, int gridPoints);   
            // Protected attributes         
            int timeStepPerPeriod_, dateNumber_;
            std::vector<Time> dates_;
            Handle<BSMOption> analitic_;
            mutable Array prices_, controlPrices_;
            mutable Handle<FiniteDifferences::StandardStepCondition> 
                                                            stepCondition_;
            mutable Handle<FiniteDifferences::StandardFiniteDifferenceModel> 
                                                            model_;
            // Methods
            void calculate() const;
            virtual void initializeModel() const;
            virtual void initializeStepCondition() const;
            virtual void executeIntermediateStep(int step) const = 0;
        };

    }

}
#endif
