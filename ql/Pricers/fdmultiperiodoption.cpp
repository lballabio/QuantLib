
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

#include <ql/Pricers/fdmultiperiodoption.hpp>
#include <ql/FiniteDifferences/americancondition.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {

    FdMultiPeriodOption::FdMultiPeriodOption(
                       Option::Type type, double underlying, double strike, 
                       Spread dividendYield, Rate riskFreeRate, 
                       Time residualTime, double volatility,
                       Size gridPoints, const std::vector<Time>& dates,
                       Size timeSteps)
    : FdBsmOption(type, underlying, strike,
                  dividendYield, riskFreeRate,
                  residualTime, volatility,
                  gridPoints),
      dates_(dates),
      dateNumber_(dates.size()),
      timeStepPerPeriod_(timeSteps),
      lastDateIsResTime_(false),
      lastIndex_(int(dateNumber_) - 1),
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
                if(dateNumber_ >= 2)
                    firstNonZeroDate_ = dates_[1];
            }

            if(QL_FABS(dates_[lastIndex_] - residualTime) < dateTollerance){
                lastDateIsResTime_ = true;
                lastIndex_ = int(dateNumber_) - 2;
            }

            QL_REQUIRE(dates_[dateNumber_-1] <= residualTime,
                       "Last date, " +
                       DoubleFormatter::toString(dates_[dateNumber_-1]) +
                       ", must be within the residual time of " +
                       DoubleFormatter::toString(residualTime) );

            if (dateNumber_ >= 2){
                if (!firstDateIsZero_)
                    firstNonZeroDate_ = dates_[0];
                for (Size j = 1; j < dateNumber_; j++)
                    QL_REQUIRE(dates_[j-1] < dates_[j],
                               "Dates must be in increasing order:" +
                               DoubleFormatter::toString(dates_[j-1]) +
                               " is not strictly smaller than " +
                               DoubleFormatter::toString(dates_[j]) );
            }
        }

    }

    void FdMultiPeriodOption::calculate() const {

        Time beginDate, endDate;
        initializeControlVariate();
        setGridLimits(underlying_, residualTime_);
        initializeGrid();
        initializeInitialCondition();
        initializeOperator();
        initializeModel();
        initializeStepCondition();
        prices_ = intrinsicValues_;
        controlPrices_ = intrinsicValues_;

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

        model_ -> rollback(prices_,        dt, 0, 1, stepCondition_);
        model_ -> rollback(controlPrices_, dt, 0, 1);

        if(firstDateIsZero_)
            executeIntermediateStep(0);

        // Option price and greeks are computed
        controlVariateCorrection_ =
            analytic_->value() - valueAtCenter(controlPrices_);

        value_ = valueAtCenter(prices_) + controlVariateCorrection_;

        delta_ =   firstDerivativeAtCenter(prices_, grid_)
            - firstDerivativeAtCenter(controlPrices_, grid_)
            + analytic_->delta(underlying_);

        gamma_ =   secondDerivativeAtCenter(prices_, grid_)
            - secondDerivativeAtCenter(controlPrices_, grid_)
            + analytic_->gamma(underlying_);

        hasBeenCalculated_ = true;
    }

    void FdMultiPeriodOption::initializeControlVariate() const {
        double discount = QL_EXP(-riskFreeRate_*residualTime_);
        double qDiscount = QL_EXP(-dividendYield_*residualTime_);
        double forward = underlying_*qDiscount/discount;
        double variance = volatility_*volatility_*residualTime_;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                             new PlainVanillaPayoff(payoff_));
        analytic_ = boost::shared_ptr<BlackFormula>(
                       new BlackFormula(forward, discount, variance, payoff));
    }

    void FdMultiPeriodOption::initializeStepCondition() const{
        stepCondition_ = boost::shared_ptr<StandardStepCondition> (
                                     new AmericanCondition(intrinsicValues_));
    }

    void FdMultiPeriodOption::initializeModel() const{
        model_ = boost::shared_ptr<StandardFiniteDifferenceModel>(
            new StandardFiniteDifferenceModel(finiteDifferenceOperator_,BCs_));
    }

    double FdMultiPeriodOption::controlVariateCorrection() const{
        if(!hasBeenCalculated_)
            calculate();
        return controlVariateCorrection_;
    }

}
