
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

    $Id$
*/

// $Source$
// $Log$
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
