/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/Vanilla/fdmultiperiodengine.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {
    DividendSchedule FDMultiPeriodEngine::emptySchedule;
    FDMultiPeriodEngine::
    FDMultiPeriodEngine(Size gridPoints, Size timeSteps,
                        bool timeDependent)
    : FDVanillaEngine(gridPoints, timeSteps, timeDependent),
      timeStepPerPeriod_(timeSteps) {}

    void FDMultiPeriodEngine::calculate(OneAssetOption::results* results) const {
        Time beginDate, endDate;
        Size dateNumber = schedule_->dividendDates.size();
        bool lastDateIsResTime = false;
        Integer firstIndex = -1;
        Integer lastIndex = dateNumber - 1;
        bool firstDateIsZero = false;
        Time firstNonZeroDate = getResidualTime();

        Real dateTolerance = 1e-6;

        if (dateNumber > 0){
            QL_REQUIRE(getDividendTime(0) >= 0,
                       "first date (" << getDividendTime(0)
                       << ") cannot be negative");
            if(getDividendTime(0) < getResidualTime() * dateTolerance ){
                firstDateIsZero = true;
                firstIndex = 0;
                if(dateNumber >= 2)
                    firstNonZeroDate = getDividendTime(1);
            }

            if (std::fabs(getDividendTime(lastIndex) - getResidualTime())
                < dateTolerance) {
                lastDateIsResTime = true;
                lastIndex = Integer(dateNumber) - 2;
            }

            if (!firstDateIsZero)
                firstNonZeroDate = getDividendTime(0);

            if (dateNumber >= 2) {
                for (Size j = 1; j < dateNumber; j++)
                    QL_REQUIRE(getDividendTime(j-1) < getDividendTime(j),
                               "dates must be in increasing order: "
                               << getDividendTime(j-1)
                               << " is not strictly smaller than "
                               << getDividendTime(j));
            }
        }

        if(lastDateIsResTime)
            executeIntermediateStep(dateNumber - 1);

        Time dt = getResidualTime()/(timeStepPerPeriod_*(dateNumber+1));

        // Ensure that dt is always smaller than the first non-zero date
        if (firstNonZeroDate <= dt)
            dt = firstNonZeroDate/2.0;

        setGridLimits();
        initializeGrid();
        initializeInitialCondition();
        initializeOperator();
        initializeModel();
        initializeStepCondition();

        prices_ = intrinsicValues_;
        Integer j = lastIndex;
        do{
            if (j == Integer(dateNumber) - 1)
                beginDate = getResidualTime();
            else
                beginDate = getDividendTime(j+1);

            if (j >= 0)
                endDate = getDividendTime(j);
            else
                endDate = dt;

            model_->rollback(prices_.values(), 
                             beginDate, endDate,
                             timeStepPerPeriod_, *stepCondition_);
            if (j >= 0)
                executeIntermediateStep(j);
        } while (--j >= firstIndex);

        model_->rollback(prices_.values(), dt, 0, 1, *stepCondition_);

        if(firstDateIsZero)
            executeIntermediateStep(0);

        results->value = valueAtCenter(prices_.values());
        results->delta = firstDerivativeAtCenter(prices_.values(), 
                                                 prices_.grid());
        results->gamma = secondDerivativeAtCenter(prices_.values(), 
                                                  prices_.grid());
        results->priceCurve = prices_;
    }

    void FDMultiPeriodEngine::initializeStepCondition() const{
        stepCondition_ = boost::shared_ptr<StandardStepCondition>(
                                                  new NullCondition<Array>());
    }

    void FDMultiPeriodEngine::initializeModel() const{
        model_ = boost::shared_ptr<StandardFiniteDifferenceModel>(
           new StandardFiniteDifferenceModel(finiteDifferenceOperator_,BCs_));
    }

}
