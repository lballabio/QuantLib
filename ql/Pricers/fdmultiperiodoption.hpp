
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file multiperiodoption.hpp
    \brief base class for option with events happening at different periods

    \fullpath
    ql/Pricers/%multiperiodoption.hpp
*/

// $Id$

#ifndef quantlib_multi_period_option_pricer_h
#define quantlib_multi_period_option_pricer_h

#include <ql/Pricers/fdbsmoption.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <vector>

namespace QuantLib {

    namespace Pricers {

        class FdMultiPeriodOption : public FdBsmOption {
          public:
            double controlVariateCorrection() const;
          protected:
            // constructor
            FdMultiPeriodOption(Option::Type type, double underlying,
                double strike, Spread dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility, int gridPoints,
                const std::vector<Time>& dates,
                int timeSteps);
            // Protected attributes
            std::vector<Time> dates_;
            Size dateNumber_;
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
