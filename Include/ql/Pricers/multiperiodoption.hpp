
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

/*! \file multiperiodoption.hpp
    \brief base class for option with events happening at different periods

    $Source$
    $Log$
    Revision 1.4  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.3  2001/05/22 13:22:33  marmar
    Method controlVariateCorrection added

    Revision 1.2  2001/04/12 09:07:14  marmar
    Last and first date are handled more precisely

    Revision 1.1  2001/04/09 14:05:49  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.5  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.4  2001/04/06 16:11:54  marmar
    Bug fixed in multi-period option

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

#include "ql/Pricers/bsmnumericaloption.hpp"
#include "ql/handle.hpp"
#include "ql/FiniteDifferences/standardstepcondition.hpp"
#include "ql/FiniteDifferences/standardfdmodel.hpp"
#include <vector>

namespace QuantLib {

    namespace Pricers {

        class MultiPeriodOption : public BSMNumericalOption {
          public:
            double controlVariateCorrection() const;
          protected:
            // constructor
            MultiPeriodOption(Type type, double underlying,
                double strike, Rate dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<Time>& dates,
                int timeSteps, int gridPoints);
            // Protected attributes
            bool firstDateIsZero_, lastDateIsResTime_;
            int timeStepPerPeriod_, dateNumber_;
            int lastIndex_, firstIndex_;
            double firstNonZeroDate_;

            std::vector<Time> dates_;
            mutable Handle<BSMOption> analytic_;
            mutable Array prices_, controlPrices_;
            mutable Handle<FiniteDifferences::StandardStepCondition>
                                                            stepCondition_;
            mutable Handle<FiniteDifferences::StandardFiniteDifferenceModel>
                                                            model_;
            // Methods
            void calculate() const;
            virtual void initializeControlVariate() const;
            virtual void initializeModel() const;
            virtual void initializeStepCondition() const;
            virtual void executeIntermediateStep(int step) const = 0;
          private:
            mutable double controlVariateCorrection_;
        };

    }

}
#endif
