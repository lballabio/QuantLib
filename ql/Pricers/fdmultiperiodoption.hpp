
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmultiperiodoption.hpp
    \brief base class for option with events happening at different periods
*/

#ifndef quantlib_multi_period_option_pricer_h
#define quantlib_multi_period_option_pricer_h

#include <ql/Pricers/fdbsmoption.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    class FdMultiPeriodOption : public FdBsmOption {
      public:
        double controlVariateCorrection() const;
      protected:
        // constructor
        FdMultiPeriodOption(Option::Type type, double underlying, 
                            double strike, Spread dividendYield, 
                            Rate riskFreeRate, Time residualTime, 
                            double volatility, Size gridPoints,
                            const std::vector<Time>& dates,
                            Size timeSteps);
        // Protected attributes
        std::vector<Time> dates_;
        Size dateNumber_;
        Size timeStepPerPeriod_;
        bool lastDateIsResTime_;
        int lastIndex_;
        bool firstDateIsZero_;
        double firstNonZeroDate_;
        int firstIndex_;
        mutable boost::shared_ptr<BlackFormula> analytic_;
        mutable Array prices_, controlPrices_;
        mutable boost::shared_ptr<StandardStepCondition> stepCondition_;
        mutable boost::shared_ptr<StandardFiniteDifferenceModel> model_;
        // Methods
        void calculate() const;
        virtual void initializeControlVariate() const;
        virtual void initializeModel() const;
        virtual void initializeStepCondition() const;
        virtual void executeIntermediateStep(Size step) const = 0;
      private:
        mutable double controlVariateCorrection_;
    };

}


#endif
