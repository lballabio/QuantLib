
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

/*! \file multiperiodoption.hpp

    \fullpath
    Include/ql/Pricers/%multiperiodoption.hpp
    \brief base class for option with events happening at different periods

*/

// $Id$
// $Log$
// Revision 1.2  2001/09/11 13:22:38  sadrejeb
// Monte Carlo modifications, cleaning up the merge of MonteCarloModel, and
// changed styleguide of typenames in pathpricer.
//
// Revision 1.1  2001/09/03 14:04:01  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.19  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.18  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.17  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.16  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.15  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.14  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.13  2001/07/19 14:27:27  sigmud
// warnings purged
//
// Revision 1.12  2001/07/13 14:48:13  nando
// warning pruning action ....
//
// Revision 1.11  2001/07/13 14:29:08  sigmud
// removed a few gcc compile warnings
//
// Revision 1.10  2001/07/06 18:24:17  nando
// slight modifications to avoid VisualStudio warnings
//
// Revision 1.9  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.8  2001/06/05 12:45:27  nando
// R019-branch-merge4 merged into trunk
//

#ifndef quantlib_multi_period_option_pricer_h
#define quantlib_multi_period_option_pricer_h

#include "ql/handle.hpp"
#include "ql/Pricers/bsmnumericaloption.hpp"
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
                Time residualTime, double volatility, int gridPoints,
                const std::vector<Time>& dates,
                int timeSteps);
            // Protected attributes
            std::vector<Time> dates_;
            unsigned int dateNumber_;
            int timeStepPerPeriod_;
            bool lastDateIsResTime_;
            int lastIndex_;
            bool firstDateIsZero_;
            double firstNonZeroDate_;
            int firstIndex_;
            mutable Handle<SingleAssetOption> analytic_;
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
