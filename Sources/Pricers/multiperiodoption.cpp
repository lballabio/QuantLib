
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file multiperiodoption.cpp
    \brief base class for option with events happening at different periods

    $Id$
*/

// $Source$
// $Log$
// Revision 1.22  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.21  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.20  2001/07/19 14:27:27  sigmud
// warnings purged
//
// Revision 1.19  2001/07/13 14:48:13  nando
// warning pruning action ....
//
// Revision 1.18  2001/07/13 14:23:11  sigmud
// removed a few gcc compile warnings
//
// Revision 1.17  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Pricers/europeanoption.hpp"
#include "ql/Pricers/americancondition.hpp"
#include "ql/Pricers/multiperiodoption.hpp"
#include "ql/FiniteDifferences/valueatcenter.hpp"

namespace QuantLib {

    namespace Pricers {
        using FiniteDifferences::valueAtCenter;
        using FiniteDifferences::firstDerivativeAtCenter;
        using FiniteDifferences::secondDerivativeAtCenter;

        MultiPeriodOption::MultiPeriodOption(Type type, double underlying,
            double strike, Rate dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility, int gridPoints,
            const std::vector<Time>& dates,
            int timeSteps)
        : BSMNumericalOption(type, underlying, strike,
                             dividendYield, riskFreeRate,
                             residualTime, volatility,
                             gridPoints),
          dates_(dates),
          dateNumber_(dates.size()),
          timeStepPerPeriod_(timeSteps),
          lastDateIsResTime_(false),
          lastIndex_(dateNumber_ - 1),
          firstDateIsZero_(false),
          firstNonZeroDate_(residualTime),
          firstIndex_(-1) {

            double dateTollerance = 1e-6;

            if (dateNumber_ > 0){
                QL_REQUIRE(dates_[0] >= 0,
                          "First date " +
                          DoubleFormatter::toString(dates_[0]) +
                          " cannot be negative");
                if(dates_[0] < residualTime * dateTollerance ){
                    firstDateIsZero_ = true;
                    firstIndex_ = 0;
                    if(dateNumber_ > 0)
                        firstNonZeroDate_ = dates_[1];
                }

                if(QL_FABS(dates_[0] - residualTime) < dateTollerance){
                    lastDateIsResTime_ = true;
                    lastIndex_ =dateNumber_ - 2;
                 }

                QL_REQUIRE(dates_[dateNumber_-1] <= residualTime,
                    "Last date, " +
                    DoubleFormatter::toString(dates_[dateNumber_-1]) +
                    ", must be within the residual time of " +
                    DoubleFormatter::toString(residualTime) );

                if (dateNumber_ > 0){
                    if (!firstDateIsZero_)
                        firstNonZeroDate_ = dates_[0];
                    for (unsigned int j = 1; j < dateNumber_; j++)
                        QL_REQUIRE(dates_[j-1] < dates_[j],
                            "Dates must be in increasing order:" +
                            DoubleFormatter::toString(dates_[j-1]) +
                            " is not strictly smaller than " +
                            DoubleFormatter::toString(dates_[j]) );
                }
            }

        }

        void MultiPeriodOption::calculate() const {

            Time beginDate, endDate;
            initializeControlVariate();
            setGridLimits(underlying_, residualTime_);
            initializeGrid();
            initializeInitialCondition();
            initializeOperator();
            initializeModel();
            initializeStepCondition();
            prices_ = initialPrices_;
            controlPrices_ = initialPrices_;

            if(lastDateIsResTime_)
                executeIntermediateStep(dateNumber_ - 1);

            double dt = residualTime_/(timeStepPerPeriod_*(dateNumber_+1));

            // Ensure that dt is always smaller than the first non-zero date
            if (firstNonZeroDate_ <= dt)
                dt = firstNonZeroDate_/2.0;

            int j = lastIndex_;
            do{
                if (j == int(dateNumber_) - 1)
                    beginDate = residualTime_;
                else
                    beginDate = dates_[j+1];

                if (j >= 0)
                    endDate = dates_[j];
                else
                    endDate = dt;

                model_ -> rollback(prices_, beginDate, endDate,
                                   timeStepPerPeriod_, stepCondition_);

                model_ -> rollback(controlPrices_, beginDate, endDate,
                                   timeStepPerPeriod_);

                if (j >= 0)
                    executeIntermediateStep(j);

            } while (--j >= firstIndex_);

            double pricePlus = + valueAtCenter(prices_)
                               - valueAtCenter(controlPrices_)
                               + analytic_ -> theta() * dt;

            model_ -> rollback(prices_,        dt, 0, 1, stepCondition_);
            model_ -> rollback(controlPrices_, dt, 0, 1);

            if(firstDateIsZero_)
                executeIntermediateStep(0);

            // Option price and greeks are computed
            controlVariateCorrection_ =
                analytic_ -> value() - valueAtCenter(controlPrices_);

            value_ =   valueAtCenter(prices_)
                     - valueAtCenter(controlPrices_)
                     + analytic_ -> value();

            delta_ =   firstDerivativeAtCenter(prices_, grid_)
                     - firstDerivativeAtCenter(controlPrices_, grid_)
                     + analytic_ -> delta();

            gamma_ =   secondDerivativeAtCenter(prices_, grid_)
                     - secondDerivativeAtCenter(controlPrices_, grid_)
                     + analytic_ -> gamma();

            // calculating theta_
            model_ -> rollback(prices_,        0, -dt, 1, stepCondition_);
            model_ -> rollback(controlPrices_, 0, -dt, 1);

            double priceMinus = + valueAtCenter(prices_)
                                - valueAtCenter(controlPrices_)
                                - analytic_ -> theta() * dt; // + analytic_ -> value()

            theta_= (pricePlus - priceMinus)/(2.0*dt);

            hasBeenCalculated_ = true;
        }

        void MultiPeriodOption::initializeControlVariate() const{
            analytic_ = Handle<SingleAssetOption> (new EuropeanOption (
                            type_, underlying_, strike_, dividendYield_,
                            riskFreeRate_, residualTime_, volatility_));
        }

        using FiniteDifferences::StandardStepCondition;

        void MultiPeriodOption::initializeStepCondition() const{
            stepCondition_ = Handle<StandardStepCondition> (
                             new AmericanCondition(initialPrices_));
        }

        using FiniteDifferences::StandardFiniteDifferenceModel;

        void MultiPeriodOption::initializeModel() const{
            model_ = Handle<StandardFiniteDifferenceModel> (
                     new StandardFiniteDifferenceModel
                            (finiteDifferenceOperator_));
        }

        double MultiPeriodOption::controlVariateCorrection() const{
            if(!hasBeenCalculated_)
                calculate();
            return controlVariateCorrection_;
        }

    }

}
